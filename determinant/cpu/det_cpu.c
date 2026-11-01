#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#define MIN_VALUE -100.0
#define MAX_VALUE 100.0


double** create_matrix (int n)
{
  double** matrix = malloc(n * sizeof(double*));
  for (int i = 0; i < n; i++)
    matrix[i] = malloc(n * sizeof(double));
  return matrix;
}

void free_matrix (double** matrix, int n)
{
  for (int i = 0; i < n; i++)
    free(matrix[i]);
  free(matrix);
}

void fill_random (double** matrix, int n)
{
  for (int i = 0; i < n; i++)
    for (int j = 0; j < n; j++)
      matrix[i][j] = MIN_VALUE + (MAX_VALUE - MIN_VALUE) * rand() / RAND_MAX;
}

int read_number (FILE* file, double* value)
{
  int c = fgetc(file);
  while (c != EOF && !isdigit(c) && c != '-' && c != '+' && c != '.')
    c = fgetc(file);
  if (c == EOF)
    return 0;
  ungetc(c, file);
  return fscanf(file, "%lf", value) == 1;
}

double** read_matrix (FILE* file, int* n)
{
  double first = 0;
  int count = 0;
  if (read_number(file, &first))
    count = (int)first;
  if (count <= 0)
  {
    return NULL;
  }

  *n = (int)round(sqrt(count));
  if ((*n) * (*n) != count)
  {
    printf("count of elements %d is not n*n\n", count);
    return NULL;
  }

  double** matrix = create_matrix(*n);
  for (int i = 0; i < *n; i++)
    for (int j = 0; j < *n; j++)
      if (!read_number(file, &matrix[i][j]))
      {
        printf("not enough elements\n");
        free_matrix(matrix, *n);
        return NULL;
      }

  return matrix;
}

double** open_file (const char* filename, int* n)
{
  FILE* file = fopen(filename, "r");
  if (!file)
  {
    printf("cant open file %s\n", filename);
    return NULL;
  }
  double** matrix = read_matrix(file, n);
  fclose(file);
  return matrix;
}

void swap_rows (double** matrix, int row1, int row2)
{
  double* tmp = matrix[row1];
  matrix[row1] = matrix[row2];
  matrix[row2] = tmp;
}

int find_pivot (double** matrix, int n, int col)
{
  int pivot = col;
  for (int i = col + 1; i < n; i++)
    if (fabs(matrix[i][col]) > fabs(matrix[pivot][col]))
      pivot = i;
  return pivot;
}

double determinant (double** matrix, int n, int* sign)
{
  double log_det = 0;
  *sign = 1;

  for (int k = 0; k < n; k++)
  {
    int pivot = find_pivot(matrix, n, k);
    if (matrix[pivot][k] == 0)
    {
      *sign = 0;
      return -INFINITY;
    }

    if (pivot != k)
    {
      swap_rows(matrix, k, pivot);
      *sign = -*sign;
    }

    if (matrix[k][k] < 0)
      *sign = -*sign;
    log_det += log(fabs(matrix[k][k]));

    for (int i = k + 1; i < n; i++)
    {
      double m = matrix[i][k] / matrix[k][k];
      for (int j = k + 1; j < n; j++)
        matrix[i][j] -= m * matrix[k][j];
    }
  }

  return log_det;
}

int main (int argc, char* argv[])
{
  int n = 0;
  int sign = 0;
  double** matrix = NULL;
  char filename[256] = "";
  int mode = 0;

  if (argc == 1)
  {
    printf("1 - random matrix\n2 - matrix from file\n3 - matrix from keyboard\nchoose mode: ");
    if (scanf("%d", &mode) != 1 || mode < 1 || mode > 3)
    {
      printf("wrong mode\n");
      return 1;
    }
    if (mode == 1)
    {
      printf("matrix size: ");
      if (scanf("%d", &n) != 1)
        n = 0;
    }
    else if (mode == 2)
    {
      printf("file name: ");
      if (scanf("%255s", filename) != 1)
        filename[0] = 0;
    }
  }
  else if (argc == 3 && strcmp(argv[1], "-f") == 0)
  {
    mode = 2;
    strncpy(filename, argv[2], 255);
  }
  else if (argc == 2)
  {
    mode = 1;
    n = atoi(argv[1]);
  }
  else
  {
    printf("usage: %s  or  %s <size>  or  %s -f <file>\n", argv[0], argv[0], argv[0]);
    return 1;
  }

  if (mode == 1)
  {
    if (n <= 0)
    {
      printf("matrix size must be > 0\n");
      return 1;
    }
    srand(time(NULL));
    matrix = create_matrix(n);
    fill_random(matrix, n);
  }
  else if (mode == 2)
  {
    matrix = open_file(filename, &n);
    if (!matrix)
      return 1;
  }
  else
  {
    printf("count of elements, then elements: ");
    matrix = read_matrix(stdin, &n);
    if (!matrix)
      return 1;
  }
  clock_t start = clock();
  double log_det = determinant(matrix, n, &sign);
  clock_t end = clock();
  double time_ms = 1000.0 * (end - start) / CLOCKS_PER_SEC;
  printf("n: %d\n", n);
  printf("sign: %d\n", sign);
  printf("ln|det|: %lf\n", log_det);
  if (log_det < 700)
    printf("det: %e\n", sign * exp(log_det));
  printf("time: %lf ms\n", time_ms);
  free_matrix(matrix, n);
  return 0;
}
