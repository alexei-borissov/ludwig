/*****************************************************************************
 *
 *  colloid_io_options.c
 *
 *  Container for options related to colloid i/o (input, output).
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

#include "colloid_io_options.h"
#include "util_json.h"
#include "util.h"

/*****************************************************************************
 *
 *  colloid_io_mode_valid
 *
 *****************************************************************************/

int colloid_io_mode_valid(colloid_io_mode_enum_t mode) {

  int valid = 0;

  switch (mode) {
  case COLLOID_IO_MODE_ANSI:
  case COLLOID_IO_MODE_MPIIO:
    valid = 1;
    break;
  default:
    valid = 0;
  }

  return valid;
}

/*****************************************************************************
 *
 *  colloid_io_mode_to_string
 *
 *****************************************************************************/

const char * colloid_io_mode_to_string(colloid_io_mode_enum_t mode) {

  const char * str = NULL;

  switch (mode) {
  case COLLOID_IO_MODE_ANSI:
    str = "ansi";
    break;
  case COLLOID_IO_MODE_MPIIO:
    str = "mpiio";
    break;
  default:
    str = "invalid";
  }

  return str;
}

/*****************************************************************************
 *
 *  colloid_io_mode_from_string
 *
 *****************************************************************************/

colloid_io_mode_enum_t colloid_io_mode_from_string(const char * str) {

  char                   value[BUFSIZ] = {0};
  colloid_io_mode_enum_t mode          = COLLOID_IO_MODE_INVALID;

  strncpy(value, str, BUFSIZ - 1);
  util_str_tolower(value, strlen(value));

  if (strcmp(value, "ansi")  == 0) mode = COLLOID_IO_MODE_ANSI;
  if (strcmp(value, "mpiio") == 0) mode = COLLOID_IO_MODE_MPIIO;

  return mode;
}

/*****************************************************************************
 *
 *  colloid_io_options_default
 *
 *****************************************************************************/

colloid_io_options_t colloid_io_options_default(void) {

  colloid_io_options_t opts = {
    .mode      = COLLOID_IO_MODE_ANSI,
    .iorformat = IO_RECORD_ASCII,
    .report    = 0,
    .iogrid    = {1, 1, 1}
  };

  return opts;
}

/*****************************************************************************
 *
 *  colloid_io_options_valid
 *
 *****************************************************************************/

int colloid_io_options_valid(const colloid_io_options_t * opts) {

  int valid = 1;

  if (0 == colloid_io_mode_valid(opts->mode))               valid = 0;
  if (0 == io_options_record_format_valid(opts->iorformat)) valid = 0;

  return valid;
}

/*****************************************************************************
 *
 *  colloid_io_options_to_json
 *
 *****************************************************************************/

int colloid_io_options_to_json(const colloid_io_options_t * opts,
                               cJSON ** json) {
  int ifail = 0;

  if (opts == NULL || json == NULL) {
    ifail = -1;
  }
  else {
    cJSON * myjson = cJSON_CreateObject();
    cJSON * iogrid = cJSON_CreateIntArray(opts->iogrid, 3);

    cJSON_AddStringToObject(myjson, "I/O mode",
                            colloid_io_mode_to_string(opts->mode));
    cJSON_AddStringToObject(myjson, "Format",
                            io_record_format_to_string(opts->iorformat));
    cJSON_AddBoolToObject(myjson, "Report", opts->report);
    cJSON_AddItemToObject(myjson, "I/O grid", iogrid);

    *json = myjson;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  colloid_io_options_from_json
 *
 *****************************************************************************/

int colloid_io_options_from_json(const cJSON * json,
                                 colloid_io_options_t * opts) {
  int ifail = 0;

  if (json == NULL || opts == NULL) {
    ifail = -1;
  }
  else {
    /* We expect four key/value pairs */
    cJSON * mode   = cJSON_GetObjectItemCaseSensitive(json, "I/O mode");
    cJSON * format = cJSON_GetObjectItemCaseSensitive(json, "Format");
    cJSON * report = cJSON_GetObjectItemCaseSensitive(json, "Report");
    cJSON * iogrid = cJSON_GetObjectItemCaseSensitive(json, "I/O grid");

    if (mode   == NULL) ifail += 1;
    if (format == NULL) ifail += 2;
    if (report == NULL) ifail += 4;
    if (iogrid == NULL) ifail += 8;

    if (mode) {
      char * str = cJSON_GetStringValue(mode);
      opts->mode = colloid_io_mode_from_string(str);
    }
    if (format) {
      char * str      = cJSON_GetStringValue(format);
      opts->iorformat = io_record_format_from_string(str);
    }

    if (report) {
      opts->report = cJSON_IsTrue(report);
    }

    if (iogrid) {
      if (3 != util_json_to_int_array(iogrid, opts->iogrid, 3)) ifail += 16;
    }
  }

  return ifail;
}


/*****************************************************************************
 *
 *  colloid_io_options_from_rt
 *
 *  Construct i/o options from the run time input (or default).
 *  This does not discriminate between input and output.
 *
 *****************************************************************************/

int colloid_io_options_from_rt(rt_t * rt, rt_enum_t level,
			       colloid_io_options_t * options) {
  int ifail = 0;
  colloid_io_options_t opts = colloid_io_options_default();

  /* mode */

  if (rt_key_present(rt, "colloid_io_options_mode")) {
    char mode[BUFSIZ] = {0};
    rt_string_parameter(rt, "colloid_io_options_mode", mode, BUFSIZ-1);
    util_str_tolower(mode, BUFSIZ-1);
    opts.mode = COLLOID_IO_MODE_INVALID;
    if (strcmp(mode, "ansi")  == 0) opts.mode = COLLOID_IO_MODE_ANSI;
    if (strcmp(mode, "mpiio") == 0) opts.mode = COLLOID_IO_MODE_MPIIO;
    if (opts.mode == COLLOID_IO_MODE_INVALID) {
      /* "colloid_io_options_mode" has been specified, but invalid ... */
      rt_vinfo(rt, level, "colloid_io_options_mode unrecognised: %s\n", mode);
      rt_fatal(rt, level, "Please check input and try again.\n");
      ifail = -1;
    }
  }

  /* i/o record format */

  if (rt_key_present(rt, "colloid_io_options_format")) {
    char format[BUFSIZ] = {0};
    rt_string_parameter(rt, "colloid_io_options_format", format, BUFSIZ-1);
    util_str_tolower(format, BUFSIZ-1);
    opts.iorformat = IO_RECORD_INVALID;
    if (strcmp(format, "ascii")  == 0) opts.iorformat = IO_RECORD_ASCII;
    if (strcmp(format, "binary") == 0) opts.iorformat = IO_RECORD_BINARY;
    if (opts.iorformat == IO_RECORD_INVALID) {
      /* "colloid_io_options_format" was present but not recognised ... */
      rt_vinfo(rt, level, "colloid_io_options_format invalid: %s\n", format);
      rt_fatal(rt, level, "Please check the input file and try again.\n");
      ifail = -2;
    }
  }

  /* report (really should be "true" for run time) */

  if (rt_switch(rt, "colloid_io_options_report")) {
    opts.report = rt_switch(rt, "colloid_io_options_report");
  }

  /* i/o grid (only default {1,1,1} supported at present) */
  /* Do not check so the key should be reported as unused */

  *options = opts;

  return ifail;
}

/*****************************************************************************
 *
 *  colloid_io_options_to_vinfo
 *
 *  Write a summary suitable for human readers; the output file
 *
 *****************************************************************************/

int colloid_io_options_to_vinfo(rt_t * rt, rt_enum_t level,
				const colloid_io_options_t * opts) {

  int ifail = 0;

  if (rt == NULL || opts == NULL) {
    ifail = -1;
  }
  else {
    int nfiles = 1; /* Always, at present */
    rt_vinfo(rt, level, "%-32.32s %s\n", "Mode:",
	     colloid_io_mode_to_string(opts->mode));
    rt_vinfo(rt, level, "%-32.32s %s\n", "Record format:",
	     io_record_format_to_string(opts->iorformat));
    rt_vinfo(rt, level, "%-32.32s %s\n", "Report:", opts->report ? "yes" : "no");
    rt_vinfo(rt, level, "%-32.32s %d\n", "Number of files:", nfiles);
    rt_vinfo(rt, level, "\n");
    ifail = 0;
  }

  return ifail;
}
