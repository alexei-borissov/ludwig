/*****************************************************************************
 *
 *  extract_colloids.c
 *
 *  Convert an output file to a csv file suitable for Paraview.
 *  The csv file uses three extra particles at (xmax, 0, 0)
 *  (0, ymax, 0) and (0, 0, zmax) to define the extent of the
 *  system.
 *
 *  If you want a different set of colloid properties, you need
 *  to arrange the header, and the output appropriately. The read
 *  can be ascii or binary and is set by the switch below.
 *
 *  For compilation instructions see the Makefile.
 *
 *  $ make extract_colloids
 *
 *  $ ./a.out <colloid file name stub> <nfile> <csv file name>
 *
 *  where the
 *    
 *  1st argument is the file name stub (in front of the last dot),
 *  2nd argument is the number of parallel files (as set with XXX_io_grid),
 *  3rd argyment is the (single) ouput file name.
 *
 *  If you have a set of files, try (eg. here with 4 parallel output files),
 *
 *  $ for f in config.cds*004-001; do g=`echo $f | sed s/.004-001//`; \
 *  echo $g; ~/ludwig/trunk/util/extract_colloids $g 4 $g.csv; done
 *
 *  Edinburgh Soft Matter and Statistical Physics Group and
 *  Edinburgh Parallel Computing Centre
 *
 *  Kevin Stratford (kevin@epcc.ed.ac.uk)
 *  (c) 2012-2019 The University of Edinburgh
 *
 *****************************************************************************/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "colloid.h"

#define NX 32
#define NY 32
#define NZ 32

static const int  iread_ascii = 1;  /* Read ascii or binary (default) */
static const int  include_ref = 0;  /* Include reference colloids at far x-,y-,z-corners */
static const int  id = 1;  	    /* Output colloid id */
static const int  cds_with_m  = 0;  /* Output coordinate and orientation */
static const int  cds_with_v  = 1;  /* Output coordinate, velocity vector and magnitude */

static const char * format3_    = "%10.5f, %10.5f, %10.5f, ";
static const char * format3end_ = "%10.5f, %10.5f, %10.5f\n";
static const char * formate3end_ = "%13.6e  %13.6e  %13.6e\n";
static const char * formate4end_ = "%14.6e, %14.6e, %14.6e, %14.6e\n";

double **** vel;
int ix,iy,iz,ixc,iyc,izc;
int xstart,xstop,ystart,ystop,zstart,zstop;
double dist;

double normv;

void colloids_to_csv_header(FILE * fp);
void colloids_to_csv_header_with_m(FILE * fp);
void colloids_to_csv_header_with_v(FILE * fp);

int main(int argc, char ** argv) {

  int n;
  int nf, nfile;
  int ncolloid;
  int ncount = 0;
  int nread;

  colloid_state_t s1;
  colloid_state_t s2;

  FILE * fp_colloids = NULL;
  FILE * fp_csv = NULL;
  char filename[FILENAME_MAX];
  FILE * fp_velo = NULL;

  if (argc < 4) {
    printf("Usage: %s <colloid_datafile_stub> <no_of_files> <colloid_csv_filename> [<colloid_grid_velocity_filename>] \n", argv[0]);
    exit(0);
  }

  nfile = atoi(argv[2]);
  printf("Number of files: %d\n", nfile);

  /* Open csv file */
  fp_csv = fopen(argv[3], "w");
  if (fp_csv == NULL) {
    printf("fopen(%s) failed\n", argv[2]);
    exit(0);
  }

  /* Allocate lattice and open vtk file */
  if (argc == 5) {

    vel = (double ****) calloc(NX, sizeof(double ***));
    for (ix = 0; ix < NX; ix++) {
      vel[ix] = (double ***) calloc(NY, sizeof(double **));
      for (iy = 0; iy < NY; iy++) {
	vel[ix][iy] = (double **) calloc(NZ, sizeof(double *));
	for (iz = 0; iz < NZ; iz++) {
	  vel[ix][iy][iz] = (double *) calloc(3, sizeof(double));
	}
      }
    }

    fp_velo = fopen(argv[4], "w");
    if (fp_velo == NULL) {
      printf("fopen(%s) failed\n", argv[3]);
      exit(0);
    }
    /* Write vtk-header */
    fprintf(fp_velo, "# vtk DataFile Version 2.0\n");
    fprintf(fp_velo, "Generated by vtk_extract.c\n");
    fprintf(fp_velo, "ASCII\n");
    fprintf(fp_velo, "DATASET STRUCTURED_POINTS\n");
    fprintf(fp_velo, "DIMENSIONS %d %d %d\n", NX, NY, NZ);
    fprintf(fp_velo, "ORIGIN %d %d %d\n", 0, 0, 0);
    fprintf(fp_velo, "SPACING %d %d %d\n", 1, 1, 1);
    fprintf(fp_velo, "POINT_DATA %d\n", NX*NY*NZ);
    fprintf(fp_velo, "VECTORS vel float\n");

  }

  if(cds_with_m) colloids_to_csv_header_with_m(fp_csv);
  if(cds_with_v) colloids_to_csv_header_with_v(fp_csv);

  for (nf = 1; nf <= nfile; nf++) {

    /* We expect extensions 00n-001 00n-002 ... 00n-00n */ 

    sprintf(filename, "%s.%3.3d-%3.3d", argv[1], nfile, nf);
    printf("Filename: %s\n", filename);

    fp_colloids = fopen(filename, "r");


    if (fp_colloids == NULL) {
        printf("fopen(%s) failed\n", filename);
        exit(0);
    }

    if (iread_ascii) {
      nread = fscanf(fp_colloids, "%d22\n", &ncolloid);
      assert(nread == 1);
    }
    else {
      nread = fread(&ncolloid, sizeof(int), 1, fp_colloids);
      assert(nread == 1);
    }
    if (nread != 1) printf("Warning: problem reading number of collloids\n");

    printf("Reading %d colloids from %s\n", ncolloid, argv[1]);

    /* Read and rewrite the data */

    for (n = 0; n < ncolloid; n++) {

      if (iread_ascii) {
	colloid_state_read_ascii(&s1, fp_colloids);
      }
      else {
	colloid_state_read_binary(&s1, fp_colloids);
      }

      /* Offset the positions */
      s2.r[0] = s1.r[0] - 0.5;
      s2.r[1] = s1.r[1] - 0.5;
      s2.r[2] = s1.r[2] - 0.5;

      /* Write coordinates and orientation 's' or velocity */
      if (id) fprintf(fp_csv, "%4d, ", s1.index);
      fprintf(fp_csv, format3_, s2.r[0], s2.r[1], s2.r[2]);
      if (cds_with_m) fprintf(fp_csv, format3end_, s1.s[0], s1.s[1], s1.s[2]);
      if (cds_with_v) {
	normv = sqrt(s1.v[0]*s1.v[0] + s1.v[1]*s1.v[1] + s1.v[2]*s1.v[2]);
	fprintf(fp_csv, formate4end_, s1.v[0], s1.v[1], s1.v[2], normv);
      }

      /* Write colloid velocity on lattice */
      if (argc == 5) {

	/* Define ROI around colloids */
	xstart = floor(s1.r[0]-s1.a0);
	ystart = floor(s1.r[1]-s1.a0);
	zstart = floor(s1.r[2]-s1.a0);

	xstop = ceil(s1.r[0]+s1.a0);
	ystop = ceil(s1.r[1]+s1.a0);
	zstop = ceil(s1.r[2]+s1.a0);

	for (ix = xstart; ix <= xstop; ix++) {
	  ixc = ix - 1;
	  /* Wrap around for PBC */
	  if (ix < 1)  ixc = ix - 1 + NX; 
	  if (ix > NX) ixc = ix - 1 - NX; 

	  for (iy = ystart; iy <= ystop; iy++) {
	    iyc = iy - 1;
	    /* Wrap around for PBC */
	    if (iy < 1)  iyc = iy - 1 + NY; 
	    if (iy > NY) iyc = iy - 1 - NY; 

	    for (iz = zstart; iz <= zstop; iz++) {
	      izc = iz - 1;
	      /* Wrap around for PBC */
	      if (iz < 1)  izc = iz - 1 + NZ; 
	      if (iz > NZ) izc = iz - 1 - NZ; 

	      dist = sqrt(pow(ix-s1.r[0],2)+pow(iy-s1.r[1],2)+pow(iz-s1.r[2],2));

	      if (dist <= s1.a0) {
		vel[ixc][iyc][izc][0] = s1.v[0]; 
		vel[ixc][iyc][izc][1] = s1.v[1]; 
		vel[ixc][iyc][izc][2] = s1.v[2]; 
	      }

	    }
	  }
	}

      }

      ncount += 1;

    }
  }

  /* Finish colloid coordinate output */
  fclose(fp_csv);
  if (include_ref) {
    printf("Wrote %d actual colloids + 3 reference colloids in header\n", ncount);
  }
  else {
    printf("Wrote %d colloids\n", ncount);
  }

  /* Write velocity output in column-major format and finish */
  if (argc == 5) {

    for (iz = 0; iz < NZ; iz++) {
      for (iy = 0; iy < NY; iy++) {
	for (ix = 0; ix < NX; ix++) {

	    fprintf(fp_velo, formate3end_, vel[ix][iy][iz][0],vel[ix][iy][iz][1],vel[ix][iy][iz][2]);

	}
      }
    }

    free(vel);
    fclose(fp_velo); 
    printf("Wrote velocity of %d colloids on lattice\n", ncount);
  }

  /* Finish */
  fclose(fp_colloids);

  return 0;
}

/*****************************************************************************
 *
 *  colloids_to_csv_header
 *
 *****************************************************************************/

void colloids_to_csv_header(FILE * fp) {

  double r[3];

  if (id) fprintf(fp, "%s", "id, ");
  fprintf(fp, "%s", "x, y, z\n");

  if (include_ref) {

    r[0] = 1.0*NX - 1.0;
    r[1] = 0.0;
    r[2] = 0.0;

    fprintf(fp, format3_, r[0], r[1], r[2]);
    fprintf(fp, "\n");

    r[0] = 0.0;
    r[1] = 1.0*NY - 1.0;
    r[2] = 0.0;

    fprintf(fp, format3_, r[0], r[1], r[2]);
    fprintf(fp, "\n");

    r[0] = 0.0;
    r[1] = 0.0;
    r[2] = 1.0*NZ - 1.0;

    fprintf(fp, format3_, r[0], r[1], r[2]);
    fprintf(fp, "\n");

  }

  return;
}

/*****************************************************************************
 *
 *  colloids_to_csv_header_with_m
 *
 *****************************************************************************/

void colloids_to_csv_header_with_m(FILE * fp) {

  double r[3];
  double m[3];

  if (id) fprintf(fp, "%s", "id, ");
  fprintf(fp, "%s", "x, y, z, mx, my, mz\n");

  if (include_ref) {

    r[0] = 1.0*NX - 1.0;
    r[1] = 0.0;
    r[2] = 0.0;

    m[0] = 1.0;
    m[1] = 0.0;
    m[2] = 0.0;

    fprintf(fp, format3_, r[0], r[1], r[2]);
    fprintf(fp, format3end_, m[0], m[1], m[2]);

    r[0] = 0.0;
    r[1] = 1.0*NY - 1.0;
    r[2] = 0.0;

    m[0] = 0.0;
    m[1] = 1.0;
    m[2] = 0.0;

    fprintf(fp, format3_, r[0], r[1], r[2]);
    fprintf(fp, format3end_, m[0], m[1], m[2]);

    r[0] = 0.0;
    r[1] = 0.0;
    r[2] = 1.0*NZ - 1.0;

    m[0] = 0.0;
    m[1] = 0.0;
    m[2] = 1.0;

    fprintf(fp, format3_, r[0], r[1], r[2]);
    fprintf(fp, format3end_, m[0], m[1], m[2]);

  }

  return;
}

/*****************************************************************************
 *
 *  colloids_to_csv_header_with_v
 *
 *****************************************************************************/

void colloids_to_csv_header_with_v(FILE * fp) {

  double r[3];

  if (id) fprintf(fp, "%s", "id, ");
  fprintf(fp, "%s", "x, y, z, vx, vy, vz, normv\n");

  if (include_ref) {

    r[0] = 1.0*NX - 1.0;
    r[1] = 0.0;
    r[2] = 0.0;

    fprintf(fp, format3_, r[0], r[1], r[2]);
    fprintf(fp, format3end_, 0, 0, 0, 0);

    r[0] = 0.0;
    r[1] = 1.0*NY - 1.0;
    r[2] = 0.0;

    fprintf(fp, format3_, r[0], r[1], r[2]);
    fprintf(fp, format3end_, 0, 0, 0, 0);

    r[0] = 0.0;
    r[1] = 0.0;
    r[2] = 1.0*NZ - 1.0;

    fprintf(fp, format3_, r[0], r[1], r[2]);
    fprintf(fp, format3end_, 0, 0, 0, 0);

  }

  return;
}
