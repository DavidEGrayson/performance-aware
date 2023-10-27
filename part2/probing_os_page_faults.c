#include "profile.h"
#include <stdio.h>

#define PAGE_COUNT 128
#define PAGE_SIZE 4096

int main()
{
  for (size_t p = 0; p < PAGE_COUNT; p++)
  {
    // Touch p newly-allocated pages and see how many faults we get

    // Note: If we use malloc we don't see the prefetching behavior.
    uint8_t * data = VirtualAlloc(0, PAGE_COUNT * PAGE_SIZE,
      MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

    uint64_t faults_before = get_total_page_faults();

    // Forward touching
    for (size_t i = 0; i < p; i++) { data[i * PAGE_SIZE] = i; }

    // Reverse touching
    //for (size_t i = 0; i < p; i++) { data[(PAGE_COUNT - 1 - i) * PAGE_SIZE] = i; }

    uint64_t faults = get_total_page_faults() - faults_before;
    int64_t extra = faults - p;
    printf("%llu, %llu, %lld\n", p, faults, extra);
  }
}
