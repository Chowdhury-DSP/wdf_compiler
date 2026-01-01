/* ========================================================================

   (C) Copyright 2024 by Molly Rocket, Inc., All Rights Reserved.

   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.

   Please see https://computerenhance.com for more information

   ======================================================================== */

#define PMC_COUNT 4

struct PMC_Source_Mapping
{
    uint32_t source_index[PMC_COUNT];
};

struct PMC_Trace_Result
{
    uint64_t Counters[PMC_COUNT];

    uint64_t TSCElapsed;
    uint64_t ContextSwitchCount;
    uint32_t Completed;
};

struct pmc_traced_region;
struct pmc_region_internals
{
    pmc_traced_region *Next;
    uint32_t TakeNextSysExitAsStart;
    uint32_t TracingThreadID;
};

struct pmc_traced_region
{
    PMC_Trace_Result Results;
    pmc_region_internals Internals;
};

struct pmc_tracer;

static PMC_Source_Mapping MapPMCNames(wchar_t const **Strings);

static void StartTracing(pmc_tracer *Tracer, PMC_Source_Mapping *Mapping);
static void StopTracing(pmc_tracer *Tracer);

static void StartCountingPMCs(pmc_tracer *Tracer, pmc_traced_region *ResultDest);
static void StopCountingPMCs(pmc_tracer *Tracer, pmc_traced_region *ResultDest);

// NOTE(casey): Region results can be read as soon as IsComplete returns true. GetOrWaitForResult will read results
// instantly if they are complete, so if you already know the results are complete via IsComplete, you can call
// GetOrWaitForResult to retrieve the results without waiting - it only waits when the results are incomplete.
static uint32_t IsComplete(pmc_traced_region *Region);
static PMC_Trace_Result GetOrWaitForResult(pmc_tracer *Tracer, pmc_traced_region *Region);
