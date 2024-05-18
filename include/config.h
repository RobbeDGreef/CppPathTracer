#pragma once

#if __has_include("../custom_config.h")
#include "../custom_config.h"
#endif

#ifndef WORK_SQUARE_SIZE
#define WORK_SQUARE_SIZE 50
#endif

#ifndef MAX_SAMPLE_OUTPUT_COLOR
#define MAX_SAMPLE_OUTPUT_COLOR 20
#endif


#define THREAD_IMPL_NAIVE       1
#define THREAD_IMPL_OPENMP      2
#define THREAD_IMPL_OPENMP_FULL 3

#ifndef THREAD_IMPLEMENTATION
#define THREAD_IMPLEMENTATION THREAD_IMPL_NAIVE
#endif

#ifndef USE_COLOR_BUFFER_PER_THREAD
#define USE_COLOR_BUFFER_PER_THREAD 0
#endif

#ifndef ENABLE_PROGRESS_INDICATOR
#define ENABLE_PROGRESS_INDICATOR 1
#endif

#ifndef USE_SLIM_BVH_NODES
#define USE_SLIM_BVH_NODES 1
#endif