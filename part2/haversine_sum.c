#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "json.h"

static double square(double x)
{
  return x * x;
}

static double degrees_to_radians(double degrees)
{
  return 0.01745329251994329577 * degrees;
}

const double earth_radius = 6372.8;

static double haversine_distance(double x0, double y0, double x1, double y1)
{
  double lat1 = y0;
  double lat2 = y1;
  double lon1 = x0;
  double lon2 = x1;

  double d_lat = degrees_to_radians(lat2 - lat1);
  double d_lon = degrees_to_radians(lon2 - lon1);
  lat1 = degrees_to_radians(lat1);
  lat2 = degrees_to_radians(lat2);

  double a = square(sin(d_lat / 2.0)) + \
    cos(lat1) * cos(lat2) * square(sin(d_lon / 2));
  double c = 2.0*asin(sqrt(a));

  return earth_radius * c;
}

int main()
{
  FILE * file = fopen("points.json", "r");
  Json * data = json_parse_file(file);
  Json * pairs = json_object_lookup(data, "pairs");
  if (pairs == NULL)
  {
    fprintf(stderr, "Error: Cannot find 'pairs' in file.\n");
    return 1;
  }
  if (pairs->type != JsonArray)
  {
    fprintf(stderr, "Error: 'pairs' is not an array.\n");
    return 1;
  }
  double sum = 0;
  size_t count = 0;
  for (Json * pair = pairs->first; pair; pair = pair->next)
  {
    double x0 = json_object_lookup(pair, "x0")->number;
    double y0 = json_object_lookup(pair, "y0")->number;
    double x1 = json_object_lookup(pair, "x1")->number;
    double y1 = json_object_lookup(pair, "y1")->number;
    printf("%20.15lf %20.15lf %20.15lf %20.15lf\n", x0, y0, x1, y1);
    sum += haversine_distance(x0, y0, x1, y1);
    count += 1;
  }
  double average = sum / count;
  printf("average: %20.15lf\n", average);
}
