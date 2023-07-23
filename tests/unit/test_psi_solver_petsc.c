/*****************************************************************************
 *
 *  test_psi_solver_petsc.c
 *
 *  Some ducking and diving is required depending on whether Petsc
 *  is available.
 *
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  (c) 2023 The University of Edinburgh
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *
 *****************************************************************************/

#include <assert.h>

#include "psi_petsc.h"

int test_psi_solver_petsc_create(pe_t * pe);

/*****************************************************************************
 *
 *  test_psi_solver_petsc_suite
 *
 *****************************************************************************/

int test_psi_solver_petsc_suite(void) {

  pe_t * pe = NULL;

  pe_create(MPI_COMM_WORLD, PE_QUIET, &pe);

  test_psi_solver_petsc_create(pe);

  pe_info(pe, "%-9s %s\n", "PASS", __FILE__);
  pe_free(pe);

  return 0;
}

/*****************************************************************************
 *
 *  test_psi_solver_petsc_create
 *
 *****************************************************************************/

int test_psi_solver_petsc_create(pe_t * pe) {

  int ifail = 0;
  int isInitialised = 0;

  PetscInitialised(&isInitialised);

  if (isInitialised == 0) {
    psi_t * psi = NULL;
    psi_solver_petsc_t * petsc = NULL;

    ifail = psi_solver_petsc_create(psi, &petsc);
    assert(ifail != 0);
    if (ifail != 0) ifail = 0;
  }

  return 0;
}
