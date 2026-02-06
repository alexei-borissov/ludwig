/*****************************************************************************
 *
 *  colloid_io_options.h
 *
 *  Edinburgh Soft Matter and Statisticsal Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2025-2026 The University of Edinburgh
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#ifndef LUDWIG_COLLOID_IO_OPTIONS_H
#define LUDWIG_COLLOID_IO_OPTIONS_H

#include "runtime.h"
#include "io_options.h"

typedef enum colloid_io_mode_enum {
  COLLOID_IO_MODE_INVALID,
  COLLOID_IO_MODE_ANSI,
  COLLOID_IO_MODE_MPIIO
} colloid_io_mode_enum_t;

int colloid_io_mode_valid(colloid_io_mode_enum_t mode);
const char * colloid_io_mode_to_string(colloid_io_mode_enum_t mode);
colloid_io_mode_enum_t colloid_io_mode_from_string(const char * str);

typedef struct colloid_io_options_s colloid_io_options_t;

struct colloid_io_options_s {
  /* There are some common features with lattice i/o options,
   * but it's not exactly the same thing... */

  colloid_io_mode_enum_t  mode;      /* ansi or  mpio */
  io_record_format_enum_t iorformat; /* ascii or binary */
  int                     report;    /* verbose report switch */
  int                     iogrid[3]; /* i/o (file) decomposition */
};

colloid_io_options_t colloid_io_options_default(void);

int colloid_io_options_valid(const colloid_io_options_t * opts);

int colloid_io_options_to_json(const colloid_io_options_t * opts,
                               cJSON ** json);
int colloid_io_options_from_json(const cJSON * json,
                                 colloid_io_options_t * opts);

int colloid_io_options_to_vinfo(rt_t * rt, rt_enum_t level,
				const colloid_io_options_t * options);
int colloid_io_options_from_rt(rt_t * rt, rt_enum_t level,
			       colloid_io_options_t * options);

#endif
