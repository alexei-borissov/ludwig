/*****************************************************************************
 *
 *  test_colloid_io_options.c
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

#include "pe.h"
#include "colloid_io_options.h"

int test_colloid_io_mode_valid(void);
int test_colloid_io_mode_to_string(void);
int test_colloid_io_mode_from_string(void);

int test_colloid_io_options_default(void);
int test_colloid_io_options_valid(void);
int test_colloid_io_options_to_json(void);
int test_colloid_io_options_from_json(void);

int test_colloid_io_options_to_vinfo(pe_t * pe);
int test_colloid_io_options_from_rt(pe_t * pe);

/*****************************************************************************
 *
 *  test_colloid_io_options_suite
 *
 *****************************************************************************/

int test_colloid_io_options_suite(void) {

  pe_t * pe = NULL;

  pe_create(MPI_COMM_WORLD, PE_QUIET, &pe);

  test_colloid_io_mode_valid();
  test_colloid_io_mode_to_string();
  test_colloid_io_mode_from_string();

  test_colloid_io_options_default();
  test_colloid_io_options_valid();
  test_colloid_io_options_to_json();
  test_colloid_io_options_from_json();

  test_colloid_io_options_to_vinfo(pe);
  test_colloid_io_options_from_rt(pe);

  pe_info(pe, "%-9s %s\n", "PASS", __FILE__);
  pe_free(pe);

  return 0;
}

/*****************************************************************************
 *
 *  test_colloid_io_mode_valid
 *
 *****************************************************************************/

int test_colloid_io_mode_valid(void) {

  int ifail = 0;

  /* valid */
  {
    colloid_io_mode_enum_t mode = COLLOID_IO_MODE_ANSI;

    ifail = colloid_io_mode_valid(mode);
    assert(ifail != 0);
  }

  /* invalid */
  {
    colloid_io_mode_enum_t mode = COLLOID_IO_MODE_INVALID;

    ifail = colloid_io_mode_valid(mode);
    assert(ifail == 0);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_io_mode_to_string
 *
 *****************************************************************************/

int test_colloid_io_mode_to_string(void) {

  int ifail = 0;

  /* invalid */
  {
    colloid_io_mode_enum_t mode = COLLOID_IO_MODE_INVALID;
    const char *           str  = colloid_io_mode_to_string(mode);

    if (0 != strncmp("invalid", str, strlen(str))) ifail = -1;
    assert(ifail == 0);
  }

  /* ansi */
  {
    colloid_io_mode_enum_t mode = COLLOID_IO_MODE_ANSI;
    const char *           str  = colloid_io_mode_to_string(mode);

    if (0 != strncmp("ansi", str, strlen(str))) ifail = -1;
    assert(ifail == 0);
  }

  /* mpiio */
  {
    colloid_io_mode_enum_t mode = COLLOID_IO_MODE_MPIIO;
    const char *           str  = colloid_io_mode_to_string(mode);

    if (0 != strncmp("mpiio", str, strlen(str))) ifail = -1;
    assert(ifail == 0);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_io_mode_from_string
 *
 *****************************************************************************/

int test_colloid_io_mode_from_string(void) {

  int ifail = 0;

  /* invalid */
  {
    colloid_io_mode_enum_t mode = colloid_io_mode_from_string("rubiish");

    if (mode != COLLOID_IO_MODE_INVALID) ifail = -1;
    assert(ifail == 0);
  }

  /* ansi */
  {
    colloid_io_mode_enum_t mode = colloid_io_mode_from_string("ansi");

    if (mode != COLLOID_IO_MODE_ANSI) ifail = -1;
    assert(ifail == 0);
  }

  /* mpiio */
  {
    colloid_io_mode_enum_t mode = colloid_io_mode_from_string("MPIIO");

    if (mode != COLLOID_IO_MODE_MPIIO) ifail = -1;
    assert(ifail == 0);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_io_options_default
 *
 *****************************************************************************/

int test_colloid_io_options_default(void) {

  colloid_io_options_t opts = colloid_io_options_default();

  assert(opts.mode      == COLLOID_IO_MODE_ANSI);
  assert(opts.iorformat == IO_RECORD_ASCII);
  assert(opts.report    == 0);

  assert(opts.iogrid[0] == 1);
  assert(opts.iogrid[1] == 1);
  assert(opts.iogrid[2] == 1);


  return opts.report;
}

/*****************************************************************************
 *
 *  test_colloid_io_options_valid
 *
 *****************************************************************************/

int test_colloid_io_options_valid(void) {

  int ifail = 0;

  /* valid */
  {
    colloid_io_options_t opts = colloid_io_options_default();

    ifail = colloid_io_options_valid(&opts);
    assert(ifail);
  }

  /* invalid */
  {
    colloid_io_options_t opts = (colloid_io_options_t) {};

    ifail = colloid_io_options_valid(&opts);
    assert(ifail == 0);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_io_options_to_json
 *
 *****************************************************************************/

int test_colloid_io_options_to_json(void) {

  int ifail = 0;

  {
    colloid_io_options_t opts = colloid_io_options_default();
    cJSON *              json = NULL;

    ifail = colloid_io_options_to_json(&opts, &json);
    assert(ifail == 0);

    /* mode */
    {
      cJSON * key = cJSON_GetObjectItemCaseSensitive(json, "I/O mode");
      char *  str = cJSON_GetStringValue(key);
      colloid_io_mode_enum_t mode = colloid_io_mode_from_string(str);
      if (mode != COLLOID_IO_MODE_ANSI) ifail = -1;
      assert(ifail == 0);
    }

    /* ior format */
    {
      cJSON * key = cJSON_GetObjectItemCaseSensitive(json, "Format");
      char *  str = cJSON_GetStringValue(key);
      io_record_format_enum_t ior = io_record_format_from_string(str);
      if (ior != IO_RECORD_ASCII) ifail = -1;
      assert(ifail == 0);
    }

    /* report */
    {
      int     report = -1;
      cJSON * key    = cJSON_GetObjectItemCaseSensitive(json, "Report");

      if (key) report = cJSON_IsTrue(key);
      if (report != 0) ifail = -1;
    }

    /* iogrid (check key exists only) */
    {
      cJSON * key = cJSON_GetObjectItemCaseSensitive(json, "I/O grid");

      if (key == NULL) ifail = -1;
      assert(ifail == 0);
    }
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_io_options_from_json
 *
 *****************************************************************************/

int test_colloid_io_options_from_json(void) {

  int ifail = 0;

  /* Form JSON from a string */
  {
    const char * str  = "{\"I/O mode\": \"mpiio\","
                        "\"Format\": \"binary\","
                        "\"Report\": true,"
                        "\"I/O grid\": [2,3,4] }";
    cJSON *      json = cJSON_Parse(str);
    colloid_io_options_t opts = (colloid_io_options_t) {};

    assert(json);

    ifail = colloid_io_options_from_json(json, &opts);
    assert(ifail == 0);
    assert(opts.mode == COLLOID_IO_MODE_MPIIO);
    assert(opts.iorformat == IO_RECORD_BINARY);
    assert(opts.report);
    assert(opts.iogrid[0] == 2);
    assert(opts.iogrid[1] == 3);
    assert(opts.iogrid[2] == 4);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_io_options_to_vinfo
 *
 *****************************************************************************/

int test_colloid_io_options_to_vinfo(pe_t * pe) {

  int ifail = 0;

  rt_t * rt = NULL;
  colloid_io_options_t opts = colloid_io_options_default();

  rt_create(pe, &rt);

  ifail = colloid_io_options_to_vinfo(rt, RT_NONE, &opts);
  assert(ifail == 0);

  rt_free(rt);

  return ifail;
}


/*****************************************************************************
 *
 *  test_colloid_io_options_from_rt
 *
 *****************************************************************************/

int test_colloid_io_options_from_rt(pe_t * pe) {

  int ifail = 0;

  /* ansi */
  {
    colloid_io_options_t options = {};
    rt_t * rt = NULL;

    rt_create(pe, &rt);
    rt_add_key_value(rt, "colloid_io_options_mode",   "ansi");
    rt_add_key_value(rt, "colloid_io_options_format", "ASCII");
    rt_add_key_value(rt, "colloid_io_options_report", "no");

    ifail = colloid_io_options_from_rt(rt, RT_NONE, &options);

    assert(ifail == 0);
    assert(options.mode == COLLOID_IO_MODE_ANSI);
    assert(options.iorformat == IO_RECORD_ASCII);
    assert(options.report    == 0);
    assert(options.iogrid[0] == 1);
    assert(options.iogrid[1] == 1);
    assert(options.iogrid[2] == 1);

    rt_free(rt);
  }

  /* Bad mode */
  {
    colloid_io_options_t options = {};
    rt_t * rt = NULL;

    rt_create(pe, &rt);
    rt_add_key_value(rt, "colloid_io_options_mode", "rubbish");

    ifail = colloid_io_options_from_rt(rt, RT_NONE, &options);
    assert(ifail == -1);

    rt_free(rt);
  }

  /* Bad format */
  {
    colloid_io_options_t options = {};
    rt_t * rt = NULL;

    rt_create(pe, &rt);
    rt_add_key_value(rt, "colloid_io_options_format", "rubbish");

    ifail = colloid_io_options_from_rt(rt, RT_NONE, &options);
    assert(ifail == -2);

    rt_free(rt);
  }

  /* Good set again: */
  {
    colloid_io_options_t options = {};
    rt_t * rt = NULL;

    rt_create(pe, &rt);
    rt_add_key_value(rt, "colloid_io_options_mode",   "MPIIO");
    rt_add_key_value(rt, "colloid_io_options_format", "binary");
    rt_add_key_value(rt, "colloid_io_options_report", "yes");

    ifail = colloid_io_options_from_rt(rt, RT_NONE, &options);

    assert(ifail == 0);
    assert(options.mode == COLLOID_IO_MODE_MPIIO);
    assert(options.iorformat == IO_RECORD_BINARY);
    assert(options.report    == 1);

    rt_free(rt);
  }

  return ifail;
}
