#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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
  json_parse_file(file);
}
