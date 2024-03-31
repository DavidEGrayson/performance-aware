// Results on David's 2.9 GHz computer:
//
// Small cache: 32 KiB,   121 GiB/s  (slightly more than 2 reads per cycle, what we expect)
// Medium cache: 256 KB,  59 GiB/s
// Big cache: 4MB or 8MB, 50 GiB/s
// Memory:                18 GiB/s


#include "profile.h"

const size_t data_size = 1 << 30;

void read_loop(size_t data_length, void * data, size_t mask);

void foo()
{
    read_loop(1, 0, 2);
}

static inline void perform_repeat_test(
  size_t data_length, void * data, size_t use_length)
{
  repeat_test_init();
  while (repeat_test_continue())
  {
    repeat_test_sample_start();
    read_loop(data_length, data, use_length - 1);
    repeat_test_sample_end();
  }
  double bandwidth = calculate_gib_per_s(data_length, global_rt.best_time);

  // printf("Use: %zu\n", use_length);
  // printf("Best time: %llu cycles\n", global_rt.best_time);
  // printf("Best time: %llu us\n", tsc_to_us(global_rt.best_time));
  // printf("Bandwidth: %4.2f GiB/s\n", bandwidth);
  // printf("Cycles per op: %.3lf\n", (double)global_rt.best_time / data_length);
  // printf("\n");

  printf("%zu,%4.2f\n", use_length, bandwidth);
}

int main()
{
  repeat_test_init();
  printf("tsc_frequency: %llu\n", tsc_frequency);

  // TODO: use VirtualAlloc so it's aligned nicely
  void * data = malloc(data_size);

  size_t use_length = 1 << 30;
  while (use_length >= 64)
  {
    perform_repeat_test(data_size, data, use_length);
    use_length >>= 1;
  }
}
