/* diskacc2.c - direct disk access library for OS/2 2.x protected mode.
 *
 * Author:  Kai Uwe Rommel <rommel@ars.muc.de>
 * Created: Fri Jul 08 1994
 */

static char *rcsid =
"$Id: diskacc2.c,v 1.5 1997/03/01 20:21:35 rommel Exp rommel $";
static char *rcsrev = "$Revision: 1.5 $";

/*
 * $Log: diskacc2.c,v $
 * Revision 1.5  1997/03/01 20:21:35  rommel
 * fixed CDOpen bugs
 *
 * Revision 1.4  1997/02/09 15:05:57  rommel
 * added a few comments
 *
 * Revision 1.3  1997/01/12 21:15:26  rommel
 * added CD-ROM routines
 *
 * Revision 1.2  1995/12/31 21:50:58  rommel
 * added physical/logical mode parameter
 *
 * Revision 1.1  1994/07/08 21:34:12  rommel
 * Initial revision
 *
 */

#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSMISC
#define INCL_NOPM
#include <os2.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "diskacc2.h"
#include "types.h"

#define PHYSICAL     0x1000
#define CDROM        0x2000

#define CATEGORY(x)  (((x) & CDROM) ? IOCTL_CDROMDISK : ((x) & PHYSICAL) ? IOCTL_PHYSICALDISK : IOCTL_DISK)
#define HANDLE(x)    ((x) & ~(PHYSICAL|CDROM))

#pragma pack(1)

extern ULONG debugging;

/* logical/physical hard disk and floppy disk access code */


ULONG DosDevIOCtl32(PVOID pData, ULONG cbData, PVOID pParms, ULONG cbParms,
          ULONG usFunction, HFILE hDevice)
{
 APIRET ulrc;

  ULONG ulParmLengthInOut = cbParms, ulDataLengthInOut = cbData,
        category = CATEGORY(hDevice), handle = HANDLE(hDevice);
  ulrc = DosDevIOCtl(handle,
                    category,
                    usFunction,
                    pParms,
                    cbParms,
                    &ulParmLengthInOut,
                    pData,
                    cbData,
                    &ulDataLengthInOut);

  if (ulrc)
     if (debugging)
        {
        printf("\nDosDevIOCtl32: rc = 0x%x, handle = %u, category = %u, function = %u\n",
              ulrc, handle, category, usFunction);
        }

  return ulrc;
}

static int test_sector(int handle, int side, int track, int sector)
{
  char buffer[1024];
  TRACK trk;

  trk.bCommand      = 0;
  trk.usHead        = side;
  trk.usCylinder    = track;
  trk.usFirstSector = 0;
  trk.cSectors      = 1;

  trk.TrackTable[0].usSectorNumber = sector;
  trk.TrackTable[0].usSectorSize   = 512;

  return DosDevIOCtl32(buffer, sizeof(buffer), &trk, sizeof(trk),
             DSK_READTRACK, handle) == 0;
}

int DskOpen(char *drv,
            int logical,
            int lock,
            ULONG *sides,
            ULONG *tracks,
            ULONG *sectors)
{
  BIOSPARAMETERBLOCK bpb;
  DEVICEPARAMETERBLOCK dpb;
  HFILE handle;
  USHORT physical;
  ULONG action;
  BYTE cmd = logical;

  if (isalpha(drv[0]) &&
      drv[1] == ':' &&
      drv[2] == 0)
     {
     if (DosOpen(drv,
                &handle,
                &action,
                0,
                FILE_NORMAL,
                FILE_OPEN,
                OPEN_FLAGS_DASD |
                OPEN_FLAGS_FAIL_ON_ERROR |
                OPEN_ACCESS_READWRITE |
                OPEN_SHARE_DENYREADWRITE,
                0))
        {
        return -1;
        }
     }
  else
     {
     if (drv[0] == '$' && (
         (isdigit(drv[1]) &&
          drv[2] == ':' &&
          drv[3] == 0) ||
         (isdigit(drv[1]) &&
          isdigit(drv[1]) &&
          drv[3] == ':' &&
          drv[4] == 0)
        ))
        {
        if (DosPhysicalDisk(INFO_GETIOCTLHANDLE,
                           &physical,
                           sizeof(physical),
                           drv + 1,
                           strlen(drv + 1) + 1))
           {
           return -1;
           }
        handle = physical | PHYSICAL;
        }
     else
        {
        return -1;
        }
     }

  if (handle & PHYSICAL)
     {
     if (DosDevIOCtl32(&dpb,
                      sizeof(dpb),
                      &cmd,
                      sizeof(cmd),
                      DSK_GETDEVICEPARAMS,
                      handle))
        {
        DosPhysicalDisk(INFO_FREEIOCTLHANDLE,
                       NULL,
                       0,
                       &physical,
                       sizeof(physical));
        return -1;
        }

     *sectors = dpb.cSectorsPerTrack;
     *tracks  = dpb.cCylinders;
     *sides   = dpb.cHeads;
     }
  else
     {
     if (DosDevIOCtl32(&bpb,
                      sizeof(bpb),
                      &cmd,
                      sizeof(cmd),
                      DSK_GETDEVICEPARAMS,
                      handle))
        {
        DosClose(handle);
        return -1;
        }

     *sectors = bpb.usSectorsPerTrack;
     *tracks  = bpb.cCylinders;
     *sides   = bpb.cHeads;
     }

  if (lock && DosDevIOCtl32(0, 0, &cmd, sizeof(cmd), DSK_LOCKDRIVE, handle))
     {
     if (handle & PHYSICAL)
        DosPhysicalDisk(INFO_FREEIOCTLHANDLE, NULL, 0, &physical, sizeof(physical));
     else
        DosClose(handle);
     return -1;
     }

  if (*sectors >= 15) /* 360k floppies ... */
     if (!test_sector(handle, 0, 0, 15))
        {
        if (*sectors == 15)
           *tracks = 40;

        *sectors = 9;
        }

  return handle;
}

int DskClose(int handle)
{
  BYTE cmd = 0;
  USHORT physical = handle & ~PHYSICAL;

  DosDevIOCtl32(0, 0, &cmd, sizeof(cmd), DSK_UNLOCKDRIVE, handle);

  if (handle & PHYSICAL)
    return DosPhysicalDisk(INFO_FREEIOCTLHANDLE, NULL, 0,
            &physical, sizeof(physical));
  else
    return DosClose(handle);
}


int DskRead(int handle, ULONG side, ULONG  track,
            ULONG sector, ULONG nsects, void *buf)
{
  TRACK trk;
  ULONG cnt, bufsize;
  APIRET rc;

  trk.bCommand      = 0;
  trk.usHead        = side;
  trk.usCylinder    = track;
  trk.usFirstSector = 0;
  trk.cSectors      = nsects;

  for (cnt = 0; cnt < nsects; cnt++)
     {
     trk.TrackTable[cnt].usSectorNumber = sector + cnt;
     trk.TrackTable[cnt].usSectorSize   = 512;
     }

  bufsize = nsects * 512;
  if (!(bufsize%65536))
     {
     bufsize++;
     }

  rc = DosDevIOCtl32(buf, bufsize, &trk, sizeof(TRACK),
                    DSK_READTRACK, handle);

  return rc;
}

int DskWrite(int handle, ULONG side, ULONG  track,
             ULONG sector, ULONG nsects, void *buf)
{
  TRACK trk;
  ULONG cnt;

  trk.bCommand      = 0;
  trk.usHead        = side;
  trk.usCylinder    = track;
  trk.usFirstSector = 0;
  trk.cSectors      = nsects;

  for (cnt = 0; cnt < nsects; cnt++)
  {
    trk.TrackTable[cnt].usSectorNumber = sector + cnt;
    trk.TrackTable[cnt].usSectorSize   = 512;
  }

  return DosDevIOCtl32(buf, nsects * 512, &trk, sizeof(trk),
                       DSK_WRITETRACK, handle);
}

/* CD-ROM access code */

static struct
{
  char sig[4];
  char cmd;
}
cdparams;

int CDFind(int number)
{
  int i;
  HFILE handle;
  ULONG action, status, datasize;
  APIRET rc;
  // used for CD number of units Ioctl
  struct {
        USHORT count;
        USHORT first;
        } cdinfo;

  // try to check for CD drives first, changers are a LONG painful process
  if (!DosOpen("\\DEV\\CD-ROM2$",
              &handle,
              &action,
              0,
              FILE_NORMAL,
              OPEN_ACTION_OPEN_IF_EXISTS,
              OPEN_SHARE_DENYNONE |
              OPEN_ACCESS_READONLY,
              NULL))
     {
     datasize=sizeof(cdinfo);
     if (!(rc=DosDevIOCtl(handle,
                         0x82,
                         0x60,
                         NULL,
                         0,
                         NULL,
                         (PVOID)&cdinfo,
                         sizeof(cdinfo),
                         &datasize)))
        {
        // mark those devices as CDs
        if (debugging)
           {
           printf("\nCDFind: cdinfo.count = %d, cdinfo.first = %d", cdinfo.count, cdinfo.first);
           }
        } /* end if */
     DosClose(handle);
     } /* end if */
  else
     { /* if DosOpen failed then OS2CDROM.DMD is missing */
     cdinfo.count = 0;
     }

  if (number == 0)
     {
     return cdinfo.count;
     }
  else
     {
     return 'A'+cdinfo.first+number-1;
     }
}


int CDOpen(char *drv, int lock, char *upc, ULONG *sectors)
{
  HFILE handle;
  ULONG action;
  ULONG lockrc;
  char upcdata[10];
//  UCHAR cdlockparms[120] = {0};
  ULONG uparmlen = 0;
  UCHAR udataarea[120] = {0};
  ULONG udatalen = 0;

  ULONG ulDriveNum;
  FSALLOCATE fsaBuffer = {0};
  APIRET rc;

  if (isalpha(drv[0]) && drv[1] == ':' && drv[2] == 0)
     {
     DosError(FERR_DISABLEHARDERR);

     if (DosOpen(drv, &handle, &action, 0, FILE_NORMAL, FILE_OPEN,
                OPEN_FLAGS_DASD | OPEN_FLAGS_FAIL_ON_ERROR |
                OPEN_ACCESS_READONLY | OPEN_SHARE_DENYREADWRITE, 0))
        {
        DosError(FERR_ENABLEHARDERR);

        return -1;
        }
     }
  else
     return -1;

  handle |= CDROM;
  memcpy(cdparams.sig, "CD01", 4);

  cdparams.cmd = 1;
  lockrc = DosDevIOCtl32(0, 0, &cdparams, sizeof(cdparams),
             CDROMDISK_LOCKUNLOCKDOOR, handle);

  if (lock && (lockrc != 0 && lockrc != 0x0000ff13) )
     {
     printf("CDOpen - CD Lockunlock failed rc = %x\n", lockrc); /* debugcdcode */
     DosClose(HANDLE(handle));
     DosError(FERR_ENABLEHARDERR);
     return -1;
     }

  ulDriveNum = drv[0] - 'A';
  rc = DosQueryFSInfo(ulDriveNum+1,
                     FSIL_ALLOC,
                     (PVOID)&fsaBuffer,
                     sizeof(FSALLOCATE));
  if ((rc != NO_ERROR) || (fsaBuffer.cUnit == 0)) /* if error in FS info */
     {
     cdparams.cmd = 0;
     DosDevIOCtl32(0, 0, &cdparams, sizeof(cdparams.sig),
                  CDROMDISK_LOCKUNLOCKDOOR, handle);
     DosClose(HANDLE(handle));
     DosError(FERR_ENABLEHARDERR);
     return -1;
     }
  else
     {
     *sectors = fsaBuffer.cUnit;
     }

  memset(upcdata, 0, sizeof(upcdata));
  if (DosDevIOCtl32(upcdata, sizeof(upcdata), &cdparams, sizeof(cdparams.sig),
          CDROMDISK_GETUPC, handle))
     {
     /* ignore possible errors but ... */
     *upc = 0;
     }
  else
     {
     memcpy(upc, upcdata + 1, 7);
     upc[7] = 0;
     }

  DosError(FERR_ENABLEHARDERR);
  return handle;
}

int CDClose(int handle)
{
  cdparams.cmd = 0;
  DosDevIOCtl32(0, 0, &cdparams, sizeof(cdparams.sig),
      CDROMDISK_LOCKUNLOCKDOOR, handle);

  return DosClose(HANDLE(handle));
}


int CDRead(int handle, ULONG sector, ULONG nsects, void *buf)
{
  ULONG nActual;
  ULONG rc;

  rc = DosSetFilePtr(HANDLE(handle), sector * 2048, FILE_BEGIN, &nActual);
  if (rc)
     {
     printf("CDRead - DosSetFilePtr return code 0x%0x, sector = %u, #sectors = %u\n", rc, sector, nsects);
     return -1;
     }

  rc = DosRead(HANDLE(handle), buf, nsects * 2048, &nActual);
  if (rc)
     {
     printf("CDRead - DosRead error return code 0x%0x, #sectors = %u\n", rc, nsects);
     return -1;
     }

  return nActual / 2048;
}

/* end of diskacc2.c */
