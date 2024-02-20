// Results on David's computer:
// - There seem to be 2 read ports.
// - There seem to be 2 write ports, but only write_loop2 can use them, not
//   write_loop3 or write_loop4!

#include "profile.h"

void read_loop1(size_t data_length, void * data);
void read_loop2(size_t data_length, void * data);
void read_loop3(size_t data_length, void * data);
void read_loop4(size_t data_length, void * data);
void write_loop1(size_t data_length, void * data);
void write_loop2(size_t data_length, void * data);
void write_loop3(size_t data_length, void * data);
void write_loop4(size_t data_length, void * data);

static inline void perform_repeat_test(void (*func)(size_t, void *),
  size_t data_length, void * data)
{
  repeat_test_init();
  while (repeat_test_continue())
  {
    repeat_test_sample_start();
    func(data_length, data);
    repeat_test_sample_end();
  }
  printf("Best time: %llu cycles\n", global_rt.best_time);
  printf("Best time: %llu us\n", tsc_to_us(global_rt.best_time));
  printf("Bandwidth: %4.2f GiB/s\n", calculate_gib_per_s(data_length, global_rt.best_time));
  printf("Cycles per op: %.3lf\n", (double)global_rt.best_time / data_length);
  printf("\n");
}

int main()
{
  repeat_test_init();
  printf("tsc_frequency: %llu\n", tsc_frequency);

  size_t data_size = (size_t)256 * 1024 * 1024;
  char * data = malloc(data_size);

  printf("== write_loop1 ==\n");
  perform_repeat_test(read_loop1, data_size, data);

  printf("== write_loop2 ==\n");
  perform_repeat_test(read_loop2, data_size, data);

  printf("== write_loop3 ==\n");
  perform_repeat_test(write_loop3, data_size, data);

  printf("== write_loop4 ==\n");
  perform_repeat_test(write_loop4, data_size, data);

  printf("== read_loop1 ==\n");
  perform_repeat_test(read_loop1, data_size, data);

  printf("== read_loop2 ==\n");
  perform_repeat_test(read_loop2, data_size, data);

  printf("== read_loop3 ==\n");
  perform_repeat_test(read_loop3, data_size, data);

  printf("== read_loop4 ==\n");
  perform_repeat_test(read_loop4, data_size, data);
}
