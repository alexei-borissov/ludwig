/*****************************************************************************
 *
 *  colloids_arrays.c
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

 #include "colloids.h"
 #include "colloids_arrays.h"

//__host__ void colloids_array_create(colloid_t * colloids_array, int n_colloids) {
//    // For now just allocate on host
//    colloids_array = (colloid_t *) malloc(n_colloids * sizeof(colloid_t));
//}
//
//__host__ void colloids_array_free(colloids_info_t *cinfo) {
//    free(cinfo->colloids);
//    free(cinfo->colloids_halo);
//}
//
//__host__ void colloids_array_resize(colloids_info_t *cinfo) {
//    colloid_t * temp_colloid;
//
//}