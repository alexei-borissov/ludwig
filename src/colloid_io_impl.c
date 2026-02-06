/*****************************************************************************
 *
 *  colloid_io_impl.c
 *
 *  A factory method to instantiate a concrete implementation.
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinbburgh Parallel Computing Centre
 *
 *  (c) 2025 The University of Edinburgh
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#include <assert.h>

#include "colloid_io_impl.h"
#include "colloid_io_impl_ansi.h"
#include "colloid_io_impl_mpio.h"

/*****************************************************************************
 *
 *  colloid_io_impl_create
 *
 *  The implementation must have a well-defined mode.
 *
 *****************************************************************************/

int colloid_io_impl_create(colloid_io_mode_enum_t  mode,
                           const colloids_info_t * info,
                           colloid_io_impl_t **    io) {
  int ifail = 0;

  assert(info);
  assert(io);

  switch (mode) {
  case COLLOID_IO_MODE_ANSI:
    {
      colloid_io_ansi_t * ansi = NULL;

      ifail = colloid_io_ansi_create(info, &ansi);
      *io   = (colloid_io_impl_t *) ansi;
    }
    break;
  case COLLOID_IO_MODE_MPIIO:
    {
      colloid_io_mpio_t * mpio = NULL;

      ifail = colloid_io_mpio_create(info, &mpio);
      *io   = (colloid_io_impl_t *) mpio;
    }
    break;
  default:
    *io   = NULL;
    ifail = -1;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  colloid_io_impl_input
 *
 *****************************************************************************/

int colloid_io_impl_input(const colloids_info_t * info,
                          colloid_io_impl_t **    io) {
  assert(info);

  return colloid_io_impl_create(info->options.input.mode, info, io);
}

/*****************************************************************************
 *
 *  colloid_io_impl_output
 *
 *****************************************************************************/

int colloid_io_impl_output(const colloids_info_t * info,
                           colloid_io_impl_t **    io) {
  assert(info);

  return colloid_io_impl_create(info->options.output.mode, info, io);
}
