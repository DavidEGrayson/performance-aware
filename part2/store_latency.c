// Investigation of https://stackoverflow.com/questions/78251852
// This code is released into the public domain.

//// Reusable profiling utility ////////////////////////////////////////////////
// The basic ideas it to use __rdtsc() to get a high-resolution timer and
// measure how long code takes to run.  We run the code many times and record
// the best (shorted) time it took to run, so we can find out what our computer
// is capable of doing when everything is cached/scheduled optimally.
// Hat tip to Casey Muratori: https://www.computerenhance.com/p/repetition-testing

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <windows.h>
#include <psapi.h>

static uint64_t tsc_frequency;
static float tsc_units_in_us;

static void measure_tsc_frequency()
{
  const unsigned int k = 10;  // We measure for 1/k seconds
  LARGE_INTEGER li;
  QueryPerformanceFrequency(&li);
  uint64_t qp_freq = li.QuadPart;
  QueryPerformanceCounter(&li);
  uint64_t start = li.QuadPart;
  uint64_t tsc_start = __rdtsc();
  while (true)
  {
    QueryPerformanceCounter(&li);
    if (li.QuadPart - start > qp_freq / k) { break; }
  }
  tsc_frequency = (__rdtsc() - tsc_start) * k;
  tsc_units_in_us = 1e6 / tsc_frequency;
}

static uint64_t tsc_to_us(uint64_t tsc)
{
  return tsc_units_in_us * tsc;
}

// Calculate bandwidth in gigabytes per second.
// gigabytes / second = byte_count/(1<<30) / (time_in_us / 1000000)
// We reduce both constants by a factor of 64 to help avoid overflow.
static double calculate_gib_per_s(uint64_t byte_count, uint64_t tsc)
{
  return (double)byte_count * 15625 / ((1 << 24) * tsc_units_in_us * tsc);
}

static uint64_t get_total_page_faults()
{
  static HANDLE metrics_handle = INVALID_HANDLE_VALUE;
  if (metrics_handle == INVALID_HANDLE_VALUE)
  {
    metrics_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ,
      false, GetCurrentProcessId());
  }

  PROCESS_MEMORY_COUNTERS_EX mc = { .cb = sizeof(mc) };
  GetProcessMemoryInfo(metrics_handle, (void *)&mc, sizeof(mc));
  return mc.PageFaultCount;
}

typedef struct RepeatTest
{
  bool timing;
  uint64_t best_time;
  uint64_t best_time_tsc;
  uint64_t start_tsc;
  size_t test_count;
  uint64_t times[4];
  uint64_t start_page_faults;
  uint64_t page_faults[4];
} RepeatTest;

RepeatTest global_rt;

static void repeat_test_init()
{
  RepeatTest * rt = &global_rt;
  if (tsc_frequency == 0) { measure_tsc_frequency(); }
  *rt = (RepeatTest) {
    .best_time = ~(uint64_t)0,
    .best_time_tsc = __rdtsc(),
  };
}

// Returns true if less than 3 seconds have passed since the best time we
// recorded.
static bool repeat_test_continue()
{
  RepeatTest * rt = &global_rt;
  return tsc_to_us(__rdtsc() - rt->best_time_tsc) < 3000000;
}

static void repeat_test_sample_start()
{
  RepeatTest * rt = &global_rt;
  rt->start_tsc = __rdtsc();
  if (rt->test_count < 4) { rt->start_page_faults = get_total_page_faults(); }
}

static void repeat_test_sample_end()
{
  RepeatTest * rt = &global_rt;

  uint64_t stop_tsc = __rdtsc();
  uint64_t time = stop_tsc - rt->start_tsc;
  if (time < rt->best_time)
  {
    rt->best_time = time;
    rt->best_time_tsc = stop_tsc;
    //printf("Maybe best time: %llu us\n", tsc_to_us(rt->best_time));
  }
  if (rt->test_count < 4)
  {
    rt->times[rt->test_count] = time;
    rt->page_faults[rt->test_count] = get_total_page_faults() - rt->start_page_faults;
  }
  rt->test_count++;
}


//// Main code /////////////////////////////////////////////////////////////////

#define SIZE 10000

typedef struct Struct {
    int32_t a, b, c;
} Struct;

void __attribute__((noinline)) do_work(const Struct * data,
  int32_t * a, int32_t * b, int32_t * c)
{
  int32_t * a_ptr = a, * b_ptr = b, * c_ptr = c;
  for (size_t i = 0; i < SIZE; i++, a_ptr++, b_ptr++, c_ptr++, data++)
  {
    *a_ptr = data->a;
    *b_ptr = data->b;
    *c_ptr = data->c;
  }
}

static inline void perform_repeat_test()
{
  const size_t data_length = SIZE * sizeof(Struct);
  Struct * data = malloc(data_length);
  int32_t * a = malloc(SIZE * sizeof(int32_t));
  int32_t * b = malloc(SIZE * sizeof(int32_t));
  int32_t * c = malloc(SIZE * sizeof(int32_t));

  repeat_test_init();
  while (repeat_test_continue())
  {
    repeat_test_sample_start();
    do_work(data, a, b, c);
    repeat_test_sample_end();
  }

  for (int i = 0; i < 4; i++)
  {
    printf("Iteration %d: %llu cycles, %llu page faults\n",
      i, global_rt.times[i], global_rt.page_faults[i]);
  }
  printf("Best time: %llu\n", global_rt.best_time);
  printf("Best time: %llu us\n", tsc_to_us(global_rt.best_time));
  double bandwidth = calculate_gib_per_s(SIZE * sizeof(Struct), global_rt.best_time);
  printf("Bandwidth: %4.2f GiB/s\n", bandwidth);
  printf("Bytes per cycle: %.1lf\n", data_length / (double)global_rt.best_time);
  printf("\n");
}

int main()
{
  repeat_test_init();
  printf("tsc_frequency: %llu\n", tsc_frequency);

  perform_repeat_test();
}
