/*****************************************************************************
 *
 *  test_colloid_options.c
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
#include <float.h>
#include <math.h>

#include "pe.h"
#include "colloid_options.h"

int test_colloid_options_default(void);
int test_colloid_options_have_colloids(void);
int test_colloid_options_ncell(void);
int test_colloid_options_ncell_valid(void);
int test_colloid_options_valid(void);

int test_colloid_options_to_json(void);
int test_colloid_options_from_json(void);

int test_colloid_options_to_vinfo(pe_t * pe);

/*****************************************************************************
 *
 *  test_colloid_options_suite
 *
 *****************************************************************************/

int test_colloid_options_suite(void) {

  pe_t * pe = NULL;

  pe_create(MPI_COMM_WORLD, PE_QUIET, &pe);

  test_colloid_options_default();
  test_colloid_options_have_colloids();
  test_colloid_options_ncell();
  test_colloid_options_ncell_valid();
  test_colloid_options_valid();

  test_colloid_options_to_json();
  test_colloid_options_from_json();

  test_colloid_options_to_vinfo(pe);

  pe_info(pe, "%-9s %s\n", "PASS", __FILE__);
  pe_free(pe);

  return 0;
}

/*****************************************************************************
 *
 *  test_colloid_options_default
 *
 *****************************************************************************/

int test_colloid_options_default(void) {

  int               ifail   = 0;
  colloid_options_t options = colloid_options_default();

  assert(options.ncell[0] == 2);
  assert(options.ncell[1] == 2);
  assert(options.ncell[2] == 2);
  assert(options.nvel     == 19);

  assert(options.bbl_build_freq == 1);

  if (fabs(options.rho0  - 1.0) > DBL_EPSILON) ifail = -1;
  assert(ifail == 0);

  if (fabs(options.drmax - 0.8) > DBL_EPSILON) ifail = -2;
  assert(ifail == 0);

  assert(colloid_io_options_valid(&options.input));
  assert(colloid_io_options_valid(&options.output));

  assert(options.nfreq == 0);
  assert(options.have_colloids == 0);

  /* If the size changes, the tests must change */
  assert(sizeof(colloid_options_t) == 96);

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_options_have_colloids
 *
 *****************************************************************************/

int test_colloid_options_have_colloids(void) {

  int ifail = 0;

  /* No colloids explicitly */
  {
    int               have_colloids = 0;
    colloid_options_t opts = colloid_options_have_colloids(have_colloids);

    assert(opts.have_colloids == have_colloids);
    assert(opts.nvel  == 19);
    assert(opts.nfreq == 0);
    if (opts.nfreq != 0) ifail = -1;
  }

  /* With have_colloids flag */
  {
    int               have_colloids = 1;
    colloid_options_t opts = colloid_options_have_colloids(have_colloids);

    assert(opts.have_colloids == have_colloids);
    assert(opts.nvel  == 19);
    assert(opts.nfreq == 0);
    if (opts.nfreq != 0) ifail = -1;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_options_ncell
 *
 *****************************************************************************/

int test_colloid_options_ncell(void) {

  int ifail = 0;

  {
    int               ncell[3] = {5, 6, 7};
    colloid_options_t opts     = colloid_options_ncell(ncell);

    assert(opts.have_colloids != 0);

    assert(opts.ncell[0] == ncell[0]);
    assert(opts.ncell[1] == ncell[1]);
    assert(opts.ncell[2] == ncell[2]);
    assert(opts.nvel     == 19);
    assert(opts.nfreq    == 0);
    if (opts.nfreq != 0) ifail = -1;
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_options_ncell_valid
 *`
 *****************************************************************************/

int test_colloid_options_ncell_valid(void) {

  int ifail = 0;

  /* valid */
  {
    int ncell[3] = {3, 4, 5};

    ifail = colloid_options_ncell_valid(ncell);
    assert(ifail);
  }

  /* invalid */
  {
    int ncell[3] = {2, 2, 1};

    ifail = colloid_options_ncell_valid(ncell);
    assert(ifail == 0);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_options_valid
 *
 *****************************************************************************/

int test_colloid_options_valid(void) {

  int ifail = 0;

  /* valid */
  {
    colloid_options_t options = colloid_options_default();

    ifail = colloid_options_valid(&options);
    assert(ifail);
  }

  /* invalid */
  {
    colloid_options_t options = {.rho0 = -1.0};

    ifail = colloid_options_valid(&options);
    assert(ifail == 0);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_options_to_json
 *
 *****************************************************************************/

int test_colloid_options_to_json(void) {

  int ifail = 0;

  {
    /* Default object and generate JSON ... */
    colloid_options_t opts = colloid_options_default();
    cJSON *           json = NULL;

    ifail = colloid_options_to_json(&opts, &json);
    assert(ifail == 0);

    if (json) {
      cJSON * jhave  = cJSON_GetObjectItemCaseSensitive(json, "have_colloids");
      cJSON * jnvel  = cJSON_GetObjectItemCaseSensitive(json, "nvel");
      cJSON * jrho0  = cJSON_GetObjectItemCaseSensitive(json, "rho0");
      cJSON * jdrmax = cJSON_GetObjectItemCaseSensitive(json, "drmax");

      assert(cJSON_IsBool(jhave));
      assert(cJSON_IsNumber(jnvel));
      assert(cJSON_IsNumber(jrho0));
      assert(cJSON_IsNumber(jdrmax));

      if (jhave) {
        /* Default option is no colloids */
        if (0 != cJSON_IsTrue(jhave)) ifail = -1;
        assert(ifail == 0);
      }
      /* Test a subset of components */
      if (jnvel) {
        int nvel = cJSON_GetNumberValue(jnvel);
        if (nvel != opts.nvel) ifail = -1;
        assert(ifail == 0);
      }

      if (jrho0) {
        double rho0 = cJSON_GetNumberValue(jrho0);
        if (fabs(rho0 - opts.rho0) > DBL_EPSILON) ifail = -1;
        assert(ifail == 0);
      }

      if (jdrmax) {
        double drmax = cJSON_GetNumberValue(jdrmax);
        if (fabs(drmax - opts.drmax) > DBL_EPSILON) ifail = -2;
        assert(ifail == 0);
      }

      cJSON_Delete(json);
    }
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_options_from_json
 *
 *****************************************************************************/

int test_colloid_options_from_json(void) {

  int ifail = 0;

  {
    /* All components are required. Slightly long-winded ... */
    const char * str = "{\"have_colloids\": true, "
                       "\"ncell\": [3, 4, 5], "
                       "\"nvel\": 27, "
                       "\"bbl_build_freq\": 10, "
                       "\"rho0\": 2.00, "
                       "\"drmax\": 1.00, "
                       "\"Input options\": { "
                       "\"I/O mode\": \"ansi\", "
                       "\"Format\":   \"binary\", "
                       "\"Report\": false, "
                       "\"I/O grid\": [6, 7, 8] }, "
                       "\"Output options\": { "
                       "\"I/O mode\": \"mpiio\", "
                       "\"Format\": \"ascii\", "
                       "\"Report\": true, "
                       "\"I/O grid\": [0, 1, 2] }, "
                       "\"nfreq\": 1000 }";

    cJSON * json = cJSON_Parse(str);
    assert(json);

    if (json) {
      colloid_options_t opts = {0};
      ifail                  = colloid_options_from_json(json, &opts);
      assert(ifail == 0);
      assert(opts.ncell[0] == 3);
      assert(opts.ncell[1] == 4);
      assert(opts.ncell[2] == 5);
      assert(opts.nvel     == 27);

      assert(opts.bbl_build_freq == 10);

      assert(fabs(opts.rho0 - 2.0)  < DBL_EPSILON);
      assert(fabs(opts.drmax - 1.0) < DBL_EPSILON);

      /* Input */
      assert(opts.input.mode      == COLLOID_IO_MODE_ANSI);
      assert(opts.input.iorformat == IO_RECORD_BINARY);
      assert(opts.input.report    == 0);
      assert(opts.input.iogrid[0] == 6);
      assert(opts.input.iogrid[1] == 7);
      assert(opts.input.iogrid[2] == 8);
      /* Output */
      assert(opts.output.mode      == COLLOID_IO_MODE_MPIIO);
      assert(opts.output.iorformat == IO_RECORD_ASCII);
      assert(opts.output.report    != 0);
      assert(opts.output.iogrid[0] == 0);
      assert(opts.output.iogrid[1] == 1);
      assert(opts.output.iogrid[2] == 2);
      /* Frequency */
      assert(opts.nfreq == 1000);
    }

    cJSON_Delete(json);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_options_to_vinfo
 *
 *****************************************************************************/

int test_colloid_options_to_vinfo(pe_t * pe) {

  int ifail = 0;
  rt_t * rt = NULL;

  colloid_options_t opts = colloid_options_default();

  rt_create(pe, &rt);

  ifail = colloid_options_to_vinfo(rt, RT_NONE, &opts);
  assert(ifail == 0);

  rt_free(rt);

  return ifail;
}
