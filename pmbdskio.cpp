/* diskio.c - disk benchmark
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Fri Jul 08 1994
 */

static char *rcsid =
"$Id: diskio.c,v 1.13 1997/03/01 20:36:55 rommel Exp rommel $";
static char *rcsrev = "$Revision: 1.13 $";

/*
 * $Log: diskio.c,v $
 * Revision 1.13  1997/03/01 20:36:55  rommel
 * handle media access errors more gracefully
 *
 * Revision 1.12  1997/03/01 20:21:35  rommel
 * catch a few bugs and obscure situations,
 * increase accuracy, too
 *
 * Revision 1.11  1997/02/09 15:06:38  rommel
 * changed command line interface
 *
 * Revision 1.10  1997/01/12 21:15:10  rommel
 * added CD-ROM benchmarks
 *
 * Revision 1.9  1995/12/31 20:24:09  rommel
 * Changed CPU load calculation
 * General cleanup
 *
 * Revision 1.8  1995/12/28 11:28:07  rommel
 * Fixed async timer problem.
 *
 * Revision 1.7  1995/12/28 10:04:15  rommel
 * Added CPU benchmark (concurrently to disk I/O)
 *
 * Revision 1.6  1995/11/24 16:02:10  rommel
 * Added bus/drive cache speed test by
 * repeatedly reading a small amount of data
 *
 * Revision 1.5  1995/08/09 13:07:02  rommel
 * Changes for new diskacc2 library, minor corrections, arguments.
 *
 * Revision 1.4  1994/07/11 14:23:00  rommel
 * Changed latency timing
 *
 * Revision 1.3  1994/07/09 13:07:20  rommel
 * Changed transfer speed test
 *
 * Revision 1.2  1994/07/08 21:53:05  rommel
 * Cleanup
 *
 * Revision 1.1  1994/07/08 21:29:41  rommel
 * Initial revision
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS
//#define INCL_NOPM
#include <os2.h>

#include "diskacc2.h"
#include "types.h"

#define INTERVAL 10

#define THREADSTACK 65536

char *pBuffer;

int time_over;
long dhry_result, dhry_stones;

extern unsigned long Number_Of_Runs;
extern long dhry_stone(void);

extern ULONG debugging;
extern ULONG debuglvl2;

HEV hSemTimer;


VOID APIENTRY timer_thread(ULONG nArg)
{
  HTIMER hTimer;
  char* routine = "\ntimer_thread:";

  // DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, 0);

  if (debugging)
     {
     printf("%s timer set", routine);
     }

  DosCreateEventSem(0, &hSemTimer, DC_SEM_SHARED, 0);

  DosAsyncTimer(nArg * 1000, (HSEM) hSemTimer, &hTimer);
  DosWaitEventSem(hSemTimer, SEM_INDEFINITE_WAIT);
  DosStopTimer(hTimer);

  DosCloseEventSem(hSemTimer);

  if (debugging)
     {
     printf("%s timer end", routine);
     }

  time_over = 1;
  Number_Of_Runs = 0;

  DosExit(EXIT_THREAD, 0);
}


int start_alarm(ULONG nSeconds)
{
  TID ttid;
  char* routine = "\nstart_alarm:";

  time_over = 0;
  Number_Of_Runs = -1;

  if (DosCreateThread(&ttid, timer_thread, nSeconds, 0, THREADSTACK))
    return printf("%s Cannot create timer thread.", routine), -1;
  if (debugging)
     {
     printf("%s DosCreateThread", routine);
     }

  return 0;
}


void stop_alarm(void)
{
char* routine = "\nstop_alarm:";

  DosPostEventSem(hSemTimer);
  if (debugging)
     {
     printf("%s DosPostEventSem", routine);
     }
}


int start_timer(QWORD *nStart)
{
char* routine = "\nstart_timer:";

  if (DosTmrQueryTime(nStart))
    return printf("%s Timer error.", routine), -1;
  if (debugging)
     {
     printf("%s DosTmrQuery", routine);
     }

  return 0;
}


int stop_timer(QWORD *nStart, int accuracy)
{
  QWORD nStop;
  ULONG nFreq;
  char* routine = "\nstop_timer:";

  if (DosTmrQueryTime(&nStop))
    return printf("%s Timer error.", routine), -1;
  if (DosTmrQueryFreq(&nFreq))
    return printf("%s Timer error.", routine), -1;

  nFreq = (nFreq + accuracy / 2) / accuracy;

  return (nStop.ulLo - nStart->ulLo) / nFreq;
}


void run_dhrystone(void)
{
  long dhry_time = dhry_stone();

  /* originally, time is measured in ms */
  dhry_time = (dhry_time + 5) / 10;
  /* now it is in units of 10 ms */

  if (dhry_time == 0)
    dhry_time = 1; /* if less than 10 ms, then assume at least 10 ms */

  /* now calculate runs per second */
  dhry_stones = Number_Of_Runs * 100 / dhry_time;
  /* by the time we cross 20 million dhrystones per second with a CPU,
     we will hopefully have only 64-bit machines to run this on ... */
}


VOID APIENTRY dhry_thread(ULONG nArg)
{
  DosSetPriority(PRTYS_THREAD, PRTYC_IDLETIME, PRTYD_MAXIMUM, 0);

  run_dhrystone();

  DosExit(EXIT_THREAD, 0);
}


int bench_hd_bus(int nHandle, ULONG nSectors)
{
  ULONG nCnt, nData = 0;
  int   nTime;
  QWORD nLocal;
  char* routine = "\nbench_hd_bus:";

  if (start_alarm(INTERVAL))
    return -1;

  if (start_timer(&nLocal))
    return -1;

  for (nCnt = 0; !time_over; nCnt++)
     {
     if (DskRead(nHandle, 0, 0, 1, nSectors, pBuffer))
        {
        stop_alarm();
        printf("%s Disk read error.", routine);
        return -1;
        }
     }

  if ((nTime = stop_timer(&nLocal, 1024)) == -1)
    return -1;

  //printf("%d k/sec\n", nData / nTime);
  nData = nCnt * nSectors * 512;
  if (debugging)
     {
     printf("%s transfer rate on track 0: %d k/sec (%d sectors/transfer)", routine, nData / nTime, nSectors);
     }

  return (int)(nData / nTime);
}


int bench_hd_transfer(int nHandle,
                     int nTrack,
                     int nDirection,
                     ULONG nSides,
                     ULONG nSectors,
                     ULONG rSectors)
{
  ULONG nCnt, nData = 0, nTime, curhead, curcyl, cursec, startsector, secpercyl;
  ULONG totsectors;
  QWORD nLocal;
  int z;
  char* routine = "\nbench_hd_transfer:";

  if (debugging)
     {
     printf("%s track %d, direction %d, sides %d, sectors %d, #sectors/read %d",
           routine,
           nTrack,
           nDirection,
           nSides,
           rSectors, nSectors);
     }

  if (start_alarm(INTERVAL))
     return -1;

  if (start_timer(&nLocal))
     return -1;

  startsector = nTrack * nSides * rSectors;
  if (nDirection == -1)
     {
     startsector -= nSectors;
     }
  secpercyl   = rSectors * nSides;

  for (nCnt = 0; !time_over; nCnt++)
     {
     totsectors = nCnt * nSectors;    /* total sectors read so far */
     totsectors = startsector + (totsectors * nDirection);
     curcyl     = totsectors / secpercyl;
     curhead    = (totsectors - (curcyl * secpercyl)) / rSectors;
     if (nDirection == -1)
        {
        curhead = nSides - curhead - 1;
        }
     cursec     = ((totsectors - (curcyl * secpercyl)) % rSectors) + 1;

     if (debugging)
        {
        if (debuglvl2)
           {
           printf("%s Read % 4u cyl % 4u head % 4u sector % 4u", routine, nCnt, curcyl, curhead, cursec);
           }
        }

     if (DskRead(nHandle,
                curhead,
                curcyl,
                cursec,
                nSectors,
                pBuffer))
        {
        stop_alarm();
        printf("%s Disk read error.", routine);
        return -1;
        }
     }

  if ((nTime = stop_timer(&nLocal, 1024)) == -1)
     return -1;

  nData = nCnt * nSectors * 512;
  if (debugging)
     {
     printf("%s Data transfer rate on track %-4d: %d k/sec", routine, nTrack, nData / nTime);
     }

  if (nTime > 0)
     {
     return (nData / nTime);
     }
  else
     {
     return -1;
     }
}


int bench_hd_cpuusage(int nHandle, ULONG nSides, ULONG nSectors)
{
  int nCnt, nData = 0, nTime, nPercent;
  QWORD nLocal;
  TID dtid;
  APIRET rc;
  char* routine = "\nbench_hd_cpuusage:";

  if (start_alarm(INTERVAL))
     return -1;

  if (DosCreateThread(&dtid, dhry_thread, 0, 0, THREADSTACK))
     return -1;

  if (start_timer(&nLocal))
     return -1;

  for (nCnt = 0; !time_over; nCnt++)
     {
     if (DskRead(nHandle, nCnt % nSides, nCnt / nSides, 1, nSectors, pBuffer))
        {
        stop_alarm();
        printf("%s Disk read error.", routine);
        return -1;
        }
     }

  if ((nTime = stop_timer(&nLocal, 1024)) == -1)
     return -1;

  if ((rc = DosWaitThread(&dtid, DCWW_WAIT)) && rc != ERROR_INVALID_THREADID)
     return -1;                             /* it may have already terminated */

  nPercent = (dhry_result - dhry_stones) * 100 / dhry_result;
  nData = nCnt * nSectors * 512;

  return nPercent;
}


int bench_hd_latency(int nHandle, ULONG nSectors)
{
  int nCnt, nSector, nTime;
  QWORD nLocal;
  char* routine = "\nbench_hd_latency:";

  srand(1);

  if (start_alarm(INTERVAL))
     return -1;

  if (start_timer(&nLocal))
     return -1;

  for (nCnt = 0; !time_over; nCnt++)
     {
     nSector = rand() * nSectors / RAND_MAX + 1;

     if (DskRead(nHandle, 0, 0, nSector, 1, pBuffer))
        {
        stop_alarm();
        printf("%s Disk read error.", routine);
        return -1;
        }
     }

  if ((nTime = stop_timer(&nLocal, 1000)) == -1)
     return -1;

  nTime = nTime * 10 / nCnt;

  return nTime;
}


int bench_hd_seek(int nHandle, ULONG nSides, ULONG nSectors, ULONG nTracks)
{
  int nCnt, nSide, nTrack, nSector, nTime;
  QWORD nLocal;
  char* routine = "\nbench_hd_seek:";

  srand(1);

  printf("%s Disk geometry passed %u:%u:%u.", routine, nSides, nTracks, nSectors);

  if (start_alarm(INTERVAL))
     return -1;

  if (start_timer(&nLocal))
     return -1;

  for (nCnt = 0; !time_over; nCnt++)
     {
     nSide   = rand() * (nSides - 1)  / RAND_MAX;
     nSector = rand() * nSectors / RAND_MAX;
     nTrack  = rand() * nTracks  / RAND_MAX;

     if (DskRead(nHandle, nSide, nTrack, nSector, 1, pBuffer))
        {
        stop_alarm();
        printf("%s Disk read error %u:%u:%u.", routine, nSide, nTrack, nSector);
        return -1;
        }
     }

  if ((nTime = stop_timer(&nLocal, 1000)) == -1)
     return -1;

  nTime = nTime * 10 / nCnt;

  if (debugging)
     {
     printf("%s Average data access time: %d.%d ms", routine, nTime/10, nTime%10);
     }

  return nTime;
}


int bench_hd(int nDisk)
{
  int nHandle;
  ULONG nSides, nTracks, nSectors;
  char szName[8];
  char* routine = "\nbench_hd:";

  sprintf(szName, "$%d:", nDisk);

  if ((nHandle = DskOpen(szName, 0, 0, &nSides, &nTracks, &nSectors)) < 0)
    return printf("%s Cannot access disk %d.\n", routine, nDisk), -1;

  printf("%s Hard disk %d: %d sides, %d cylinders, %d sectors per track = %d MB",
        routine,
        nDisk,
        nSides,
        nTracks,
        nSectors,
        nSides * nTracks * nSectors / 2048);

  if ((pBuffer = (char*)malloc(nSectors * 512)) == NULL)
    return printf("%s Not enough memory.", routine), -1;

  bench_hd_bus(nHandle, nSectors);
  bench_hd_transfer(nHandle, 0, 1, nSides, nSectors, nSectors);
  bench_hd_transfer(nHandle, nTracks - 2, -1, nSides, nSectors, nSectors);
  bench_hd_cpuusage(nHandle, nSides, nSectors);
  bench_hd_latency(nHandle, nSectors);
  bench_hd_seek(nHandle, nSides, nSectors, nTracks);

  free(pBuffer);
  DskClose(nHandle);

  return 0;
}

int bench_cd_transfer(int nHandle)
{
  int nCnt, nData = 0, nTime, nRate;
  QWORD nLocal;
  ULONG rc;
  char* routine = "\nbench_cd_transfer:";

  if (start_alarm(INTERVAL))
    return -1;

  if (start_timer(&nLocal))
    return -1;

  for (nCnt = 0; !time_over; nCnt++)
     {
     rc = CDRead(nHandle, nCnt * 32, 32, pBuffer);
     if (rc == -1)
        {
        stop_alarm();
        printf("%s CD-ROM read error, rc = %x.", routine, rc);
        return -1;
        }
     }

  if ((nTime = stop_timer(&nLocal, 1024)) == -1)
     return -1;

  nData = nCnt * 32 * 2048;

  nRate = nData / nTime;

  return nRate;
}


int bench_cd_inner(int nHandle, ULONG nSectors)
{
  int nCnt, nData = 0, nTime, nRate, nOffset;
  QWORD nLocal;
  ULONG rc;
  char* routine = "\nbench_cd_inner:";

  if (start_alarm(INTERVAL))
     return -1;

  if (start_timer(&nLocal))
     return -1;

  for (nCnt = 0; !time_over; nCnt++)
     {
     nOffset = nCnt % (nSectors/32);  /* wrap if we hit the end of the CD */
     rc = CDRead(nHandle, nOffset * 32, 32, pBuffer);
     if (rc == -1)
        {
        stop_alarm();
        printf("%s CD-ROM read error, rc = %x, sector = %d.", routine, rc, nOffset);
        return -1;
        }
     }

  if ((nTime = stop_timer(&nLocal, 1024)) == -1)
     return -1;

  nData = nCnt * 32 * 2048;

  nRate = nData / nTime;

  return nRate;
}


int bench_cd_outer(int nHandle, ULONG nSectors)
{
  int nCnt, nData = 0, nTime, nRate, nOffset, nRead = 64;
  QWORD nLocal;
  ULONG rc;
  char* routine = "\nbench_cd_outer:";

  if (start_alarm(INTERVAL))
     return -1;

  if (start_timer(&nLocal))
     return -1;

  for (nCnt = 0; !time_over; nCnt++)
     {
     nOffset = (nCnt % nRead);
     rc = CDRead(nHandle, (nSectors-1-(nRead*32)) + (nOffset * 32), 32, pBuffer);
     if (rc == -1)
        {
        stop_alarm();
        printf("%s CD-ROM read error, rc = %x.", routine, rc);
        return -1;
        }
     }

  if ((nTime = stop_timer(&nLocal, 1024)) == -1)
     return -1;

  nData = nCnt * 32 * 2048;

  nRate = nData / nTime;

  return nRate;
}


int bench_cd_cpuusage(int nHandle)
{
  int nCnt, nData = 0, nTime, nPercent;
  QWORD nLocal;
  TID dtid;
  APIRET rc;
  char* routine = "\nbench_cd_cpuusage:";

  if (start_alarm(INTERVAL))
    return -1;

  if (DosCreateThread(&dtid, dhry_thread, 0, 0, THREADSTACK))
    return -1;

  if (start_timer(&nLocal))
    return -1;

  for (nCnt = 0; !time_over; nCnt++)
     {
     rc = CDRead(nHandle, nCnt * 32, 32, pBuffer);
     if (rc == -1)
        {
        stop_alarm();
        printf("%s CD-ROM read error, rc = %x.", routine, rc);
        return -1;
        }
     }

  if ((nTime = stop_timer(&nLocal, 1024)) == -1)
    return -1;

  if ((rc = DosWaitThread(&dtid, DCWW_WAIT)) && rc != ERROR_INVALID_THREADID)
    return -1;                             /* it may have already terminated */

  nPercent = (dhry_result - dhry_stones) * 100 / dhry_result;
  nData = nCnt * 32 * 2048;

  return nPercent;
}


int bench_cd_seek(int nHandle, ULONG nSectors)
{
  int nCnt, nSector, nTime;
  QWORD nLocal;
  ULONG rc;
  char* routine = "\nbench_cd_seek:";

  srand(1);

  if (start_alarm(INTERVAL))
    return -1;

  if (start_timer(&nLocal))
    return -1;

  for (nCnt = 0; !time_over; nCnt++)
   {
    nSector = (nSectors * 1000) / RAND_MAX;
    nSector = nSector * rand() / 1000;
    if (nSector >= nSectors)
       {
       nSector = nSectors-1;
       }

    rc = CDRead(nHandle, nSector, 1, pBuffer);
    if (rc == -1)
      {
      stop_alarm();
      printf("%s CD-ROM read error, rc = %x, sector = %d.", routine, rc, nSector);
      return -1;
      }
   }

  if ((nTime = stop_timer(&nLocal, 1000)) == -1)
    return -1;

  nTime = nTime * 10 / nCnt;

  return nTime;
}


int bench_cd(int nDrive)
{
  int nHandle, nDriveLetter;
  ULONG nSectors;
  char szDrive[3], szUPC[8];
  char* routine = "\nbench_cd:";

  if ((nDriveLetter = CDFind(nDrive)) == -1)
    return printf("%s Cannot access CD-ROM drive %d.", routine, nDrive), -1;

  szDrive[0] = (char) nDriveLetter;
  szDrive[1] = ':';
  szDrive[2] = 0;

  if ((nHandle = CDOpen(szDrive, 1, szUPC, &nSectors)) == -1)
    return -1;

  printf("%s CD-ROM drive %s %d sectors = %d MB",
        routine,
        szDrive,
        nSectors,
        nSectors / 512);

  if ((pBuffer = (char*)malloc(32 * 2048)) == NULL)
    return printf("%s Not enough memory.", routine), -1;

  /* spin up and seek to first sector */
  if (CDRead(nHandle, 0, 32, pBuffer) == -1)
    return printf("%s CD-ROM read error.", routine), -1;

  bench_cd_transfer(nHandle);
  bench_cd_cpuusage(nHandle);
  bench_cd_seek(nHandle, nSectors);

  free(pBuffer);
  CDClose(nHandle);

  return 0;
}


int bench_dhry(void)
{
char* routine = "\nbench_dhry:";

  printf("%s Dhrystone benchmark for this CPU: ", routine);
  fflush(stdout);

  if (start_alarm(INTERVAL / 2))
    return -1;

  run_dhrystone();

  dhry_result = dhry_stones;

  if (dhry_result == 0)
    dhry_result = 1; /* to avoid dividing by zero later on */

  printf("%d runs/sec\n", dhry_result);

  return 0;
}


