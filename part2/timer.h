uint64_t tsc_frequency;

void timer_init()
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
}

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

typedef struct Profile
{
  ProfileBlock blocks[PROFILE_BLOCK_CAPACITY];
  ProfileFrame frames[64];
  size_t block_count;
  size_t frame_count;
} Profile;

Profile global_profile;

void profile_init()
{
  if (tsc_frequency == 0) { timer_init(); }
  Profile * profile = &global_profile;
  memset(profile, 0, sizeof(*profile));
}

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

void profile_print()
{
  Profile * profile = &global_profile;
  assert(profile->frame_count == 0);
  uint64_t total_time = profile->frames[0].block->total_time;
  for (size_t i = 0; i < PROFILE_BLOCK_CAPACITY; i++)
  {
    ProfileBlock * block = &profile->blocks[i];
    assert(block->frame_count == 0);
    if (block->name == NULL) { continue; }
    float percent = 100.0 * block->exclusive_time / total_time;
    printf("  %-18s %10llu %10llu (%.1f%%)\n", block->name,
      block->total_time, block->exclusive_time, percent);
  }
}
