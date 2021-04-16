/*****************************************************************************
 *
 *  util_sum.h
 *
 *  Compensated sums. File to be compiled at -O0.
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2021 The University of Edinburgh
 *
 *  Contributing authors:
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#ifndef LUDWIG_UTIL_SUM_H
#define LUDWIG_UTIL_SUM_H

#include "pe.h"

typedef struct kahan_s kahan_t;  /* Compensated sum */
typedef struct klein_s klein_t;  /* Doubly compensated sum */

struct kahan_s {
  double sum;
  double cs;
};

struct klein_s {
  double sum;
  double cs;
  double ccs;
};

__host__ __device__ void    kahan_add(kahan_t * kahan, double val);

__host__ __device__ void    klein_add(klein_t * klein, double val);
__host__ __device__ void    klein_atomic_add(klein_t * sum, double val);
__host__ __device__ double  klein_sum(const klein_t * klein);
__host__ __device__ klein_t klein_zero(void);

#endif
