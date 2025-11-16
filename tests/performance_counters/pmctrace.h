/* ========================================================================

   (C) Copyright 2024 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

//
// NOTE(casey): Interface
//

// NOTE(casey): This MAX is conservative. In practice, the CPU usually allows fewer PMC counters.
#define MAX_TRACE_PMC_COUNT 8

using u64 = uint64_t;
using u32 = uint32_t;
using b32 = uint32_t;
using u8 = uint8_t;

struct pmc_name_array
{
    wchar_t const *Strings[MAX_TRACE_PMC_COUNT];
};

struct pmc_source_mapping
{
    u32 SourceIndex[MAX_TRACE_PMC_COUNT];
    u32 PMCCount;
    b32 Valid;
};

struct pmc_trace_result
{
    u64 Counters[MAX_TRACE_PMC_COUNT];

    u64 TSCElapsed;
    u64 ContextSwitchCount;
    u32 PMCCount;
    b32 Completed;
};

struct pmc_traced_region;
struct pmc_region_internals
{
    pmc_traced_region *Next;
    b32 TakeNextSysExitAsStart;
    u32 TracingThreadID;
};

struct pmc_traced_region
{
    pmc_trace_result Results;
    pmc_region_internals Internals;
};

struct pmc_tracer;

// NOTE(casey): Although MapPMCNames can take an array of up to MAX_TRACE_PMC_COUNT entries, the underlying CPU
// imposes its own limits, so the mapping may fail if you try to use more names than the CPU supports. It's best
// to use 4 or less names for compatibility, although some CPUs will allow more.
static b32 IsValid(pmc_source_mapping *Mapping);
static pmc_source_mapping MapPMCNames(pmc_name_array *SourceNames);

static b32 NoErrors(pmc_tracer *Tracer);
static char const *GetErrorMessage(pmc_tracer *Tracer);

// NOTE(casey): By default, no debug log is kept, so GetDebugLog will return 0. To enable logging, you must
// build with PMC_DEBUG_LOG defined to 1.
static char const *GetDebugLog(pmc_tracer *Tracer);

static void StartTracing(pmc_tracer *Tracer, pmc_source_mapping *Mapping);
static void StopTracing(pmc_tracer *Tracer);

static void StartCountingPMCs(pmc_tracer *Tracer, pmc_traced_region *ResultDest);
static void StopCountingPMCs(pmc_tracer *Tracer, pmc_traced_region *ResultDest);

// NOTE(casey): Region results can be read as soon as IsComplete returns true. GetOrWaitForResult will read results
// instantly if they are complete, so if you already know the results are complete via IsComplete, you can call
// GetOrWaitForResult to retrieve the results without waiting - it only waits when the results are incomplete.
static b32 IsComplete(pmc_traced_region *Region);
static pmc_trace_result GetOrWaitForResult(pmc_tracer *Tracer, pmc_traced_region *Region);
