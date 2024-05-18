#pragma once

#define FALSE   0
#define TRUE    1

#define THREAD_IMPL_NAIVE               1
#define THREAD_IMPL_OPENMP_BLOCKS       2
#define THREAD_IMPL_OPENMP_PER_PIXEL    3

#define TRIANGLE_INTERSECTION_MOLLER_TRUMBORE   1
#define TRIANGLE_INTERSECTION_CRAMMER           2

#if __has_include("../../custom_config.h")
#include "../../custom_config.h"
#endif

#ifndef WORK_SQUARE_SIZE
#define WORK_SQUARE_SIZE 50
#endif

#ifndef MAX_SAMPLE_OUTPUT_COLOR
#define MAX_SAMPLE_OUTPUT_COLOR 20
#endif

#ifndef THREAD_IMPLEMENTATION
#define THREAD_IMPLEMENTATION THREAD_IMPL_NAIVE
#endif

#ifndef USE_COLOR_BUFFER_PER_THREAD
#define USE_COLOR_BUFFER_PER_THREAD FALSE
#endif

#ifndef ENABLE_PROGRESS_INDICATOR
#define ENABLE_PROGRESS_INDICATOR TRUE
#endif

#ifndef USE_SLIM_BVH_NODES
#define USE_SLIM_BVH_NODES FALSE
#endif

#ifndef TRIANGLE_INTERSECTION_ALGO
#define TRIANGLE_INTERSECTION_ALGO TRIANGLE_INTERSECTION_MOLLER_TRUMBORE
#endif