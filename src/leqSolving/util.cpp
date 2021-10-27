#include <sys/stat.h>

#include "leqSolving.hpp"

// memory
double getPeakMemoryUsage() {
#ifdef __linux__
  FILE* fmem = fopen("/proc/self/status", "r");
  char membuf[128];
  double vmPeak = 0;
  while (fgets(membuf, 128, fmem)) {
    char* ch;
    if ((ch = strstr(membuf, "VmPeak:"))) {
      vmPeak = atol(ch + 7);
      continue;
    }
  }
  fclose(fmem);
  return vmPeak;
#else
  return -1;
#endif
}

double getCurrMemoryUsage() {
#ifdef __linux__
  FILE* fmem = fopen("/proc/self/status", "r");
  char membuf[128];
  double vmSize = 0;
  while (fgets(membuf, 128, fmem)) {
    char* ch;
    if ((ch = strstr(membuf, "VmSize:"))) {
      vmSize = atol(ch + 7);
      break;
    }
  }
  fclose(fmem);
  return vmSize;
#else
  return -1;
#endif
}

void showMemoryUsage(bool flag) {
  constexpr double    MEMORY_SCALE = 1024.0;
  if (flag)
    fprintf(stderr, "[Info] Peak Memory Usage: %f MB\n\n", getPeakMemoryUsage() / MEMORY_SCALE);
  else
    fprintf(stderr, "[Info] Curr Memory Usage: %f MB\n\n", getCurrMemoryUsage() / MEMORY_SCALE);
}
