// Loosely adapted from Casey Muratori's "pmctrace"
// and Mārtiņš Možeiko's miniperf (https://gist.github.com/mmozeiko/bd5923bcd9d20b5b9946691932ec95fa)

#define PMC_COUNT 4

struct PMC_Source_Mapping
{
    uint32_t source_index[PMC_COUNT];
};

struct PMC_Trace_Result
{
    uint64_t counters[PMC_COUNT];
    uint64_t tsc_elapsed;
};

struct PMC_Traced_Region
{
    PMC_Trace_Result start_counters;
    PMC_Trace_Result end_counters;

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
