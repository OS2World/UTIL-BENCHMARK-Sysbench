

#define INCL_DOS
#define INCL_DOSFILEMGR
//#define INCL_DOSNLS        /* National Language Support values */
#define INCL_DOSERRORS     /* DOS error values */
#define INCL_DOSSEMAPHORES   /* Semaphore values */
#define INCL_DOSDATETIME     /* Timer support    */
#define INCL_DOSMEMMGR

#include <stdio.h>
#include <os2.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
//#include <stdarg.h>

//#define FILESIZE 4*1024*1024
ULONG FILESIZE = 4*1024*1024;

HEV     hevEvent1     = 0;                   /* Event semaphore handle    */
HTIMER  htimerEvent1  = 0;                   /* Timer handle              */
ULONG   ulPostCount   = 0;                   /* Semaphore post count      */
HEV     hevFileIO     = 0;                   /* Event semaphore handle    */
HTIMER  htimerEvent2  = 0;                   /* Timer handle              */
ULONG   ulPostCount2  = 0;                   /* Semaphore post count      */
volatile int Timeout  = 0;

void _Optlink TimeThread(void*);
double DoFileIO(ULONG, BOOL, BOOL, BOOL);
ULONG GetRand(ULONG, ULONG);
extern double dtime(void);
extern void logit(char*);


void _Optlink TimeThread(void* arg)
{
APIRET     rc;
DATETIME   datetime = {0};
    rc = DosCreateEventSem(NULL,           /* Unnamed semaphore            */
                           &hevEvent1,     /* Handle of semaphore returned */
                           DC_SEM_SHARED,  /* Indicate a shared semaphore  */
                           FALSE);         /* Put in RESET state           */

    rc = DosStartTimer(15000L,             /* 15 second interval            */
                      (HSEM)hevEvent1,     /* Semaphore to post            */
                      &htimerEvent1);      /* Timer handler (returned)     */

    rc = DosWaitEventSem(hevEvent1, SEM_INDEFINITE_WAIT); /* Wait indefinitely for timer or post */

    rc = DosResetEventSem(hevEvent1,        /* Reset the semaphore         */
                         &ulPostCount);     /* And get count (should be 1) */

    rc = DosPostEventSem(hevFileIO);        /* stop disk i/o loop */

    rc = DosStopTimer(htimerEvent1);       /* Stop the timer             */

    rc = DosCloseEventSem(hevEvent1);      /* Get rid of semaphore       */

    _endthread();

 }


double DoFileIO(ULONG buffersize, BOOL cache, BOOL reading, BOOL random)
{
PVOID dummy;
ULONG iocount     = 0;
HFILE hFilehandle = 0;
ULONG ulAction    = 0;
ULONG ulOpenmode  = 0;
ULONG ulActual    = 0;
ULONG ulPosition  = 0;
APIRET  rrc       = 0;
char filename[15] = "diskte$t.tmp";
char tmp[256];
double starttime, endtime, elaptime;
ULONG  numblks    = FILESIZE/buffersize;
int    i          = 0;

 printf("Filesize %d buffersize=%d numblks=%d\n", FILESIZE, buffersize, numblks);
 rrc = DosAllocMem(&dummy,             /* place to put data buffer ptr */
                  buffersize,
                  PAG_WRITE | PAG_COMMIT); /* we want to write to it and we want it now */

 if (rrc != NO_ERROR)
    {
    sprintf(tmp, "DosAllocMem failed with return code %d", rrc);
    logit(tmp);
    return 0;
    }

 memset(dummy, 0, buffersize);      /* set data buffer to zeros */

 ulOpenmode = OPEN_SHARE_DENYNONE | OPEN_ACCESS_READWRITE; /* set open mode */

 if (!cache)
    {
    ulOpenmode = ulOpenmode | OPEN_FLAGS_WRITE_THROUGH | OPEN_FLAGS_NO_CACHE; /* if not caching disable it */
    }

 if (random)
    {
    ulOpenmode = ulOpenmode | OPEN_FLAGS_RANDOM;
    }
 else
    {
    ulOpenmode = ulOpenmode | OPEN_FLAGS_SEQUENTIAL;
    }

 sprintf(filename, "dis%05u.tmp", buffersize);  /* choose filename */

 if (!reading)  /* if not reading */
    {
    rrc = DosOpen(filename,         /* open file for write */
                 &hFilehandle,
                 &ulAction,
                 FILESIZE,
                 FILE_ARCHIVED | FILE_NORMAL,
                 OPEN_ACTION_CREATE_IF_NEW |
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 ulOpenmode,
                 0L);
    }
 else        /* if reading file */
    {
    rrc = DosOpen(filename,
                 &hFilehandle,
                 &ulAction,
                 FILESIZE,
                 FILE_NORMAL,
                 OPEN_ACTION_FAIL_IF_NEW |
                 OPEN_ACTION_OPEN_IF_EXISTS,
                 ulOpenmode,
                 0L);
    }
 //printf("\nOpened file %s with flags %08x\n", filename, ulOpenmode);
 if (rrc != NO_ERROR)
    {
    sprintf(tmp, "DosOpen of file %s for %s failed with return code %d", filename,
          reading ? "write" : "read",
          rrc);
    logit(tmp);
    DosFreeMem(dummy); /* free of buffer */
    return 0;
    }
/*
 if ((!reading) && (!random) && (!cache)) // if first time for this file
    {
    for (i = 0; i < numblks; i++)
       {
       memset(dummy, i, buffersize);

       rrc = DosWrite(hFilehandle,
                     dummy,
                     buffersize,
                     &ulActual);

       if (rrc != NO_ERROR)
          {
          sprintf(tmp, "Preformat DosWrite error code %d", rrc);
          logit(tmp);
          DosFreeMem(dummy);
          DosClose(hFilehandle);
          return 0;
          }
       }
    rrc = DosResetBuffer(hFilehandle);
    if (rrc != NO_ERROR)
       printf("Preformat DosResetBuffer rc = %d\n", rrc);
    rrc = DosSetFilePtr(hFilehandle, 0, FILE_BEGIN, &ulActual);
    if (rrc != NO_ERROR)
       printf("Preformat DosSetFilePtr rc = %d\n", rrc);
    }
*/
 _beginthread(TimeThread, NULL, 8192, NULL);    /* start timer thread */

 DosSleep(200);

 starttime = dtime();   /* log start time of actual i/o loop */

 rrc = DosQueryEventSem(hevFileIO,
                       &ulPostCount2);

 while (ulPostCount2 == 0)        /* do until timer pops */
    {
    if (!reading)       /* if writing file */
       {
       if (random)      /* randomly */
          {
          ulPosition = GetRand(numblks, buffersize);
          //printf("% 8d % 12u\n", iocount, ulPosition);
          //logit(tmp);
          rrc = DosSetFilePtr(hFilehandle, ulPosition, FILE_BEGIN, &ulActual); /* point there */
          if (rrc != NO_ERROR)
             {
             sprintf(tmp, "DosSetFilePtr write random error code %d", rrc);
             logit(tmp);
             }
          }      /* end if random write */

       memset(dummy, iocount, sizeof(iocount)); /* make this buffer different from last */

       rrc = DosWrite(hFilehandle,              /* write data */
                     dummy,
                     buffersize,
                     &ulActual);

       if (rrc == NO_ERROR)
          {
          iocount++;    /* increment count if OK */
          }
       else
          {
          sprintf(tmp, "DosWrite error code %d", rrc);
          logit(tmp);
          DosFreeMem(dummy); /* free of buffer */
          DosClose(hFilehandle); /* close file */
          return 0;
          }

       if (!(iocount%numblks) && !random) /* if sequential access and eof */
          {
          ulPosition = 0;
          rrc = DosSetFilePtr(hFilehandle, 0, FILE_BEGIN, &ulActual); /* start at beginning again */
          if (rrc != NO_ERROR)
             {
             sprintf(tmp, "DosSetFilePtr write eof error code %d", rrc);
             logit(tmp);
             }
          }
       }
    else   /* if reading from file */
       {
       if (random)
          {
          ulPosition = GetRand(numblks, buffersize);              /*  set byte displacement */
          //printf("% 8d % 12u\n", iocount, ulPosition);

          rrc = DosSetFilePtr(hFilehandle, ulPosition, FILE_BEGIN, &ulActual);
          if (rrc != NO_ERROR)
             {
             sprintf(tmp, "DosSetFilePtr read random error code %d", rrc);
             logit(tmp);
             }
          }

       rrc = DosRead(hFilehandle,      /* read data */
               dummy,
               buffersize,
               &ulActual);

       if (rrc != NO_ERROR)
          {
          sprintf(tmp, "DosRead error code %d", rrc);
          logit(tmp);
          DosFreeMem(dummy); /* free of buffer */
          DosClose(hFilehandle); /* close file */
          return 0;
          }
       else
          {
          iocount++; /* increment counter if OK */
          }
       if (ulActual < buffersize)  /* if data read < amount requested */
          {
          ulPosition = 0;
          rrc = DosSetFilePtr(hFilehandle, 0, FILE_BEGIN, &ulPosition);  /* reset to start of file */
          if (rrc != NO_ERROR)
             {
             sprintf(tmp, "DosSetFilePtr read eof error code %d", rrc);
             logit(tmp);
             }
          }
       }
    rrc = DosQueryEventSem(hevFileIO,
                          &ulPostCount2);
    }

 DosClose(hFilehandle); /* close file */

 endtime = dtime();  /* log time at end of disk i/o loop */

 if (reading)
    {
    if (random)
       {
       if (cache)
          {
          DosDelete(filename); /* if last use of file */
          }
       }
    }

 DosFreeMem(dummy); /* free of buffer */

 elaptime = endtime-starttime;  /* get elapsed time for i/o */
 if (elaptime <= 0)
    elaptime = 1;
 printf("%s %u buffers of %d bytes (%dMB) in %f seconds\n",
       reading ? "Read" : "Wrote",
       iocount,
       buffersize,
       (iocount*buffersize)/(1024*1024),
       elaptime);
 return (((double)buffersize*(double)iocount)/elaptime); /* return kb/sec */

}

ULONG GetRand(ULONG numblks, ULONG buffersize)
  {
  ULONG rnm, segment, k = 1;

  rnm = rand();
  segment = (numblks-1)/(RAND_MAX+1); // how many segments in the file
  if (segment > 0)
     {
     while (rnm > segment)
        {
        rnm = rnm << k;
        rnm = rnm >> k++;
        }
     segment = rnm;
     rnm = rand();
     }
  k = 1;
  while (rnm > (numblks-1)%(RAND_MAX+1))
     {
     rnm = rnm << k;
     rnm = rnm >> k++;
     }

  return ((segment * 32768) + rnm) * buffersize;  /* set byte displacement into file */
  }

