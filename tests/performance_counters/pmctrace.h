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
    uint64_t counters[PMC_COUNT];

    uint64_t tsc_elapsed;
    uint32_t completed;
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

static PMC_Source_Mapping map_pmc_names(wchar_t const **Strings);

static void StartTracing(pmc_tracer *Tracer, PMC_Source_Mapping *Mapping);
static void StopTracing(pmc_tracer *Tracer);

static void StartCountingPMCs(pmc_tracer *Tracer, pmc_traced_region *ResultDest);
static void StopCountingPMCs(pmc_tracer *Tracer, pmc_traced_region *ResultDest);

static PMC_Trace_Result get_or_wait_for_result(pmc_tracer *tracer, pmc_traced_region *region);
