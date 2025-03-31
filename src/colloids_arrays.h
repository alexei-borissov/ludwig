/*****************************************************************************
 *
 *  colloids_arrays.h
 *
 *  Functions related to array of colloids functionality.
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2010-2024 The University of Edinburgh
 *
 *  Contributing authors:
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#ifndef LUDWIG_COLLOIDS_ARRAYS_H
#define LUDWIG_COLLOIDS_ARRAYS_H

__host__ void colloids_array_create(colloids_info_t *cinfo);
__host__ void colloids_array_free(colloids_info_t *cinfo);
__host__ void colloids_array_resize(colloids_info_t *cinfo);

#endif