#include "util_sys.h"

at_t util_sys_seconds()
{
  return (at_t)time(0);
}

unsigned long long util_sys_ms(long epoch)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  if(epoch > tv.tv_sec) return 0;
  return (unsigned long long)(tv.tv_sec - epoch) * 1000 + (unsigned long long)(tv.tv_usec) / 1000;
}

unsigned short util_sys_short(unsigned short x)
{
  return ntohs(x);
}

unsigned long util_sys_long(unsigned long x)
{
  return ntohl(x);
}

void util_sys_random_init(void)
{
  struct timeval tv;
  unsigned int seed;

  // TODO ifdef for srandomdev when avail
  gettimeofday(&tv, NULL);
  seed = ((unsigned int)getpid() << 16) ^ (unsigned int)tv.tv_sec ^ (unsigned int)tv.tv_usec;
  srand(seed);
}

long util_sys_random(void)
{
  // TODO, use ifdef for arc4random
  return rand();
}

#ifdef DEBUG
static int _logging = 1;
#else
static int _logging = 0;
#endif

void util_sys_logging(int enabled)
{
  if(enabled < 0)
  {
    _logging ^= 1;
  }else{
    _logging = enabled;
  }
  LOG("log output enabled");
}

void *util_sys_log(uint8_t level, const char *file, int line, const char *function, const char * format, ...)
{
  char buffer[256];
  va_list args;
  if(!_logging) return NULL;
  // https://en.wikipedia.org/wiki/Syslog#Severity_level
  char *lstr = NULL;
  switch(level)
  {
    case 0: lstr = "EMERG  "; break;
    case 1: lstr = "ALERT  "; break;
    case 2: lstr = "CRIT   "; break;
    case 3: lstr = "ERROR  "; break;
    case 4: lstr = "WARN   "; break;
    case 5: lstr = "NOTICE "; break;
    case 6: lstr = "INFO   "; break;
    case 7: lstr = "DEBUG  "; break;
    case 8: lstr = "CRAZY  "; break;
    default: lstr = "?????? "; break;
  }
  va_start (args, format);
  vsnprintf (buffer, 256, format, args);
  printf("%s%s:%d %s() %s\n",lstr,file, line, function, buffer);
  va_end (args);
  return NULL;
}
