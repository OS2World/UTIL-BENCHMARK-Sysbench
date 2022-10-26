
// SysBench graphics test module

#define INCL_WIN
#define INCL_GPI
#define INCL_DOS
#include <os2.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "pmb.h"
#include "types.h"

#define ID_WINDOW1 8000
#define PMB_GFX_CLASS "SysBench gfx winclass"
#define WIN_WIDTH 640
#define WIN_HEIGHT 460
#define MIN_GFX_TIME 10.0
#define MIN_MEASURE 0.1
#define MARGINAL 1.1
#define RUN_NUM 500

//static HAB  hab;

extern void err(char* s);
extern void warn(char* s);
extern void logit(char* s);
extern HAB anchorblock(void);
extern double rtime(void);    // real time in milliseconds
extern double test_time;
extern LONG TitleBarHeight, MenuHeight, SizeBorderSize, BorderSize;
extern int ForeignWarp;

VOID APIENTRY paint_vlines(ULONG unused);
VOID APIENTRY paint_dlines(ULONG unused);
VOID APIENTRY paint_hlines(ULONG unused);
VOID APIENTRY paint_bitblitss(ULONG unused);
VOID APIENTRY paint_bitblitssXOR(ULONG unused);
VOID APIENTRY paint_bitblitms(ULONG unused);
VOID APIENTRY paint_text(ULONG unused);
VOID APIENTRY paint_fillrect(ULONG unused);
VOID APIENTRY paint_patrect(ULONG unused);
static MRESULT EXPENTRY GfxWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
static void Open(void *paintfun);
static void Close(void);
static HWND hwndGClient = NULLHANDLE;         /* Client area window handle    */
static HWND hwndGFrame = NULLHANDLE;          /* Frame window handle          */
static HMQ  hmq;
static ULONG flCreate;                       /* Window creation control flags*/
static HAB bkHab;
static TID paint_tid;
static double result;
int    PutOnTop = 0;
ULONG  WinWidth, WinHeight;

RECTL rctlScreen;

static void Open(void *paintfun) {
  s32 w,h, x,y;
  RECTL rctl;
  QMSG qmsg;                            /* Message from message queue   */
  hwndGClient = NULLHANDLE;         /* Client area window handle    */
  hwndGFrame = NULLHANDLE;          /* Frame window handle          */
//  hab = anchorblock();

  PutOnTop = 0;

  if ((bkHab = WinInitialize(0)) == 0L) /* Initialize PM     */
    err("Can't get anchor block handle for background thread");

  if ((hmq = WinCreateMsgQueue( bkHab, 0 )) == 0L)/* Create a msg queue */
    err("Can't create message queue for graphics test window");

  if (!WinRegisterClass(bkHab,
                       (PSZ)PMB_GFX_CLASS,
                       (PFNWP)GfxWindowProc,
                       CS_SIZEREDRAW |
                       CS_SYNCPAINT,
                       0))
     {
     err("GFX test error: can't register class for child test window");
     }

  flCreate = 0 /* | FCF_TITLEBAR | */ /* FCF_BORDER */ /* | FCF_SYSMENU */;

  if ((hwndGFrame = WinCreateStdWindow(
               HWND_DESKTOP,            /* Desktop window is parent     */
               WS_DISABLED,             /* window styles           */
               &flCreate,               /* Frame control flag           */
               PMB_GFX_CLASS,           /* Client window class name     */
              //0        1         2         3         4         5
              //123456789012345678901234567890123456789012345678901234567890
               "Switch away from this window to terminate the test",    /* window text               */
               0,                       /* No special class style       */
               (HMODULE)0L,             /* Resource is in .EXE file     */
               ID_WINDOW1,              /* Frame window identifier      */
               &hwndGClient             /* Client window handle         */
               )) == 0L)
    err("Can't create graphics test window");

  WinQueryWindowRect(HWND_DESKTOP, &rctlScreen);

  WinWidth  = rctlScreen.xRight-rctlScreen.xLeft;
  WinHeight = rctlScreen.yTop-rctlScreen.yBottom;

  rctl.xLeft = 0;
  rctl.yBottom = 0;
  rctl.xRight = WinWidth;
  rctl.yTop = WinHeight;
  if (!WinCalcFrameRect(hwndGFrame, &rctl, false))
    err("Gfx test: WinCalcFrameRect() error");

  // now adjust position to make it centered on the screen
//  x = ((rctlScreen.xRight-rctlScreen.xLeft+1) - (rctl.xRight-rctl.xLeft+1))/2;
//  y = ((rctlScreen.yTop-rctlScreen.yBottom+1) - (rctl.yTop-rctl.yBottom+1))/2;

//  w = rctl.xRight-rctl.xLeft+1;
//  h = rctl.yTop-rctl.yBottom+1;

/*  x = 0;
  y = 0;
  w = WinWidth+(2*(BorderSize+SizeBorderSize));
  h = WinHeight+(2*(BorderSize+SizeBorderSize)); */
  x = rctl.xLeft;
  y = rctl.yBottom;
  w = rctl.xRight;
  h = rctl.yTop;

  WinSetWindowPos( hwndGFrame,      /* Shows and activates frame    */
                   HWND_TOP,            /* window at position x,y  */
                   x, y, w, h,         /* and size w,h     */
                   SWP_SIZE | SWP_MOVE | SWP_SHOW | SWP_ACTIVATE
                 );

  WinEnableWindowUpdate(hwndGFrame, TRUE);
  WinEnableWindow(hwndGFrame, TRUE);
  WinShowPointer(HWND_DESKTOP, false);

  DosCreateThread(&paint_tid, (PFNTHREAD)paintfun, 0, 0, 64000);

  while( WinGetMsg( bkHab, &qmsg, 0L, 0, 0 ) )
    WinDispatchMsg( bkHab, &qmsg );
}

static void Close(void)
{
//  WinSetVisibleRegionNotify(hwndGFrame, FALSE);
  WinShowPointer(HWND_DESKTOP, true);
  WinDestroyWindow(hwndGFrame);           /* Tidy up...                   */
  WinDestroyMsgQueue( hmq );             /* Tidy up...                   */
  WinTerminate(bkHab);
}

double pmb_gfx_vlines(void) {
  Open((void*)paint_vlines);
  Close();
  return result;
}

double pmb_gfx_dlines(void) {
  Open((void*)paint_dlines);
  Close();
  return result;
}

double pmb_gfx_hlines(void) {
  Open((void*)paint_hlines);
  Close();
  return result;
}

double pmb_gfx_bitblitss(void) {
  Open((void*)paint_bitblitss);
  Close();
  return result;
}

double pmb_gfx_bitblitms(void) {
  Open((void*)paint_bitblitms);
  Close();
  return result;
}

double pmb_gfx_textrender(void) {
  Open((void*)paint_text);
  Close();
  return result;
}

double pmb_gfx_fillrect(void) {
  Open((void*)paint_fillrect);
  Close();
  return result;
}

double pmb_gfx_patrect(void) {
  Open((void*)paint_patrect);
  Close();
  return result;
}

static MRESULT EXPENTRY
GfxWindowProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
//FILE *fp;

// fp = fopen("sbmsg.log", "a+");
// if (fp)
//    {
//    fprintf(fp, "%08x %08x pad mp1 = %08x mp2 = %08x\n", hwnd, msg, mp1, mp2);
//    fclose(fp);
//    }

  switch( msg )
  {
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
      WinEndPaint( hps );                  // Drawing is complete
      }
      break;

//   case WM_VRNENABLED:
//      {
//      if (LONGFROMMP(mp1))
//         {
//         PutOnTop = 1;
//         }
//      }
//      break;

   case WM_SETFOCUS:
      {
      if (SHORT1FROMMP(mp2) == FALSE)       /* if graphics window loses focus */
         {
         PutOnTop = 1;                      /* set to terminate test */
         }
      }
      break;

    case WM_CLOSE:
      exit(1); //WinPostMsg( hwnd, WM_QUIT, (MPARAM)0,(MPARAM)0 );
      break;

    default:
      return WinDefWindowProc( hwnd, msg, mp1, mp2 );
  }
  return (MRESULT)FALSE;
}

VOID APIENTRY paint_vlines(ULONG unused) {
  HPS hps;
  POINTL p1, p2;
  double t1, t2;
  RECTL rc;
  s32 i, j, c, runs, runs_10;
  int notstop = 1;
  APIRET retcode;

  hps = WinGetPS(hwndGClient);
  GpiSetBackMix( hps, BM_OVERPAINT );  // how it mixes,
  GpiSetMix( hps, FM_OVERPAINT );  // how it mixes,
  GpiSetColor(hps, CLR_WHITE);
  GpiSetBackColor( hps, CLR_BLACK);
  rc.xLeft = 0;
  rc.xRight = WinWidth;
  rc.yBottom = 0;
  rc.yTop = WinHeight;
  WinFillRect(hps, &rc, CLR_BLACK);

  DosSleep(200);

  p1.x = WinWidth/2;
  p1.y = 1;
  p2.x = WinWidth/2;
  p2.y = WinHeight-2;
  GpiMove(hps, &p1);
  runs = RUN_NUM;
  runs_10 = runs/10;

  while(notstop)
     {
     j = 0;
     c = 0;
     t1 = rtime();
     for(i = 0; i < runs; i++)
        {
        GpiLine(hps, &p2);  /* draw line from bottom to top */
        GpiLine(hps, &p1);  /* and back again */
        if (j++ == runs_10) /* every 1/10th of the test switch colours */
           {
           GpiSetColor(hps, (c++)%6+1);
           j = 0;
           if (PutOnTop)
              {
              result = -1;
              WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
              DosExit(EXIT_THREAD, 0);
              }
           }
        }

     t2 = rtime();
     test_time = (t2-t1);

     if (test_time < MIN_GFX_TIME)   /* if total time in test < min time */
        {
        if ((test_time) < MIN_MEASURE)  /* if total time in test < 0.1 */
           {
           runs = MIN_GFX_TIME/MIN_MEASURE*runs; /* increase number of runs by factor 100 */
           runs_10 = runs/10;
           }
        else                         /* if 10 > total time in test > 0.1 */
           {
           runs = MIN_GFX_TIME*MARGINAL/(test_time)*runs; /* increase runs by enough to make tottime > 10 */
           runs_10 = runs/10;
           }
        }
     else                              /* if total time in test > 10 */
        {
        break;                          /* stop test */
        }                               /* otherwise repeat with new run # */
     }

  result = (runs*2.0*(WinHeight-2))/(test_time);
  WinReleasePS(hps);
  WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
}

VOID APIENTRY paint_hlines(ULONG unused) {
  HPS hps;
  POINTL p1, p2;
  RECTL rc;
  double t1, t2;
  s32 i, j, c, runs, runs_10;
  int notstop = 1;

  hps = WinGetPS(hwndGClient);
  GpiSetBackMix( hps, BM_OVERPAINT );  // how it mixes,
  GpiSetMix( hps, FM_OVERPAINT );  // how it mixes,
  GpiSetColor(hps, CLR_WHITE);
  GpiSetBackColor( hps, CLR_BLACK);
  rc.xLeft = 0;
  rc.xRight = WinWidth;
  rc.yBottom = 0;
  rc.yTop = WinHeight;
  WinFillRect(hps, &rc, CLR_BLACK);

  DosSleep(200);

  p1.x = 0; // (WinWidth - (WinHeight-2))/2;
  p1.y = WinHeight/2;
  p2.x = WinWidth; // p1.x + WinHeight - 3;
  p2.y = WinHeight/2;
  GpiMove(hps, &p1);
  runs = RUN_NUM;
  runs_10 = runs/10;

  while(notstop)
     {
     j = 0;
     c = 0;
     t1 = rtime();
     for(i = 0; i < runs; i++)
        {
        GpiLine(hps, &p2);
        GpiLine(hps, &p1);
        if (j++ == runs_10)
           {
           GpiSetColor(hps, (c++)%6+1);
           j = 0;
           if (PutOnTop)
              {
              result = -1;
              WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
              DosExit(EXIT_THREAD, 0);
              }
           }
        }
     t2 = rtime();

     test_time = (t2-t1);
     if (test_time < MIN_GFX_TIME)   /* if total time in test < min time */
        {
        if ((test_time) < MIN_MEASURE)  /* if total time in test < 0.1 */
           {
           runs = MIN_GFX_TIME/MIN_MEASURE*runs; /* increase number of runs by factor 100 */
           runs_10 = runs/10;
           }
        else                         /* if 10 > total time in test > 0.1 */
           {
           runs = MIN_GFX_TIME*MARGINAL/(test_time)*runs; /* increase runs by enough to make tottime > 10 */
           runs_10 = runs/10;
           }
        }
     else                              /* if total time in test > 10 */
        {
        break;                          /* stop test */
        }                               /* otherwise repeat with new run # */
     }

  result = (runs*2.0*(p2.x-p1.x))/(test_time);
  WinReleasePS(hps);
  WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
}

VOID APIENTRY paint_dlines(ULONG unused) {
  HPS hps;
  POINTL p1, p2;
  RECTL rc;
  double t1, t2;
  s32 i, j, c, runs, runs_10;
  int notstop = 1;

  hps = WinGetPS(hwndGClient);
  GpiSetBackMix( hps, BM_OVERPAINT );  // how it mixes,
  GpiSetMix( hps, FM_OVERPAINT );  // how it mixes,
  GpiSetColor(hps, CLR_WHITE);
  GpiSetBackColor( hps, CLR_BLACK);

  rc.xLeft = 0;
  rc.xRight = WinWidth;
  rc.yBottom = 0;
  rc.yTop = WinHeight;
  WinFillRect(hps, &rc, CLR_BLACK);
  DosSleep(200);

  p1.x = (WinWidth - (WinHeight-2))/2;
  p1.y = 1 + (WinHeight/4);
  p2.x = p1.x + WinHeight - 3;
  p2.y = WinHeight-2 - WinHeight/4;
  GpiMove(hps, &p1);
  runs = RUN_NUM;
  runs_10 = runs/10;

  while(notstop)
     {
     j = 0;
     c = 0;
     t1 = rtime();
     for(i = 0; i < runs; i++)
        {
        GpiLine(hps, &p2);
        GpiLine(hps, &p1);
        if (j++ == runs_10)
           {
           GpiSetColor(hps, (c++)%6+1);
           j = 0;
           if (PutOnTop)
              {
              result = -1;
              WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
              DosExit(EXIT_THREAD, 0);
              }
           }
        }
     t2 = rtime();
     test_time = (t2-t1);

     if (test_time < MIN_GFX_TIME)   /* if total time in test < min time */
        {
        if ((test_time) < MIN_MEASURE)  /* if total time in test < 0.1 */
           {
           runs = MIN_GFX_TIME/MIN_MEASURE*runs; /* increase number of runs by factor 100 */
           runs_10 = runs/10;
           }
        else                         /* if 10 > total time in test > 0.1 */
           {
           runs = MIN_GFX_TIME*MARGINAL/(test_time)*runs; /* increase runs by enough to make tottime > 10 */
           runs_10 = runs/10;
           }
        }
     else                              /* if total time in test > 10 */
      {
      break;                          /* stop test */
      }                               /* otherwise repeat with new run # */
   }

  result = (runs*2.0*(WinHeight-2))/(test_time);
  WinReleasePS(hps);
  WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
}

// define size of blit rectangle
//#define BLIT_X 303 // not giving any special treatment to even numbered bitblits...
//#define BLIT_Y 303
#define BLIT_X (WinHeight*(63.0/100))
#define BLIT_Y (WinHeight*(63.0/100))

VOID APIENTRY paint_bitblitss(ULONG unused) {
  HPS hps;
  POINTL p1, p2;
  RECTL rc;
  double t1, t2;
  s32 i, x, y, j, c, runs, runs_10;
  POINTL tran[3];
  int notstop = 1;

  hps = WinGetPS(hwndGClient);
  GpiSetBackMix( hps, BM_OVERPAINT );  // how it mixes,
  GpiSetMix( hps, FM_OVERPAINT );  // how it mixes,
  GpiSetColor(hps, CLR_WHITE);
  GpiSetBackColor( hps, CLR_BLACK);

  DosSleep(200);

  runs = RUN_NUM;

  while(notstop)
     {
     // paint background pattern
     rc.xLeft = 0;
     rc.xRight = WinWidth;
     rc.yBottom = 0;
     rc.yTop = WinHeight;
     WinFillRect(hps, &rc, CLR_BLACK);

     GpiSetColor(hps, CLR_PINK);
     for (x = 0; x < (WinWidth+WinHeight); x += 20)
        {
        p1.x = WinWidth;
        p1.y = x;
        if (p1.y > WinHeight)
           {
           p1.x = WinWidth-(x - WinHeight);
           p1.y = WinHeight;
           }
        p2.x = WinWidth-x;
        p2.y = 0;

        GpiMove(hps, &p1);
        GpiLine(hps, &p2);
        }

     GpiSetColor(hps, CLR_YELLOW);
     for (x = 0; x < (WinWidth+WinHeight); x += 20)
        {
        p1.x = x;
        p1.y = 0;
        p2.x = x-(WinHeight-1);
        p2.y = WinHeight-1;
        GpiMove(hps, &p1);
        GpiLine(hps, &p2);
        }

     i = 0;
     t1 = rtime();
     while(notstop)
        {
        tran[0].y = WinHeight-BLIT_Y;
        tran[1].y = WinHeight;
        tran[2].y = WinHeight-BLIT_Y;

        for (x = 1; x <= WinWidth-BLIT_X; x++)
           {
           tran[0].x = x;
           tran[1].x = x + BLIT_X;
           tran[2].x = x-1;
           GpiBitBlt(hps,
                    hps,
                    3,
                    tran,
                    ROP_SRCCOPY,
                    0);
           i++;
           if (i == runs)
              {
              goto done;
              }
           }

        if (PutOnTop)
           {
           result = -1;
           WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
           DosExit(EXIT_THREAD, 0);
           }

        tran[0].x = WinWidth-BLIT_X;
        tran[1].x = WinWidth;
        tran[2].x = WinWidth-BLIT_X;

        for (y = WinHeight-BLIT_Y-1; y >= 0; y--)
           {
           tran[0].y = y;
           tran[1].y = y + BLIT_Y;
           tran[2].y = y+1;
           GpiBitBlt(hps,
                    hps,
                    3,
                    tran,
                    ROP_SRCCOPY,
                    0);
           i++;
           if (i == runs)
              {
              goto done;
              }
           }

        if (PutOnTop)
           {
           result = -1;
           WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
           DosExit(EXIT_THREAD, 0);
           }

        tran[0].y = 0;
        tran[1].y = BLIT_Y;
        tran[2].y = 0;

        for (x = WinWidth-BLIT_X-1; x >= 0; x--)
           {
           tran[0].x = x;
           tran[1].x = x + BLIT_X;
           tran[2].x = x+1;
           GpiBitBlt(hps,
                    hps,
                    3,
                    tran,
                    ROP_SRCCOPY,
                    0);
           i++;
           if (i == runs)
              {
              goto done;
              }
           }

        if (PutOnTop)
           {
           result = -1;
           WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
           DosExit(EXIT_THREAD, 0);
           }

        tran[0].x = 0;
        tran[1].x = BLIT_X;
        tran[2].x = 0;

        for (y = 1; y <= WinHeight-BLIT_Y; y++)
           {
           tran[0].y = y;
           tran[1].y = y + BLIT_Y;
           tran[2].y = y-1;
           GpiBitBlt(hps,
                    hps,
                    3,
                    tran,
                    ROP_SRCCOPY,
                    0);
           i++;
           if (i == runs)
              {
              goto done;
              }
           }

        if (PutOnTop)
           {
           result = -1;
           WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
           DosExit(EXIT_THREAD, 0);
           }
        }      /* end while (notstop) number 1 */

done:
     t2 = rtime();
     test_time = (t2-t1);
     if (test_time < MIN_GFX_TIME)   /* if total time in test < min time */
        {
        if ((test_time) < MIN_MEASURE)  /* if total time in test < 0.1 */
           {
           runs = MIN_GFX_TIME/MIN_MEASURE*runs; /* increase number of runs by factor 100 */
           runs_10 = runs/10;
           }
        else                         /* if 10 > total time in test > 0.1 */
           {
           runs = MIN_GFX_TIME*MARGINAL/(test_time)*runs; /* increase runs by enough to make tottime > 10 */
           runs_10 = runs/10;
           }
        }
     else                              /* if total time in test > 10 */
        {
        break;                          /* stop test */
        }                               /* otherwise repeat with new run # */
     }                          /* end while (notstop) # 2 */

  result = runs/(test_time)*(BLIT_X*BLIT_Y);
  WinReleasePS(hps);
  WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
}


// Memory -> screen bitblit
VOID APIENTRY paint_bitblitms(ULONG unused)
{
  POINTL p1, p2;
  RECTL rc;
  double t1, t2;
  s32 i, x, y, j, c, runs, runs_10;
  POINTL tran[3];
  SIZEL sizl;
  HDC hdc, hdcMemory;
  HPS hps, hpsMemory;
  BITMAPINFOHEADER2 bmih;
  HBITMAP hbm;
  int notstop = 1;

  hps = WinGetPS(hwndGClient);
  GpiSetBackMix( hps, BM_OVERPAINT );  // how it mixes,
  GpiSetMix( hps, FM_OVERPAINT );  // how it mixes,
  GpiSetColor(hps, CLR_WHITE);
  GpiSetBackColor( hps, CLR_BLACK);

  /* Create presentation space for memory image of screen. */
  hdcMemory = DevOpenDC(bkHab,
                       OD_MEMORY,
                       (PSZ) "*",
                       0L,
                       0L,
                       0L);
  sizl.cx = 0;
  sizl.cy = 0;
  hpsMemory = GpiCreatePS(bkHab,
                         hdcMemory,
                         &sizl,
                         PU_PELS | GPIT_MICRO | GPIA_ASSOC | GPIF_DEFAULT);

  /* Create bitmap for memory image of screen. */
  memset(&bmih, 0, sizeof(bmih));
  bmih.cbFix = sizeof(bmih);
  bmih.cx = BLIT_X;
  bmih.cy = BLIT_Y;
  bmih.cPlanes = 1;
  bmih.cBitCount = 8;
  hbm = GpiCreateBitmap(hpsMemory, &bmih, 0L, NULL, NULL);
  GpiSetBitmap(hpsMemory, hbm);

  DosSleep(200);

  runs = RUN_NUM;
  while(notstop)
     {
     // paint background pattern
     rc.xLeft = 0;
     rc.xRight = WinWidth;
     rc.yBottom = 0;
     rc.yTop = WinHeight;
     WinFillRect(hps, &rc, CLR_BLACK);
     GpiSetColor(hps, CLR_GREEN);

     for (x = 0; x < (WinWidth+WinHeight); x += 20)
        {
        p1.x = WinWidth;
        p1.y = x;
        if (p1.y > WinHeight)
           {
           p1.x = WinWidth-(x - WinHeight);
           p1.y = WinHeight;
           }
        p2.x = WinWidth-x;
        p2.y = 0;

        GpiMove(hps, &p1);
        GpiLine(hps, &p2);
        }

     GpiSetColor(hps, CLR_BLUE);
     for (x = 0; x < (WinWidth+WinHeight); x += 20)
        {
        p1.x = x;
        p1.y = 0;
        p2.x = x-(WinHeight-1);
        p2.y = WinHeight-1;
        GpiMove(hps, &p1);
        GpiLine(hps, &p2);
        }

     tran[0].y = 0;
     tran[1].y = BLIT_Y;
     tran[2].y = WinHeight-BLIT_Y;
     tran[0].x = 0;
     tran[1].x = BLIT_X;
     tran[2].x = 0;

     GpiBitBlt(hpsMemory,
              hps,
              3,
              tran,
              ROP_SRCCOPY,
              BBO_IGNORE);

     i = 0;
     t1 = rtime();
     while(notstop)
        {
        tran[0].y = WinHeight-BLIT_Y;
        tran[1].y = WinHeight;
        tran[2].y = 0;

        for (x = 1; x <= WinWidth-BLIT_X; x++)
           {
           tran[0].x = x;
           tran[1].x = x + BLIT_X;
           tran[2].x = 0;
           GpiBitBlt(hps,
                    hpsMemory,
                    3,
                    tran,
                    ROP_SRCCOPY,
                    BBO_IGNORE);
           i++;
           if (i == runs)
              {
              goto done;
              }
           }

        if (PutOnTop)
           {
           result = -1;
           WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
           DosExit(EXIT_THREAD, 0);
           }

        tran[0].x = WinWidth-BLIT_X;
        tran[1].x = WinWidth;
        tran[2].x = 0;
        for (y = WinHeight-BLIT_Y-1; y >= 0; y--)
           {
           tran[0].y = y;
           tran[1].y = y + BLIT_Y;
           tran[2].y = 0;
           GpiBitBlt(hps,
                    hpsMemory,
                    3,
                    tran,
                    ROP_SRCCOPY,
                    BBO_IGNORE);
           i++;
           if (i == runs)
              {
              goto done;
              }
           }

        if (PutOnTop)
           {
           result = -1;
           WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
           DosExit(EXIT_THREAD, 0);
           }

        tran[0].y = 0;
        tran[1].y = BLIT_Y;
        tran[2].y = 0;
        for (x = WinWidth-BLIT_X-1; x >= 0; x--)
           {
           tran[0].x = x;
           tran[1].x = x + BLIT_X;
           tran[2].x = 0;
           GpiBitBlt(hps,
                    hpsMemory,
                    3,
                    tran,
                    ROP_SRCCOPY,
                    BBO_IGNORE);
           i++;
           if (i == runs)
              {
              goto done;
              }
           }

        if (PutOnTop)
           {
           result = -1;
           WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
           DosExit(EXIT_THREAD, 0);
           }

        tran[0].x = 0;
        tran[1].x = BLIT_X;
        tran[2].x = 0;
        for (y = 1; y <= WinHeight-BLIT_Y; y++)
           {
           tran[0].y = y;
           tran[1].y = y + BLIT_Y;
           tran[2].y = 0;
           GpiBitBlt(hps,
                    hpsMemory,
                    3,
                    tran,
                    ROP_SRCCOPY,
                    BBO_IGNORE);
           i++;
           if (i == runs)
              {
              goto done;
              }
           }

        if (PutOnTop)
           {
           result = -1;
           WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
           DosExit(EXIT_THREAD, 0);
           }
        }

done:
     t2 = rtime();
     test_time = (t2-t1);
     if (test_time < MIN_GFX_TIME)   /* if total time in test < min time */
        {
        if ((test_time) < MIN_MEASURE)  /* if total time in test < 0.1 */
           {
           runs = MIN_GFX_TIME/MIN_MEASURE*runs; /* increase number of runs by factor 100 */
           runs_10 = runs/10;
           }
        else                         /* if 10 > total time in test > 0.1 */
           {
           runs = MIN_GFX_TIME*MARGINAL/(test_time)*runs; /* increase runs by enough to make tottime > 10 */
           runs_10 = runs/10;
           }
        }
     else                              /* if total time in test > 10 */
        {
        break;                          /* stop test */
        }                               /* otherwise repeat with new run # */
     }

  result = runs/(test_time)*(BLIT_X*BLIT_Y);
  WinReleasePS(hps);
//  GpiDestroyPS(hpsMemory);
  GpiDestroyPS(hps);
  DevCloseDC(hdcMemory);
  DevCloseDC(hdc);
  GpiDeleteBitmap(hbm);
  WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
}


VOID APIENTRY paint_fillrect(ULONG unused) {
  HPS hps;
  POINTL p1, p2;
  RECTL rc;
  double t1, t2;
  s32 i, j, c, runs, runs_10;
  int notstop = 1;

  hps = WinGetPS(hwndGClient);
  GpiSetBackMix( hps, BM_OVERPAINT );  // how it mixes,
  GpiSetMix( hps, FM_OVERPAINT );  // how it mixes,
  GpiSetColor(hps, CLR_WHITE);
  GpiSetBackColor( hps, CLR_BLACK);
  rc.xLeft = 0;
  rc.xRight = WinWidth;
  rc.yBottom = 0;
  rc.yTop = WinHeight;
  WinFillRect(hps, &rc, CLR_BLACK);

  DosSleep(200);

  p1.x = 1;
  p1.y = 1;
  p2.x = WinWidth-2;
  p2.y = WinHeight-2;
  GpiMove(hps, &p1);
  runs = RUN_NUM;
  runs_10 = runs/10;
  while(notstop)
     {
     j = 0;
     c = 0;
     t1 = rtime();
     for(i = 0; i < runs; i++)
        {
        GpiBox(hps, DRO_FILL, &p2,0,0);
        GpiBox(hps, DRO_FILL, &p1,0,0);
        if (j++ == runs_10)
           {
           GpiSetColor(hps, (c++)%6+1);
           j = 0;
           if (PutOnTop)
              {
              result = -1;
              WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
              DosExit(EXIT_THREAD, 0);
              }
           }
        }
     t2 = rtime();
     test_time = (t2-t1);
     if (test_time < MIN_GFX_TIME)   /* if total time in test < min time */
        {
        if ((test_time) < MIN_MEASURE)  /* if total time in test < 0.1 */
           {
           runs = MIN_GFX_TIME/MIN_MEASURE*runs; /* increase number of runs by factor 100 */
           runs_10 = runs/10;
           }
        else                         /* if 10 > total time in test > 0.1 */
           {
           runs = MIN_GFX_TIME*MARGINAL/(test_time)*runs; /* increase runs by enough to make tottime > 10 */
           runs_10 = runs/10;
           }
        }
     else                              /* if total time in test > 10 */
        {
        break;                          /* stop test */
        }                               /* otherwise repeat with new run # */
     }

  result = runs*2.0/(test_time)*(WinHeight-2)*(WinWidth-2);
  WinReleasePS(hps);
  WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
}

VOID APIENTRY paint_patrect(ULONG unused) {
  HPS hps;
  POINTL p1, p2;
  RECTL rc;
  double t1, t2;
  s32 i, j, c, runs, runs_10;
  int notstop = 1;

  hps = WinGetPS(hwndGClient);
  GpiSetBackMix( hps, BM_OVERPAINT );  // how it mixes,
  GpiSetMix( hps, FM_OVERPAINT );  // how it mixes,
  GpiSetColor(hps, CLR_WHITE);
  GpiSetBackColor( hps, CLR_WHITE);
//  GpiSetPattern(hps, PATSYM_DIAG3);
  GpiSetPattern(hps, PATSYM_HALFTONE);
  rc.xLeft = 0;
  rc.xRight = WinWidth;
  rc.yBottom = 0;
  rc.yTop = WinHeight;
  WinFillRect(hps, &rc, CLR_BLACK);

  DosSleep(200);

  p1.x = 1;
  p1.y = 1;
  p2.x = WinWidth-2;
  p2.y = WinHeight-2;
  GpiMove(hps, &p1);
  runs = RUN_NUM;
  runs_10 = runs/10;

  while(notstop)
     {
     j = 0;
     c = 0;
     t1 = rtime();
     for(i = 0; i < runs; i++)
        {
        GpiBox(hps, DRO_FILL, &p2,0,0);
        GpiBox(hps, DRO_FILL, &p1,0,0);
        if (j++ == runs_10)
           {
           GpiSetColor(hps, (c++)%6+1);
           j = 0;
           if (PutOnTop)
              {
              result = -1;
              WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
              DosExit(EXIT_THREAD, 0);
              }
           }
        }
     t2 = rtime();
     test_time = (t2-t1);
     if (test_time < MIN_GFX_TIME)   /* if total time in test < min time */
        {
        if ((test_time) < MIN_MEASURE)  /* if total time in test < 0.1 */
           {
           runs = MIN_GFX_TIME/MIN_MEASURE*runs; /* increase number of runs by factor 100 */
           runs_10 = runs/10;
           }
        else                         /* if 10 > total time in test > 0.1 */
           {
           runs = MIN_GFX_TIME*MARGINAL/(test_time)*runs; /* increase runs by enough to make tottime > 10 */
           runs_10 = runs/10;
           }
        }
     else                              /* if total time in test > 10 */
        {
        break;                          /* stop test */
        }                               /* otherwise repeat with new run # */
     }

  result = runs*2.0/(test_time)*(WinHeight-2)*(WinWidth-2);
  WinReleasePS(hps);
  WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
}

VOID APIENTRY paint_text(ULONG unused) {
  RECTL rc;
  HPS hps;
  FATTRS fat;
  LONG match;
  FONTMETRICS fmMetrics;
  s32 fontw, fonth, row, col, xoff, yoff, slen;
  RECTL printRect, winRect;
  char* string = "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789-+!@#$%^&*()"
                 "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789-+!@#$%^&*()"
                 "AaBbCcDdEeFfGgH(-;Subliminal Message;-)tUuVvWwXxYyZz0123456789-+!@#$%^&*()"
                 "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789-+!@#$%^&*()"
                 "AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqRrSsTtUuVvWwXxYyZz0123456789-+!@#$%^&*()";
               // 12345678901234567890123456789012345678901234567890123456789012345678901234567890
               // 0        1         2         3         4         5         6         7
  POINTL p1, p2[400];
  double t1, t2;
  s32 i, j, c, runs, runs_10;
  s32 dlen;
  char tmp[256];
  s32 flag;
  int notstop = 1;

  hps = WinGetPS(hwndGClient);
  GpiSetBackMix( hps, BM_OVERPAINT );  // how it mixes,
  GpiSetMix( hps, FM_OVERPAINT );  // how it mixes,
  GpiSetColor(hps, CLR_BLACK);
  GpiSetBackColor( hps, CLR_WHITE);
  rc.xLeft = 0;
  rc.xRight = WinWidth;
  rc.yBottom = 0;
  rc.yTop = WinHeight;
  WinFillRect(hps, &rc, CLR_WHITE);

  WinQueryWindowRect(hwndGClient, &winRect);

  // selecting fonts is a mess...

  fat.usRecordLength = sizeof(FATTRS); /* sets size of structure   */
  fat.fsSelection = 0;         /* uses default selection           */
  fat.lMatch = 0L;             /* does not force match             */
  fat.idRegistry = 0;          /* uses default registry            */
  fat.usCodePage = 850;        /* code-page 850                    */
  fat.lMaxBaselineExt = 16L;   /* requested font height is 12 pels */
  fat.lAveCharWidth = 8L;      /* requested font width is 12 pels  */
  fat.fsType = 0;              /* uses default type                */
  fat.fsFontUse = FATTR_FONTUSE_NOMIX;/* doesn't mix with graphics */

  /* "System Monospaced": valid lAveCharWidth, lMaxBaselineExt pairs: (8,8), (8,12), (8,16), (9,20)
     "System VIO": valid lAveCharWidth, lMaxBaselineExt pairs: (8,16) (8,12) ...
   */

  if (ForeignWarp)
     {
     strcpy(fat.szFacename ,"System Monospaced");
     }
  else
     {
     strcpy(fat.szFacename ,"System VIO");
     }

  match = GpiCreateLogFont(hps,        /* presentation space               */
                           NULL,       /* does not use logical font name   */
                           1L,         /* local identifier                 */
                           &fat);      /* structure with font attributes   */

  // match should now be 2 == FONT_MATCH */

  if (match != 2)
     {
     logit("Can't get the right font, text render benchmark");
     exit(1);
     }

  GpiSetCharSet(hps, 1L);      /* sets font for presentation space */
  GpiQueryFontMetrics ( hps,
                        sizeof ( fmMetrics ) ,
                        &fmMetrics ) ;
  fonth = fmMetrics.lMaxBaselineExt;
  fontw = fmMetrics.lMaxCharInc;

  col = 0;
  row = 0;

  slen = strlen(string);
  p1.x = 2;
  p1.y = fonth;
  DosSleep(200);
  GpiMove(hps, &p1);

  GpiQueryCharStringPos(hps, 0, slen, string, 0, p2);
  for (i = 0; i < slen; i++)
     {
     if (p2[i].x >= (rctlScreen.xRight-rctlScreen.xLeft))
        {
        break;
        }
     }

  slen = i - 1;
  dlen = p2[slen].x - p2[0].x;

  runs = RUN_NUM;
  runs_10 = runs/10;

  xoff = 0;
  yoff = 0;
  c = 0;

  while(notstop)
     {
     t1 = rtime();

     for(i = 0; i < runs; i++)
        {
      //p1.x = xoff++ % 29 + 3;
        p1.x = 2;
        yoff += fonth + 3;
        xoff++;
        xoff = xoff % 74;
      //p1.y = WinHeight - (yoff % (WinHeight - fonth));
        p1.y = WinHeight - (yoff % WinHeight);
        GpiCharStringAt(hps, &p1, slen, string+xoff);
        if ((i%runs_10) == 0)
           {
           if (PutOnTop)
              {
              result = -1;
              WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
              DosExit(EXIT_THREAD, 0);
              }
           }
        }
     xoff = yoff = 0;

     t2 = rtime();
     test_time = (t2-t1);
     if (test_time < MIN_GFX_TIME)   /* if total time in test < min time */
        {
        if ((test_time) < MIN_MEASURE)  /* if total time in test < 0.1 */
           {
           runs = MIN_GFX_TIME/MIN_MEASURE*runs; /* increase number of runs by factor 100 */
           runs_10 = runs/10;
           }
        else                         /* if 10 > total time in test > 0.1 */
           {
           runs = MIN_GFX_TIME*MARGINAL/(test_time)*runs; /* increase runs by enough to make tottime > 10 */
           runs_10 = runs/10;
           }
        }
     else                              /* if total time in test > 10 */
        {
        break;                          /* stop test */
        }                               /* otherwise repeat with new run # */
     }

  result = (double)runs*(double)dlen*(double)fonth/((double)test_time);
  WinReleasePS(hps);
  WinPostMsg( hwndGClient, WM_QUIT, (MPARAM)0,(MPARAM)0 );
}




