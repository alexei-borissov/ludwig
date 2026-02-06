/*****************************************************************************
 *
 *  test_colloid_array_util.c
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

#include "pe.h"
#include "colloid_array_util.h"

int test_colloid_array_alloc(void);
int test_colloid_array_realloc(void);
int test_colloid_array_free(void);

/*****************************************************************************
 *
 *  test_colloid_array_util_suite
 *
 *****************************************************************************/

int test_colloid_array_util_suite(void) {

  int    ifail = 0;
  pe_t * pe    = NULL;

  pe_create(MPI_COMM_WORLD, PE_QUIET, &pe);

  /* If struct changes, the tests need updating... */
  assert(sizeof(colloid_array_t) == 16);

  test_colloid_array_alloc();
  test_colloid_array_realloc();
  test_colloid_array_free();

  pe_info(pe, "%-9s %s\n", "PASS", __FILE__);
  pe_free(pe);

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_array_alloc
 *
 *  Split into two parts for standard, and managed, as the managed
 *  versions require separate device tests.
 *
 *****************************************************************************/

int test_colloid_array_alloc_host(void);
int test_colloid_array_alloc_managed(void);

int test_colloid_array_alloc(void) {

  test_colloid_array_alloc_host();
  test_colloid_array_alloc_managed();

  return 0;
}

/*****************************************************************************
 *
 *  test_colloid_array_realloc
 *
 *  This is split into two again.
 *
 *****************************************************************************/

int test_colloid_array_realloc_host(void);
int test_colloid_array_realloc_managed(void);

int test_colloid_array_realloc(void) {

  test_colloid_array_realloc_host();
  test_colloid_array_realloc_managed();

  return 0;
}

/*****************************************************************************
 *
 *  test_colloid_array_free
 *
 *  For completeness' sake.
 *
 *****************************************************************************/

int test_colloid_array_free(void) {

  int ifail = 0;

  /* Host */
  {
    int managed = 0;
    int ntotal  = 1;

    colloid_array_t buf = {0};

    ifail = colloid_array_alloc(managed, ntotal, &buf);
    assert(ifail == 0);

    colloid_array_free(&buf);
    assert(buf.managed == 0);
    assert(buf.ntotal  == 0);
    assert(buf.data    == NULL);
  }

  /* Managed */
  {
    int managed = 1;
    int ntotal  = 1;

    colloid_array_t buf = {0};

    ifail = colloid_array_alloc(managed, ntotal, &buf);
    assert(ifail == 0);

    colloid_array_free(&buf);
    assert(buf.managed == 0);
    assert(buf.ntotal  == 0);
    assert(buf.data    == NULL);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_array_alloc_host
 *
 *****************************************************************************/

int test_colloid_array_alloc_host(void) {

  int ifail   = 0;
  int managed = 0; /* all host allocations */

  /* Check zero-sized alloc fails elegantly ... */
  {
    int             ntotal = 0;
    colloid_array_t buf    = {0};

    ifail = colloid_array_alloc(managed, ntotal, &buf);
    assert(ifail != 0);
  }

  /* Allocation should give us the right to access relevant elements */
  {
    int             ntotal = 19;
    colloid_array_t buf    = {0};

    ifail = colloid_array_alloc(managed, ntotal, &buf);
    assert(ifail == 0);

    assert(buf.managed == managed);
    assert(buf.ntotal  == ntotal);
    assert(buf.data != NULL);

    for (int i = 0; i < buf.ntotal; i++) {
      buf.data[i] = (colloid_state_t) {0};
    }

    colloid_array_free(&buf);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_array_alloc_managed
 *
 *  There are two kernels:
 *
 *****************************************************************************/

__global__ void kernel1(int ntotal, colloid_array_t buf) {

  int i = blockIdx.x; /* Index via block (all threads) */

  assert(buf.managed == 1);
  assert(buf.ntotal  == ntotal);
  assert(buf.data    != NULL);

  if (i >= ntotal) {
    printf("fail for all threads: blockIdx.x %d\n", i);
    assert(0);
  }

  if (buf.data[i].index != 1 + i) {
    printf("fail at %d\n", i);
    assert(0);
  }

  assert(buf.data[i].index == 1 + i);

  return;
}

__global__ void kernel2(int ntotal, colloid_array_t buf) {

  int i = blockIdx.x; /* Index via block (all threads) */

  assert(buf.managed == 1);
  assert(buf.ntotal  == ntotal);
  assert(buf.data    != NULL);

  if (i >= ntotal) {
    printf("fail for all threads: blockIdx.x %d\n", i);
    assert(0);
  }

  if (threadIdx.x == 0) {
    buf.data[i].index = 1 + i;
  }

  return;
}

/*****************************************************************************
 *
 *  test_colloid_array_alloc_managed
 *
 *  Driver.
 *
 *****************************************************************************/

int test_colloid_array_alloc_managed(void) {

  int ifail   = 0;
  int managed = 1; /* managed allocations */

  /* Host assignment */
  {
    int             ntotal = 32;
    colloid_array_t buf    = {0};

    ifail = colloid_array_alloc(managed, ntotal, &buf);
    assert(ifail == 0);
    assert(buf.managed == managed);
    assert(buf.ntotal  == ntotal);
    assert(buf.data    != NULL);

    /* We have the right to access these elements ... */
    for (int i = 0; i < buf.ntotal; i++) {
      buf.data[i]       = (colloid_state_t) {0};
      buf.data[i].index = 1 + i;
    }

    /* Values should be reflected in a kernel */
    /* Run one block for each array element.  */
    {
      dim3 blocks  = {  1, 1, 1};
      dim3 threads = {128, 1, 1};

      blocks.x = ntotal;

      tdpLaunchKernel(kernel1, blocks, threads, 0, 0, ntotal, buf);
      tdpAssert(tdpStreamSynchronize(0));
    }

    colloid_array_free(&buf);
  }

  /* Kernel assignment */
  {
    int             ntotal = 129;
    colloid_array_t buf    = {0};

    ifail = colloid_array_alloc(managed, ntotal, &buf);
    assert(ifail == 0);

    /* Kernel; again one block per element, */
    /* kernel2 assigns values */
    {
      dim3 blocks  = { 1, 1, 1};
      dim3 threads = {32, 1, 1};

      blocks.x = ntotal;

      tdpLaunchKernel(kernel2, blocks, threads, 0, 0, ntotal, buf);
      tdpAssert(tdpStreamSynchronize(0));
    }

    /* And check ... */
    for (int i = 0; i < buf.ntotal; i++) {
      if (buf.data[i].index != 1 + i) {
        ifail = -1;
      }
      assert(ifail == 0);
    }

    colloid_array_free(&buf);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_array_realloc_host
 *
 *****************************************************************************/

int test_colloid_array_realloc_host(void) {

  int ifail = 0;

  /* realloc with an empty object */
  {
    int             newtotal = 20;
    colloid_array_t buf      = {0};

    ifail = colloid_array_realloc(newtotal, &buf);
    assert(ifail == 0);

    assert(buf.managed == 0);
    assert(buf.ntotal  == newtotal);
    assert(buf.data);

    /* Check elements can be accessed */
    for (int i = 0; i < buf.ntotal; i++) {
      buf.data[i] = (colloid_state_t) {0};
    }

    colloid_array_free(&buf);
  }

  /* Check existing data is preserved */
  {
    int managed  = 0;
    int ntotal   = 10;
    int newtotal = 20;

    colloid_array_t buf = {0};

    colloid_array_alloc(managed, ntotal, &buf);
    for (int i = 0; i < buf.ntotal; i++) {
      buf.data[i]       = (colloid_state_t) {0};
      buf.data[i].index = 1 + i;
    }

    /* re-allocate */
    ifail = colloid_array_realloc(newtotal, &buf);
    assert(ifail == 0);
    assert(buf.managed == managed);
    assert(buf.ntotal == newtotal);
    assert(buf.data);

    /* Check existing data, and assess the new data. */
    for (int i = 0; i < buf.ntotal; i++) {
      if (i < ntotal) {
        if (buf.data[i].index != 1 + i) {
          ifail = -1;
        }
        assert(ifail == 0);
      }
      buf.data[i] = (colloid_state_t) {0};
    }

    colloid_array_free(&buf);
  }

  return ifail;
}

/*****************************************************************************
 *
 *  test_colloid_array_realloc_managed
 *
 *  Two kernels are required.
 *
 *****************************************************************************/

int test_colloid_array_realloc_managed(void) {

  int ifail = 0;

  /* Allocate and set some values using kernel2, then re-allocate,
   * set the additional values on the host, and check with kernel1. */
  {
    int managed  = 1;
    int ntotal   = 10;
    int newtotal = 20;

    colloid_array_t buf = {0};

    ifail = colloid_array_alloc(managed, ntotal, &buf);

    {
      dim3 blocks  = { 1, 1, 1};
      dim3 threads = {32, 1, 1};

      blocks.x = ntotal;

      tdpLaunchKernel(kernel2, blocks, threads, 0, 0, ntotal, buf);
      tdpAssert(tdpStreamSynchronize(0));
    }

    /* Re-allocate */
    ifail = colloid_array_realloc(newtotal, &buf);
    assert(ifail == 0);

    assert(buf.managed == 1);
    assert(buf.ntotal  == newtotal);
    assert(buf.data);

    /* set additional values ... */
    for (int i = ntotal; i < newtotal; i++) {
      buf.data[i]       = (colloid_state_t) {0};
      buf.data[i].index = 1 + i;
    }

    /* Recheck */
    {
      dim3 blocks  = { 1, 1, 1};
      dim3 threads = {32, 1, 1};

      blocks.x = newtotal;

      tdpLaunchKernel(kernel1, blocks, threads, 0, 0, newtotal, buf);
      tdpAssert(tdpStreamSynchronize(0));
    }

    colloid_array_free(&buf);
  }

  return ifail;
}
