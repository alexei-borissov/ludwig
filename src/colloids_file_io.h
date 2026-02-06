/*****************************************************************************
 *
 *  colloids_file_io.h
 *
 *  This provides a container to deal with file io (in what would
 *  otherwise be a rather circular dependency between the info
 *  and impl objects).
 *
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2025-2026 The University of Edinburgh
 *
 *****************************************************************************/

#ifndef LUDWIG_COLLOIDS_FILE_IO_H
#define LUDWIG_COLLOIDS_FILE_IO_H

#include "colloid_io_impl.h"

typedef struct colloids_file_io_s {
  colloids_info_t *   info;   /* reference to info */
  colloid_io_impl_t * input;  /* file input implementation */
  colloid_io_impl_t * output; /* file output implementation */
} colloids_file_io_t;

int colloids_file_io_initialise(const colloids_info_t * info,
                                colloids_file_io_t * io);
void colloids_file_io_finalise(colloids_file_io_t * io);

int colloids_file_io_write(const colloids_file_io_t * io,
                           const char * filename);
int colloids_file_io_read(colloids_file_io_t * io, const char * filename);

#endif
