/*****************************************************************************
 *
 *  colloid_io_impl_mpio.h
 *
 *  (c) 2025 The University of Edinburgh
 *
 *  Edinburgh Soft MAtter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *****************************************************************************/

#ifndef LUDWIG_COLLOID_IO_IMPL_MPIO_H
#define LUDWIG_COLLOID_IO_IMPL_MPIO_H

#include "colloid_io_impl.h"
#include "io_subfile.h"

typedef struct colloid_io_mpio_s colloid_io_mpio_t;

struct colloid_io_mpio_s {
  colloid_io_impl_t super;   /* superclass block */
  colloids_info_t * info;    /* colloid info, incl. i/o options */
  io_subfile_t      subfile; /* file decomposition */
  MPI_Comm          comm;    /* Cartesian communicator handle */
};

int colloid_io_mpio_create(const colloids_info_t * info,
                           colloid_io_mpio_t **    io);
void colloid_io_mpio_free(colloid_io_mpio_t ** io);

int colloid_io_mpio_initialise(const colloids_info_t * info,
                               colloid_io_mpio_t *     io);
int colloid_io_mpio_finalise(colloid_io_mpio_t * io);

int colloid_io_mpio_read(colloid_io_mpio_t * io, const char * filename);
int colloid_io_mpio_write(colloid_io_mpio_t * io, const char * filename);

#endif
