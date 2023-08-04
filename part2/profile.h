uint64_t tsc_frequency;
float tsc_units_in_us;

void measure_tsc_frequency()
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
    if (li.QuadPart - start > qp_freq / k)
    {
      break;
    }
  }
  tsc_frequency = (__rdtsc() - tsc_start) * k;
  tsc_units_in_us = 1e6 / tsc_frequency;
}

uint64_t tsc_to_us(uint64_t tsc)
{
  return tsc_units_in_us * tsc;
}

#ifdef PROFILE

// Represents a region in the code we want to profile.
typedef struct ProfileBlock
{
  const char * name;
  // Total time this block was on the stack.
  uint64_t total_time;

  // Total time this block was at the top of the stack.
  uint64_t exclusive_time;

  // Number of frames for this block currently on the stack.
  size_t frame_count;
} ProfileBlock;

typedef struct ProfileFrame
{
  ProfileBlock * block;
  uint64_t start_tsc;
  uint64_t child_time;
} ProfileFrame;

#define PROFILE_BLOCK_CAPACITY 64

#endif

typedef struct Profile
{
  uint64_t start_tsc;
  uint64_t end_tsc;
#ifdef PROFILE
  ProfileBlock blocks[PROFILE_BLOCK_CAPACITY];
  ProfileFrame frames[64];
  size_t block_count;
  size_t frame_count;
#endif
} Profile;

Profile global_profile;

void profile_init()
{
  Profile * profile = &global_profile;
  memset(profile, 0, sizeof(*profile));
  profile->start_tsc = __rdtsc();
}

#ifdef PROFILE
size_t profile_next_block_index()
{
  Profile * profile = &global_profile;
  assert(profile->block_count < PROFILE_BLOCK_CAPACITY);
  return profile->block_count++;
}

// Note: The string pointed to by 'name' should stay in scope
// as long as the profile object is used.
void profile_block_start(const char * name, size_t block_index)
{
  Profile * profile = &global_profile;
  assert(block_index < PROFILE_BLOCK_CAPACITY);
  ProfileBlock * block = &profile->blocks[block_index];
  block->name = name;
  profile->frames[profile->frame_count++] = (struct ProfileFrame){
    .start_tsc = __rdtsc(),
    .block = block,
  };
  block->frame_count++;
}

// This defintion would usually work, but it wouldn't work if there are multiple
// compilation units or if some other part of the code runs __COUNTER__
// hundreds of times.
// #define profile_block(name) profile_block_start(name, __COUNTER__)

#define profile_block(name) profile_block_start(name, \
  ({ static int i = -1; if (i == -1) i = profile_next_block_index(); i; }))

void profile_block_done()
{
  uint64_t now_tsc = __rdtsc();
  Profile * profile = &global_profile;
  assert(profile->frame_count);
  ProfileFrame * frame = &profile->frames[--profile->frame_count];

  uint64_t total_time = now_tsc - frame->start_tsc;
  uint64_t exclusive_time = total_time - frame->child_time;

  // Add this frame's run time to the child time of its parent frame.
  if (profile->frame_count)
  {
    profile->frames[profile->frame_count - 1].child_time += total_time;
  }

  frame->block->exclusive_time += exclusive_time;
  frame->block->frame_count--;

  // Update the block's total time, unless it still has another frame on the
  // stack.
  if (frame->block->frame_count == 0)
  {
    frame->block->total_time += total_time;
  }
}
#else
#define profile_block(name)
#define profile_block_done()
#endif

void profile_end()
{
  Profile * profile = &global_profile;
  profile->end_tsc = __rdtsc();
}

void profile_print()
{
  Profile * profile = &global_profile;

  if (!profile->end_tsc) { profile_end(); }

  if (tsc_frequency == 0) { measure_tsc_frequency(); }

  uint64_t total_time = profile->end_tsc - profile->start_tsc;
  printf("Total run time:                    %10llu us\n", tsc_to_us(total_time));
#if PROFILE
  assert(profile->frame_count == 0);
  for (size_t i = 0; i < PROFILE_BLOCK_CAPACITY; i++)
  {
    ProfileBlock * block = &profile->blocks[i];
    assert(block->frame_count == 0);
    if (block->name == NULL) { continue; }
    float percent = 100.0 * block->exclusive_time / total_time;
    printf("  %-18s %10llu us %10llu us (%.1f%%)\n", block->name,
      tsc_to_us(block->total_time),
      tsc_to_us(block->exclusive_time), percent);
  }
#endif
}
