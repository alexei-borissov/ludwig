/*****************************************************************************
 *
 *  colloid_io_impl_ansi.h
 *
 *  Edinburgh Soft MAtter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2025 The University of Edinburgh
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#ifndef LUDWIG_COLLOID_IO_IMPL_ANSI_H
#define LUDWIG_COLLOID_IO_IMPL_ANSI_H

#include "colloid_io_impl.h"
#include "io_subfile.h"

typedef struct colloid_io_ansi_s colloid_io_ansi_t;

struct colloid_io_ansi_s {
  colloid_io_impl_t super;   /* superclass block */
  colloids_info_t * info;    /* colloid info, incl. i/o options */
  io_subfile_t      subfile; /* file decomposition */
  MPI_Comm          comm;    /* Cartesian communicator handle */
};

int colloid_io_ansi_create(const colloids_info_t * info,
                           colloid_io_ansi_t ** io);
void colloid_io_ansi_free(colloid_io_ansi_t ** io);

int colloid_io_ansi_initialise(const colloids_info_t * info,
                               colloid_io_ansi_t * io);
int colloid_io_ansi_finalise(colloid_io_ansi_t * io);

int colloid_io_ansi_read(colloid_io_ansi_t * io, const char * filename);
int colloid_io_ansi_write(colloid_io_ansi_t * io, const char * filename);

#endif
