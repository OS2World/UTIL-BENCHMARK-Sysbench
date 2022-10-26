/* pmbdskfe.c - Sysbench front end to Kai Uwe Rommel's Diskio code
 *
 * Author:  Trevor Hemsley <Trevor-Hemsley@dial.pipex.com?
 * Created: Mon Apr 14 1997
 */

#define INCL_DOS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSERRORS

#define LL2F(high, low) (4294967296.0*(high)+(low))
#define NUMCPUS 16
#define KB      1024
#define PI      3.1415926
#define SECTORS_ON_FULL_CD (333000.0-150.0)
#define SECTORS_PER_TRANSFER 128   /* number of sectors to transfer per HD i/o (rounded down later) */

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "perfutil.h"
#include "pmbdatat.h"

/* prototype definitions for functions */
double pmb_diskio_avseek(int disknr);
double pmb_buscache_xfer(int disknr);
double pmb_diskio_inner(int disknr);
double pmb_diskio_outer(int disknr);
double pmb_diskio_transfer0(int disknr);
double pmb_diskio_transfern2(int disknr);
double pmb_diskio_transfern(int disknr);
double pmb_diskio_cpupct(int disknr);
double pmb_diskio_disksize(int nDisk);
int    pmb_diskio_nrdisks(void);

int    pmb_cdio_nrcds(void);
double pmb_cdio_disksize(int nDrive);
double pmb_cdio_avseek(int nDrive);
double pmb_cdio_transfer(int nDrive);
double pmb_cdio_cpupct(int nDrive);

int nHandle;
ULONG nSides, nTracks, nSectors;
ULONG amount;
BOOL doneDhry = 0;
double ir = 0;

extern char *pBuffer;

extern int time_over;
extern double dhry_time, dhry_result;

extern int    bench_hd_seek(int nHandle, ULONG nSides, ULONG nSectors, ULONG nTracks);
extern int    bench_hd_bus(int nHandle, ULONG nSectors);
extern int    bench_hd_transfer(int nHandle, int nTrack, int nDirection, ULONG nSides, ULONG nSectors, ULONG rSectors);
extern int    bench_hd_cpuusage(int nHandle, ULONG nSides, ULONG nSectors);
extern int    bench_cd_inner(int nHandle, ULONG nSectors);
extern int    bench_cd_outer(int nHandle, ULONG nSectors);
extern int    bench_cd_transfer(int nHandle);
extern int    bench_cd_cpuusage(int nHandle);
extern int    bench_cd_seek(int nHandle, ULONG nSectors);

extern int    bench_dhry(void);
//extern double bench_concurrent(void);

extern void   err(char* s);

extern ULONG  numcpus;
extern struct glob_data data;
extern volatile ULONG  PerfSysSup;
extern char invocationpath[CCHMAXPATH];
extern ULONG debugging;
double GetCPUsage(void);

#include "diskacc2.h"


double pmb_diskio_avseek(int disknr)
{
  double r;
  char szName[8];
  char* routine = "\npmb_diskio_avseek:";

  disknr++;
  sprintf(szName, "$%d:", disknr);

  if ((nHandle = DskOpen(szName, 0, 0, &nSides, &nTracks, &nSectors)) < 0)
     return printf("%s Cannot access disk %d.", routine, disknr), -1;

  if ((pBuffer = (char*)malloc(nSectors * 512)) == NULL)
     return printf("%s Not enough memory.", routine), -1;

  r = bench_hd_seek(nHandle, nSides, nSectors, nTracks);

  free(pBuffer);
  DskClose(nHandle);

  return r;
}


double pmb_buscache_xfer(int disknr)
{
  double r;
  char szName[8];
  char* routine = "\npmb_buscache_xfer:";

  disknr++;

  sprintf(szName, "$%d:", disknr);

  if ((nHandle = DskOpen(szName, 0, 0, &nSides, &nTracks, &nSectors)) < 0)
     {
     return printf("%s Cannot access disk", routine), -1;
     }

  nSectors = nSectors * (SECTORS_PER_TRANSFER / nSectors);

  amount = nSectors * 512;
  if (!(amount%65536))
     {
     amount++;
     }

  if ((pBuffer = (char*)malloc(amount)) == NULL)
     {
     return printf("%s Not enough memory.", routine), -1;
     }

  r = bench_hd_bus(nHandle, nSectors);

  free(pBuffer);
  DskClose(nHandle);
  return r;
}


double pmb_diskio_transfer0(int disknr)
{
  int r;
  char szName[8];
  ULONG rSectors;
  char* routine = "\npmb_diskio_transfer:";

  disknr++;
  sprintf(szName, "$%d:", disknr);

  if ((nHandle = DskOpen(szName, 0, 0, &nSides, &nTracks, &nSectors)) < 0)
    return printf("%s Cannot access disk %d.", routine, disknr), -1;

  rSectors = nSectors;
  nSectors = nSectors * (SECTORS_PER_TRANSFER / nSectors);

  amount = nSectors * 512;
  if (!(amount%65536))
     {
     amount++;
     }

  if ((pBuffer = (char*)malloc(amount)) == NULL)
    return printf("%s Not enough memory.", routine), -1;

  r = bench_hd_transfer(nHandle, 0,  1, nSides, nSectors, rSectors);

  free(pBuffer);
  DskClose(nHandle);

  if (r <= 0)
     {
     printf("%s track 0 bench failed", routine);
     return -1;
     }

  return r;
}


double pmb_diskio_transfern2(int disknr)
{
  int r;
  char szName[8];
  ULONG rSectors;
  char* routine = "\npmb_diskio_transfern2:";

  disknr++;
  sprintf(szName, "$%d:", disknr);

  if ((nHandle = DskOpen(szName, 0, 0, &nSides, &nTracks, &nSectors)) < 0)
    return printf("%s Cannot access disk %d.", routine, disknr), -1;

  rSectors = nSectors;
  nSectors = nSectors * (SECTORS_PER_TRANSFER / nSectors);

  amount = nSectors * 512;
  if (!(amount%65536))
     {
     amount++;
     }

  if ((pBuffer = (char*)malloc(amount)) == NULL)
    return printf("%s Not enough memory.", routine), -1;

  r =  bench_hd_transfer(nHandle, nTracks/2,  1, nSides, nSectors, rSectors);

  free(pBuffer);
  DskClose(nHandle);

  if (r <= 0)
     {
     return printf("%s track n/2 bench failed", routine), -1;
     }

  return r;
}


double pmb_diskio_transfern(int disknr)
{
  int r;
  char szName[8];
  ULONG rSectors;
  char* routine = "\npmb_diskio_transfern:";

  disknr++;
  sprintf(szName, "$%d:", disknr);

  if ((nHandle = DskOpen(szName, 0, 0, &nSides, &nTracks, &nSectors)) < 0)
    return printf("%s Cannot access disk %d.", routine, disknr), -1;

  rSectors = nSectors;
  nSectors = nSectors * (SECTORS_PER_TRANSFER / nSectors);

  amount = nSectors * 512;
  if (!(amount%65536))
     {
     amount++;
     }

  if ((pBuffer = (char*)malloc(amount)) == NULL)
    return printf("%s Not enough memory.", routine), -1;

  r = bench_hd_transfer(nHandle, nTracks-1, -1, nSides, nSectors, rSectors);

  free(pBuffer);
  DskClose(nHandle);

  if (r <= 0)
     {
     printf("%s track n bench failed", routine);
     return -1;
     }

  return r;
}

double pmb_diskio_cpupct(int disknr)
{
  double r;
  ULONG rSectors;
  char szName[8];
  APIRET rc;
  int i;
  char* routine = "\npmb_diskio_cpupct:";

  disknr++;

  sprintf(szName, "$%d:", disknr);

  if ((nHandle = DskOpen(szName, 0, 0, &nSides, &nTracks, &nSectors)) < 0)
     {
     return printf("%s Cannot access disk", routine), -1;
     }
  if (PerfSysSup == 1)
     {
     rSectors = nSectors;
     nSectors = nSectors * (SECTORS_PER_TRANSFER / nSectors);
     }

  amount = nSectors * 512;
  if (!(amount%65536))
     {
     amount++;
     }

  if ((pBuffer = (char*)malloc(amount)) == NULL)
     {
     return printf("%s Not enough memory", routine), -1;
     }

  if (PerfSysSup == 0)
     {
     if (!doneDhry)
        {
        rc = DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, 0);

        r = bench_dhry();      /* get base value for CPU when we're running at hi priority */

        rc = DosSetPriority(PRTYS_THREAD, PRTYC_REGULAR, 0, 0);

        doneDhry = 1;
        }
     }

  if (PerfSysSup == 1)
     {
     char  filename[15] = "sbcpuse.log";

     DosDelete(filename);

     GetCPUsage();

     r = bench_hd_transfer(nHandle,        0,  1, nSides, nSectors, rSectors);
     if (r > 0)
        {
        r = bench_hd_transfer(nHandle, nTracks/2,  1, nSides, nSectors, rSectors);
        if (r > 0)
           {
           r = bench_hd_transfer(nHandle, nTracks-1, -1, nSides, nSectors, rSectors);
           if (r > 0)
              {
              r = GetCPUsage();
              }
           }
        }
     DosDelete(filename); /* nuke this in case any test fails */
     }
  else
     {
     r = bench_hd_cpuusage(nHandle, nSides, nSectors);
     }

  free(pBuffer);
  DskClose(nHandle);
  if (r <= 0)
     {
     return printf("%s bench_hd_transfer failed", routine), -1;
     }

  return r;
}


// return number of bytes on disk nDisk
double pmb_diskio_disksize(int nDisk)
  {
  char szName[8];
  char* routine = "\npmb_diskio_disksize:";

  nDisk++;

  sprintf(szName, "$%d:", nDisk);

  if ((nHandle = DskOpen(szName, 0, 0, &nSides, &nTracks, &nSectors)) < 0)
     {
     printf("%s Cannot access disk %d (%s).", routine, nDisk, szName);
     exit(1);
     }

  DskClose(nHandle);
  return (nSides * nTracks * nSectors) / 2;    /* divide by 2 to get Kb on disk */
  }


int pmb_diskio_nrdisks(void)
  {
  USHORT nDisks;
  int nCount;
  char* routine = "\npmb_diskio_nrdisks:";

  if (DosPhysicalDisk(INFO_COUNT_PARTITIONABLE_DISKS, &nDisks, sizeof(nDisks), 0, 0))
     {
     printf("%s Cannot determine number of disks.", routine);
     exit(1);
     }

  return nDisks;
  }


// return number of bytes on disk nDisk
double pmb_cdio_disksize(int nDrive) {
  int nHandle, nDriveLetter;
  ULONG nSectors;
  char szDrive[3], szUPC[8];
  char* routine = "\npmb_cdio_disksize:";

  nDrive++;

  if ((nDriveLetter = CDFind(nDrive)) == -1)
    return printf("%s Cannot access CD-ROM drive %d.", routine, nDrive), -1;

  szDrive[0] = (char) nDriveLetter;
  szDrive[1] = ':';
  szDrive[2] = 0;

  if (debugging)
     {
     printf("%s szDrive = %s", routine, szDrive);
     }

  if ((nHandle = CDOpen(szDrive, 1, szUPC, &nSectors)) == -1)
     {
     return printf("%s CDOpen failed", routine), -1;
     }

  CDClose(nHandle);

  if (debugging)
     {
     printf("%s nSectors = %u, UPC = %14x", routine, nSectors, szUPC);
     }

  return (nSectors * 2);    /* multiply by 2 to get Kb on disk */
}


int pmb_cdio_nrcds(void) {
  USHORT nCDROMs;
  char* routine = "\npmb_cdio_nrcds:";

  nCDROMs = CDFind(0);

  if (debugging)
     {
     printf("%s nCDROMs = %d", routine, nCDROMs);
     }

  return nCDROMs;
}


double pmb_cdio_avseek(int nDrive)
{
  int nHandle, nDriveLetter;
  ULONG nSectors;
  char szDrive[3], szUPC[8];
  double r;
  ULONG rc;
  char* routine = "\npmb_cdio_avseek:";

  nDrive++;

  if ((nDriveLetter = CDFind(nDrive)) == -1)
    return printf("%s Cannot access CD-ROM drive %d.", routine, nDrive), -1;

  if (debugging)
     {
     printf("%s nDriveLetter = %d", routine, nDriveLetter);
     }

  szDrive[0] = (char) nDriveLetter;
  szDrive[1] = ':';
  szDrive[2] = 0;

  if (debugging)
     {
     printf("%s szDrive = %s", routine, szDrive);
     }

  if ((nHandle = CDOpen(szDrive, 1, szUPC, &nSectors)) == -1)
     {
     return printf("%s CDOpen failed", routine), -1;
     }

  data.cd_drive_size[nDrive-1] = nSectors * 2;

  if (debugging)
     {
     printf("%s nHandle = %d, sectors = %u", routine, nHandle, nSectors);
     }

  if ((pBuffer = (char*)malloc(32 * 2048)) == NULL)
    return printf("%s Not enough memory.", routine), -1;

  /* spin up and seek to first sector */
  rc = CDRead(nHandle, 0, 32, pBuffer) == -1;
  if (rc == -1 )
     {
     printf("%s CD-ROM read error, rc = %x", routine, rc);
     CDClose(nHandle);
     return -1;
     }

  r = bench_cd_seek(nHandle, nSectors);

  if (debugging)
     {
     printf("%s seek time = %f", routine, r);
     }

  free(pBuffer);
  CDClose(nHandle);

  return r;
}


double pmb_cdio_inner(int nDrive)
{
  int nHandle, nDriveLetter;
  ULONG nSectors;
  char szDrive[3], szUPC[8];
  double r;
  ULONG rc;
  char* routine = "\npmb_cdio_inner:";

  nDrive++;

  if ((nDriveLetter = CDFind(nDrive)) == -1)
    return printf("%s Cannot access CD-ROM drive %d.", routine, nDrive), -1;

  if (debugging)
     {
     printf("%s nDriveLetter = %d", routine, nDriveLetter);
     }

  szDrive[0] = (char) nDriveLetter;
  szDrive[1] = ':';
  szDrive[2] = 0;

  if (debugging)
     {
     printf("%s szDrive = %s", routine, szDrive);
     }

  if ((nHandle = CDOpen(szDrive, 1, szUPC, &nSectors)) == -1)
     {
     return printf("%s CDOpen failed", routine), -1;
     }

  data.cd_drive_size[nDrive-1] = nSectors * 2;

  if (debugging)
     {
     printf("%s nHandle = %d", routine, nHandle);
     }

  if ((pBuffer = (char*)malloc(32 * 2048)) == NULL)
    return printf("%s Not enough memory.", routine), -1;

  /* spin up and seek to first sector */
  rc = CDRead(nHandle, 0, 32, pBuffer);
  if (rc == -1 )
     {
     printf("%s CD-ROM read error, rc = %x", routine, rc);
     CDClose(nHandle);
     return -1;
     }

  r = bench_cd_inner(nHandle, nSectors);
  ir = r;                             /* save value for outer test calcs later */

  if (debugging)
     {
     printf("%s transfer rate = %f", routine, r);
     }

  free(pBuffer);
  CDClose(nHandle);

  return r;
}


double pmb_cdio_outer(int nDrive)
{
  int nHandle, nDriveLetter;
  ULONG nSectors;
  char szDrive[3], szUPC[8];
  double r, r2;
  ULONG rc;
  char* routine = "\npmb_cdio_outer:";

  nDrive++;

  if ((nDriveLetter = CDFind(nDrive)) == -1)
    return printf("%s Cannot access CD-ROM drive %d.", routine, nDrive), -1;

  if (debugging)
     {
     printf("%s nDriveLetter = %d", routine, nDriveLetter);
     }

  szDrive[0] = (char) nDriveLetter;
  szDrive[1] = ':';
  szDrive[2] = 0;

  if (debugging)
     {
     printf("%s szDrive = %s", routine, szDrive);
     }

  if ((nHandle = CDOpen(szDrive, 1, szUPC, &nSectors)) == -1)
     {
     return printf("%s CDOpen failed", routine), -1;
     }

  if (debugging)
     {
     printf("%s nHandle = %d", routine, nHandle);
     }

  if ((pBuffer = (char*)malloc(32 * 2048)) == NULL)
    return printf("%s Not enough memory.", routine), -1;

  /* spin up and seek to first sector */
  rc = CDRead(nHandle, 0, 32, pBuffer);
  if (rc == -1 )
     {
     printf("%s CD-ROM read error, rc = %x", routine, rc);
     CDClose(nHandle);
     return -1;
     }

  r = bench_cd_outer(nHandle, nSectors);

  if (debugging)
     {
     printf("%s transfer rate = %f", routine, r);
     }

  if ((r - ir > 100) &&   /* if difference between inner and outer rates < 100Kb */
      (nSectors > SECTORS_ON_FULL_CD/5)) /* and CD is not at least 20% full */
     {
     r2 = (SECTORS_ON_FULL_CD/nSectors)/PI;

     r2 = r + ((r-ir) * r2); /* factor rate up to very outside of disk */
     if (r2/r <= 3.0)        /* if not an impossible value! */
        {
        r = r2;
        }

     if (debugging)
        {
        printf("%s adjusted transfer rate = %f", routine, r);
        }
     }

  free(pBuffer);
  CDClose(nHandle);

  return r;
}


double pmb_cdio_cpupct(int nDrive)
{
  int nHandle, nDriveLetter;
  ULONG nSectors;
  char szDrive[3], szUPC[8];
  double r;
  APIRET rc = 0;
  double      ts_val[NUMCPUS], ts_val_prev[NUMCPUS];
  double      idle_val[NUMCPUS], idle_val_prev[NUMCPUS];
  double      busy_val[NUMCPUS], busy_val_prev[NUMCPUS];
  double      intr_val[NUMCPUS], intr_val_prev[NUMCPUS];
  double      ts_delta[NUMCPUS];
  CPUUTIL     CPUUtil[NUMCPUS];
  double      percentcpu[NUMCPUS];
  int i;
  char* routine = "\npmb_cdio_cpupct:";

  nDrive++;

  if ((nDriveLetter = CDFind(nDrive)) == -1)
    return printf("%s Cannot access CD-ROM drive %d.", routine, nDrive), -1;

  if (debugging)
     {
     printf("%s nDriveLetter = %d", routine, nDriveLetter);
     }

  szDrive[0] = (char) nDriveLetter;
  szDrive[1] = ':';
  szDrive[2] = 0;

  if ((nHandle = CDOpen(szDrive, 1, szUPC, &nSectors)) == -1)
     {
     printf("%s CDOpen failed", routine);
     return -1;
     }

  data.cd_drive_size[nDrive-1] = nSectors * 2;

  if (debugging)
     {
     printf("%s nHandle = %d", routine, nHandle);
     }

  if ((pBuffer = (char*)malloc(32 * 2048)) == NULL)
    return printf("%s Not enough memory.", routine), -1;

  /* spin up and seek to first sector */
  rc = CDRead(nHandle, 0, 32, pBuffer);
  if (rc == -1 )
     {
     printf("%s CD-ROM read error, rc = %x", routine, rc);
     CDClose(nHandle);
     return -1;
     }


  if (PerfSysSup == 0)
     {
     if (!doneDhry)
        {
        rc = DosSetPriority(PRTYS_THREAD, PRTYC_TIMECRITICAL, PRTYD_MAXIMUM, 0);

        r = bench_dhry();      /* get base value for CPU when we're running at hi priority */

        rc = DosSetPriority(PRTYS_THREAD, PRTYC_REGULAR, 0, 0);

        doneDhry = 1;
        }
     }

  if (PerfSysSup == 1)
     {
     char  filename[15] = "sbcpuse.log";

     /* spin up and seek to first sector */
     rc = CDRead(nHandle, 0, 32, pBuffer);
     if (rc == -1 )
        {
        printf("%s CD-ROM read error, rc = %x", routine, rc);
        CDClose(nHandle);
        return -1;
        }

     DosDelete(filename);

     GetCPUsage();

     r = bench_cd_transfer(nHandle);

     if (debugging)
        {
        printf("%s transfer result = %f", routine, r);
        }

     r = GetCPUsage();
     }
  else
     {
     r = bench_cd_cpuusage(nHandle);
     }

  if (debugging)
     {
     printf("%s result = %f", routine, r);
     }

  free(pBuffer);
  CDClose(nHandle);

  return r;
}


double GetCPUsage(void)
{
    int                       i = 0;
    double               result = 0;
    APIRET                   rc = 0;
    UCHAR LoadError[CCHMAXPATH] = {0};
    RESULTCODES      resultcode = {0};
    PID                pidChild = {0};
    UCHAR   exefile[CCHMAXPATH] = {0};
    char* routine = "\nGetCPUusage:";

    strcpy((char*)exefile, invocationpath);
    strcat((char*)exefile, "\\");
    strcat((char*)exefile, "sbcpuse.run");

    rc = DosExecPgm((PCHAR)LoadError,
                   sizeof(LoadError),
                   EXEC_SYNC,
                   0,
                   0,
                   &resultcode,
                   (PCSZ)exefile);
    if (rc)
       {
       char tmp[40+CCHMAXPATH];
       HAB hab;
       HMQ hmq;

       if (rc == 2)
          {
          sprintf(tmp, "File not found.");
          }
       else
          {
          sprintf(tmp, "DosExecPgm rc = %x", rc);
          }

       sprintf(tmp, "%s\nFile: %s", tmp, LoadError);
       if (debugging)
          {
          printf("%s", tmp);
          }

       hab = WinInitialize(0);
       hmq = WinCreateMsgQueue(hab, 0);

       WinMessageBox(HWND_DESKTOP,
                    HWND_DESKTOP,
                    tmp,
                    "Warning !",
                    1,
                    MB_OK | MB_WARNING | MB_APPLMODAL | MB_MOVEABLE);
       WinDestroyMsgQueue(hmq);
       WinTerminate(hab);
       return -1;
       }

    result = (double)resultcode.codeResult/100.0;
    if (debugging)
       {
       printf("%s result = %f", routine, result);
       }

    return (double)result;
}


