#include "profile.h"

int main()
{
  FILE * file = fopen("points.json", "r");

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  char * data = malloc(file_size);
  size_t bytes_read;

  while (repeat_testing_continue())
  {
    fseek(file, 0, SEEK_SET);
    bytes_read = fread(data, 1, file_size, file);
  }

  printf("%4.2f GiB/s\n", calculate_gib_per_s(bytes_read, global_rt.best_time));
}
