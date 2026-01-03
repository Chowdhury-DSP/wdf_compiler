// Loosely adapted from Casey Muratori's "pmctrace"

#define PMC_COUNT 4
#define PMC_RING_CAPACITY 1024 // 1048576 // 1024 * 1024

struct PMC_Source_Mapping
{
    uint32_t source_index[PMC_COUNT];
};

struct PMC_Trace_Result
{
    uint64_t counters[PMC_COUNT];
    uint64_t tsc_elapsed;
    uint32_t negate;
};

struct PMC_Ring_Buffer
{
    PMC_Trace_Result buffer[PMC_RING_CAPACITY];
    uint32_t index {};
};

struct PMC_Traced_Region
{
    PMC_Ring_Buffer buffer {};
    uint32_t take_next_sys_exit_as_start;
    uint32_t completed;
};

struct PMC_Tracer;

static PMC_Source_Mapping map_pmc_names(wchar_t const **strings);

static void start_tracing(PMC_Tracer *tracer, PMC_Source_Mapping *mapping);
static void stop_tracing(PMC_Tracer *tracer);

static void start_counting(PMC_Tracer *tracer, PMC_Traced_Region *result_dest);
static void stop_counting(PMC_Tracer *tracer, PMC_Traced_Region *result_dest);

static PMC_Trace_Result get_or_wait_for_result(PMC_Tracer *tracer, PMC_Traced_Region *region);
