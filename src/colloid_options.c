/*****************************************************************************
 *
 *  colloid_options.c
 *
 *  Container for options required to instantiate the colloid_info_t
 *  management object.
 *
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2025-2026 The University of Edinburgh
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#include <assert.h>
#include <string.h>

#include "colloid_options.h"
#include "util_json.h"
#include "util.h"

static const double RHO_ZERO_DEFAULT      = 1.0; /* unit density lb units */
static const double DRMAX_DEFAULT         = 0.8; /* displacement max. */
static const int    HAVE_COLLOIDS_DEFAULT = 0;   /* no colloids */

/*****************************************************************************
 *
 *  colloid_options_default
 *
 *****************************************************************************/

colloid_options_t colloid_options_default(void) {

  /* Default is no colloids, but other values are required. */

  colloid_options_t options = {
      .ncell          = {2, 2, 2},         /* min. local list dimensions */
      .nvel           = 19,                /* default bbl model */
      .bbl_build_freq = 1,                 /* rebuild every step */
      .rho0           = RHO_ZERO_DEFAULT,
      .drmax          = DRMAX_DEFAULT,
      .input          = colloid_io_options_default(),
      .output         = colloid_io_options_default(),
      .nfreq          = 0,                            /* No output */

      .have_colloids  = HAVE_COLLOIDS_DEFAULT         /* colloids or none */
  };

  return options;
}

/*****************************************************************************
 *
 *  colloid_options_have_colloids
 *
 *****************************************************************************/

colloid_options_t colloid_options_have_colloids(int have_colloids) {

  colloid_options_t options = colloid_options_default();

  options.have_colloids = have_colloids;

  return options;
}

/*****************************************************************************
 *
 *  colloid_options_ncell
 *
 *****************************************************************************/

colloid_options_t colloid_options_ncell(const int ncell[3]) {

  colloid_options_t options = colloid_options_default();

  /* Request for ncell => have_colloids */
  options.have_colloids = 1;
  options.ncell[0]      = ncell[0];
  options.ncell[1]      = ncell[1];
  options.ncell[2]      = ncell[2];

  return options;
}

/*****************************************************************************
 *
 *  colloid_options_ncell_valid
 *
 *  A minimal requirement.
 *
 *****************************************************************************/

int colloid_options_ncell_valid(const int ncell[3]) {

  int isvalid = (ncell[0] > 1 && ncell[1] > 1 && ncell[2] > 1);

  return isvalid;
}

/*****************************************************************************
 *
 *  colloid_options_valid
 *
 *****************************************************************************/

int colloid_options_valid(const colloid_options_t * options) {

  int isvalid = 1;

  assert(options);

  if (0 == colloid_options_ncell_valid(options->ncell)) isvalid = 0;
  if (options->bbl_build_freq < 1)                      isvalid = 0;
  if (options->rho0 <= 0.0)                             isvalid = 0;
  if (options->drmax <= 0.0)                            isvalid = 0;
  if (0 == colloid_io_options_valid(&options->input))   isvalid = 0;
  if (0 == colloid_io_options_valid(&options->output))  isvalid = 0;
  if (options->nfreq < 0)                               isvalid = 0;

  return isvalid;
}

/*****************************************************************************
 *
 *  colloid_options_to_json
 *
 *****************************************************************************/

int colloid_options_to_json(const colloid_options_t * opts, cJSON ** json) {

  int ifail = 0;

  if (json == NULL || opts == NULL) {
    ifail = -1;
  }
  else {
    /* Create new JSON object ... */
    cJSON * myjson = cJSON_CreateObject();
    cJSON * ncell  = cJSON_CreateIntArray(opts->ncell, 3);

    cJSON_AddBoolToObject(myjson, "have_colloids", opts->have_colloids);
    cJSON_AddItemToObject(myjson, "ncell", ncell);
    cJSON_AddNumberToObject(myjson, "nvel", opts->nvel);
    cJSON_AddNumberToObject(myjson, "bbl_build_freq", opts->bbl_build_freq);
    cJSON_AddNumberToObject(myjson, "rho0", opts->rho0);
    cJSON_AddNumberToObject(myjson, "drmax", opts->drmax);

    {
      cJSON * input  = NULL;
      cJSON * output = NULL;

      /* input */
      ifail = colloid_io_options_to_json(&opts->input, &input);
      if (ifail == 0) cJSON_AddItemToObject(myjson, "Input options", input);

      /* output */
      ifail = colloid_io_options_to_json(&opts->output, &output);
      if (ifail == 0) cJSON_AddItemToObject(myjson, "Output options", output);
    }

    cJSON_AddNumberToObject(myjson, "nfreq", opts->nfreq);

    *json = myjson;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  colloid_options_from_json
 *
 *  All components mandatory at the moment.
 *
 *****************************************************************************/

int colloid_options_from_json(const cJSON * json, colloid_options_t * opts) {

  int ifail = 0;

  if (opts == NULL) {
    ifail = -1;
  }
  else {
    /* all elements currently required to be present */
    cJSON * have   = cJSON_GetObjectItemCaseSensitive(json, "have_colloids");
    cJSON * ncell  = cJSON_GetObjectItemCaseSensitive(json, "ncell");
    cJSON * nvel   = cJSON_GetObjectItemCaseSensitive(json, "nvel");
    cJSON * bbl    = cJSON_GetObjectItemCaseSensitive(json, "bbl_build_freq");
    cJSON * rho0   = cJSON_GetObjectItemCaseSensitive(json, "rho0");
    cJSON * drmax  = cJSON_GetObjectItemCaseSensitive(json, "drmax");
    cJSON * input  = cJSON_GetObjectItemCaseSensitive(json, "Input options");
    cJSON * output = cJSON_GetObjectItemCaseSensitive(json, "Output options");
    cJSON * nfreq  = cJSON_GetObjectItemCaseSensitive(json, "nfreq");

    if (0 == cJSON_IsBool(have))                                  ifail +=  1;
    if (3 != util_json_to_int_array(ncell, opts->ncell, 3))       ifail +=  2;
    if (0 == cJSON_IsNumber(bbl))                                 ifail +=  4;
    if (0 == cJSON_IsNumber(nvel))                                ifail +=  8;
    if (0 == cJSON_IsNumber(rho0))                                ifail += 16;
    if (0 == cJSON_IsNumber(drmax))                               ifail += 32;

    if (have) opts->have_colloids = cJSON_IsTrue(have);
    if (nvel) opts->nvel = cJSON_GetNumberValue(nvel);
    if (bbl)  opts->bbl_build_freq = cJSON_GetNumberValue(bbl);
    if (rho0) opts->rho0 = cJSON_GetNumberValue(rho0);
    if (drmax) opts->drmax = cJSON_GetNumberValue(drmax);

    if (0 != colloid_io_options_from_json(input, &opts->input))   ifail +=  64;
    if (0 != colloid_io_options_from_json(output, &opts->output)) ifail += 128;
    if (0 == cJSON_IsNumber(nfreq))                               ifail += 256;

    if (nfreq) opts->nfreq = cJSON_GetNumberValue(nfreq);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  colloid_options_to_vinfo
 *
 *  Produce output suitable for human readers; standard output.
 *
 *****************************************************************************/

int colloid_options_to_vinfo(rt_t * rt, rt_enum_t level,
			     const colloid_options_t * opts) {
  int ifail = 0;

  if (rt == NULL || opts == NULL) {
    ifail = -1;
  }
  else {
    /* Options */
    rt_vinfo(rt, level, "Colloid Options\n");
    rt_vinfo(rt, level, "---------------\n");
    rt_vinfo(rt, level, "%-32.32s %d %d %d\n", "Cell list:",
	     opts->ncell[X], opts->ncell[Y], opts->ncell[Z]);
    rt_vinfo(rt, level, "%-32.32s %d\n", "LB link set:", opts->nvel);
    rt_vinfo(rt, level, "%-32.32s %d\n", "BBL rebuild freq:",
	     opts->bbl_build_freq);
    rt_vinfo(rt, level, "%-32.32s%14.7e\n", "Solid density rho0:",
	     opts->rho0);
    rt_vinfo(rt, level, "%-32.32s%14.7e\n", "Max. displacement per dt:",
	     opts->drmax);
    rt_vinfo(rt, level, "\n");
    /* input */
    rt_vinfo(rt, level, "Colloid i/o input options\n");
    rt_vinfo(rt, level, "-------------------------\n");
    colloid_io_options_to_vinfo(rt, level, &opts->input);
    /* output */
    rt_vinfo(rt, level, "Colloid i/o output options\n");
    rt_vinfo(rt, level, "--------------------------\n");
    colloid_io_options_to_vinfo(rt, level, &opts->output);
    rt_vinfo(rt, level, "%-32.32s %d\n", "Colloid output frequency:",
	     opts->nfreq);
    rt_vinfo(rt, level, "\n");
  }

  return ifail;
}
