/*****************************************************************************
 *
 *  colloids_file_io.c
 *
 *  Top level driver for colloid read/write.
 *  The colloid_io_options_t report should be true at run time so that
 *  errors do not pass silently.
 *
 *
 *  Edinburgh Sott Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2025-2026 The University of Edinburgh
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#include <assert.h>

#include "colloids_file_io.h"

/*****************************************************************************
 *
 *  colloids_file_io_initialise
 *
 *****************************************************************************/

int colloids_file_io_initialise(const colloids_info_t * info,
                                colloids_file_io_t *    io) {
  assert(io);

  *io      = (colloids_file_io_t) {0};
  io->info = (colloids_info_t *) info;

  if (io->info) {
    if (0 != colloid_io_impl_input(info, &io->input))   goto err;
    if (0 != colloid_io_impl_output(info, &io->output)) goto err;
  }

  return 0;

err:
  colloids_file_io_finalise(io);
  return -1;
}

/*****************************************************************************
 *
 *  colloids_file_io_finalise
 *
 *****************************************************************************/

void colloids_file_io_finalise(colloids_file_io_t * io) {

  assert(io);

  if (io->info) {
    if (io->output) io->output->impl->free(&io->output);
    if (io->input)  io->input->impl->free(&io->input);
  }

  *io = (colloids_file_io_t) {0};

  return;
}

/*****************************************************************************
 *
 *  colloids_file_io_write
 *
 *  A call with no io->output is considered erroneous.
 *  All errors will return a non-zero value.
 *
 *****************************************************************************/

int colloids_file_io_write(const colloids_file_io_t * io,
                           const char *               filename) {
  int ifail = MPI_ERR_IO;

  assert(io);
  assert(filename);

  if (io->output) {
    double t0 = 0.0; /* timer ... */
    double t1 = 0.0;

    t0    = MPI_Wtime();
    ifail = io->output->impl->write(io->output, filename);
    t1    = MPI_Wtime();

    /* Report */

    if (io->info->options.output.report) {
      pe_t * pe = io->info->pe;

      if (ifail == MPI_SUCCESS) {
        /* Format information and size in bytes? */
        pe_info(pe, "Wrote colloids to file: %s\n", filename);
        pe_info(pe, "Number of colloids:     %d\n", io->info->ntotal);
        pe_info(pe, "Time taken (seconds):   %5.2f\n", t1 - t0);
      }
      else {
        int  len                       = 0;
        char msg[MPI_MAX_ERROR_STRING] = {0};
        MPI_Error_string(ifail, msg, &len);
        pe_info(pe, "Error writing file: %s\n", filename);
        pe_info(pe, "%s\n", msg);
      }
    }
  }

  return ifail;
}

/*****************************************************************************
 *
 *  colloids_file_io_read
 *
 *  A call with NULL io->input is considered erroneous.
 *  All errors return a non-zero value.
 *
 *****************************************************************************/

int colloids_file_io_read(colloids_file_io_t * io, const char * filename) {

  int ifail = MPI_ERR_IO; /* io->input must exist ... */

  assert(io);
  assert(filename);

  if (io->input) {
    double t0 = 0.0; /* difference timer ... */
    double t1 = 0.0;

    t0    = MPI_Wtime();
    ifail = io->input->impl->read(io->input, filename);
    t1    = MPI_Wtime();

    /* Handle errors */

    if (io->info->options.input.report) {
      pe_t * pe = io->info->pe;

      if (ifail == MPI_SUCCESS) {
        pe_info(pe, "Read colloids from file: %s\n", filename);
        pe_info(pe, "Number of colloids:      %d\n", io->info->ntotal);
        pe_info(pe, "Time taken (seconds):    %5.2f\n", t1 - t0);
      }
      else {
        int  len                       = 0;
        char msg[MPI_MAX_ERROR_STRING] = {0};
        MPI_Error_string(ifail, msg, &len);
        pe_info(pe, "Error reading file: %s\n", filename);
        pe_info(pe, "%s\n", msg);
      }
    }
  }

  return ifail;
}
