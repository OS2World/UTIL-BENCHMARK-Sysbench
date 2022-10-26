
// SysBench DIVE test module
// made for the warp II beta toolkit. Must probably be modified to run on OS/2 3.0
// corrected bug where paint_rot tried to paint horizontal lines not vertical 28/06/96


#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#define  _MEERROR_H_
#include "mmioos2.h"                   /* It is from MMPM toolkit           */
#include "dive.h"
#include "fourcc.h"
#include "pmb.h"
#include "types.h"

#define ID_WINDOW1 8000
#define PMB_DIVE_CLASS "SysBench dive winclass"
#define MIN_DIVE_TIME 10.0
#define MIN_DIVE2_TIME 1.0
#define MIN_MEASURE 0.1
#define MARGINAL 1.1
#define PI 3.14159265358979323846

//static HAB  hab;

//extern double cos(double);
//extern double sin(double);
extern void err(char* s);
extern void warn(char* s);
extern void logit(char* s);
//extern HAB anchorblock(void);
extern double rtime(void);    // real time in seconds
extern double dtime(void);    // used CPU time in seconds
extern double test_time;

void APIENTRY paint_videobw(ULONG unused);
void APIENTRY paint_rot(ULONG unused);
void APIENTRY paint_ms12(ULONG unused);

static MRESULT EXPENTRY DiveWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
static void Open(void *paintfun);
static void Close(void);
void end_Win(HDIVE hDive, char *p, HWND hwndClient);
void clr_lines(s32 y, s32 lines);
void display_lines(u8* buf, s32 y, s32 lines);
inline void display_lines2(u8* src, u8* dest, s32 yin, s32 yout);
inline s32 fun(s32 x, double param);

static HWND hwndClient = NULLHANDLE;         /* Client area window handle    */
static HWND hwndFrame = NULLHANDLE;          /* Frame window handle          */
static HMQ  hmq;
static ULONG flCreate;                       /* Window creation control flags*/
static HAB bkHab;
static TID paint_tid;
static double result;
static char* framebuf; // the main framebuffer
static DIVE_CAPS dc;
static HDIVE hDive;
static RECTL rect;
static char szName[50];

static void Open(void *paintfun) {
  s32 w,h, x,y;
  RECTL rctl, rctlScreen;
  QMSG qmsg;                            /* Message from message queue   */
  hwndClient = NULLHANDLE;         /* Client area window handle    */
  hwndFrame = NULLHANDLE;          /* Frame window handle          */
//  hab = anchorblock();

  if ((bkHab = WinInitialize(0)) == 0L) /* Initialize PM     */
    err("Can't get anchor block handle for background thread");

  if ((hmq = WinCreateMsgQueue( bkHab, 0 )) == 0L)/* Create a msg queue */
    err("Can't create message queue for graphics test window");

  if (!WinRegisterClass(bkHab, (PSZ)PMB_DIVE_CLASS, (PFNWP)DiveWindowProc, 0, 0)) {
    err("DIVE test error: can't register class for child test window");
  }

  flCreate = FCF_TASKLIST;

  if ((hwndFrame = WinCreateStdWindow(
               HWND_DESKTOP,            /* Desktop window is parent     */
               0,                       /* window styles           */
               &flCreate,               /* Frame control flag           */
               PMB_DIVE_CLASS,    /* Client window class name     */
               "SysBench dive test window",    /* window text               */
               0,                       /* No special class style       */
               (HMODULE)0L,             /* Resource is in .EXE file     */
               ID_WINDOW1,               /* Frame window identifier      */
               &hwndClient              /* Client window handle         */
               )) == 0L)
    err("Can't create dive test window");

  WinQueryWindowRect(HWND_DESKTOP, &rect);
  WinSetWindowPos(hwndFrame, HWND_TOP, rect.xLeft, rect.yBottom,
    rect.xRight-rect.xLeft+1, rect.yTop-rect.yBottom+1, SWP_SIZE | SWP_MOVE | SWP_ACTIVATE | SWP_SHOW | SWP_ZORDER);

  DosCreateThread(&paint_tid, (PFNTHREAD)paintfun, 0, 0, 64000);
  while( WinGetMsg( bkHab, &qmsg, 0L, 0, 0 ) )
    WinDispatchMsg( bkHab, &qmsg );
}

static void Close(void) {
  WinDestroyWindow(hwndFrame);           /* Tidy up...                   */
  WinDestroyMsgQueue( hmq );             /* Tidy up...                   */
  WinTerminate(bkHab);
}

double pmb_dive_bw(void) {
  Open((void*)paint_videobw);
  Close();
  return result;
}

double pmb_dive_rot(void) {
  Open((void*)paint_rot);
  Close();
  return result;
}

double pmb_dive_ms11(void) {
  Open((void*)paint_ms12);
  Close();
  return result;
}

static MRESULT EXPENTRY DiveWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
  switch( msg )
  {
    case WM_CREATE:
      break;

    case WM_COMMAND:
      break;

    case WM_ERASEBACKGROUND:
      return (MRESULT)( FALSE ); // TRUE -> yes, erase the background

    case WM_PAINT:
      {
        HPS    hps;
        RECTL  rc;
        POINTL pt;

        hps = WinBeginPaint( hwnd, 0L, &rc );
        GpiSetColor( hps, CLR_BLACK );      // colour of the text,
        GpiSetBackColor( hps, CLR_BLACK );   // its background and
        GpiSetBackMix( hps, BM_OVERPAINT );  // how it mixes,
        GpiSetMix( hps, FM_OVERPAINT );  // how it mixes,
//        WinFillRect(hps, &rc, CLR_BLACK);
        WinEndPaint( hps );                  // Drawing is complete
        break;
      }
    case WM_CLOSE:
      WinPostMsg( hwnd, WM_QUIT, (MPARAM)0,(MPARAM)0 );
      break;
    default:
      return WinDefWindowProc( hwnd, msg, mp1, mp2 );
  }
  return (MRESULT)FALSE;
}

void APIENTRY paint_videobw(ULONG unused) {
  HPS hps;
  POINTL p1, p2;
  double t1, t2, test_time;
  RECTL rc;
  PVOID apa;
  ULONG ret;
  PDIVE_CAPS pdc;
  s32    i, j, c, runs, runs_10, framesize,
         sh, scanlen, frames, numApertures,
         ApertureNum, Datalen, Offset,
         ApertureOffset, RemainLen, z, zz;
  char* framecopy;
  char* framecopy1;
  APIRET rc1 = 0UL;
  int notstop = 1;

  memset(&dc, 0, sizeof(dc));
  dc.ulStructLen = sizeof(dc);
  dc.ulFormatLength = 0;
  ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN);
  if (DIVE_SUCCESS != ret) {
    if (ret == DIVE_ERR_INSUFFICIENT_LENGTH) {
      dc.pFormatData = calloc(dc.ulFormatLength,1);
      ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN); // let's try again
      if (DIVE_SUCCESS != ret) {
        err("Error in call to DiveQueryCaps()");
      }
    } else {
      err("Error in call to DiveQueryCaps()");
    }
  }

  rc1 = DiveOpen(&hDive, FALSE, &apa);

  if (rc1 != DIVE_SUCCESS)
     {
     if (rc1 == DIVE_ERR_TOO_MANY_INSTANCES)
        {
        logit("Dive error - too many instances");
        result = -1;
        }
     if (rc1 == DIVE_ERR_SSMDD_NOT_INSTALLED)
        {
        logit("Dive error - SSMDD.SYS not installed");
        result = -1;
        }
     if (rc1 == DIVE_ERR_NO_DIRECT_ACCESS)
        {
        logit("Dive error - no direct access");
        result = -1;
        }
     if (rc1 == DIVE_ERR_ALLOCATION_ERROR)
        {
        logit("Dive error - allocation error, check MMOS/2 installed");
        result = -1;
        }
     if (rc1 != DIVE_SUCCESS)
        {
        result = -1;
        free(dc.pFormatData);
        WinPostMsg( hwndClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
        return;
         }
  }

  framebuf = (char*)apa;

  WinQueryWindowRect(HWND_DESKTOP, &rect);
     ret = DiveAcquireFrameBuffer(hDive, &rect);
     if (ret != DIVE_SUCCESS)
        {
        logit("Error in DiveAcquireFrameBuffer()");
        result = -1;
        end_Win(hDive, (char*)dc.pFormatData,hwndClient);
        return;
        }

  framesize = dc.ulVerticalResolution * dc.ulScanLineBytes;
  framecopy = (char*)malloc(framesize);
  if (framecopy == null) {
    logit("Can't get memory for copy of frame buffer");
    result = -1;
    end_Win(hDive, (char*)dc.pFormatData,hwndClient);
    return;
  }

  if (dc.fScreenDirect)
     {                                                /* if direct access allowed */
     if (dc.ulApertureSize >= framesize)
        {                    /* and if video buffer larger or equal to our buffer */
        memcpy(framecopy, framebuf, framesize); /* copy from video buffer to ours */
        }
     else
        {                               /* if our buffer larger than video buffer */
         framecopy1 = framecopy;
         numApertures =  framesize/dc.ulApertureSize;
         for (i = 0; i < numApertures; i++)
            {
             rc1 = DiveSwitchBank(hDive, i);
             if (rc1 != DIVE_SUCCESS)
                {
                logit("DiveSwitchBank 1 returned an error");
                }
             memcpy(framecopy1, framebuf, dc.ulApertureSize);
             framecopy1 += dc.ulApertureSize;
             }
         if (framesize%dc.ulApertureSize) /* if not an exact multiple of aperture size */
            {
            rc1 = DiveSwitchBank(hDive, numApertures); /* switch to last bank */
            memcpy(framecopy1, framebuf, framesize%dc.ulApertureSize);    /* and copy last bit */
            }
         }
      }
  else
     {
     logit("No Direct Screen access possible.");
     result = -1;
     end_Win(hDive, (char*)dc.pFormatData,hwndClient);
     return;
     }

  frames = 0;
  sh = dc.ulVerticalResolution;
  scanlen = dc.ulScanLineBytes;

  runs = 200;

  while (notstop)
     {
     frames = 0;
     t1 = dtime();

     if (dc.ulApertureSize >= framesize)
        {                                     /* old style behaviour - copy in one block */
        for (i = 0; i < runs; i++) {
           frames++;
           j = i % (sh-1) + 1;
           memcpy(framebuf, framecopy+(sh-j)*scanlen, j*scanlen);
           memcpy(framebuf + j*scanlen, framecopy, (sh-j)*scanlen);
           }
        }
     else
        {      /* video memory is accessible in multiple banks of y lots of xKb (64Kb?) */
        numApertures = (framesize/dc.ulApertureSize);
        if (framesize%dc.ulApertureSize) /* if not exact number to buffer */
           {
           numApertures++;              /* increase by 1 */
           }

        framecopy1 = framecopy;
        for (i = 0; i < runs; i++)
           {
           frames++;
           j = (i % (sh-1)) + 1;
           Datalen = j * scanlen;
           ApertureNum = 0;
           rc1 = DiveSwitchBank(hDive, ApertureNum);           /* switch to correct bank */
           if (rc1 != DIVE_SUCCESS)
              {
              logit("DiveSwitchBank 2 returned an error");
              }

           Offset = 0;
           while (Datalen > 0)
              {
               if (Datalen < dc.ulApertureSize)
                  {
                  memcpy(framebuf,framecopy+Offset+((sh-j)*scanlen), Datalen);
                  Datalen = 0;
                  }
               else
                  {  /* first part of data > 64Kb */
                  memcpy(framebuf,framecopy+Offset+((sh-j)*scanlen), dc.ulApertureSize);
                  Datalen -= dc.ulApertureSize;
                  ApertureNum++;
                  Offset += dc.ulApertureSize;
                  rc1 = DiveSwitchBank(hDive, ApertureNum);
                  if (rc1 != DIVE_SUCCESS)
                     {
                     logit("DiveSwitchBank 3 returned an error");
                     }
                  }
               }     /* end While (Datalen > 0) */

           Datalen = (sh-j) * scanlen;     /* length of rest of screen */
//          RemainLen = Datalen%dc.ulApertureSize; /* length of data in first block */
           ApertureOffset = (j*dc.ulScanLineBytes)%dc.ulApertureSize; /* length of data in first block */
           RemainLen = dc.ulApertureSize-ApertureOffset; /* offset into aperture to write to */
           if (RemainLen != 0)
              {
              memcpy(framebuf+ApertureOffset, framecopy, RemainLen); /* copy first block */
              ApertureNum++;
              }
           z = 0;
           for (c = ApertureNum; c < numApertures; c++)
              {
               rc1 = DiveSwitchBank(hDive, c);           /* switch to correct bank */
               if (rc1 != DIVE_SUCCESS)
                  {
                  logit("DiveSwitchBank 5 returned an error");
                  }
               if (c == numApertures-1) /* if last frame */
                  {
                  zz = (dc.ulVerticalResolution * dc.ulScanLineBytes) % dc.ulApertureSize;
                  }
               else
                  {
                  zz = dc.ulApertureSize;
                  }
               memcpy(framebuf,framecopy + RemainLen + (z * dc.ulApertureSize) , zz);
               z++;
               }
            }        /* end For (i=0;i<runs)    */
         }           /* end Else if (dc.ulApertureSize >= framesize) */

    t2 = dtime();
    test_time = (t2-t1);

    if ((test_time) < MIN_DIVE_TIME) {
      if ((test_time) < MIN_MEASURE) {
        runs = MIN_DIVE_TIME/MIN_MEASURE*runs;
      } else {
        runs = MIN_DIVE_TIME*MARGINAL/(test_time)*runs;
      }
    } else {
      break;
    }
  }

  result = ((double) frames * scanlen * sh) / (test_time);

  DiveDeacquireFrameBuffer(hDive);

  end_Win(hDive, (char*)dc.pFormatData,hwndClient); /*
  DiveClose(hDive);
  free(dc.pFormatData);
  WinPostMsg( hwndClient, WM_QUIT, (MPARAM)0,(MPARAM)0 ); */
}

void APIENTRY paint_rot(ULONG unused) {
  HPS hps;
  POINTL p1, p2;
  double t1, t2, test_time;
  RECTL rc;
  PVOID apa;
  ULONG ret;
  PDIVE_CAPS pdc;
  s32 i, j, c, runs, runs_10, framesize, sh, scanlen, frames, yin, yout, x, y, i1, numApertures;
  char* framecopy;
  char* framecopy1;
  s32 speed, rev, arg_nrev, sh2, wh, wh2;
  double phase = 0.0, phase_steps, c1, c2, c3, c4, c5, c6, c7, c8;
  bool forw;
  APIRET rc1 = 0UL;
  int notstop = 1;

  memset(&dc, 0, sizeof(dc));
  dc.ulStructLen = sizeof(dc);
  dc.ulFormatLength = 0;
  ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN);
  if (DIVE_SUCCESS != ret) {
    if (ret == DIVE_ERR_INSUFFICIENT_LENGTH) {
      dc.pFormatData = calloc(dc.ulFormatLength,1);
      ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN); // let's try again
      if (DIVE_SUCCESS != ret) {
        err("Error in call to DiveQueryCaps()");
      }
    } else {
      err("Error in call to DiveQueryCaps()");
    }
  }

  rc1 = DiveOpen(&hDive, FALSE, &apa);

  if (rc1 != DIVE_SUCCESS) {
     if (rc1 == DIVE_ERR_TOO_MANY_INSTANCES)
        {
        logit("Dive error - too many instances");
        }
     if (rc1 == DIVE_ERR_SSMDD_NOT_INSTALLED)
        {
        logit("Dive error - SSMDD.SYS not installed");
        }
     if (rc1 == DIVE_ERR_NO_DIRECT_ACCESS)
        {
        logit("Dive error - no direct access");
        }
     if (rc1 == DIVE_ERR_ALLOCATION_ERROR)
        {
        logit("Dive error - allocation error, check MMOS/2 installed");
        result = -1;
        }
     if (rc1 != DIVE_SUCCESS)
        {
         result = -1;
         free(dc.pFormatData);
         WinPostMsg( hwndClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
         return;
         }
  }

  framebuf = (char*)apa;

   WinQueryWindowRect(HWND_DESKTOP, &rect);
      ret = DiveAcquireFrameBuffer(hDive, &rect);
      if (ret != DIVE_SUCCESS)
         {
          logit("Error in DiveAcquireFrameBuffer()");
          result = -1;
          end_Win(hDive, (char*)dc.pFormatData,hwndClient);
          return;
          }

  framesize = dc.ulVerticalResolution * dc.ulScanLineBytes;
  framecopy = (char*)malloc(framesize);
  if (framecopy == null) {
    logit("Can't get memory for copy of frame buffer");
    result = -1;
    end_Win(hDive, (char*)dc.pFormatData,hwndClient);
    return;
  }

  if (dc.fScreenDirect)
     {                                                /* if direct access allowed */
     if (dc.ulApertureSize >= framesize)
        {                    /* and if video buffer larger or equal to our buffer */
        memcpy(framecopy, framebuf, framesize); /* copy from video buffer to ours */
        }
     else
        {                               /* if our buffer larger than video buffer */
         framecopy1 = framecopy;
         numApertures =  framesize/dc.ulApertureSize;
         for (i = 0; i < numApertures; i++)
            {
             rc1 = DiveSwitchBank(hDive, i);
             if (rc1 != DIVE_SUCCESS)
                {
                logit("DiveSwitchBank 1 returned an error");
                }
             memcpy(framecopy1, framebuf, dc.ulApertureSize);
             framecopy1 += dc.ulApertureSize;
             }
          if (framesize%dc.ulApertureSize)
             {
             rc1 = DiveSwitchBank(hDive, numApertures);
             memcpy(framecopy1, framebuf, framesize%dc.ulApertureSize);
             }
         }        /* end else our buffer > video aperture */
      }           /* end if fScreenDirect */
  else
     {
     logit("No Direct Screen access possible.");
     result = -1;
     end_Win(hDive, (char*)dc.pFormatData,hwndClient);
     return;
     }

  frames = 0;
  sh = dc.ulVerticalResolution;
  scanlen = dc.ulScanLineBytes;

  runs = 10;

  // First, find out the approximate speed of this display
  while (notstop)
    {
    frames = 0;
    t1 = dtime();
    for (i = 0; i < runs; i++)
      {
      frames++;

      if (dc.fScreenDirect)
         {                                                /* if direct access allowed */
         if (dc.ulApertureSize >= framesize)
            {                    /* and if video buffer larger or equal to our buffer */
            memcpy(framebuf, framecopy, framesize); /* copy from video buffer to ours */
            }
         else
            {                               /* if our buffer larger than video buffer */
            framecopy1 = framecopy;
            for (i1 = 0; i1 < framesize/dc.ulApertureSize; i1++)
               {
                rc1 = DiveSwitchBank(hDive, i1);
                if (rc1 != DIVE_SUCCESS)
                   {
                   logit("DiveSwitchBank 1 returned an error");
                   }
                memcpy(framebuf, framecopy+(i1*dc.ulApertureSize), dc.ulApertureSize);
                }
            }
         }
     else
        {
        logit("No Direct Screen access possible.");
        result = -1;
        end_Win(hDive, (char*)dc.pFormatData,hwndClient);
        return;
        }
    }  /* end for i = 0 to runs */
    t2 = dtime();
    test_time = (t2-t1);
    if ((test_time) < MIN_DIVE2_TIME)
       {
       if ((test_time) < MIN_MEASURE)
          {
          runs = MIN_DIVE2_TIME/MIN_MEASURE*runs;
          }
       else
          {
          runs = MIN_DIVE2_TIME*MARGINAL/(test_time)*runs;
          }
       }
     else
        {
        break;
        }
     }

  // speed = ((double) frames * scanlen * sh) / (t2-t1); // MB/s
  speed = ((double)frames)/(test_time); // fps

  sh = dc.ulVerticalResolution;
  sh2 = sh/2;

  phase_steps = .35/speed;
  arg_nrev = 1;

  t1 = dtime();
  frames = 0;

  for (rev = 0; rev < arg_nrev; rev++)
    {
    for (phase = 0.0; phase < 2.0*PI; phase += phase_steps)
      {
      frames++;
      wh2 = sh2 * cos(phase);
      if (wh2 < 0)
        {
        forw = false;
        wh2 = -wh2;
        }
      else
        {
        forw = true;
        }
      wh = wh2 << 1;
      if (forw)
        {
        clr_lines(0, sh2-wh2);
        for (i = 0; i < wh; i++)
            {
            display_lines((u8*)framecopy+((i*sh)/wh)*dc.ulScanLineBytes, i+sh2-wh2, 1);
            }
        clr_lines(sh2+wh2, sh2-wh2);
        }
      else
        {      /* if not forw */
         clr_lines(0, sh2-wh2);
         for (i = 0; i < wh; i++)
             {
             display_lines((u8*)framecopy+(((wh-1-i)*sh)/wh)*dc.ulScanLineBytes, i+sh2-wh2, 1);
             }
         clr_lines(sh2+wh2, sh2-wh2);
         }     /* end else if not forw */
      }        /* end for phase =0  */
  }            /* end for rev = 0 */
  phase_steps *= 1.4; // a little faster than the rotation

  c1 = 24.0 * (double)dc.ulVerticalResolution / 768.0;
  c2 = 2.0 * (double)dc.ulVerticalResolution / 768.0;
  c3 = 4.0/(dc.ulVerticalResolution/6.0/PI);
  c4 = .7/(dc.ulVerticalResolution/6.0/PI);
  c5 = 1.0;
  c6 = 2.3;
  c7 = 0.0;
  c8 = 1.0;
  for (rev = 0; rev < arg_nrev; rev++)
     {
     for (phase = 0.0; phase < 2.0*PI; phase += phase_steps)
        {
        frames++;
        for (yin = 0; yin < sh; yin++)
           {
           double tt, hh;
//           FILE* fp;
//           fp = fopen("dive.dbg", "a+");
//           fprintf(fp, "c5 = %f c7 = %f yin = %f c4 = %f", c5, c7, yin, c4);
//           fclose(fp);
           tt = sin(phase * c5 + c7 + yin * c4);
//           fp = fopen("dive.dbg", "a+");
//           fprintf(fp, "c6 = %f c8 = %f yin = %f c3 = %f", c6, c8, yin, c3);
//           fclose(fp);
           hh = sin(phase * c6 + c8 + yin * c3);
           yout = ((double)yin) + c1 * tt + c2 * hh;
           if (yout < 0 || yout >= dc.ulVerticalResolution)
              {
              clr_lines(yin, 1);
              }
           else
              {
              display_lines2((u8*)framecopy, (u8*)framebuf, yout, yin);
              }
           }
        }
     }

  t2 = dtime();
  test_time = (t2-t1);

  // normalise result to 640 x 480 @ 256 colours with 640 bytes/scanline
  result = ((double) frames * sh * scanlen ) / (test_time * 480 * 640);

  DiveDeacquireFrameBuffer(hDive);

  end_Win(hDive, (char*)dc.pFormatData,hwndClient);

}

 /* _Inline */
void clr_lines(s32 y, s32 lines)
 {
  s32 n /* = (lines*dc.ulScanLineBytes) >> 4 */;
/*   u32* p = (u32*)(framebuf + y*dc.ulScanLineBytes); */
  s32 i, j, k, m, q, framesize, ll;
  APIRET rc;

  framesize = dc.ulVerticalResolution * dc.ulScanLineBytes;
  if (dc.fScreenDirect)
     {                                                /* if direct access allowed */
     if (dc.ulApertureSize >= framesize)
        {                    /* and if video buffer larger or equal to our buffer */

        j = y*dc.ulScanLineBytes;          /* offset into screen image */
        n = lines;
        for (i = 0; i < n; i++)
           {
           memset(framebuf + j, 0, dc.ulScanLineBytes);
           j += dc.ulScanLineBytes;
           }                             /* end for i = 0; i < n  */
        }
     else
        {                 /* otherwise we have a bank switched video card */
        j = y*dc.ulScanLineBytes;          /* offset into screen image */
        k = j/dc.ulApertureSize;          /* k is Aperture number we want */
        DiveSwitchBank(hDive, k);         /* set it active */
        n = lines;
        for (i = 0; i < n; i++)
           {
           m = j/dc.ulApertureSize;        /* current aperture number */
           q = j%dc.ulApertureSize;        /* offset into aperture    */
           if (m != k)
              {                            /* if not same aperture as before */
              rc = DiveSwitchBank(hDive, m);  /* switch to correct aperture */
              if (ll != dc.ulScanLineBytes)
                 {
                 memset(framebuf, 0, dc.ulScanLineBytes-ll);
                 }
              k = m;
              }
           if (q+dc.ulScanLineBytes > dc.ulApertureSize)
              {
              ll = dc.ulApertureSize-q;
              }
           else
              {
              ll = dc.ulScanLineBytes;
              }
           memset(framebuf + q, 0, ll);
           j += dc.ulScanLineBytes;
           }                             /* end for i = 0; i < n  */
        }                                /* end else */
     }                                      /* end if dc.ulScreenDirect */
  }                                         /* end clr_lines */


/* _Inline  */
void display_lines(u8* buf, s32 y, s32 lines)
 {
 s32 framesize, datalen, ApertureNum, ApertureDisp, copylen;
 char* aframe;
 u8* buf1;

  framesize = dc.ulVerticalResolution * dc.ulScanLineBytes;
  if (dc.fScreenDirect)
     {                                                /* if direct access allowed */
     if (dc.ulApertureSize >= framesize)
        {                    /* and if video buffer larger or equal to our buffer */
        memcpy(framebuf + y*dc.ulScanLineBytes, buf, lines*dc.ulScanLineBytes);
        }                    /* just copy from one to the other */
     else
        {                    /* we have a banked switched video card */
        buf1 = buf;
        aframe = framebuf;   /* + (y*dc.ulScanLineBytes); */
        ApertureNum = (y*dc.ulScanLineBytes)/dc.ulApertureSize;
        ApertureDisp = (y*dc.ulScanLineBytes)%dc.ulApertureSize;
        datalen = lines*dc.ulScanLineBytes;
/*        sprintf(szName, "Ap = %d, apdisp= %x, len = %d", ApertureNum, ApertureDisp, datalen);
        logit(szName); */
        while (datalen > 0)
           {
           if (ApertureDisp > 0)
              {
              DiveSwitchBank(hDive, ApertureNum);
              if (datalen > dc.ulApertureSize-ApertureDisp)
                 {
                 copylen = dc.ulApertureSize-ApertureDisp;
                 ApertureNum++;
                 }
              else
                 {
                 copylen = datalen;
                 }
              memcpy(aframe + ApertureDisp, buf1, copylen);
              datalen = datalen - copylen;
              ApertureDisp = 0;
              buf1 = buf1 + copylen;
              }
           else
              {                /* if ApertureDisp = 0 */
              DiveSwitchBank(hDive, ApertureNum);
              if (datalen > dc.ulApertureSize)
                 {
                 copylen = dc.ulApertureSize;
                 ApertureNum++;
                 }
              else
                 {
                 copylen = datalen;
                 }
              memcpy(aframe, buf1, copylen);
              datalen = datalen - copylen;
              buf1 = buf1 + copylen;
              }            /* end if ApertureDisp != 0 */
           }               /* end while datalen > 0  */
        }                  /* end else for banked video card */
     }                     /* end if directScreen */
  }                        /* end function */


inline void display_lines2(u8* src, u8* dest, s32 yin, s32 yout)
 {
 s32  ApertureNum, ApertureDisp, scanlen, framesize, aframe, sl;

 scanlen = dc.ulScanLineBytes;
 framesize = dc.ulVerticalResolution * dc.ulScanLineBytes;

 if (dc.fScreenDirect)
    {                                                /* if direct access allowed */
    if (dc.ulApertureSize >= framesize)
       {                    /* and if video buffer larger or equal to our buffer */
       memcpy(dest + (yout * scanlen), src + (yin * scanlen), scanlen);
       }
    else
       {
       aframe = yout * scanlen;
       ApertureNum = aframe/dc.ulApertureSize;
       ApertureDisp = aframe%dc.ulApertureSize;
       if (dc.ulApertureSize-ApertureDisp < scanlen)
          {
          sl = dc.ulApertureSize-ApertureDisp;
          }
       else
          {
          sl = scanlen;
          }
       DiveSwitchBank(hDive, ApertureNum);
       memcpy(dest + ApertureDisp, src + (yin * scanlen), sl);
       }
    }
}

inline s32 fun(s32 x, double param)
 {
  s32 yout;
  yout = ((double)x) + 20.0 * sin(param+x/(dc.ulVerticalResolution/6.0/PI)) +
         10.0 * sin(param*2.3+1.0+3*x/(dc.ulVerticalResolution/6.0/PI));
  if (yout < 0 || yout >= dc.ulHorizontalResolution)
    yout = -1;
  return yout;
}

// this function is NOT ready! It is under construction
void APIENTRY paint_ms12(ULONG unused) {
  HPS hps;
  POINTL p1, p2;
  double t1, t2, test_time;
  RECTL rc;
  PVOID apa;
  ULONG ret;
  PDIVE_CAPS pdc;
  s32   i, j, c, runs, runs_10, framesize,
        sh, scanlen, frames, numApertures,
        ApertureNum, Datalen, Offset,
        ApertureOffset, RemainLen, z, zz;
  char* framecopy;
  char* framecopy1;
  APIRET rc1 = 0UL;
  int notstop = 1;

  memset(&dc, 0, sizeof(dc));
  dc.ulStructLen = sizeof(dc);
  dc.ulFormatLength = 0;
  ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN);
  if (DIVE_SUCCESS != ret) {
    if (ret == DIVE_ERR_INSUFFICIENT_LENGTH) {
      dc.pFormatData = calloc(dc.ulFormatLength,1);
      ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN); // let's try again
      if (DIVE_SUCCESS != ret) {
        err("Error in call to DiveQueryCaps()");
      }
    } else {
      err("Error in call to DiveQueryCaps()");
    }
  }

  rc1 = DiveOpen(&hDive, FALSE, &apa);

  if (rc1 != DIVE_SUCCESS)
     {
     if (rc1 == DIVE_ERR_TOO_MANY_INSTANCES)
        {
        logit("Dive error - too many instances");
        result = -1;
        }
     if (rc1 == DIVE_ERR_SSMDD_NOT_INSTALLED)
        {
        logit("Dive error - SSMDD.SYS not installed");
        result = -1;
        }
     if (rc1 == DIVE_ERR_NO_DIRECT_ACCESS)
        {
        logit("Dive error - no direct access");
        result = -1;
        }
     if (rc1 == DIVE_ERR_ALLOCATION_ERROR)
        {
        logit("Dive error - allocation error, check MMOS/2 installed");
        result = -1;
        }
     if (rc1 != DIVE_SUCCESS)
        {
        result = -1;
        free(dc.pFormatData);
        WinPostMsg( hwndClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
        return;
        }
     }

  framebuf = (char*)apa;

  WinQueryWindowRect(HWND_DESKTOP, &rect);
     ret = DiveAcquireFrameBuffer(hDive, &rect);
     if (ret != DIVE_SUCCESS)
        {
        logit("Error in DiveAcquireFrameBuffer()");
        result = -1;
        end_Win(hDive, (char*)dc.pFormatData,hwndClient);
        return;
        }

  framesize = dc.ulVerticalResolution * dc.ulScanLineBytes;
  framecopy = (char*)malloc(framesize);
  if (framecopy == null)
     {
     logit("Can't get memory for copy of frame buffer");
     result = -1;
     end_Win(hDive, (char*)dc.pFormatData,hwndClient);
     return;
     }

  if (dc.fScreenDirect)
     {                                                /* if direct access allowed */
     if (dc.ulApertureSize >= framesize)
        {                    /* and if video buffer larger or equal to our buffer */
        memcpy(framecopy, framebuf, framesize); /* copy from video buffer to ours */
        }
     else
        {                               /* if our buffer larger than video buffer */
        framecopy1 = framecopy;
        numApertures =  framesize/dc.ulApertureSize;
        for (i = 0; i < numApertures; i++)
            {
            rc1 = DiveSwitchBank(hDive, i);
            if (rc1 != DIVE_SUCCESS)
               {
               logit("DiveSwitchBank 1 returned an error");
               }
            memcpy(framecopy1, framebuf, dc.ulApertureSize);
            framecopy1 += dc.ulApertureSize;
            }
         if (framesize%dc.ulApertureSize)
            {
            rc1 = DiveSwitchBank(hDive, numApertures);
            memcpy(framecopy1, framebuf, framesize%dc.ulApertureSize);
            }
        }
     }
  else
     {
     logit("No Direct Screen access possible.");
     result = -1;
     end_Win(hDive, (char*)dc.pFormatData,hwndClient);
     return;
     }

  frames = 0;
  sh = dc.ulVerticalResolution;
  scanlen = dc.ulScanLineBytes;

  runs = 200;

  while (notstop)
     {
     frames = 0;
     t1 = dtime();

     if (dc.ulApertureSize >= framesize)
        {                                     /* old style behaviour - copy in one block */
        for (i = 0; i < runs; i++) {
           frames++;
           j = i % (sh-1) + 1;
           memcpy(framebuf, framecopy+(sh-j)*scanlen, j*scanlen);
           memcpy(framebuf + j*scanlen, framecopy, (sh-j)*scanlen);
           }
        }
     else
        {      /* video memory is accessible in multiple banks of y lots of xKb (64Kb?) */
        numApertures = (framesize/dc.ulApertureSize);
        if (framesize%dc.ulApertureSize)
           {
           numApertures++;
           }

        framecopy1 = framecopy;
        for (i = 0; i < runs; i++)
           {
           frames++;
           j = i % (sh-1) + 1;
           Datalen = j * scanlen;
           ApertureNum = 0;
           rc1 = DiveSwitchBank(hDive, ApertureNum);           /* switch to correct bank */
           if (rc1 != DIVE_SUCCESS)
              {
              logit("DiveSwitchBank 2 returned an error");
              }

           Offset = 0;
           while (Datalen > 0)
              {
               if (Datalen < dc.ulApertureSize)
                  {
                  memcpy(framebuf,framecopy+Offset+((sh-j)*scanlen), Datalen);
                  Datalen -= dc.ulApertureSize;
                  }
               else
                  {  /* first part of data > 64Kb */
                  memcpy(framebuf,framecopy+Offset+((sh-j)*scanlen), dc.ulApertureSize);
                  Datalen -= dc.ulApertureSize;
                  ApertureNum++;
                  Offset += dc.ulApertureSize;
                  rc1 = DiveSwitchBank(hDive, ApertureNum);
                  if (rc1 != DIVE_SUCCESS)
                     {
                     logit("DiveSwitchBank 3 returned an error");
                     }
                  }
               }     /* end While (Datalen > 0) */

           Datalen = (sh-j) * scanlen;     /* length of rest of screen */
//          RemainLen = Datalen%dc.ulApertureSize; /* length of data in first block */
           ApertureOffset = (j*dc.ulScanLineBytes)%dc.ulApertureSize; /* length of data in first block */
           RemainLen = dc.ulApertureSize-ApertureOffset; /* offset into aperture to write to */
           if (RemainLen != 0)
              {
              memcpy(framebuf+ApertureOffset, framecopy, RemainLen); /* copy first block */
              ApertureNum++;
              }
           z = 0;
           for (c = ApertureNum; c < numApertures; c++)
              {
               rc1 = DiveSwitchBank(hDive, c);           /* switch to correct bank */
               if (rc1 != DIVE_SUCCESS)
                  {
                  logit("DiveSwitchBank 5 returned an error");
                  }
               if (c == numApertures-1) /* if last frame */
                  {
                  zz = (dc.ulVerticalResolution * dc.ulScanLineBytes) % dc.ulApertureSize;
                  }
               else
                  {
                  zz = dc.ulApertureSize;
                  }
               memcpy(framebuf,framecopy + RemainLen + (z * dc.ulApertureSize) , zz);
               z++;
               }
            }        /* end For (i=0;i<runs)    */
         }           /* end Else if (dc.ulApertureSize >= framesize) */

    t2 = dtime();
    test_time = (t2-t1);

    if ((test_time) < MIN_DIVE_TIME) {
      if ((test_time) < MIN_MEASURE) {
        runs = MIN_DIVE_TIME/MIN_MEASURE*runs;
      } else {
        runs = MIN_DIVE_TIME*MARGINAL/(test_time)*runs;
      }
    } else {
      break;
    }
  }
  // return result normalised for a 640x480 screen @ 256 colours
  result = ((double) frames * scanlen * sh) / (test_time * 480 * 640);

  DiveDeacquireFrameBuffer(hDive);

  end_Win(hDive, (char*)dc.pFormatData,hwndClient);
}

void end_Win(HDIVE hDive, char *p, HWND hwndClient)
{
  DiveClose(hDive);
  free(dc.pFormatData);
  WinPostMsg( hwndClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
}
