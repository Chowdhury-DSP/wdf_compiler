#include <cassert>
#include <windows.h>
#include <evntrace.h>
#include <evntcons.h>
#include <intrin.h>
#include <immintrin.h>
#pragma comment (lib, "advapi32")

using u64 = uint64_t;
using u32 = uint32_t;
using b32 = uint32_t;
using u8 = uint8_t;

enum Trace_Marker_Type : u32
{
    Trace_Marker_Open,
    Trace_Marker_Close,
};

struct Win32_Trace_Description
{
    EVENT_TRACE_PROPERTIES_V2 Properties;
    WCHAR Name[1024];
};

struct ETW_Marker
{
    EVENT_TRACE_HEADER header;
    PMC_Traced_Region *region;
};

struct PMC_Tracer_CPU
{
    PMC_Traced_Region *region;

    u64 last_sys_enter_counters[PMC_COUNT];
    u64 last_sys_enter_tsc;
};

#define PMC_TRACE_RESULT_MASK 0xff
struct PMC_Tracer
{
    Win32_Trace_Description Win32TraceDesc;
    TRACEHANDLE MarkerRegistrationHandle;
    TRACEHANDLE TraceHandle;
    TRACEHANDLE TraceSession;
    HANDLE ProcessingThread;

    PMC_Source_Mapping Mapping;
    PMC_Tracer_CPU CPU;
};

#define WIN32_TRACE_OPCODE_SYSTEMCALL_ENTER 51
#define WIN32_TRACE_OPCODE_SYSTEMCALL_EXIT 52

static GUID Win32DPCEventGuid = {0xce1dbfb4, 0x137e, 0x4da6, {0x87, 0xb0, 0x3f, 0x59, 0xaa, 0x10, 0x2c, 0xbc}};

static GUID TraceMarkerProviderGuid = {0xb877a9af, 0x4155, 0x40f2, {0xa9, 0xba, 0x34, 0xbe, 0xdf, 0xaf, 0xd1, 0x22}};
static GUID TraceMarkerCategoryGuid = {0x5c96d7f7, 0xb1ea, 0x4fbe, {0x86, 0x55, 0xe0, 0x43, 0x1e, 0x23, 0x2e, 0x53}};

static void *Win32AllocateSize(u64 Size)
{
    void *Result = VirtualAlloc(0, Size, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    return Result;
}

static void Win32Deallocate(void *Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

static b32 GUIDsAreEqual(GUID A, GUID B)
{
    __m128i Compare = _mm_cmpeq_epi8(_mm_loadu_si128((__m128i *)&A), _mm_loadu_si128((__m128i *)&B));
    int Mask = _mm_movemask_epi8(Compare);
    b32 Result = (Mask == 0xffff);
    return Result;
}

static void Win32FindPMCData(PMC_Tracer *Tracer, EVENT_RECORD *Event, u64 *PMCData)
{
    EVENT_EXTENDED_ITEM_PMC_COUNTERS *PMC = 0;
    u64 PMCDataSize = 0;
    u32 PMCPresent = 0;
    for(u32 EDIndex = 0; EDIndex < Event->ExtendedDataCount; ++EDIndex)
    {
        EVENT_HEADER_EXTENDED_DATA_ITEM *Item = Event->ExtendedData + EDIndex;
        if(Item->ExtType == EVENT_HEADER_EXT_TYPE_PMC_COUNTERS)
        {
            PMC = (EVENT_EXTENDED_ITEM_PMC_COUNTERS *)Item->DataPtr;
            PMCDataSize = Item->DataSize;
            ++PMCPresent;
        }
    }

    assert(PMCDataSize == (sizeof(u64)*PMC_COUNT) && "Unexpected PMC data size");
    assert(PMCPresent == 1 && "Unexpected PMC data count");

    for(u32 PMCIndex = 0; PMCIndex < PMC_COUNT; ++PMCIndex)
    {
        PMCData[PMCIndex] = PMC->Counter[PMCIndex];
    }
}

static void CALLBACK Win32ProcessETWEvent(EVENT_RECORD *Event)
{
    PMC_Tracer *Tracer = (PMC_Tracer *)Event->UserContext;
    PMC_Tracer_CPU *CPU = &Tracer->CPU;

    GUID EventGUID = Event->EventHeader.ProviderId;
	UCHAR Opcode = Event->EventHeader.EventDescriptor.Opcode;
    u64 TSC = Event->EventHeader.TimeStamp.QuadPart;
    u64 PMCData[PMC_COUNT] = {};

    if(GUIDsAreEqual(EventGUID, TraceMarkerCategoryGuid))
    {
        PMC_Traced_Region *region = *((PMC_Traced_Region **)(Event->UserData));

        if(Opcode == Trace_Marker_Open)
        {
            // OPEN
            CPU->region = region;
            region->take_next_sys_exit_as_start = true;
        }
        else if(Opcode == Trace_Marker_Close)
        {
            // CLOSE
            PMC_Trace_Result *results = &region->results;
            for(u32 PMCIndex = 0; PMCIndex < PMC_COUNT; ++PMCIndex)
                results->counters[PMCIndex] += CPU->last_sys_enter_counters[PMCIndex];
            results->tsc_elapsed += CPU->last_sys_enter_tsc;
            results->completed = true;
        }
        else
        {
            assert(false && "Unrecognized ETW marker type");
        }
    }
    else if(GUIDsAreEqual(EventGUID, Win32DPCEventGuid))
    {
        if(Opcode == WIN32_TRACE_OPCODE_SYSTEMCALL_ENTER)
        {
            // ENTER
            if(CPU->region)
            {
                CPU->last_sys_enter_tsc = TSC;
                Win32FindPMCData(Tracer, Event, CPU->last_sys_enter_counters);
            }
        }
        else if(Opcode == WIN32_TRACE_OPCODE_SYSTEMCALL_EXIT)
        {
            // EXIT
            if (PMC_Traced_Region *region = CPU->region)
            {
                if(region->take_next_sys_exit_as_start)
                {
                    region->take_next_sys_exit_as_start = false;
                    Win32FindPMCData(Tracer, Event, PMCData);

                    PMC_Trace_Result *results = &region->results;
                    for(u32 PMCIndex = 0; PMCIndex < PMC_COUNT; ++PMCIndex)
                        results->counters[PMCIndex] -= PMCData[PMCIndex];
                    results->tsc_elapsed -= TSC;
                }
            }
        }
    }
}

static DWORD CALLBACK Win32ProcessEventThread(void *Arg)
{
    TRACEHANDLE Session = (TRACEHANDLE)Arg;
    ProcessTrace(&Session, 1, 0, 0);
    return 0;
}

static ULONG WINAPI ControlCallback(WMIDPREQUESTCODE, void *, ULONG *, void *)
{
    return ERROR_SUCCESS;
}

static PMC_Source_Mapping map_pmc_names(wchar_t const **Strings)
{
    PMC_Source_Mapping mapping = {};

    ULONG BufferSize;
    TraceQueryInformation(0, TraceProfileSourceListInfo, 0, 0, &BufferSize);
    BYTE *Buffer = (BYTE *)Win32AllocateSize(BufferSize);
    if(Buffer)
    {
        if(TraceQueryInformation(0, TraceProfileSourceListInfo, Buffer, BufferSize, &BufferSize) == ERROR_SUCCESS)
        {
            u32 FoundCount = 0;

            for(PROFILE_SOURCE_INFO *Info = (PROFILE_SOURCE_INFO *)Buffer;
                ;
                Info = (PROFILE_SOURCE_INFO *)((u8 *)Info + Info->NextEntryOffset))
            {
                for(u32 SourceNameIndex = 0; SourceNameIndex < PMC_COUNT; ++SourceNameIndex)
                {
                    wchar_t const *SourceString = Strings[SourceNameIndex];
                    if(SourceString)
                    {
                        if(lstrcmpW(Info->Description, SourceString) == 0)
                        {
                            mapping.source_index[SourceNameIndex] = Info->Source;
                            ++FoundCount;
                            break;
                        }
                    }
                }

                if(Info->NextEntryOffset == 0)
                {
                    break;
                }
            }
        }
    }

    Win32Deallocate(Buffer);

    return mapping;
}

static void SetTracePMCSources(PMC_Tracer *Tracer, PMC_Source_Mapping *Mapping)
{
    ULONG Status = TraceSetInformation(Tracer->TraceHandle, TracePmcCounterListInfo,
                                       Mapping->source_index, PMC_COUNT * sizeof(Mapping->source_index[0]));
    assert(Status == ERROR_SUCCESS && "Unable to select PMCs");

    CLASSIC_EVENT_ID EventIDs[] =
    {
        {Win32DPCEventGuid, WIN32_TRACE_OPCODE_SYSTEMCALL_ENTER},
        {Win32DPCEventGuid, WIN32_TRACE_OPCODE_SYSTEMCALL_EXIT},
    };

    ULONG EventListStatus = TraceSetInformation(Tracer->TraceHandle, TracePmcEventListInfo, EventIDs, sizeof(EventIDs));
    assert(EventListStatus == ERROR_SUCCESS && "Unable to select events");
}

static void Win32RegisterTraceMarker(PMC_Tracer *Tracer)
{
    TRACE_GUID_REGISTRATION MarkerEventClassGuids[] = {(LPGUID)&TraceMarkerCategoryGuid, 0};
    ULONG Status = RegisterTraceGuids((WMIDPREQUEST)ControlCallback, 0, (LPGUID)&TraceMarkerProviderGuid,
                                      sizeof(MarkerEventClassGuids)/sizeof(TRACE_GUID_REGISTRATION),
                                      MarkerEventClassGuids,
                                      0, 0, &Tracer->MarkerRegistrationHandle);
    assert(Status == ERROR_SUCCESS && "ETW marker registration failed");
}

static void Win32CreateTrace(PMC_Tracer *Tracer, PMC_Source_Mapping *SourceMapping)
{
    const WCHAR TraceName[] = L"Win32PMCTrace";

    EVENT_TRACE_PROPERTIES_V2 *Props = &Tracer->Win32TraceDesc.Properties;
    Props->Wnode.BufferSize = sizeof(Tracer->Win32TraceDesc);
    Props->LoggerNameOffset = offsetof(Win32_Trace_Description, Name);

    // If there's a previously running trace, this should kill it...
    // but doesn't always seem to work?
    ControlTraceW(0, TraceName, (EVENT_TRACE_PROPERTIES *)Props, EVENT_TRACE_CONTROL_STOP);

    Props->Wnode.ClientContext = 3;
    Props->Wnode.Flags = WNODE_FLAG_TRACED_GUID | WNODE_FLAG_VERSIONED_PROPERTIES;
    Props->LogFileMode = EVENT_TRACE_REAL_TIME_MODE | EVENT_TRACE_SYSTEM_LOGGER_MODE;
    Props->VersionNumber = 2;
    Props->EnableFlags = EVENT_TRACE_FLAG_CSWITCH | EVENT_TRACE_FLAG_NO_SYSCONFIG | EVENT_TRACE_FLAG_SYSTEMCALL;
    ULONG StartStatus = StartTraceW(&Tracer->TraceHandle, TraceName, (EVENT_TRACE_PROPERTIES*)Props);
    assert(StartStatus == ERROR_SUCCESS && "Unable to start trace - may occur if not run as admin");

    Tracer->Mapping = *SourceMapping;
    SetTracePMCSources(Tracer, &Tracer->Mapping);

    EVENT_TRACE_LOGFILEW Log = {};
    Log.LoggerName = Tracer->Win32TraceDesc.Name;
    Log.EventRecordCallback = Win32ProcessETWEvent;
    Log.ProcessTraceMode = PROCESS_TRACE_MODE_EVENT_RECORD | PROCESS_TRACE_MODE_RAW_TIMESTAMP | PROCESS_TRACE_MODE_REAL_TIME;
    Log.Context = Tracer;

    Tracer->TraceSession = OpenTraceW(&Log);
    assert(Tracer->TraceSession != INVALID_PROCESSTRACE_HANDLE && "Unable to open trace");

    Tracer->ProcessingThread = CreateThread(0, 0, Win32ProcessEventThread, (void *)Tracer->TraceSession, 0, 0);
    assert(Tracer->ProcessingThread != 0 && "Unable to create processing thread");
}

static void start_tracing(PMC_Tracer *Tracer, PMC_Source_Mapping *SourceMapping)
{
    *Tracer = {};
    Win32RegisterTraceMarker(Tracer);
    Win32CreateTrace(Tracer, SourceMapping);
}

static void stop_tracing(PMC_Tracer *Tracer)
{
    if(Tracer->TraceHandle)
    {
        ControlTraceW(Tracer->TraceHandle, 0, (EVENT_TRACE_PROPERTIES *)&Tracer->Win32TraceDesc.Properties, EVENT_TRACE_CONTROL_STOP);
    }

    if(Tracer->TraceSession != INVALID_PROCESSTRACE_HANDLE)
    {
        CloseTrace(Tracer->TraceSession);
    }

    if(Tracer->ProcessingThread)
    {
        WaitForSingleObject(Tracer->ProcessingThread, INFINITE);
        CloseHandle(Tracer->ProcessingThread);
    }

    if(Tracer->MarkerRegistrationHandle)
    {
        UnregisterTraceGuids(Tracer->MarkerRegistrationHandle);
    }
}

static void start_counting(PMC_Tracer *tracer, PMC_Traced_Region *region)
{
    ETW_Marker trace_marker = {};
    trace_marker.header.Size = sizeof(trace_marker);
    trace_marker.header.Flags = WNODE_FLAG_TRACED_GUID;
    trace_marker.header.Guid = TraceMarkerCategoryGuid;
    trace_marker.header.Class.Type = Trace_Marker_Open;

    trace_marker.region = region;

    region->results = {};

    auto status = TraceEvent(tracer->TraceHandle, &trace_marker.header);
    assert(status == ERROR_SUCCESS && "Unable to insert ETW open marker");
}

static void stop_counting(PMC_Tracer *tracer, PMC_Traced_Region *region)
{
    ETW_Marker trace_marker = {};
    trace_marker.header.Size = sizeof(trace_marker);
    trace_marker.header.Flags = WNODE_FLAG_TRACED_GUID;
    trace_marker.header.Guid = TraceMarkerCategoryGuid;
    trace_marker.header.Class.Type = Trace_Marker_Close;

    trace_marker.region = region;

    auto status = TraceEvent(tracer->TraceHandle, &trace_marker.header);
    assert(status == ERROR_SUCCESS && "Unable to insert ETW close marker");
}

static PMC_Trace_Result get_or_wait_for_result(PMC_Tracer *tracer, PMC_Traced_Region *region)
{
    while(region->results.completed == false)
    {
        // Spin-lock... we shouldn't be here very long...
        _mm_pause();
    }

    return region->results;
}
