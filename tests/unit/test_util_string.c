/*****************************************************************************
 *
 *  test_util_string.c
 *
 *****************************************************************************/

#include <assert.h>

#include "pe.h"
#include "util_string.h"

int test_util_strnlen(void);

/*****************************************************************************
 *
 *  test_util_string_suite
 *
 *****************************************************************************/

int test_util_string_suite(void) {

  pe_t * pe = NULL;

  pe_create(MPI_COMM_WORLD, PE_QUIET, &pe);

  test_util_strnlen();

  pe_info(pe, "%-9s %s\n", "PASS", __FILE__);
  pe_free(pe);

  return 0;
}

/******************************************************************************
 *
 *  test_util_strnlen
 *
 *****************************************************************************/

int test_util_strnlen(void) {

  int ifail = 0;

  /* Empty */
  {
    char s[BUFSIZ] = {0};
    size_t len = util_strnlen(s, BUFSIZ);
    if (len != 0) ifail = -1;
    assert(ifail == 0);
  }

  /* Length */
  {
    const char * s = "Length";
    size_t len = util_strnlen(s, BUFSIZ);
    if (len != 6) ifail = -1;
    assert(ifail == 0);
  }

  return ifail;
}
