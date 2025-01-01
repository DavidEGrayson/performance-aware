#include <stdio.h>
#include <math.h>

#define PI M_PI

double our_sin(double x)
{
  return sin(x);  // TODO
}

double our_cos(double x)
{
  return cos(x);  // TODO
}

double our_asin(double x)
{
  return asin(x);  // TODO
}

double our_sqrt(double x)
{
  return sqrt(x);  // TODO
}

typedef struct ReferenceValue {
  double input;  // NAN indicates end of array
  double output;
} ReferenceValue;

typedef struct FunctionTestingParams {
  char * name;  // NULL indicates end of array
  double range_min;
  double range_max;
  double (*our_func)(double);
  double (*library_func)(double);
  ReferenceValue * reference_values;
} FunctionTestingParams;

// These are from Wolfram Alpha.
ReferenceValue
  reference_sin_values[] = {
    { 0.12001186534234830549, 0.11972398729484292 },
    { 1.99118979704618944, 0.91292842802683179 },
    { NAN },
  },
  reference_cos_values[] = {
    { 1.12001186534234830549, 0.43567176624614906 },
    { NAN },
  },
  reference_asin_values[] = {
    { 0.12001186534234830549, 0.12030183411009670 },
    { NAN },
  },
  reference_sqrt_values[] = {
    { 0.12001186534234830549, 0.34642728723694429 },
    { NAN },
  }
;

FunctionTestingParams params_list[] = {
  { "sin", -PI, PI, our_sin, sin, reference_sin_values },
  { "cos", -PI/2, PI/2, our_cos, cos, reference_cos_values },
  { "asin", 0, 1, our_asin, asin, reference_asin_values },
  { "sqrt", 0, 1, our_sqrt, sqrt, reference_sqrt_values },
  { NULL },
};

int main()
{
  size_t error_count = 0;
  FunctionTestingParams * params = params_list;
  while (params->name)
  {
    printf("Checking reference values for %s...\n", params->name);
    ReferenceValue * rv = params->reference_values;
    while (!isnan(rv->input))
    {
      double actual = params->our_func(rv->input);
      if (actual != rv->output)
      {
        printf("%s(%.17f): expected %.17f, got %0.17f, diff %0.17f\n",
          params->name, rv->input, rv->output, actual, actual - rv->output);
        error_count++;
      }
      rv++;
    }
    params++;
  }
  return error_count != 0;
}
