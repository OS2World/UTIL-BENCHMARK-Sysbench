
// memspeed test

#include <stdlib.h>
#include <string.h>
#include <os2.h>

#include "types.h"

#define MB (1024*1024)
#define KB (1024)

#define MIN_TIME 10.0 // minimal test time in seconds

extern double dtime(void);
extern void err(char* s);

inline void touchf(long* p1, long n);
inline void touchb(long* p1, long n);
inline long readmem(long* p1, long n);
inline void writemem(long* p1, long n);
inline int memcpy2(long* p1, long* p2, long n);
volatile long bluttanbla;

double
pmb_memspeed(s32 bytes) {
  char* p;
  char* p1;
  char* p2;
  ULONG i,j, runs;
  double t2, t1, tot_time, coerce1, coerce2;
  ULONG mem2;
  double value, tot_copied;
  int notstop = 1;

  mem2 = bytes/2;
  p = (char*)malloc(bytes);
  if (!p) {
    err("memspeed test: can't allocate enough memory to do test");
  }

  /* First, touch all the memory to activate it */
  touchf((long*)p, bytes);
  touchb((long*)(p+bytes), bytes);
  touchf((long*)p, bytes);
  touchb((long*)(p+bytes), bytes);

  runs = 1;
  p1 = p;
  p2 = p+mem2;
  while (notstop) {
    t1 = dtime();
    for (i = 0; i < runs; i++) {
      memcpy(p2, p1, mem2); // copy forwards
      memcpy(p1, p2, mem2); // copy backwards
    }
    t2 = dtime();
    tot_time = (t2-t1);
    if ((tot_time) < MIN_TIME) {
      if ((tot_time) < 0.1) {
        runs *= MIN_TIME*10;
      } else {
        runs = (MIN_TIME*1.2)/(tot_time)*runs;
      }
    } else {
      bluttanbla = p[(rand() * rand()) % bytes]; // fool optimizer
      free(p);
      coerce1 = (double)runs;
      coerce2 = (double)bytes;
      tot_copied = coerce1*coerce2;
      value = (tot_copied/tot_time);
      return value;
    }
  }
  // we won't get here
  err("memspeed: internal error 1");
  return -1.0;
}

double
pmb_memspeedr(s32 bytes) {

  char* p;
  ULONG i,j, runs;
  double t2, t1, tot_time, tot_copied, coerce1, coerce2, value;
  int notstop = 1;

  p = (char*)malloc(bytes);
  if (!p) {
    err("memspeed test: can't allocate enough memory to do test");
  }

  /* First, touch all the memory to activate it */
  touchf((long*)p, bytes);
  touchb((long*)(p+bytes), bytes);

  runs = 1;
  while (notstop) {
    t1 = dtime();
    for (i = 0; i < runs; i++) {
      bluttanbla += readmem((long*)p, bytes);
    }
    t2 = dtime();
    tot_time = (t2-t1);
    if ((tot_time) < MIN_TIME) {
      if ((tot_time) < 0.1) {
        runs *= MIN_TIME*10;
      } else {
        runs = (MIN_TIME*1.2)/(tot_time)*runs;
      }
    } else {
      bluttanbla += p[(rand() * rand()) % bytes]; // fool optimizer
      free(p);
      coerce1 = (double)runs;
      coerce2 = (double)bytes;
      tot_copied = coerce1*coerce2;
      value = tot_copied/tot_time;
      return value;
    }
  }
  // we won't get here
  err("memspeed: internal error 1");
  return -1.0;
}

double
pmb_memspeedw(s32 bytes) {

  char* p;
  ULONG i,j, runs;
  double t2, t1, tot_time, tot_copied, coerce1, coerce2, value;
  int notstop = 1;
  p = (char*)malloc(bytes);
  if (!p) {
    err("memspeed test: can't allocate enough memory to do test");
  }

  /* First, touch all the memory to activate it */
  touchf((long*)p, bytes);
  touchb((long*)(p+bytes), bytes);

  runs = 1;
  while (notstop) {
    t1 = dtime();
    for (i = 0; i < runs; i++) {
      writemem((long*)p, bytes);
    }
    t2 = dtime();
    tot_time = (t2-t1);
    if ((tot_time) < MIN_TIME) {
      if ((tot_time) < 0.1) {
        runs *= MIN_TIME*10;
      } else {
        runs = (MIN_TIME*1.2)/(tot_time)*runs;
      }
    } else {
      bluttanbla = p[(rand() * rand()) % bytes]; // fool optimizer
      free(p);
      coerce1 = (double)runs;
      coerce2 = (double)bytes;
      tot_copied = coerce1*coerce2;
      value = tot_copied/tot_time;
      return value;
    }
  }
  // we won't get here
  err("memspeed: internal error 1");
  return -1.0;
}

// not used:
inline int
memcpy2(long* p1, long* p2, long n) {
  int i;
  n = n >> 6;
  for (i = 0; i < n; i++) {
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;

    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
/*
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;

    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++;
    *p1++ = *p2++; */
  }
return 0;
}

inline void
touchf(long* p1, long n) {
  int i;
  int r;
  n = n/4;
  for (i = 0; i < n; i++) {
    bluttanbla += *p1;
    *p1++ = rand();
  }
}

inline void
touchb(long* p1, long n) {
  int i;
  int r;
  n = n/4;
  for (i = 0; i < n; i++) {
    *(--p1)= rand();
    bluttanbla += *p1;
  }
}

inline void
writemem(long* p1, long n) {
  int i;
  n = n >> 7;
  for (i = 0; i < n; i++) {
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;

    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;

    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;

    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
    *p1++ = 0;
  }

}

inline long
readmem(long* p1, long n) {
  int i;
  long t = 0;
  n = n >> 7;
  for (i = 0; i < n; i++) {
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);

    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);

    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);

    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
    t += (*p1++);
  }
  return t;
}

