#include "profile.h"

int main()
{
  repeat_test_init();
  printf("tsc_frequency: %llu\n", tsc_frequency);

  size_t data_size = (size_t)256 * 1024 * 1024;
  char * data = malloc(data_size);

  while (repeat_test_continue())
  {
    repeat_test_sample_start();
    for (size_t i = 0; i < data_size; i++)
    {
      data[i] = i;
    }
    repeat_test_sample_end();
  }
  printf("Best time: %llu cycles\n", global_rt.best_time);
  printf("Best time: %llu us\n", tsc_to_us(global_rt.best_time));
  printf("Bandwidth: %4.2f GiB/s\n", calculate_gib_per_s(data_size, global_rt.best_time));
  printf("Cycles per byte: %.3lf\n", (double)global_rt.best_time / data_size);
}
