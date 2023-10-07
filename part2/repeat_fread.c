#include "profile.h"

int main()
{
  FILE * file = fopen("points.json", "r");

  fseek(file, 0, SEEK_END);
  size_t file_size = ftell(file);
  fseek(file, 0, SEEK_SET);
  char * data = malloc(file_size);
  size_t bytes_read;

  bool do_malloc = false;

  while (true)
  {
    printf("Testing do_malloc=%d\n", do_malloc);
    while (repeat_testing_continue())
    {
      // TODO: don't include this part in the time measurements, just measure fread
      if (do_malloc)
      {
        free(data);
        data = malloc(file_size);
      }
      fseek(file, 0, SEEK_SET);
      uint64_t faults_start = get_total_page_faults();
      bytes_read = fread(data, 1, file_size, file);
      uint64_t faults = get_total_page_faults() - faults_start;
      printf("Faults: %llu\n", faults);
    }
    printf("%4.2f GiB/s\n\n", calculate_gib_per_s(bytes_read, global_rt.best_time));
    do_malloc ^= 1;
  }
}
