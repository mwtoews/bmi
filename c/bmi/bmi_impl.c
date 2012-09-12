#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>

#include <bmi.h>

typedef struct
{
  double dt;
  double t;

  int n_x;
  int n_y;

  double dx;
  double dy;
  double **z;

  double **temp_z;
} Self;

void *
BMI_Initialize (const char *config_file)
{
  Self *self = NULL;

  self = malloc (sizeof (Self));

  if (config_file)
  { /* Read input file */
    FILE *fp = NULL;

    double dt = 0.;
    int n_x = 0;
    int n_y = 0;

    fp = fopen (config_file, "r");
    if (!fp)
      return NULL;
    fscanf (fp, "%f, %d, %d", &dt, &n_x, &n_y);

    self->dt = dt;
    self->n_x = n_x;
    self->n_y = n_y;
    self->dx = 1.;
    self->dy = 1.;
  }
  else
  { /* Set to default values */
    self->dt = 1.;
    self->n_x = 10;
    self->n_y = 20;
    self->dx = 1.;
    self->dy = 1.;
  }

  { /* Initialize data */
    int i;
    const int len = self->n_x * self->n_y;
    double top_x = self->n_x - 1;

    /* Allocate memory */
    self->temp_z = (double **)malloc (sizeof (double*) * self->n_y);
    self->z = (double **)malloc (sizeof (double*) * self->n_y);

    self->z[0] = (double *)malloc (sizeof (double) * self->n_x * self->n_y);
    self->temp_z[0] = (double *)malloc (sizeof (double) * self->n_x * self->n_y);
    for (i=1; i<self->n_y; i++) {
      self->z[i] = self->z[i-1] + self->n_x;
      self->temp_z[i] = self->temp_z[i-1] + self->n_x;
    }

    self->t = 0;
    for (i = 0; i < len; i++)
      self->z[0][i] = random ()*1./RAND_MAX;
    for (i = 0; i < self->n_y; i++) {
      self->z[i][0] = 0.;
      self->z[i][self->n_x-1] = 0.;
    }
    for (i = 0; i < self->n_x; i++) {
      self->z[0][i] = 0.;
      self->z[self->n_y-1][i] = top_x*top_x*.25 - (i-top_x*.5) * (i-top_x*.5);
    }
    

    memcpy (self->temp_z[0], self->z[0], sizeof (double)*self->n_x*self->n_y);
  }

  return (void *)self;
}

void
BMI_Update_until (void *handle, double dt)
{
  Self *self = (Self *) handle;

  self->t += self->dt;

  {
    int i, j;
    const double rho = 0.;
    const double dx2 = self->dx * self->dx;
    const double dy2 = self->dy * self->dy;
    const double dx2_dy2_rho = dx2 * dy2 * rho;
    const double denom = 1./(2 * (dx2 + dy2));
    double **z = self->z;

    for (i=1; i<self->n_y-1; i++)
      for (j=1; j<self->n_x-1; j++)
        self->temp_z[i][j] = denom * (dx2 * (z[i-1][j] + z[i+1][j]) +
                                      dy2 * (z[i][j-1] + z[i][j+1]) -
                                      dx2_dy2_rho);
  }

  memcpy (self->z[0], self->temp_z[0], sizeof (double) * self->n_y * self->n_x);

  return;
}

void
BMI_Finalize (void *handle)
{
  Self *self = (Self *) handle;

  if (self)
  {
    free (self->temp_z[0]);
    free (self->temp_z);
    free (self->z[0]);
    free (self->z);
    free (self);
  }

  return;
}

const char *
BMI_Get_var_type (void *handle, const char *long_var_name)
{
  if (strcasecmp (long_var_name, "grid_longitude") == 0)
    return "double";
  else if (strcasecmp (long_var_name, "grid_latitude") == 0)
    return "double";
  else if (strcasecmp (long_var_name, "height_above_sea_floor") == 0)
    return "double";
  else
    return NULL;
}

const char *
BMI_Get_var_units (void *handle, const char *long_var_name)
{
  if (strcmp (long_var_name, "grid_longitude") == 0)
    return "arc_degree";
  else if (strcmp (long_var_name, "grid_latitude") == 0)
    return "arc_degree";
  else if (strcmp (long_var_name, "height_above_sea_floor") == 0)
    return "meter";
  else
    return NULL;
}

int
BMI_Get_var_rank (void *handle, const char *long_var_name)
{
  if (strcmp (long_var_name, "grid_longitude") == 0)
    return 2;
  else if (strcmp (long_var_name, "grid_latitude") == 0)
    return 2;
  else if (strcmp (long_var_name, "height_above_sea_floor") == 0)
    return 2;
  else
    return -1;
}

int *
BMI_Get_grid_shape (void *handle, const char *long_var_name, int * n_dim)
{
  int * shape = NULL;

  if (strcmp (long_var_name, "height_above_sea_floor") == 0) {
    Self *self = (Self *) handle;

    shape = (int *)malloc (sizeof (int)*2);

    shape[0] = self->n_y;
    shape[1] = self->n_x;

    *n_dim = 2;
  }
  else
    *n_dim = 0;

  return shape;
}

double *
BMI_Get_grid_spacing (void *handle, const char *long_var_name, int * n_dim)
{
  double * spacing = NULL;

  if (strcmp (long_var_name, "height_above_sea_floor") == 0) {
    Self *self = (Self *) handle;

    spacing = (double *)malloc (sizeof (double)*2);

    spacing[0] = self->dy;
    spacing[1] = self->dx;

    *n_dim = 2;
  }
  else
    *n_dim = 0;

  return spacing;
}

double *
BMI_Get_grid_origin (void *handle, const char *long_var_name, int * n_dim)
{
  double * origin = NULL;

  if (strcmp (long_var_name, "height_above_sea_floor") == 0) {
    origin = (double *)malloc (sizeof (double)*2);

    origin[0] = 0.;
    origin[1] = 0.;

    *n_dim = 2;
  }
  else
    *n_dim = 0;

  return origin;
}

BMI_Grid_type
BMI_Get_grid_type (void *handle, const char *long_var_name)
{
  if (strcmp (long_var_name, "height_above_sea_floor") == 0)
    return BMI_GRID_TYPE_UNIFORM;
  else
    return BMI_GRID_TYPE_UNKNOWN;
}

double *
BMI_Get_double (void *handle, const char *long_var_name, int * n_dims, int **shape)
{
  double * val = NULL;

  if (strcmp (long_var_name, "height_above_sea_floor")==0) {
    Self *self = (Self *) handle;

    val = self->z[0];
    *n_dims = 2;
  }

  if (shape != NULL)
    *shape = BMI_Get_grid_shape (handle, long_var_name, n_dims);

  return val;
}

void
BMI_Set_double (void *handle, const char *long_var_name, double *array)
{
  if (strcmp (long_var_name, "height_above_sea_floor")==0) {
    Self *self = (Self *) handle;
    memcpy (self->z[0], array, sizeof (double) * self->n_x * self->n_y);
  }

  return;
}

// Assume string arrays are NULL-terminated
const char *
BMI_Get_component_name (void *handle)
{
  return "Example C model";
}

const char *input_var_names[] = {
  "height_above_sea_floor",
  NULL
};

const char **
BMI_Get_input_var_names (void *handle)
{
  return input_var_names;
}

const char *output_var_names[] = {
  "grid_longitude",
  "height_above_sea_floor",
  NULL
};

const char **
BMI_Get_output_var_names (void *handle)
{
  return output_var_names;
}

double
BMI_Get_start_time (void *handle) {
  return 0.;
}

double
BMI_Get_end_time (void *handle) {
  return DBL_MAX;
}

double
BMI_Get_current_time (void *handle) {
  Self *self = (Self *) handle;
  return self->t;
}
