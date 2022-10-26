// Sysbench main file
#define INCL_DOS
#define INCL_BASE
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES   /* Semaphore values */
#define INCL_DOSNMPIPES

#define INCL_WIN
#define INCL_PM
#define IDM_RESOURCE 1
#define INCL_GPI
#define INCL_GPILCIDS
#define INCL_GPIPRIMITIVES
#define INCL_GPICONTROL

#include <os2.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys\stat.h>

#include <dive.h>
#include <fourcc.h>
#include "types.h"
#include "pmb.h"
#include "pmbbench.h"
#include "pmbdatat.h"
#include "diskacc2.h"
#include <bsedev.h>

#define CLS_CLIENT          "SysBenchWindowClass"
#define THR_DONE            (WM_USER + 1)
#define THR_UPDATE          (WM_USER + 2)
#define NUM_LINES           (110)
#define LINES_PER_COMPONENT 5
#define LINES_PER_DISK      11
#define LINES_PER_CD        8
#define PIPESIZE            512
#define HF_STDOUT           1      /* Standard output handle */

// function numbers used by pmrmqry.c
#define FUNC_GET_CONTROLLER_NUMBERS 0
#define FUNC_GET_CONTROLLER_NAMES   1
#define FUNC_GET_DISK_AND_CD_NAMES  2

// ********** IMPORTED FUNCTIONS
extern double pmb_diskio_disksize(int nr);
extern int    pmb_diskio_nrdisks(void);
extern double pmb_cdio_disksize(int nDrive);
extern int    pmb_cdio_nrcds(void);
extern void   logit(char* s);
extern void   _Optlink DoFileIOAll(void*);
extern void   _Optlink DoFileIO4(void*);
extern void   _Optlink DoFileIO8(void*);
extern void   _Optlink DoFileIO16(void*);
extern void   _Optlink DoFileIO32(void*);
extern void   _Optlink DoFileIO64(void*);

extern void   _Optlink DoAllCDIOAll(void*);
extern void   _Optlink DoAllCDIO(void*);
extern void   _Optlink DoCDIOAvSeek(void* p);
extern void   _Optlink DoCDIOSpeed(void* p);
extern void   _Optlink DoCDIOTransSpeed(void* p);
extern void   _Optlink DoCDIOCPUUsage(void* p);

extern void   _Optlink DoSimDiskIO(void* p);
extern double CalcSimIOAv(void);
extern void pmcpu3b(void);
extern void pmrmqry(int);


// ********** EXPORTED FUNCTIONS
void err(char* s);
void InfoBox(char* s);
void WarnBox(char* s);
void ErrorBox(char* s);
int gtWarp = 0;
ULONG numcpus = 0;
ULONG debugging = 0;
ULONG debuglvl2 = 0;

ULONG nodetect  = 0;
volatile ULONG PerfSysSup = 0;
ULONG ulDriveNum = 1;
ULONG ulDriveMap = 0;
ULONG ulDriveMap1 = 0;
void AddTitle(char*);
void DelTitle(void);

// ********** LOCAL FUNCTIONS
static void      SetTitle(char*);
static void      UpdateWindow(HPS, PRECTL, s32);
static void      Print(s32, s32, char*, PRECTL,
                       s32, HPS, s32, s32);
static void      PrintTitle(s32, ULONG, char*, PRECTL,
                       s32, HPS, s32);
static void      SetMenuState(bool);
static void      UpdateAll(void);
extern void      SaveResults(void);
extern void      SaveResultsHtml(void);
void _Optlink    ShowWaitWindow(void*);
void _Optlink    Flashlight(void*);
MRESULT APIENTRY fnShowWait(HWND, ULONG, MPARAM, MPARAM);
void _Optlink    GetDriveInfo(void*);
void _Optlink    WakeMeUp(void*);
PSZ              DoScanConfigSys(PSZ, int);
void             GetMachineStuff(HWND);
void             GetVerNum(void);
MRESULT APIENTRY fnMachineStuff   (HWND, ULONG, MPARAM, MPARAM);
MRESULT APIENTRY fnMachdlgStuff   (HWND, ULONG, MPARAM, MPARAM);
MRESULT APIENTRY fnDiskStuff      (HWND, ULONG, MPARAM, MPARAM);
MRESULT APIENTRY fnAboutBox       (HWND, ULONG, MPARAM, MPARAM);
MRESULT EXPENTRY ClientWindowProc (HWND, ULONG, MPARAM, MPARAM);

void ToggleExpand(ULONG);
extern void GetBIOSInfo(void);

ULONG VideoMem = 0;
char  VideoMan[15] = "Unknown";
char  VideoType[15] = "type";
USHORT VideoAdapter, VideoChip;
char  BIOSname[50]  = "Unknown";
char  BusName[9]    = "ISA";

ULONG  BootDriveLetter;
ULONG  action,
       minfree,
       pdisknum,
       swapfilegrown   = 0,
       maxswapfilesize = 0;
float  startsize,
       fatcachesize,
       jfscachesize,
       hpfscachesize,
       hpfs386cachesize;
char   CSpath[_MAX_PATH];
extern ULONG FILESIZE;
BYTE   coproc;
int    fatdisks        = 0,
       hpfsdisks       = 0,
       jfsdisks        = 0,
       curdiskFAT      = 0,
       FastSwap        = 0;

double fatdiskspace    = 0,
       jfsdiskspace    = 0,
       hpfsdiskspace   = 0;
char   Protectonly[10] = "NO";
BOOL   fileiodisabled  = 0;
HWND   hwndDlgB;
HWND   hwndDlgD;

volatile int  flashit  = 0;
HEV    hevEvent2       = 0,
       hevEvent3       = 0;
ULONG  AllExpanded     = 0;


// ********** EXPORTED DATA
double        test_time;
HWND   hwndClient = NULLHANDLE;
static HWND   hwndVertScroll;
static HWND   hwndMenu;
HWND          hwndPage1,
              hwndPage2,
              hwndPage3;
char          invocationpath[CCHMAXPATH];
int           ForeignWarp = 0;

// ********** LOCAL DATA
static bool thread_running;
static s32  fontW;
static s32  fontH;
static HPS  mainHps        = NULLHANDLE;
static HAB  hab            = NULLHANDLE;
static s32 scroll          = 0;
static s32 oldscroll       = 0;
static HWND hwndFrame      = NULLHANDLE;
ULONG sysinfo[QSV_MAXREAL] = {0};
ULONG     vernum[2]        = {0};
char*   pszPrintNewObjects = "PRINT_NEW_OBJECTS";
char*   pszCSDLEVEL        = "CSDLEVEL";
char*   pszFIXLEVEL        = "FIXLEVEL";
char   CSDlevel[20]        = "XR_W1234padpadpadpa";
char   FIXlevel[20]        = "XR_W12345padpadpadp";
ULONG    CSDlevelLen       = 20;
ULONG    FIXlevelLen       = 20;
char    version[10]        = "8.000";
char    vv[5], mmm[5];
char   registered[4] = "(R)";

char pszFullFile[CCHMAXPATH] = "RESULT.TXT";/* File filter string       */
char pszFullFileHTML[CCHMAXPATH] = "RESULT.HTM";/* File filter string       */

HINI hini;
char szIniFileName[CCHMAXPATH] = "SYSBENCH.INI";
char* pszApp          = "SYSBENCH";
char* pszKeyName      = "Name";
char* pszKeyMobo      = "Mobo";
char* pszKeyProcessor = "Processor";
char* pszKeyMake      = "Make";
char* pszKeyCache     = "Cache";
char* pszKeyChipset   = "Chipset";
char* pszKeyGraphics  = "Graphics";
char* pszKeyDisk      = "Disk";
char* pszKeyBeep      = "Beep";
char* pszKeyXpnd      = "Expand";
char* pszVersion      = "Version";

char szDiskname[101];
char pszDisk[20];

char Machinename[100]       = "Unknown machine name";
char Moboname[100]          = "Unknown motherboard make";
DISKCONTROLLER Processor    = {"Unknown processor", ""};
char MachineMake[100]       = "Unknown machine manufacturer";
DISKCONTROLLER CacheAmount  = {"Unknown external cache", ""};
DISKCONTROLLER Chipset      = {"Unknown motherboard chipset", ""};
DISKCONTROLLER Graphicscard = {"Unknown graphics card", ""};
DISKCONTROLLER *DiskController[MAX_CONTROLLERS];
int  StorageControllers = 0;

int  Beep                 = 0,
//     Chipsetided          = 0,
//     Videochipsetided     = 0,
//     Diskcontrollerided   = 0,
     DeleteDisk           = 0,
     Version              = 0;

DISKDESC *szDisk[40];

int NamedC = 0,
    MobodC = 0,
    ChipdC = 0,
    MakedC = 0,
    CachdC = 0,
    ProcdC = 0,
    GrapdC = 0,
    DiskdC = 0,
    DisknC;
int MDataPrompt = 0;
ULONG Machnlen,
      Mobonlen,
      Procnlen,
      Mmaknlen,
      Cachnlen,
      Chipnlen,
      Grapnlen,
      Disknlen,
      BeepnLen,
      Versionlen;

int AutoBench = 0;
int Initialised = 0, inifileexists = 0, ConvertINI = 0;

int disp_lines = NUM_LINES;
LONG TitleBarHeight,
     MenuHeight,
     SizeBorderSize,
     BorderSize;
RECTL       rect;

struct glob_data data = {
  1,            /* selected disk */
  1,            /* number of disks */
  { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 }, /* array of disk sizes */
  1,            /* selected CD */
  1,            /* number of CDs */
  { 100, 100, 100, 100, 100, 100, 100, 100, 100, 100 }, /* CD ROM sizes */
  {
    {
      0,  /* bit settings */
      1,  /* repeat count for group */
      "Graphics",     /* group title */
      8,              /* number of data lines */
      {
        { "BitBlt S->S copy", -1.0, MN, "Million pixels/second" },
        { "BitBlt M->S copy", -1.0, MN, "Million pixels/second" },
        { "Filled Rectangle", -1.0, MN, "Million pixels/second" },
        { "Pattern Fill",     -1.0, MN, "Million pixels/second" },
        { "Vertical Lines",   -1.0, MN, "Million pixels/second" },
        { "Horizontal Lines", -1.0, MN, "Million pixels/second" },
        { "Diagonal Lines",   -1.0, MN, "Million pixels/second" },
        { "Text Render",      -1.0, MN, "Million pixels/second" }
      },
      -1.0,               /* total */
      "PM-Graphics-marks"          /* unit total */
    },
    {
      0,   /* bit settings */
      1,   /* repeat count for group */
      "CPU integer",  /* group title */
      4,              /* number of data lines */
      {
        { "Dhrystone",      -1.0, 10, "VAX 11/780 MIPS equivalent" },
        { "Hanoi",          -1.0, 10, "moves/25 microseconds" },
        { "Heapsort",       -1.0, 10, "Million Instructions Per Second" },
        { "Sieve",          -1.0, 10, "Million Instructions Per Second" }
      },
      -1.0,    /* total */
      "CPU integer-marks"   /* unit total */
    },
    {
      0,    /* bit settings */
      1,     /* repeat count for group */
      "CPU float",  /* group title */
      3,          /* number of data lines */
      {
        { "Linpack",                  -1.0, 100, "MFLOPS" },
        { "Flops",                    -1.0, 10,  "MFLOPS" },
        { "Fast Fourier Transfrm",    -1.0, 100, "VAX FFT's" }
      },
      -1.0,         /* total */
      "CPU floating point-marks"    /* unit total */
    },
    {
      0,    /* bit settings */
      1,    /* repeat count for group */
      "Direct Interface to video extensions - DIVE",   /* group title */
      3,                    /* number of data lines */
      {
        { "Video bus bandwidth",    -1.0, MB, "Megabytes/second" },
        { "DIVE fun",               -1.0, 1, "fps normalised to 640x480x256"  },
        { "M->S, DD,   1.00:1",     -1.0, 1, "fps normalised to 640x480x256"  }
      },
      -1.0,          /* total */
      "DIVE-marks"   /* number of data lines */
    },
     {
      0,        /* bit settings */
      1,        /* repeat count for group */
      "File I/O - Drive C:",    /* group title */
      40,          /* number of data lines */
      {
        { "4Kb seq.   Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "4Kb seq.   Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "4Kb random Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "4Kb random Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "4Kb seq.   Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "4Kb seq.   Cached   r",  -1.0, KB, "Kilobytes/second" },
        { "4Kb random Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "4Kb random Cached   r",  -1.0, KB, "Kilobytes/second" },
        { "8Kb seq.   Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "8Kb seq.   Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "8Kb random Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "8Kb random Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "8Kb seq.   Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "8Kb seq.   Cached   r",  -1.0, KB, "Kilobytes/second" },
        { "8Kb random Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "8Kb random Cached   r",  -1.0, KB, "Kilobytes/second" },
        { "16K seq.   Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "16K seq.   Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "16K random Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "16K random Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "16K seq.   Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "16K seq.   Cached   r",  -1.0, KB, "Kilobytes/second" },
        { "16K random Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "16K random Cached   r",  -1.0, KB, "Kilobytes/second" },
        { "32K seq.   Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "32K seq.   Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "32K random Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "32K random Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "32K seq.   Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "32K seq.   Cached   r",  -1.0, KB, "Kilobytes/second" },
        { "32K random Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "32K random Cached   r",  -1.0, KB, "Kilobytes/second" },
        { "64K seq.   Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "64K seq.   Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "64K random Uncached w",  -1.0, KB, "Kilobytes/second" },
        { "64K random Uncached r",  -1.0, KB, "Kilobytes/second" },
        { "64K seq.   Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "64K seq.   Cached   r",  -1.0, KB, "Kilobytes/second" },
        { "64K random Cached   w",  -1.0, KB, "Kilobytes/second" },
        { "64K random Cached   r",  -1.0, KB, "Kilobytes/second" },
      },
      -1.0,        /* total */
      "File I/O-marks"  /* number of data lines */
    },
    {
      0,            /* bit settings */
      1,            /* repeat count for group */
      "Memory",    /* group title */
      27,          /* number of data lines */
      {
        { "5    kB copy", -1.0, MB, "Megabytes/second" },
        { "10   kB copy", -1.0, MB, "Megabytes/second" },
        { "20   kB copy", -1.0, MB, "Megabytes/second" },
        { "40   kB copy", -1.0, MB, "Megabytes/second" },
        { "80   kB copy", -1.0, MB, "Megabytes/second" },
        { "160  kB copy", -1.0, MB, "Megabytes/second" },
        { "320  kB copy", -1.0, MB, "Megabytes/second" },
        { "640  kB copy", -1.0, MB, "Megabytes/second" },
        { "1280 kB copy", -1.0, MB, "Megabytes/second" },
        { "5    kB read", -1.0, MB, "Megabytes/second" },
        { "10   kB read", -1.0, MB, "Megabytes/second" },
        { "20   kB read", -1.0, MB, "Megabytes/second" },
        { "40   kB read", -1.0, MB, "Megabytes/second" },
        { "80   kB read", -1.0, MB, "Megabytes/second" },
        { "160  kB read", -1.0, MB, "Megabytes/second" },
        { "320  kB read", -1.0, MB, "Megabytes/second" },
        { "640  kB read", -1.0, MB, "Megabytes/second" },
        { "1280 kB read", -1.0, MB, "Megabytes/second" },
        { "5    kB write", -1.0, MB, "Megabytes/second" },
        { "10   kB write", -1.0, MB, "Megabytes/second" },
        { "20   kB write", -1.0, MB, "Megabytes/second" },
        { "40   kB write", -1.0, MB, "Megabytes/second" },
        { "80   kB write", -1.0, MB, "Megabytes/second" },
        { "160  kB write", -1.0, MB, "Megabytes/second" },
        { "320  kB write", -1.0, MB, "Megabytes/second" },
        { "640  kB write", -1.0, MB, "Megabytes/second" },
        { "1280 kB write", -1.0, MB, "Megabytes/second" },
      },
      -1.0,   /* total */
      "Memory-marks"   /* number of data lines */
    },
    {
      0,         /* bit settings */
      1,         /* repeat count for group */
      "Simultaneous Disk I/O", /* group title */
      10,          /* number of data lines */
      {
        { "Disk 1",   -1.0, KB, "Megabytes/second" },
        { "Disk 2",   -1.0, KB, "Megabytes/second" },
        { "Disk 3",   -1.0, KB, "Megabytes/second" },
        { "Disk 4",   -1.0, KB, "Megabytes/second" },
        { "Disk 5",   -1.0, KB, "Megabytes/second" },
        { "Disk 6",   -1.0, KB, "Megabytes/second" },
        { "Disk 7",   -1.0, KB, "Megabytes/second" },
        { "Disk 8",   -1.0, KB, "Megabytes/second" },
        { "Disk 9",   -1.0, KB, "Megabytes/second" },
        { "Disk 10",   -1.0, KB, "Megabytes/second" },
      },
      -1.0,               /* total */
      "Simultaneous I/O-marks"    /* number of data lines */
    },
    {
      0,         /* bit settings */
      1,         /* repeat count for group */
      "Disk I/O", /* group title */
      7,          /* number of data lines */
      {
        { "Avg. data access time",   -1.0, 1.0e-03, "milliseconds" },
        { "Cache/Bus xfer rate  ",   -1.0, MB, "Megabytes/second" },
        { "Trk 0 xfer rate fwds.",   -1.0, MB, "Megabytes/second" },
        { "Middle trk rate fwds.",   -1.0, MB, "Megabytes/second" },
        { "Last trk rate bckwds.",   -1.0, MB, "Megabytes/second" },
        { "Average Transfer rate",   -1.0, MB, "Megabytes/second" },
        { "Disk use CPU load    ",   -1.0, 1, "percent" }
      },
      -1.0,               /* total */
      "Disk I/O-marks"    /* number of data lines */
    },
    {
      0,              /* bit settings */
      1,              /* repeat count for group */
      "CD-ROM I/O",   /* group title */
      4,              /* number of data lines */
      {
        { "Avg. data access time",   -1.0, 1.0e-03, "milliseconds" },
        { "Inner sectors rate",      -1.0, KB, "Kilobytes/second" },
        { "Outer sectors rate",      -1.0, KB, "Kilobytes/second" },
        { "CD-ROM use CPU load  ",   -1.0, 1, "percent" }
      },
      -1.0,                /* total */
      "CD I/O-marks"       /* number of data lines */
    },
  }
};


INT main (int argc, char * argv[])
{
  FATTRS      fat;
  LONG        match;
  HMQ         hmq          = NULLHANDLE;
  HWND        hwndDeskTop;
  ERRORID     erridErrorCode;/* last error id code                   */
  ULONG       flCreate     = 0UL,
              ulPostCount  = 0;
  BOOL        bLoop;
  QMSG        qmsg;
  s32         x, y, w, h, i, j;
  CHAR        tmp[256];
  char* n1;
  int         rc           = 1,
              index        = 0;
  APIRET      ulrc         = 0;
  BOOL        drivechecked = 0;
  DATETIME    datetime     = {0};
  FONTMETRICS fmMetrics ;
  UCHAR       errormsg[CCHMAXPATH];
  HMODULE     modhandle;
  ULONG       perfsysord = 976;
  PFN         modaddr;
  ULONG  aulCpList[8]  = {0},                /* Code page list        */
         ulBufSize     = 8 * sizeof(ULONG),  /* Size of output list   */
         ulListSize    = 0;                  /* Size of list returned */
  setvbuf(stdout, NULL, _IONBF, 0);
  setvbuf(stderr, NULL, _IONBF, 0);

  ulrc = DosQueryCp(ulBufSize,      /* Length of output code page list  */
                    aulCpList,      /* List of code pages               */
                    &ulListSize);   /* Length of list returned          */

  if (ulrc != NO_ERROR)
     {
     printf("DosQueryCp error: return code = %u\n",rc);
     }
  else
     {
     if (aulCpList[0] == 850)
        {
        strcpy(registered, "©");
        }
     else
        {
        strcpy(registered, "(R)");
        }
     }

  strcpy(invocationpath, argv[0]);

  j = strlen(invocationpath);

  for (i = 0; i < j; i++)
     {
     if (invocationpath[j-i] == 92) /* search from last char backwards for \ */
        {
        invocationpath[j-i] = 0;    /* truncate string here */
        break;
        }
     }
  strcpy(szIniFileName, invocationpath);
  strcat(szIniFileName, "\\sysbench.ini");
  strcpy(pszFullFile, invocationpath);
  strcat(pszFullFile, "\\result.txt");
  strcpy(pszFullFileHTML, invocationpath);
  strcat(pszFullFileHTML, "\\result.txt");

  if (argc > 1)
    {                                             /* if any argument passed */
    for(index = 1; index < argc; index++)
       {
       if (strcmpi(argv[index],"/all") == 0)
          {
          AutoBench = 1;                 /* set auto mode on */
          }
       else
          {
          if (strstr(strupr(argv[index]),"/R:"))
             {
             char* n1 = strstr(strupr(argv[index]), "/R:");
             n1 = n1 +3;
             strcpy(pszFullFile, n1);
             }
          else
             {
             if (strstr(strupr(argv[index]),"/H:"))
                {
                char* n1 = strstr(strupr(argv[index]), "/H:");
                n1 = n1 +3;
                strcpy(pszFullFileHTML, n1);
                }
             else
                {
                if (strcmpi(argv[index], "/debug") == 0)
                   {
                   debugging = 1;
                   printf("Sysbench %s started\n", SYSB_VER);
                   }
                else
                   {
                   if (strcmpi(argv[index], "/nodetect") == 0)
                      {
                      nodetect = 1;
                      }
                   else
                      {
                      if (strcmpi(argv[index], "/v") == 0)
                         {
                         debuglvl2 = 1;
                         }
                      }
                   }
                }
             }
          }
       }                                 /* end for */
    }

  for (i = 0; i < 40; i++)
     {
     szDisk[i] = new DISKDESC;
     szDisk[i]->Controller = MAX_CONTROLLERS;
     szDisk[i]->Disknum = 0;
     strcpy(szDisk[i]->desc.user, "Unknown disk");
     strcpy(szDisk[i]->desc.detected, "Unknown disk");
     }

  for (i = 0; i < MAX_CONTROLLERS; i++)
     {
     DiskController[i] = new DISKCONTROLLER;
     strcpy(DiskController[i]->desc.detected, "unknown controller");
     strcpy(DiskController[i]->desc.user, "unknown controller");
     }

  rc = DosQuerySysInfo(QSV_MAX_PATH_LENGTH,      /* get system information */
                      QSV_NUMPROCESSORS,         /* including number of processors */
                      (PVOID)sysinfo,
                      sizeof(ULONG)*QSV_MAXREAL);

  if (rc) /* and since I don't trust IBM, check the rc from that call */
     {
     rc = DosQuerySysInfo(QSV_MAX_PATH_LENGTH,   /* and if it didn't work */
                         QSV_FOREGROUND_PROCESS, /* issue the same one again but for less info */
                         (PVOID)sysinfo,         /* as this should work for Warp 3 and probably 2.1 */
                         sizeof(ULONG)*QSV_MAXREAL);
     }

  vernum[0] = sysinfo[QSV_VERSION_MAJOR - QSV_MAX_PATH_LENGTH];
  vernum[1] = sysinfo[QSV_VERSION_MINOR - QSV_MAX_PATH_LENGTH];

  if ((vernum[0] <= 20) &&
      (vernum[1] <  30))     /* if prior to Warp */
     {
     gtWarp = 0;
     }
  else
     {
     gtWarp = 1;
     }

  CSDlevelLen = 20;
  FIXlevelLen = 20;

  if (!PrfQueryProfileData(HINI_SYSTEMPROFILE, /* get the contents of OS2SYS.INI PRINT_NEW_OBJECTS */
                          pszPrintNewObjects,
                          pszCSDLEVEL,
                          &CSDlevel,
                          &CSDlevelLen))
     {
     strcpy(CSDlevel, "Unknown");   /* fill in with defaults if not found */
     }

  if (!PrfQueryProfileData(HINI_SYSTEMPROFILE, /* get the contents of OS2SYS.INI PRINT_NEW_OBJECTS */
                          pszPrintNewObjects,
                          pszFIXLEVEL,
                          &FIXlevel,
                          &FIXlevelLen))
     {
     strcpy(FIXlevel, "Unknown");   /* fill in with defaults if not found */
     }

  GetVerNum(); // I think this is only needed by the next chunk of code but...
               // do it anyway just in case it's needed by something further down

  if ( (vernum[0] == 20) &&
       (vernum[1] == 30) )
     {
     n1 = strstr(version, ".");
     if (n1)
        {
        strcpy(mmm, n1+1);
        }
     rc = sscanf(version, "%[^.]s", vv);
     if (atoi(vv) <= 8)
        {
        if (strcmp(mmm, "162") <= 0)
           {
           char tmp[500];
           APIRET rc;

           sprintf(tmp, "Due to a bug in RESOURCE.SYS at your level (%s) of the operating system,"
                     " hardware detection will be disabled. Any OS/2 V3 fixpack will correct this. If you"
                     " have replaced the version of RESOURCE.SYS with one with a later date then you may"
                     " reply YES to this prompt to enable detection."
                     "\n\nPlease reply NO if you have not replaced this file - your system will abruptly"
                     " stop with a TRAP 000D and data loss/file corruption may occur if you reply YES.\nver = %s min = %s",
                     version, vv, mmm);

           nodetect = 1;
           hab = WinInitialize(0);
           hmq = WinCreateMsgQueue(hab, 0);

           rc = WinMessageBox(HWND_DESKTOP,
                        HWND_DESKTOP,
                        tmp,
                        "No detection!!",
                        1,
                        MB_YESNO | MB_WARNING | MB_MOVEABLE | MB_DEFBUTTON2);

           if (rc == MBID_YES)
              {
              nodetect = 0;
              }

           WinDestroyMsgQueue(hmq);
           WinTerminate(hab);
           }
        }
     }

  numcpus = sysinfo[QSV_NUMPROCESSORS - QSV_MAX_PATH_LENGTH];
  if (!numcpus)
     {
     numcpus = 1;
     }

  rc = DosLoadModule((PCSZ)errormsg,
                    sizeof(errormsg),
                   "DOSCALL1.DLL",
                   &modhandle);

  if (rc)
     {
     char tmp[60];
     sprintf(tmp, "Return code %d from DosLoadModule for DOSCALL1.DLL", rc);
     hab = WinInitialize(0);
     hmq = WinCreateMsgQueue(hab, 0);

     WinMessageBox(HWND_DESKTOP,
                  HWND_DESKTOP,
                  tmp,
                  "Error!!",
                  1,
                  MB_OK | MB_ERROR | MB_MOVEABLE);

     WinDestroyMsgQueue(hmq);
     WinTerminate(hab);

     exit(EXIT_FAILURE);
     }

  rc = DosQueryProcAddr(modhandle,
                       perfsysord,
                       0,
                       &modaddr);

  DosFreeModule(modhandle);

  if (rc)
     {
     PerfSysSup = 0;
     }
  else
     {
     PerfSysSup = 1;
     }

  rc = DosCreateEventSem(NULL,          /* Unnamed semaphore            */
                        &hevEvent2,     /* Handle of semaphore returned */
                        DC_SEM_SHARED,  /* Indicate a shared semaphore  */
                        FALSE);         /* Put in RESET state           */

  _beginthread(ShowWaitWindow, NULL, START_STACKSIZE, NULL); /* show Wait... window */

  GetMachineInfo();                /* scan CONFIG.SYS for SWAPPATH etc. */

  GetBIOSInfo();

  DosSleep(500);                   /* give time for show wait thread to display dialog */

  ulrc = DosQueryCurrentDisk(&ulDriveNum, &ulDriveMap);

  DosError(FERR_DISABLEHARDERR);

  FastSwap = 0;

  for (i = 2; i <= 25; i++)   /* do from first possible HD upwards */
     {
     if ( ((ulDriveMap << (31-i)) >> 31) ) /* if bit in drive array is on */
        {
        char szDeviceName[3];
        FSINFOBUF    VolumeInfo       = {0};        /* File system info buffer */
        BYTE         fsqBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH)] = {0};
        ULONG        cbBuffer         = sizeof(fsqBuffer);        /* Buffer length) */
        PFSQBUFFER2  pfsqBuffer       = (PFSQBUFFER2) fsqBuffer;
        ULONG        ulOrdinal        = 0;     /* Ordinal of entry in name list      */
        PBYTE        pszFSDName       = NULL;  /* pointer to FS name                 */
        ULONG        aulFSInfoBuf[40] = {0};         /* File system info buffer     */

        ulrc = DosQueryFSInfo(i+1,
                             FSIL_VOLSER,      /* Request volume information */
                             &VolumeInfo,      /* Buffer for information     */
                             sizeof(FSINFOBUF));  /* Size of buffer          */

        if (ulrc != NO_ERROR)
           {
           if (ulrc == ERROR_BAD_NETPATH ||
               ulrc == ERROR_NOT_READY ||
               ulrc == ERROR_GEN_FAILURE ||
               ulrc == ERROR_BAD_NET_NAME)
              {
              continue;
              }
           else
              {
              sprintf(tmp, "DosQueryFSInfo error: return code = %u disk %c:\n", ulrc, 'A'+i);
              logit(tmp);
              continue;
              }
           }

        szDeviceName[0] = 'A'+i;
        szDeviceName[1] = ':';
        szDeviceName[2] = 0;

        ulrc = DosQueryFSAttach(szDeviceName,   /* Logical drive of attached FS      */
                             ulOrdinal,       /* ignored for FSAIL_QUERYNAME       */
                             FSAIL_QUERYNAME, /* Return data for a Drive or Device */
                             pfsqBuffer,      /* returned data                     */
                             &cbBuffer);      /* returned data length              */

        if (ulrc != NO_ERROR)
           {
           sprintf(tmp, "DosQueryFSAttach error: return code = %u\n", ulrc);
           logit(tmp);
           return 1;
           }
        else
           {
           pszFSDName = (PBYTE)(pfsqBuffer->szName + pfsqBuffer->cbName + 1);
           }

        if (!strcmp(pszFSDName, "FAT"))
           {
           fatdisks++;
           curdiskFAT = 1;
           }
        else
           {
           if (!strcmp(pszFSDName, "HPFS"))
              {
              hpfsdisks++;
              curdiskFAT = 2;
              }
           else
              {
              if (!strcmp(pszFSDName, "JFS"))
                 {
                 jfsdisks++;
                 curdiskFAT = 3;
                 }
              else
                 {
                 curdiskFAT = 0; /*  if anything except FAT or HPFS */
                 }
              }
           }

        if (curdiskFAT)
           {
           rc = DosQueryFSInfo(i+1,            /* Drive number      */
                              FSIL_ALLOC,             /* Level 1 allocation info */
                              (PVOID)aulFSInfoBuf,    /* Buffer                  */
                              sizeof(aulFSInfoBuf));  /* Size of buffer          */

           if (rc != NO_ERROR)
              {
              sprintf(tmp, "DosQueryFSInfo error: return code = %u disk %c:\n", rc, 'A'+i);
              logit(tmp);
              return 1;
              }
           else
              {
              HFILE handle = NULLHANDLE;   /* Handle for device */
              ULONG ulCategory = IOCTL_DISK;         /* Device category */
              ULONG ulFunction = DSK_GETDEVICEPARAMS; /* Device-specific function */
              ULONG ulParmLen  = 0;            /* Input and output parameter size */
              ULONG ulDataLen  = 2;            /* Input and output data size */
              ULONG ulAction   = 0;
              BIOSPARAMETERBLOCK bpb;
              struct  biosparms {
                   UCHAR Command;
                   UCHAR rsv;
                   };
              struct biosparms uchParms  = {0, 0};

              if (curdiskFAT == 1)
                 {
                 fatdiskspace = fatdiskspace + (aulFSInfoBuf[1] * aulFSInfoBuf[2] * (USHORT)aulFSInfoBuf[4])/(MB);
                 }
              if (curdiskFAT == 2)
                 {
                 hpfsdiskspace = hpfsdiskspace + (aulFSInfoBuf[1] * aulFSInfoBuf[2] * (USHORT)aulFSInfoBuf[4])/(MB);
                 }
              if (curdiskFAT == 3)
                 {
                 jfsdiskspace = jfsdiskspace + (aulFSInfoBuf[1] * aulFSInfoBuf[2] * (USHORT)aulFSInfoBuf[4])/(MB);
                 }

              rc = DosOpen(szDeviceName,
                          &handle,
                          &action,
                          0,
                          FILE_NORMAL,
                          FILE_OPEN,
                          OPEN_FLAGS_DASD |
                          OPEN_FLAGS_FAIL_ON_ERROR |
                          OPEN_ACCESS_READWRITE |
                          OPEN_SHARE_DENYREADWRITE,
                          0);
              if (rc == NO_ERROR)
                 {
                 rc = DosDevIOCtl(handle,
                                 ulCategory,
                                 ulFunction,
                                 (void*)&uchParms,    /* Input/Output parameter list */
                                 sizeof(uchParms),    /* Maximum output parameter size */
                                 &ulParmLen,          /* Input:  size of parameter list */
                                                      /* Output: size of parameters returned */
                                 &bpb,                /* Input/Output data area */
                                 sizeof(bpb),                   /* Maximum output data size */
                                 &ulDataLen);         /* Input:  size of input data area */
                                                      /* Output: size of data returned   */
                 if (rc == NO_ERROR)
                    {
                    if ((bpb.fsDeviceAttr & (0xffff-0x04)) == bpb.fsDeviceAttr)
                       {
                       if (sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH] > (16 * MB))
                          {
                          FastSwap = 1;
                          if (debugging)
                             {
                             printf("Set flag for >16MB problem\n");
                             }
                          }
                       printf("Device %s does not support >16MB access\n", szDeviceName);
                       }
                    if (debugging)
                       {
                       printf("Device %s, bpb.fsDeviceAttr = %04x\n", szDeviceName, bpb.fsDeviceAttr);
                       }
                    }
                 DosClose(handle);
                 }
              }

           if ((aulFSInfoBuf[1] * aulFSInfoBuf[3] * (USHORT)aulFSInfoBuf[4]) > (10 * MB)) /* if freespace > 10Mb */
              {
              ulDriveMap1 = ulDriveMap1 | (0x80000000 >> 31-i);
              }
           }
        }
     }

  data.nr_cd_drives = pmb_cdio_nrcds();
  if (data.nr_cd_drives > MAX_CD_DRIVES)
     {
     logit("Number of CD drives is too high");
     exit(1);
     }

  for (i = 0; i < data.nr_cd_drives; i++)  /* if we do this now, we spin the CD's up */
     {                                     /* and avoid the wait with a half painted window */
     data.cd_drive_size[i] = pmb_cdio_disksize(i); /* in the GetDriveInfo call later */
     }

  data.nr_fixed_disks = pmb_diskio_nrdisks();

  DosError(FERR_ENABLEHARDERR);

  disp_lines = NUM_LINES + (data.nr_fixed_disks * LINES_PER_DISK) + (data.nr_cd_drives * LINES_PER_CD);
  data.c[comp_disk].nrepeatcount = data.nr_fixed_disks;
  data.c[comp_cd].nrepeatcount   = data.nr_cd_drives;
  data.c[comp_alldisks].ndatalines = data.nr_fixed_disks;

  hab = WinInitialize ( 0UL );

  if (hab == NULLHANDLE)
     {
     erridErrorCode = WinGetLastError(hab);
     printf("WinGetLastError after WinInitialize returned %x\n", erridErrorCode);
     exit(2);
     }

  hmq = WinCreateMsgQueue ( hab, 0UL );

  if (hmq == NULLHANDLE)
     {
     erridErrorCode = WinGetLastError(hab);
     printf("WinGetLastError after WinCreateMsgQueue returned %x\n", erridErrorCode);
     exit(2);
     }

  rc = DosWaitEventSem(hevEvent2, 60000); /* wait 1 minute for flashlight thread to initialise */

  if (rc != NO_ERROR)
     {
     sprintf(tmp, "DosWaitEventSem returned rc = %d", rc);
     logit(tmp);
     return 1;
     }
  else
     {
     WinSendMsg(hwndDlgB, /* simulate pushing the non-existent button */
               WM_COMMAND,
               (MPARAM)MPFROMSHORT(IDD_SHOWWAIT5),
               (MPARAM)MPFROM2SHORT(CMDSRC_PUSHBUTTON, TRUE));
     DosSleep(500);
     rc = DosResetEventSem(hevEvent2,            /* reset him so we can wait again */
                          &ulPostCount);
     }

  rc = WinRegisterClass(hab,
                       CLS_CLIENT,
                       ClientWindowProc,
                       CS_SIZEREDRAW,       /*  CS_SYNCPAINT |  */
                       0UL );

  if ( rc != TRUE )
     {
     logit("WinRegisterClass failed") ;
     exit(4);
     }

  flCreate = FCF_TITLEBAR    |
             FCF_SYSMENU     |
             FCF_SIZEBORDER  |
             FCF_MENU        |
             FCF_MINMAX      |
             FCF_TASKLIST    |
             FCF_NOBYTEALIGN |
             FCF_VERTSCROLL;

  hwndDeskTop = WinQueryDesktopWindow(hab,
                                     NULLHANDLE);

  WinQueryWindowRect(HWND_DESKTOP, &rect);

  w = 636;
  h = (rect.yTop-rect.yBottom) - 40;
  x = (rect.xRight-rect.xLeft)/2 - w/2;
  y = 40;

  hwndFrame = WinCreateStdWindow(hwndDeskTop,
                                WS_ANIMATE,  /* window is invisible but animated */
                                &flCreate,
                                CLS_CLIENT,
                                SYSB_VER,
                                CS_SIZEREDRAW,
                                NULLHANDLE,
                                WND_MAIN,
                                &hwndClient );

  if ( hwndFrame == NULLHANDLE )
     {
     logit("hwndFrame is NULLHANDLE");
     exit(6);
     }

  mainHps = WinGetPS(hwndClient);

  fat.usRecordLength  = sizeof(FATTRS); /* sets size of structure   */
  fat.fsSelection     = 0;              /* uses default selection           */
  fat.lMatch          = 0;              /* does not force match             */
  fat.idRegistry      = 0;              /* uses default registry            */
  fat.usCodePage      = 0;              /* code-page 850                    */
  fat.fsType          = 0;              /* uses default type                */
  fat.fsFontUse       = 0;              /* doesn't mix with graphics */
  fat.lMaxBaselineExt = 12L;            /* requested font height is 12 pels */
  fat.lAveCharWidth   = 8L;             /* requested font width is 8 pels  */
  strcpy(fat.szFacename ,"System VIO");

  match = GpiCreateLogFont(mainHps,        /* presentation space               */
                          NULL,       /* does not use logical font name   */
                          1L,         /* local identifier                 */
                          &fat);      /* structure with font attributes   */

  // match should now be 2 == FONT_MATCH */

   if (match != FONT_MATCH)
      {
      ForeignWarp = 1;                      /* show that we're not on US/UK */
      strcpy(fat.szFacename ,"Courier");    /* try this instead */
      fat.lMaxBaselineExt = 13L;            /* requested font height is 12 pels */
      fat.lAveCharWidth   = 8L;             /* requested font width is 8 pels   */
      fat.fsType          = 0;              /* uses default type                */
      fat.fsFontUse       = 0;              /* doesn't mix with graphics */
      fat.usRecordLength  = sizeof(FATTRS); /* sets size of structure   */
      fat.fsSelection     = 0;              /* uses default selection           */
      fat.lMatch          = 0;              /* does not force match             */
      fat.idRegistry      = 0;              /* uses default registry            */
      fat.usCodePage      = 0;              /* code-page 850                    */
      match = GpiCreateLogFont(mainHps,        /* presentation space               */
                              NULL,       /* does not use logical font name   */
                              1L,         /* local identifier                 */
                              &fat);      /* structure with font attributes   */
      if (match != FONT_MATCH)
         {
         logit("Can't get the right font - tried System VIO and Courier");
         exit(1);
         }
      }

  hwndMenu = WinWindowFromID(hwndFrame, FID_MENU);

  thread_running = true;               /* set drives info running */
  SetTitle("Initialising");
  SetMenuState(false);

  _beginthread(GetDriveInfo, NULL, START_STACKSIZE, NULL); /* get Drive information */

  GpiSetCharSet(mainHps, 1L);      /* sets font for presentation space */

  GpiQueryFontMetrics(mainHps,
                     sizeof ( fmMetrics ) ,
                     &fmMetrics ) ;

  fontH = (14 * fmMetrics.lMaxBaselineExt)/10;
  fontW = fmMetrics.lMaxCharInc;

  GpiSetBackMix( mainHps, BM_OVERPAINT );  // how it mixes,
  GpiSetMix( mainHps, FM_OVERPAINT );  // how it mixes,

  hwndVertScroll = WinWindowFromID( hwndFrame, FID_VERTSCROLL);

  TitleBarHeight = WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR);
  MenuHeight     = WinQuerySysValue(HWND_DESKTOP, SV_CYMENU);
  BorderSize     = WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER);
  SizeBorderSize = WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER);

  i = (1 + ((NUM_COMPONENTS-2) + data.nr_fixed_disks + data.nr_cd_drives) *
        LINES_PER_COMPONENT) * fontH;

  i = i + TitleBarHeight + MenuHeight + (2 * (BorderSize + SizeBorderSize));

  if (i > rect.yTop-rect.yBottom)
     {
     i = (rect.yTop-rect.yBottom)-5;
     }
  h = i;
  y = (rect.yTop-rect.yBottom)-i;

  WinSetWindowPos(hwndFrame,
                 false,
                 x,
                 y,
                 w,
                 h,
                 SWP_SIZE |
                 SWP_MOVE |
                 SWP_SHOW |
                 SWP_ACTIVATE);

  WinQueryWindowRect(hwndClient,
                    &rect);

  WinSendMsg( hwndVertScroll,
              SBM_SETSCROLLBAR,
              MPFROMSHORT(0),
              MPFROM2SHORT(0, (disp_lines * fontH) - (rect.yTop - rect.yBottom)));

  WinSendMsg( hwndVertScroll,
              SBM_SETTHUMBSIZE,
              MPFROM2SHORT(rect.yTop - rect.yBottom, disp_lines * fontH),
              NULL);

  rc = DosCloseEventSem(hevEvent2); /* close and delete semaphore */

  UpdateAll();

  WinEnableWindowUpdate(hwndFrame,
                       TRUE);
  WinEnableWindow (hwndFrame, TRUE);

  if ( hwndFrame != NULLHANDLE )
     {
     bLoop = WinGetMsg ( hab,
                        &qmsg,
                        NULLHANDLE,
                        0,
                        0 );
     while ( bLoop )
        {
        WinDispatchMsg ( hab, &qmsg );
        bLoop = WinGetMsg ( hab,
                           &qmsg,
                           NULLHANDLE,
                           0,
                           0 );
        }

     WinReleasePS(mainHps);
     WinDestroyWindow ( hwndFrame );
     }

  WinDestroyMsgQueue ( hmq );
  WinTerminate ( hab );
  return 0;
}

MRESULT EXPENTRY ClientWindowProc ( HWND hwndWnd,
                                 ULONG ulMsg,
                                 MPARAM mpParm1,
                                 MPARAM mpParm2 )
{
  void *pv = null;
  static bool initialized = false;
  RECTL rect;
  s32 tmp, tmp1;
  bool updateScroll;
  QMSG qmsgPeek;
  bool fDone, exitmsg = 0;
  char tmps[512];
  HAB hab;
  int i, iterations, comp, line, j;
  FILE* fp;
  APIRET rc;
  struct stat sbuffer;
  char savProc[100], savChip[100], savCache[100], savGraphics[100];

  switch ( ulMsg )
     {
     case WM_CREATE:
       /* The client window has been created but is not visible yet.        */
       /* Initialize the window here.                                       */

       j = stat(szIniFileName, &sbuffer);
       if (j) /* if no INI file */
          {
          inifileexists = 0;
          Beep = 0;                                /* fill in with defaults if not found */
          AllExpanded = 0;
          strcpy(Machinename, "Unknown machine");
          strcpy(Moboname, "Unknown motherboard");
          strcpy(MachineMake, "Unknown manufacturer");
          strcpy(Processor.desc.detected, "Unknown processor");
          strcpy(Processor.desc.user, "Unknown processor");
          strcpy(CacheAmount.desc.detected, "Unknown external cache amount");
          strcpy(CacheAmount.desc.user, "Unknown external cache amount");
          strcpy(Chipset.desc.detected, "Unknown motherboard chipset");
          strcpy(Chipset.desc.user, "Unknown motherboard chipset");
          sprintf(Graphicscard.desc.detected, "%s %s %dMB", VideoMan, VideoType, VideoMem/MB);
          sprintf(Graphicscard.desc.user, "%s %s %dMB", VideoMan, VideoType, VideoMem/MB);
          Version = 94;
          }
       else
          {
          inifileexists = 1;
          hab  = WinQueryAnchorBlock( hwndWnd);
          hini = PrfOpenProfile(hab, szIniFileName); /* open our profile file */

          BeepnLen = sizeof(Beep);
          if (!PrfQueryProfileData(hini,             /* get the contents of "beep" field */
                                  pszApp,
                                  pszKeyBeep,
                                  &Beep,
                                  &BeepnLen))
             {
             Beep = 0;   /* fill in with defaults if not found */
             }

          BeepnLen = sizeof(AllExpanded);
          if (!PrfQueryProfileData(hini,             /* get the contents of "expanded" field */
                                  pszApp,
                                  pszKeyXpnd,
                                  &AllExpanded,
                                  &BeepnLen))
             {
             AllExpanded = 0;   /* fill in with defaults if not found */
             }

          Machnlen = sizeof(Machinename);
          if (!PrfQueryProfileData(hini,             /* get the contents of machine name field */
                                  pszApp,
                                  pszKeyName,
                                  &Machinename,
                                  &Machnlen))
             {
             strcpy(Machinename,    "Unknown machine");  /* fill in with defaults if not found */
             MDataPrompt = 1;
             }

          Mobonlen = sizeof(Moboname);
          if (!PrfQueryProfileData(hini,             /* get the contents of motherboard name field */
                                  pszApp,
                                  pszKeyMobo,
                                  &Moboname,
                                  &Mobonlen))
             {
             strcpy(Moboname,       "Unknown motherboard");
             MDataPrompt = 1;
             }

          Mmaknlen = sizeof(MachineMake);
          if (!PrfQueryProfileData(hini,             /* get the contents of machine make field */
                                  pszApp,
                                  pszKeyMake,
                                  &MachineMake,
                                  &Mmaknlen))
             {
             strcpy(MachineMake,    "Unknown manufacturer");
             MDataPrompt = 1;
             }

          Versionlen = sizeof(Version);
          if (!PrfQueryProfileData(hini,             /* get the contents of processor field */
                                  pszApp,
                                  pszVersion,
                                  &Version,
                                  &Versionlen))
             {
             Version = 94;
             ConvertINI = 1;
             }
          else
             {
             ConvertINI = 0;
             }

          if (ConvertINI)
             {
             Procnlen = sizeof(Processor.desc.user);
             if (!PrfQueryProfileData(hini,             /* get the contents of processor field */
                                     pszApp,
                                     pszKeyProcessor,
                                     &Processor.desc.user,
                                     &Procnlen))
                { /* if key not in ini file */
                strcpy(Processor.desc.detected, "Unknown processor");
                strcpy(Processor.desc.user, "Unknown processor");
                }
             else /* if key in old style INI file, copy it */
                {
                strcpy(Processor.desc.detected, Processor.desc.user);
                }
             }
          else
             {
             Procnlen = sizeof(Processor);
             if (!PrfQueryProfileData(hini,             /* get the contents of processor field */
                                     pszApp,
                                     pszKeyProcessor,
                                     &Processor,
                                     &Procnlen))
                {    /* if key not in INI file the set it up */
                strcpy(Processor.desc.detected, "Unknown processor");
                strcpy(Processor.desc.user, "Unknown processor");
                }
             }

          if (ConvertINI)
             {
             Cachnlen = sizeof(CacheAmount.desc.user);
             if (!PrfQueryProfileData(hini,             /* get the contents of cache amount field */
                                     pszApp,
                                     pszKeyCache,
                                     &CacheAmount.desc.user,
                                     &Cachnlen))
                {
                strcpy(CacheAmount.desc.detected, "Unknown external cache amount");
                strcpy(CacheAmount.desc.user, "Unknown external cache amount");
                }
             else
                {
                strcpy(CacheAmount.desc.detected, CacheAmount.desc.user);
                }
             }
          else
             {
             Cachnlen = sizeof(CacheAmount);
             if (!PrfQueryProfileData(hini,             /* get the contents of cache amount field */
                                     pszApp,
                                     pszKeyCache,
                                     &CacheAmount,
                                     &Cachnlen))
                {
                strcpy(CacheAmount.desc.detected, "Unknown external cache amount");
                strcpy(CacheAmount.desc.user, "Unknown external cache amount");
                }
             }

          if (ConvertINI)
             {
             Chipnlen = sizeof(Chipset.desc.user);
             if (!PrfQueryProfileData(hini,             /* get the contents of chipset field */
                                     pszApp,
                                     pszKeyChipset,
                                     &Chipset.desc.user,
                                     &Chipnlen))
                {
                strcpy(Chipset.desc.user, "Unknown motherboard chipset");
                strcpy(Chipset.desc.detected, "Unknown motherboard chipset");
                }
             else
                {
                strcpy(Chipset.desc.detected, Chipset.desc.user);
                }
             }
          else
             {
             Chipnlen = sizeof(Chipset);
             if (!PrfQueryProfileData(hini,             /* get the contents of chipset field */
                                     pszApp,
                                     pszKeyChipset,
                                     &Chipset,
                                     &Chipnlen))
                {
                strcpy(Chipset.desc.user, "Unknown motherboard chipset");
                strcpy(Chipset.desc.detected, "Unknown motherboard chipset");
                }
             }

          if (ConvertINI)
             {
             Grapnlen = sizeof(Graphicscard.desc.user);
             if (!PrfQueryProfileData(hini,             /* get the contents of video card field */
                                     pszApp,
                                     pszKeyGraphics,
                                     &Graphicscard.desc.user,
                                     &Grapnlen))
                {
                sprintf(Graphicscard.desc.detected, "%s %s %dMB", VideoMan, VideoType, VideoMem/MB);
                sprintf(Graphicscard.desc.user, "%s %s %dMB", VideoMan, VideoType, VideoMem/MB);
                }
             else
                {
                sprintf(Graphicscard.desc.detected, Graphicscard.desc.user);
                }
             }
          else
             {
             Grapnlen = sizeof(Graphicscard);
             if (!PrfQueryProfileData(hini,             /* get the contents of video card field */
                                     pszApp,
                                     pszKeyGraphics,
                                     &Graphicscard,
                                     &Grapnlen))
                {
                sprintf(Graphicscard.desc.detected, "%s %s %dMB", VideoMan, VideoType, VideoMem/MB);
                sprintf(Graphicscard.desc.user, "%s %s %dMB", VideoMan, VideoType, VideoMem/MB);
                }
             }

       if (ConvertINI)
          {
          ULONG Keylen;
          char pszDisk[20] = "Disk";

          Keylen = sizeof(DiskController[0]->desc.user);
          if (!PrfQueryProfileData(hini,
                                  pszApp,
                                  pszDisk,
                                  DiskController[0]->desc.user,  /* read INI data into temp area */
                                  &Keylen))
             {
             strcpy(DiskController[0]->desc.detected, "Unknown disk");
             strcpy(DiskController[0]->desc.user, "Unknown disk");
             }
          else
             {
             strcpy(DiskController[0]->desc.detected, DiskController[0]->desc.user);
             DeleteDisk = 1;
             }
          }
       else
          {
          for (i = 0; i <20; i++)
             {
             ULONG Keylen;
             char pszDisk[20];
             sprintf(pszDisk, "Controller%d", i);

             Keylen = sizeof(DISKCONTROLLER);
             if (!PrfQueryProfileData(hini,
                                     pszApp,
                                     pszDisk,
                                     DiskController[i],  /* read INI data into temp area */
                                     &Keylen))
                {
                strcpy(DiskController[i]->desc.detected, "unknown storage controller");
                strcpy(DiskController[i]->desc.user, "unknown storage controller");
                }
             else
                {
                StorageControllers++;
                }
             }
          }

       for (i = 0; i < MAX_FIXED_DISKS+MAX_CD_DRIVES; i++)
          {
          ULONG Keylen;
          char pszDisk[20];
          sprintf(pszDisk, "DiskCD%d", i);

          if (ConvertINI)
             {
             Keylen = sizeof(szDisk[i]->desc.user);
             if (!PrfQueryProfileData(hini,
                                     pszApp,
                                     pszDisk,
                                     szDisk[i]->desc.user,  /* read INI data into temp area */
                                     &Keylen))
                {
                strcpy(szDisk[i]->desc.detected, "Undefined");  /* else set it default */
                strcpy(szDisk[i]->desc.user, "Undefined");  /* else set it default */
                }
             else
                {
                strcpy(szDisk[i]->desc.detected, szDisk[i]->desc.user);  /* else set it default */
                }
             }
          else
             {
             Keylen = sizeof(DISKDESC);
             if (!PrfQueryProfileData(hini,
                                     pszApp,
                                     pszDisk,
                                     szDisk[i],  /* read INI data into temp area */
                                     &Keylen))
                {
                strcpy(szDisk[i]->desc.detected, "Undefined");  /* else set it default */
                strcpy(szDisk[i]->desc.user, "Undefined");  /* else set it default */
                }
             }
          }
       }

    for (i = 0; i < 20; i++)
       {
       szDisk[i+20]->Controller = szDisk[i]->Controller;
       szDisk[i+20]->Disknum = szDisk[i]->Disknum;
       strcpy(szDisk[i+20]->desc.detected, szDisk[i]->desc.detected);
       strcpy(szDisk[i+20]->desc.user, szDisk[i]->desc.user);

       strcpy(DiskController[i+20]->desc.detected, DiskController[i]->desc.detected);
       strcpy(DiskController[i+20]->desc.user, DiskController[i]->desc.user);
       }

    strcpy(savProc, Processor.desc.detected);
    strcpy(savCache, CacheAmount.desc.detected);
    strcpy(savChip, Chipset.desc.detected);
    strcpy(savGraphics, Graphicscard.desc.detected);

    if (!nodetect)
       {
       pmrmqry(FUNC_GET_CONTROLLER_NUMBERS);
       pmrmqry(FUNC_GET_CONTROLLER_NAMES);
       pmrmqry(FUNC_GET_DISK_AND_CD_NAMES);
       pmcpu3b();
       }

    for (i = 0; i < 20; i++)
       {
       if ((szDisk[i+20]->Controller != szDisk[i]->Controller) ||
           (szDisk[i+20]->Disknum != szDisk[i]->Disknum) ||
           (strcmp(szDisk[i+20]->desc.detected, szDisk[i]->desc.detected)))
          {
          MDataPrompt = 1;
          strcpy(szDisk[i]->desc.user, szDisk[i]->desc.detected);
          }
       if ((strcmp(savProc, Processor.desc.detected)))
          {
          MDataPrompt = 1;
          strcpy(Processor.desc.user, Processor.desc.detected);
          }
       if ((strcmp(savCache, CacheAmount.desc.detected)))
          {
          MDataPrompt = 1;
          strcpy(CacheAmount.desc.user, CacheAmount.desc.detected);
          }
       if ((strcmp(savChip, Chipset.desc.detected)))
          {
          MDataPrompt = 1;
          strcpy(Chipset.desc.user, Chipset.desc.detected);
          }
       if ((strcmp(savGraphics, Graphicscard.desc.detected)))
          {
          MDataPrompt = 1;
          strcpy(Graphicscard.desc.user, Graphicscard.desc.detected);
          }
       }

    for (i = 0; i < 20; i++)
       {
       if ((strcmp(DiskController[i+20]->desc.detected, DiskController[i]->desc.detected)))
          {
          MDataPrompt = 1;
          strcpy(DiskController[i]->desc.user, DiskController[i]->desc.detected);
          }
       }

    for (i = i; i <MAX_FIXED_DISKS+MAX_CD_DRIVES; i++)
       {
       strcpy(data.c[comp_disk+i].title, szDisk[i]->desc.user); /* set it up in real list */
       }

    if (nodetect && !inifileexists)
       {
       MDataPrompt = 1;
       }

    rc = PrfCloseProfile(hini);

    if (MDataPrompt)
       {
       WinPostMsg(hwndClient,
                 WM_COMMAND,                        /* and send ourselves a message to display dialog box */
                 (MPARAM)MPFROMSHORT(MI_MACHINE_DATA),  /* to be processed when we're initialised */
                 (MPARAM)0);
       }
  break;

  case WM_COMMAND:

    if (thread_running)
       {
       switch (SHORT1FROMMP(mpParm1))
          {
          case MI_PROJ_QUIT:
             WinPostMsg(hwndWnd, WM_CLOSE, NULL, NULL);
             break;

          case MI_PROJ_ABOUT:
             {
             WinDlgBox(HWND_DESKTOP,
                       hwndClient,
                       fnAboutBox,
                       NULL,
                       IDD_PROD_INFO,
                       NULL);
             }
             break;

          case MI_MACHINE_DATA:
             if (!Initialised)
                {
                _beginthread(WakeMeUp, NULL, START_STACKSIZE, NULL); /* Come back when we are */
                }
             else
                {
                GetMachineStuff(hwndWnd);
                }
             break;

          case MI_PROJ_EXPAND:
             {
             if (AllExpanded)
                {
                AllExpanded = 0;

                iterations = NUM_COMPONENTS + (data.nr_fixed_disks - 1) + (data.nr_cd_drives - 1);
                line = 1;
                for (comp = 0; comp < iterations; comp++)
                   {
                   line++;

                   if (((data.c[comp].bitsettings & (0xffffffff - GROUP_EXPANDED)) != data.c[comp].bitsettings) ||
                       ((data.c[comp].bitsettings & (0xffffffff - GROUP_AUTOEXPANDED)) != data.c[comp].bitsettings) ||
                       (AllExpanded))
                      {
                      line = line + data.c[comp].ndatalines;
                      }
                   line++;
                   line++;
                   line++;
                   line++;
                   }

                WinQueryWindowRect(hwndClient, &rect);

                j = (rect.yTop-rect.yBottom) / fontH;     /* client window size divided by font size */
                j = j + ((scroll + (fontH-1) ) / fontH);  /* plus number of lines missing off top, rounded up */
                j = j - (line - data.c[comp].ndatalines);   /* is # of surplus lines */

                if (j > 0)
                   {
                   scroll = scroll - (j * fontH);
                   }

                WinCheckMenuItem(hwndMenu,
                                MI_PROJ_EXPAND,
                                AllExpanded);
                WinInvalidateRect(hwndClient,
                                 NULL,
                                 FALSE);
                }
             else
                {
                AllExpanded = 1;
                WinCheckMenuItem(hwndMenu,
                                MI_PROJ_EXPAND,
                                AllExpanded);
                WinInvalidateRect(hwndClient,
                                 NULL,
                                 FALSE);
                }
          }
          break;

          }
       return false;
       }

    if ((SHORT1FROMMP(mpParm1) > MI_MENU_DISKIO_SELECT) &&
        (SHORT1FROMMP(mpParm1) <= (data.nr_fixed_disks + MI_MENU_DISKIO_SELECT)))
       {
       s32 i, disk, stuff;
       disk = SHORT1FROMMP(mpParm1) - (MI_MENU_DISKIO_SELECT + 1);
       if (disk == data.selected_disk)
          return false;
       data.selected_disk = disk;
       for (i = 0; i < data.nr_fixed_disks; i++)
          {
          stuff = 0;
          WinCheckMenuItem(hwndMenu,
                          MI_MENU_DISKIO_SELECT + i + 1,
                          stuff);
          }
       stuff = 1;
       WinCheckMenuItem(hwndMenu,
                       MI_MENU_DISKIO_SELECT + data.selected_disk + 1,
                       stuff);
       UpdateAll();
       return false;
       }

    if ((SHORT1FROMMP(mpParm1) > MI_MENU_CDIO_SELECT) &&
        (SHORT1FROMMP(mpParm1) <= (data.nr_cd_drives + MI_MENU_CDIO_SELECT)))
       {
       s32 i, disk, stuff;
       stuff = 0;
       disk = SHORT1FROMMP(mpParm1) - (MI_MENU_CDIO_SELECT + 1);
       if (disk == data.selected_cd)
          return false;
       data.selected_cd = disk;
       for (i = 0; i < data.nr_cd_drives; i++)
          {
          WinCheckMenuItem(hwndMenu,
                          MI_MENU_CDIO_SELECT + i + 1,
                          stuff);
          }
       stuff = 1;
       WinCheckMenuItem(hwndMenu,
                       MI_MENU_CDIO_SELECT + data.selected_cd + 1,
                       stuff);
       UpdateAll();
       return false;
       }


    if ((SHORT1FROMMP(mpParm1) > MI_MENU_FILEIO_SELECT) &&
        (SHORT1FROMMP(mpParm1) <= (MI_MENU_FILEIO_SELECT + 26)))
       {
       ULONG ulDriveNum1;
       int stuff = 0;
       ulDriveNum1 = SHORT1FROMMP(mpParm1) - MI_MENU_FILEIO_SELECT;
       WinCheckMenuItem(hwndMenu,
                       MI_MENU_FILEIO_SELECT + ulDriveNum,
                       stuff);
       stuff = 1;
       WinCheckMenuItem(hwndMenu,
                       MI_MENU_FILEIO_SELECT + ulDriveNum1,
                       stuff);
       ulDriveNum = ulDriveNum1;
       DosSetDefaultDisk(ulDriveNum);
       sprintf(tmps, "File I/O - Drive %c:", 'A'+ ulDriveNum-1);
       strcpy(data.c[comp_file].title, tmps);
       UpdateAll();
       return false;
       }


    switch (SHORT1FROMMP(mpParm1))
       {
       case MI_PROJ_QUIT:
          sprintf(tmps,"");
          if (swapfilegrown)
             {
             sprintf(tmps, "Swapfile was larger than initial allocation after %u of these tests (maximum size was %uKB).\n\n",
                    swapfilegrown,
                    (maxswapfilesize/1024));
             exitmsg = 1;
             }

          if (fatcachesize)                           /* if any FAT cache allocated */
             {
             float ramsize = sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH];
             float cachepercent = ramsize / fatcachesize;
             BOOL cachewarn = 0;

             if (fatdisks)
                {
                if (ramsize >= (4 * MB) &&
                    ramsize <= (8 * MB) &&
                    fatcachesize > (256 * KB))
                    {
                    cachewarn = 1;
                    sprintf(tmps, "%sWith 4-8Mb RAM you might want to set DISKCACHE=256 or less\n\n", tmps);
                    }
                if (ramsize >= ((8 * MB)+1) &&
                    ramsize <= (12 * MB) &&
                    fatcachesize > (384 * KB))
                    {
                    cachewarn = 1;
                    sprintf(tmps, "%sWith 8-12Mb RAM you might want to set DISKCACHE=384 or less\n\n", tmps);
                    }
                if (ramsize >= ((12 * MB)+1) &&
                    ramsize <= (16 * MB) &&
                    fatcachesize > (512 * KB))
                    {
                    cachewarn = 1;
                    sprintf(tmps, "%sWith 12-16Mb RAM you might want to set DISKCACHE=512 or less\n\n", tmps);
                    }
                if (ramsize >= ((16 * MB)+1) &&
                   cachepercent <= 10.1)
                   {
                   cachewarn = 1;
                   sprintf(tmps, "%sYour FAT diskcache is set to 10%% or more of your RAM\n\n", tmps);
                   }
                if (cachewarn)
                   {
                   sprintf(tmps, "%sDISKCACHE is set to %.0fKb of your total %.0uKb RAM\n\n",
                          tmps,
                          (fatcachesize/KB),
                          (sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH]/KB));
                   exitmsg = 1;
                   }
                }
             else
                {
                sprintf(tmps, "%sDISKCACHE is set to %.0fKb but you have no FAT disks\n\n",
                       tmps,
                       (fatcachesize/KB));
                exitmsg = 1;
                }
             }

          if (hpfscachesize)
             {
             float ramsize = sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH];
             float cachepercent = ramsize / hpfscachesize;
             BOOL cachewarn = 0;

             if (hpfsdisks)
                {
                if (ramsize >= (4 * MB) &&
                    ramsize <= (8 * MB) &&
                    (hpfscachesize >= (48 * KB) && hpfscachesize < (128 * KB) ))
                    {
                    cachewarn = 1;
                    sprintf(tmps, "%sWith 4-8Mb RAM you might want to set IFS=HPFS.IFS /CACHE between 48 and 128\n\n", tmps);
                    }
                if (ramsize >= ((8 * MB)+1) &&
                    ramsize <= (12 * MB) &&
                    (hpfscachesize >= (128 * KB) && hpfscachesize < (384 * KB)))
                    {
                    cachewarn = 1;
                    sprintf(tmps, "%sWith 8-12Mb RAM you might want to set IFS=HPFS.IFS /CACHE between 128 and 384\n\n", tmps);
                    }
                if (ramsize >= ((12 * MB)+1) &&
                    ramsize <= (16 * MB) &&
                    (hpfscachesize >= (512 * KB) && hpfscachesize < (1024 * KB)))
                    {
                    cachewarn = 1;
                    sprintf(tmps, "%sWith 12-16Mb RAM you might want to set IFS=HPFS.IFS /CACHE between 512 and 1024\n\n", tmps);
                    }
                if (ramsize >= ((16 * MB)+1) &&
                    ramsize <= (24 * MB) &&
                    (hpfscachesize >= (1024 * KB) && hpfscachesize < (1536 * KB)))
                    {
                    cachewarn = 1;
                    sprintf(tmps, "%sWith 16-24Mb RAM you might want to set IFS=HPFS.IFS /CACHE between 1024 and 1536\n\n", tmps);
                    }
                if (ramsize >= ((24 * MB)+1) &&
                   (hpfscachesize >= (1536 * KB) && hpfscachesize <= (2048 * KB)-1 ))
                   {
                   cachewarn = 1;
                   sprintf(tmps, "%sWith more than 24Mb RAM you might want to set IFS=HPFS.IFS /CACHE between 1536 and 2048\n\n", tmps);
                   }

                if (cachewarn)
                   {
                   sprintf(tmps, "%sHPFS.IFS /CACHE is set to %.0fKb of your total %.0uKb RAM\n\n",
                          tmps,
                          (hpfscachesize/KB),
                          (sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH]/KB));
                   exitmsg = 1;
                   }
                }
             else
                {
                sprintf(tmps, "%sIFS=HPFS.IFS /CACHE is set to %.0fKb but you have no HPFS disks\n\n",
                       tmps,
                       (hpfscachesize/KB));
                exitmsg = 1;
                }
             }

          if (fatdiskspace > hpfsdiskspace)
             {
             if (fatcachesize < hpfscachesize)
                {
                sprintf(tmps, "%sYou have more FAT diskspace than HPFS but your DISKCACHE is less than your HPFS cache.\n\n", tmps);
                }
             }
          if (fatdiskspace < hpfsdiskspace)
             {
             if (fatcachesize > hpfscachesize)
                {
                sprintf(tmps, "%sYou have more HPFS diskspace than FAT but your DISKCACHE is greater than your HPFS cache.\n\n", tmps);
                }
             }

          if (data.c[comp_cpuint].total > 0)
             {
             if (data.c[comp_cpuint].total >= 40 &&
                 sysinfo[QSV_MAX_WAIT - QSV_MAX_PATH_LENGTH] > 2)
                {
                sprintf(tmps, "%sBased on integer score consider setting MAXWAIT=2 instead of current setting of %d\n\n",
                       tmps,
                       sysinfo[QSV_MAX_WAIT - QSV_MAX_PATH_LENGTH]);
                exitmsg = 1;
                }

             if (data.c[comp_cpuint].total >= 70 &&
                 sysinfo[QSV_MAX_WAIT - QSV_MAX_PATH_LENGTH] > 1)
                {
                sprintf(tmps, "%sBased on integer score consider setting MAXWAIT=1 instead of current setting of %d\n\n",
                       tmps,
                       sysinfo[QSV_MAX_WAIT - QSV_MAX_PATH_LENGTH]);
                exitmsg = 1;
                }

             if (data.c[comp_cpuint].total <= 39 &&
                 sysinfo[QSV_MAX_WAIT - QSV_MAX_PATH_LENGTH] < 3)
                {
                sprintf(tmps, "%sBased on integer score consider setting MAXWAIT=3 instead of current setting of %d\n\n",
                       tmps,
                       sysinfo[QSV_MAX_WAIT - QSV_MAX_PATH_LENGTH]);
                exitmsg = 1;
                }
             }

          if (FastSwap)
             {
             if (debugging)
                {
                printf("Found >16Mb problem switch set\n");
                }
             sprintf(tmps, "%sOne or more devices may limit use of RAM to 16MB\n\n", tmps);
             exitmsg = 1;
             }

          if (exitmsg)
             {
             if (!AutoBench)
                {
                InfoBox(tmps); /* display diagnostic messages about system */
                }
             }

          WinPostMsg(hwndWnd, WM_CLOSE, NULL, NULL);
          break;

       case MI_PROJ_ABOUT:
          {
          WinDlgBox(HWND_DESKTOP,
                    hwndClient,
                    fnAboutBox,
                    NULL,
                    IDD_PROD_INFO,
                    NULL);
          }
          break;

       case MI_PROJ_SAVE:
          SaveResults();
          break;

       case MI_PROJ_SAVE_HTML:
          SaveResultsHtml();
          break;

       case MI_PROJ_EXPAND:
          {
          if (AllExpanded)
             {
             AllExpanded = 0;

             iterations = NUM_COMPONENTS + (data.nr_fixed_disks - 1) + (data.nr_cd_drives - 1);
             line = 1;
             for (comp = 0; comp < iterations; comp++)
                {
                line++;
                if (((data.c[comp].bitsettings & (0xffffffff - GROUP_EXPANDED)) != data.c[comp].bitsettings) ||
                    ((data.c[comp].bitsettings & (0xffffffff - GROUP_AUTOEXPANDED)) != data.c[comp].bitsettings) ||
                    (AllExpanded))
                   {
                   line = line + data.c[comp].ndatalines;
                   }
                line++;
                line++;
                line++;
                line++;
                }

             WinQueryWindowRect(hwndClient, &rect);

             j = (rect.yTop-rect.yBottom) / fontH;     /* client window size divided by font size */
             j = j + ((scroll + (fontH-1) ) / fontH);  /* plus number of lines missing off top, rounded up */
             j = j - (line - data.c[comp].ndatalines);   /* is # of surplus lines */

             if (j > 0)
                {
                scroll = scroll - (j * fontH);
                }

             WinCheckMenuItem(hwndMenu,
                             MI_PROJ_EXPAND,
                             AllExpanded);
             WinInvalidateRect(hwndClient,
                              NULL,
                              FALSE);
             }
          else
             {
             AllExpanded = 1;
             WinCheckMenuItem(hwndMenu,
                             MI_PROJ_EXPAND,
                             AllExpanded);
             WinInvalidateRect(hwndClient,
                              NULL,
                              FALSE);
             }
          }
          break;

       case MI_PROJ_ALL:
          SetTitle("Running All tests");
          SetMenuState(false);
          _beginthread(DoAll, null, START_STACKSIZE, pv);
          break;

       case MI_GFX_BITBLIT_SS:
          SetTitle("Running BitBlit S->S");
          SetMenuState(false);
          _beginthread(DoGfxBlitBlitSS, null, START_STACKSIZE, pv);
          break;

       case MI_GFX_BITBLIT_MS:
          SetTitle("Running BitBlit M->S");
          SetMenuState(false);
          _beginthread(DoGfxBlitBlitMS, null, START_STACKSIZE, pv);
          break;

       case MI_GFX_FILLRECT:
          SetTitle("Running Filled Rectangle");
          SetMenuState(false);
          _beginthread(DoGfxFillRect, null, START_STACKSIZE, pv);
          break;

       case MI_GFX_PATFIL:
          SetTitle("Running Pattern Fill");
          SetMenuState(false);
          _beginthread(DoGfxPatFil, null, START_STACKSIZE, pv);
          break;

       case MI_GFX_VLINES:
          SetTitle("Running VerticalLines");
          SetMenuState(false);
          _beginthread(DoGfxVLines, null, START_STACKSIZE, pv);
          break;

       case MI_GFX_HLINES:
          SetTitle("Running HorizontalLines");
          SetMenuState(false);
          _beginthread(DoGfxHLines, null, START_STACKSIZE, pv);
          break;

       case MI_GFX_DLINES:
          SetTitle("Running DiagonalLines");
          SetMenuState(false);
          _beginthread(DoGfxDLines, null, START_STACKSIZE, pv);
          break;

       case MI_GFX_TEXTRENDER:
          SetTitle("Running TextRender");
          SetMenuState(false);
          _beginthread(DoGfxTextRender, null, START_STACKSIZE, pv);
          break;

       case MI_GFX_ALL:
          SetTitle("Running All Graphics tests");
          SetMenuState(false);
          _beginthread(DoAllGraphics, null, START_STACKSIZE, pv);
          break;

       case MI_CPUINT_DHRY:
          SetTitle("Running Dhrystone");
          SetMenuState(false);
          _beginthread(DoCPUIntDhry, null, START_STACKSIZE, pv);
          break;

       case MI_CPUINT_HANOI:
          SetTitle("Running Hanoi");
          SetMenuState(false);
          _beginthread(DoCPUIntHanoi, null, START_STACKSIZE, pv);
          break;

       case MI_CPUINT_HEAPS:
          SetTitle("Running Heapsort");
          SetMenuState(false);
          _beginthread(DoCPUIntHeaps, null, START_STACKSIZE, pv);
          break;

       case MI_CPUINT_SIEVE:
          SetTitle("Running Sieve");
          SetMenuState(false);
          _beginthread(DoCPUIntSieve, null, START_STACKSIZE, pv);
          break;

       case MI_CPUINT_ALL:
          SetTitle("Running All CPU integer tests");
          SetMenuState(false);
          _beginthread(DoAllCPUInt, null, START_STACKSIZE, pv);
          break;

       case MI_CPUFLOAT_LINPACK:
          SetTitle("Running Linpack");
          SetMenuState(false);
          _beginthread(DoCPUFloatLinpack, null, START_STACKSIZE, pv);
          break;

       case MI_CPUFLOAT_FLOPS:
          SetTitle("Running FLOPS");
          SetMenuState(false);
          _beginthread(DoCPUFloatFlops, null, START_STACKSIZE, pv);
          break;

       case MI_CPUFLOAT_FFT:
          SetTitle("Running FFT");
          SetMenuState(false);
          _beginthread(DoCPUFloatFFT, null, START_STACKSIZE, pv);
          break;

       case MI_CPUFLOAT_ALL:
          SetTitle("Running All CPU float tests");
          SetMenuState(false);
          _beginthread(DoAllCPUFloat, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_5:
          SetTitle("Running Memcopy 5kB");
          SetMenuState(false);
          _beginthread(DoMem5, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_10:
          SetTitle("Running Memcopy 10kB");
          SetMenuState(false);
          _beginthread(DoMem10, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_20:
          SetTitle("Running Memcopy 20kB");
          SetMenuState(false);
          _beginthread(DoMem20, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_40:
          SetTitle("Running Memcopy 40kB");
          SetMenuState(false);
          _beginthread(DoMem40, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_80:
          SetTitle("Running Memcopy 80kB");
          SetMenuState(false);
          _beginthread(DoMem80, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_160:
          SetTitle("Running Memcopy 160kB");
          SetMenuState(false);
          _beginthread(DoMem160, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_320:
          SetTitle("Running Memcopy 320kB");
          SetMenuState(false);
          _beginthread(DoMem320, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_640:
          SetTitle("Running Memcopy 640kB");
          SetMenuState(false);
          _beginthread(DoMem640, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_1280:
          SetTitle("Running Memcopy 1280kB");
          SetMenuState(false);
          _beginthread(DoMem1280, null, START_STACKSIZE, pv);
          break;

       case MI_MEMR_5:
          SetTitle("Running Memory Read 5kB");
          SetMenuState(false);
          _beginthread(DoMemR5, null, START_STACKSIZE, pv);
          break;

       case MI_MEMR_10:
          SetTitle("Running Memory Read 10kB");
          SetMenuState(false);
          _beginthread(DoMemR10, null, START_STACKSIZE, pv);
          break;

       case MI_MEMR_20:
          SetTitle("Running Memory Read 20kB");
          SetMenuState(false);
          _beginthread(DoMemR20, null, START_STACKSIZE, pv);
          break;

       case MI_MEMR_40:
          SetTitle("Running Memory Read 40kB");
          SetMenuState(false);
          _beginthread(DoMemR40, null, START_STACKSIZE, pv);
          break;

       case MI_MEMR_80:
          SetTitle("Running Memory Read 80kB");
          SetMenuState(false);
          _beginthread(DoMemR80, null, START_STACKSIZE, pv);
          break;

       case MI_MEMR_160:
          SetTitle("Running Memory Read 160kB");
          SetMenuState(false);
          _beginthread(DoMemR160, null, START_STACKSIZE, pv);
          break;

       case MI_MEMR_320:
          SetTitle("Running Memory Read 320kB");
          SetMenuState(false);
          _beginthread(DoMemR320, null, START_STACKSIZE, pv);
          break;

       case MI_MEMR_640:
          SetTitle("Running Memory Read 640kB");
          SetMenuState(false);
          _beginthread(DoMemR640, null, START_STACKSIZE, pv);
          break;

       case MI_MEMR_1280:
          SetTitle("Running Memory Read 1280kB");
          SetMenuState(false);
          _beginthread(DoMemR1280, null, START_STACKSIZE, pv);
          break;

       case MI_MEMW_5:
          SetTitle("Running Memory Write 5kB");
          SetMenuState(false);
          _beginthread(DoMemW5, null, START_STACKSIZE, pv);
          break;

       case MI_MEMW_10:
          SetTitle("Running Memory Write 10kB");
          SetMenuState(false);
          _beginthread(DoMemW10, null, START_STACKSIZE, pv);
          break;

       case MI_MEMW_20:
          SetTitle("Running Memory Write 20kB");
          SetMenuState(false);
          _beginthread(DoMemW20, null, START_STACKSIZE, pv);
          break;

       case MI_MEMW_40:
          SetTitle("Running Memory Write 40kB");
          SetMenuState(false);
          _beginthread(DoMemW40, null, START_STACKSIZE, pv);
          break;

       case MI_MEMW_80:
          SetTitle("Running Memory Write 80kB");
          SetMenuState(false);
          _beginthread(DoMemW80, null, START_STACKSIZE, pv);
          break;

       case MI_MEMW_160:
          SetTitle("Running Memory Write 160kB");
          SetMenuState(false);
          _beginthread(DoMemW160, null, START_STACKSIZE, pv);
          break;

       case MI_MEMW_320:
          SetTitle("Running Memory Write 320kB");
          SetMenuState(false);
          _beginthread(DoMemW320, null, START_STACKSIZE, pv);
          break;

       case MI_MEMW_640:
          SetTitle("Running Memory Write 640kB");
          SetMenuState(false);
          _beginthread(DoMemW640, null, START_STACKSIZE, pv);
          break;

       case MI_MEMW_1280:
          SetTitle("Running Memory Write 1280kB");
          SetMenuState(false);
          _beginthread(DoMemW1280, null, START_STACKSIZE, pv);
          break;

       case MI_MEM_ALL:
          SetTitle("Running All Memory tests");
          SetMenuState(false);
          _beginthread(DoAllMem, null, START_STACKSIZE, pv);
          break;

       case MI_DIVE_VIDEO_BW:
          SetTitle("Running Video Bandwidth");
          SetMenuState(false);
          _beginthread(DoDiveVBW, null, START_STACKSIZE, pv);
          break;

       case MI_DIVE_ROTATE_SCREEN:
          SetTitle("Running DIVE fun");
          SetMenuState(false);
          _beginthread(DoDiveRot, null, START_STACKSIZE, pv);
          break;

       case MI_DIVE_MS_11:
          SetTitle("Running DIVE M->S 1:1");
          SetMenuState(false);
          _beginthread(DoDiveMS11, null, START_STACKSIZE, pv);
          break;

       case MI_DIVE_ALL:
          SetTitle("Running All DIVE tests");
          SetMenuState(false);
          _beginthread(DoAllDIVE, null, START_STACKSIZE, pv);
          break;

       case MI_DISKIO_AVSEEK:
          SetTitle("Running Average Data Access Time");
          SetMenuState(false);
          _beginthread(DoDiskIOAvSeek, null, START_STACKSIZE, pv);
          break;

       case MI_DISKIO_CBXFER:
          SetTitle("Running Cache/Bus Transfer");
          SetMenuState(false);
          _beginthread(DoDiskCacheXfer, null, START_STACKSIZE, pv);
          break;

       case MI_DISKIO_TRANS_SPEED:
          SetTitle("Running Avg. transfer rate");
          SetMenuState(false);
          _beginthread(DoDiskIOTransSpeed, null, START_STACKSIZE, pv);
          break;

       case MI_DISKIO_CPU_USAGE:
          SetTitle("Running CPU Usage percentage");
          SetMenuState(false);
          _beginthread(DoDiskIOCPUUsage, null, START_STACKSIZE, pv);
          break;

       case MI_DISKIO_ALL:
          SetTitle("Running disk I/O tests");
          SetMenuState(false);
          _beginthread(DoAllDiskIO, null, START_STACKSIZE, pv);
          break;

       case MI_DISKIO_ALL_DISKS:
          SetTitle("Running All disks I/O tests");
          SetMenuState(false);
          _beginthread(DoAllDiskIOAll, null, START_STACKSIZE, pv);
          break;

       case MI_ALL_DISKS:
          SetTitle("Running Simultaneous I/O tests");
          SetMenuState(false);
          _beginthread(DoSimDiskIO, null, START_STACKSIZE, pv);
          break;

       case MI_FILEIO_ALL:
          SetTitle("Running All file I/O tests");
          SetMenuState(false);
          _beginthread(DoFileIOAll, null, START_STACKSIZE, pv);
          break;

       case MI_FILEIO_4KB:
          SetTitle("Running 4KB file I/O tests");
          SetMenuState(false);
          _beginthread(DoFileIO4, null, START_STACKSIZE, pv);
          break;

       case MI_FILEIO_8KB:
          SetTitle("Running 8KB file I/O tests");
          SetMenuState(false);
          _beginthread(DoFileIO8, null, START_STACKSIZE, pv);
          break;

       case MI_FILEIO_16KB:
          SetTitle("Running 16KB file I/O tests");
          SetMenuState(false);
          _beginthread(DoFileIO16, null, START_STACKSIZE, pv);
          break;

       case MI_FILEIO_32KB:
          SetTitle("Running 32KB file I/O tests");
          SetMenuState(false);
          _beginthread(DoFileIO32, null, START_STACKSIZE, pv);
          break;

       case MI_FILEIO_64KB:
          SetTitle("Running 64KB file I/O tests");
          SetMenuState(false);
          _beginthread(DoFileIO64, null, START_STACKSIZE, pv);
          break;

       case MI_CDIO_AVSEEK:
          SetTitle("Running CD Seek Time");
          SetMenuState(false);
          _beginthread(DoCDIOAvSeek, null, START_STACKSIZE, pv);
          break;

       case MI_CDIO_TRANS_INNER:
          SetTitle("Running CD transfer rate");
          SetMenuState(false);
          _beginthread(DoCDIOSpeed, null, START_STACKSIZE, pv);
          break;

       case MI_CDIO_CPU_USAGE:
          SetTitle("Running CD CPU percentage");
          SetMenuState(false);
          _beginthread(DoCDIOCPUUsage, null, START_STACKSIZE, pv);
          break;

       case MI_CDIO_ALL:
          SetTitle("Running CD I/O tests");
          SetMenuState(false);
          _beginthread(DoAllCDIO, null, START_STACKSIZE, pv);
          break;

       case MI_CDIO_ALL_DRIVES:
          SetTitle("Running All drives CD test");
          SetMenuState(false);
          _beginthread(DoAllCDIOAll, null, START_STACKSIZE, pv);
          break;

       case MI_MACHINE_DATA:
          GetMachineStuff(hwndWnd);
          break;

       case MI_PROJ_BEEP:
          if (!WinIsMenuItemChecked(hwndMenu, MI_PROJ_BEEP))
             {
             int stuff = 1;
             WinCheckMenuItem(hwndMenu, MI_PROJ_BEEP, stuff);
             Beep = 1;
             }
          else
             {
             int stuff = 0;
             WinCheckMenuItem(hwndMenu, MI_PROJ_BEEP, stuff);
             Beep = 0;
             }
          break;
       }
       break;

  case WM_PAINT:
     {
     FATTRS      fat;
     LONG        match;
     FONTMETRICS fmMetrics ;
     HPS               hpsPaint ;
     RECTL             rclRect ;
     RECTL             rclWindow ;
     ULONG             ulCharHeight ;
     HWND              hwndEnum ;
     HWND              hwndFrame ;
     HENUM             heEnum ;
     POINTL            point;
     HRGN              newHrgn, oldHrgn, dummy;
     s32               complexity;
     RECTL             clipRect;

     if (!initialized)
        {
        initialized = true;
        }

     hpsPaint = WinBeginPaint ( hwndWnd,
                             NULLHANDLE,
                             &rclRect ) ;

     GpiQueryClipBox(hpsPaint, &clipRect);
     clipRect.xRight++;
     clipRect.yTop++;
     newHrgn = GpiCreateRegion(mainHps, 1, &clipRect);
     GpiSetClipRegion(mainHps, newHrgn, &oldHrgn);
     if (NULLHANDLE != oldHrgn)
        {
        GpiDestroyRegion(mainHps, oldHrgn);
        }

     WinFillRect(mainHps, &rclRect, CLR_BLACK);

     WinQueryWindowRect ( hwndWnd, &rclWindow );
     UpdateWindow(mainHps,
                 &rclWindow,
                 scroll);

     oldscroll = scroll;
     WinEndPaint(hpsPaint);
     }
     break;

  case THR_DONE:
     SetTitle("Ready");
     Initialised = 1;
     SetMenuState(true);
     thread_running = false;
     if (AutoBench)
        {
        if (AutoBench == 2)
           {
           WinPostMsg(hwndClient,
                     WM_COMMAND,
                     (MPARAM)MPFROMSHORT(MI_PROJ_SAVE),
                     (MPARAM)MPFROM2SHORT(CMDSRC_MENU, TRUE));
           AutoBench++;
           }
        }
     break;

  case THR_UPDATE:
     UpdateAll();
     if (AutoBench)
        {
        if (AutoBench == 1)
           {
           WinPostMsg(hwndClient,
                     WM_COMMAND,
                     (MPARAM)MPFROMSHORT(MI_PROJ_ALL),
                     (MPARAM)MPFROM2SHORT(CMDSRC_MENU, TRUE));
           AutoBench++;
           }
        }
     break;

  case WM_SIZE:
     {
     s32 tmp;

     WinQueryWindowRect(hwndClient, &rect);

     tmp = disp_lines * fontH - (rect.yTop - rect.yBottom);

     if (tmp < 0)
        tmp = 0;
     if (scroll > tmp)
        scroll = tmp;

     WinSendMsg( hwndVertScroll,
               SBM_SETSCROLLBAR,
               MPFROMSHORT(scroll),
               MPFROM2SHORT(0, tmp));

     WinSendMsg( hwndVertScroll,
               SBM_SETTHUMBSIZE,
               MPFROM2SHORT(rect.yTop - rect.yBottom, disp_lines * fontH),
               NULL);
     }
     break;

  case WM_VSCROLL:
     updateScroll = false;
     WinQueryWindowRect(hwndClient, &rect);
     switch( SHORT2FROMMP( mpParm2) )
        {
        case SB_LINEUP:
          scroll -= fontH;
          updateScroll = true;;
          break;

        case SB_LINEDOWN:
          scroll += fontH;
          updateScroll = true;;
          break;

        case SB_PAGEUP:
          scroll -= rect.yTop-rect.yBottom;
          updateScroll = true;;
          break;

        case SB_PAGEDOWN:
          scroll += rect.yTop-rect.yBottom;
          updateScroll = true;;
          break;

        case SB_SLIDERTRACK:
        case SB_SLIDERPOSITION:
           scroll = SHORT1FROMMP( mpParm2);
           break;

        case SB_ENDSCROLL:
           break;
        default:
           break;
        }

    if (updateScroll)
       {
       WinQueryWindowRect(hwndClient, &rect);
       tmp = disp_lines * fontH - (rect.yTop - rect.yBottom);
       if (tmp < 0)
          tmp = 0;
       if (scroll > tmp)
          scroll = tmp;
       WinSendMsg( hwndVertScroll,
                  SBM_SETSCROLLBAR,
                  MPFROMSHORT(scroll),
                  MPFROM2SHORT(0, tmp));
       WinSendMsg( hwndVertScroll,
                  SBM_SETTHUMBSIZE,
                  MPFROM2SHORT(rect.yTop - rect.yBottom, disp_lines * fontH),
                  NULL);
       }

    tmp = disp_lines * fontH - (rect.yTop - rect.yBottom);
    if (scroll > tmp)
       scroll = tmp;
    if (scroll < 0)
       scroll = 0;

    WinScrollWindow(hwndClient,
                   0,
                   scroll-oldscroll,
                   NULL,
                   NULL,
                   NULLHANDLE,
                   NULL,
                   SW_INVALIDATERGN);

    WinUpdateWindow(hwndClient);
    break;

  case WM_ERASEBACKGROUND:
     return MRFROMSHORT ( false );  // No, we'll do this ourselves
     break;

  case WM_BUTTON1CLICK:
     {
     POINTL mousepos;
     SWP    windowpos, cwindowpos;
     ULONG  linenum, iterations, comp, line = 0, i;
     int    j;
     RECTL  rect;

     WinQueryMsgPos(hab,
                   &mousepos);

     WinQueryWindowPos(hwndFrame,   /* get frame window pos on desktop */
                      &windowpos);

     windowpos.cy = windowpos.cy - TitleBarHeight;
     windowpos.cy = windowpos.cy - MenuHeight;
     windowpos.cy = windowpos.cy - (2 * (BorderSize + SizeBorderSize));

     mousepos.x = mousepos.x - windowpos.x; /* get mouse pos within my window */
     mousepos.y = mousepos.y - windowpos.y;
     mousepos.y = windowpos.cy - mousepos.y;

     linenum = ((mousepos.y + scroll) / fontH);      /* get which line was clicked */

     iterations = NUM_COMPONENTS + (data.nr_fixed_disks - 1) + (data.nr_cd_drives - 1);

     for (comp = 0; comp < iterations; comp++)
        {
        line++;

        if (line == linenum)
           break;

        if (((data.c[comp].bitsettings & (0xffffffff - GROUP_EXPANDED)) != data.c[comp].bitsettings) ||
            ((data.c[comp].bitsettings & (0xffffffff - GROUP_AUTOEXPANDED)) != data.c[comp].bitsettings) ||
            (AllExpanded))
           {
           line = line + data.c[comp].ndatalines;
           if (line >= linenum)
              {
              comp = iterations+1; /* get out of loop */
              break;
              }
           }
        line++;
        line++;
        line++;
        line++;
        }

     if (comp < iterations)
        {
        if ((data.c[comp].bitsettings & (0xffffffff - GROUP_EXPANDED)) != data.c[comp].bitsettings)
           {
           data.c[comp].bitsettings = data.c[comp].bitsettings & (0xffffffff - GROUP_EXPANDED);

           WinQueryWindowRect(hwndClient, &rect);

           j = (rect.yTop-rect.yBottom) / fontH;     /* client window size divided by font size */

           j = j + ((scroll + (fontH-1) ) / fontH);  /* plus number of lines missing off top, rounded up */

           j = j - (disp_lines - data.c[comp].ndatalines);   /* is # of surplus lines */

           if (j > 0)
              {
              scroll = scroll - (j * fontH);
              }
           }
        else
           {
           data.c[comp].bitsettings = data.c[comp].bitsettings | GROUP_EXPANDED;
           }
        WinInvalidateRect(hwndClient,
                         NULL,
                         FALSE);
        }

     return WinDefWindowProc ( hwndWnd,
                              ulMsg,
                              mpParm1,
                              mpParm2 );
     }
     break;

  case WM_BUTTON2CLICK:
     {
     POINTL mousepos;
     SWP    windowpos, cwindowpos;
     ULONG  linenum, iterations, comp, line = 0, i;
     int    j;
     RECTL  rect;

     WinQueryMsgPos(hab,
                   &mousepos);

     WinQueryWindowPos(hwndFrame,   /* get frame window pos on desktop */
                      &windowpos);

     windowpos.cy = windowpos.cy - TitleBarHeight;
     windowpos.cy = windowpos.cy - MenuHeight;
     windowpos.cy = windowpos.cy - (2 * (BorderSize + SizeBorderSize));

     mousepos.x = mousepos.x - windowpos.x; /* get mouse pos within my window */
     mousepos.y = mousepos.y - windowpos.y;
     mousepos.y = windowpos.cy - mousepos.y;

     linenum = ((mousepos.y + scroll) / fontH);      /* get which line was clicked */

     iterations = NUM_COMPONENTS + (data.nr_fixed_disks - 1) + (data.nr_cd_drives - 1);

     for (comp = 0; comp < iterations; comp++)
        {
        line++;

        if (line == linenum)
           break;

        if (((data.c[comp].bitsettings & (0xffffffff - GROUP_EXPANDED)) != data.c[comp].bitsettings) ||
            ((data.c[comp].bitsettings & (0xffffffff - GROUP_AUTOEXPANDED)) != data.c[comp].bitsettings) ||
            (AllExpanded))
           {
           line = line + data.c[comp].ndatalines;
           if (line >= linenum)
              {
              comp = iterations+1; /* get out of loop */
              break;
              }
           }
        line++;
        line++;
        line++;
        line++;
        }

     if (comp < iterations)
        {
        if (comp >= (NUM_COMPONENTS-2))
           {
           char *szDiskname = (char*)malloc(101);
           char* n1;
//           HWND hwndDlgD;
           SHORT sTextLength;
           int i, j;
           APIRET rc;
           WINPARM *winparm1;

           winparm1 = new WINPARM;

           i = comp - (NUM_COMPONENTS-2);

           if (i >= data.nr_fixed_disks)
              winparm1->number = i + 10 - data.nr_fixed_disks;       /* it's a CD */
           else
              winparm1->number = i;            /* otherwise it's a disk */

           n1 = strstr(data.c[comp_disk+i].title, " - ");

           strcpy(szDisk[winparm1->number]->desc.user, n1+3);
           winparm1->desc = szDisk[winparm1->number]->desc.user;

           hwndDlgD = WinLoadDlg(HWND_DESKTOP,
                                hwndClient,
                                fnDiskStuff,
                                NULLHANDLE,
                                IDD_DISK,
                                &winparm1);
           winparm1->hwnd = (HWND)hwndDlgD;

           rc = WinProcessDlg(hwndDlgD);
           if (winparm1->number >= 10)
              {
              i = winparm1->number + data.nr_fixed_disks - 10;
              sprintf(data.c[comp_disk+i].title, "CD-ROM I/O disk %d-%d: %5.0f MB - %s",
                     szDisk[winparm1->number]->Controller-1,
                     winparm1->number-9,
                     data.cd_drive_size[winparm1->number-10]/(KB),
                     szDisk[winparm1->number]->desc.user);
              }
           else
              {
              i = winparm1->number;
              sprintf(data.c[comp_disk+i].title, "Disk I/O disk %d-%d: %5.0f MB - %s",
                     szDisk[i]->Controller-1,
                     i+1,
                     data.fixed_disk_size[i]/(KB),
                     szDisk[i]->desc.user);
              }
           hini = PrfOpenProfile(hab, szIniFileName); /* open our profile file */

           sprintf(pszDisk, "DiskCD%d", winparm1->number); /* set key name */

           if (Version >= 94)
              {
              PrfWriteProfileData(hini,
                                 pszApp,
                                 pszDisk,
                                 szDisk[winparm1->number],
                                 sizeof(DISKDESC));
              }
           else
              {
              PrfWriteProfileData(hini,
                                 pszApp,
                                 pszDisk,
                                 &szDisk[winparm1->number]->desc.user,
                                 101);
              }

           rc = PrfCloseProfile(hini);

           delete winparm1;
           free(szDiskname);

           WinInvalidateRect(hwndClient,
                            NULL,
                            FALSE);
           }
        }

     return WinDefWindowProc ( hwndWnd,
                              ulMsg,
                              mpParm1,
                              mpParm2 );
     }
     break;

  case WM_CHAR:
     if (!(CHARMSG(&ulMsg)->fs & KC_KEYUP))
        {
        switch (CHARMSG(&ulMsg)->vkey)
           {
           case VK_UP:
           case VK_DOWN:
           case VK_PAGEUP:
           case VK_PAGEDOWN:
              return WinSendMsg(hwndVertScroll, ulMsg, mpParm1, mpParm2);
           }
        }
     break;

  case WM_CLOSE:
     {
     hini = PrfOpenProfile(hab, szIniFileName); /* open our profile file */

     PrfWriteProfileData(hini,
                        pszApp,
                        pszKeyBeep,
                        &Beep,
                        sizeof(Beep));
     PrfWriteProfileData(hini,
                        pszApp,
                        pszKeyXpnd,
                        &AllExpanded,
                        sizeof(AllExpanded));
     PrfWriteProfileData(hini,
                        pszApp,
                        pszVersion,
                        &Version,
                        sizeof(Versionlen));
     if (DeleteDisk)
        {
        PrfWriteProfileData(hini,
                           pszApp,
                           "Disk",
                           NULL,
                           NULL);
        }
     rc = PrfCloseProfile(hini);

     return WinDefWindowProc ( hwndWnd,
                              ulMsg,
                              mpParm1,
                              mpParm2 );
     }
     break;

  default:
     return WinDefWindowProc ( hwndWnd,
                              ulMsg,
                              mpParm1,
                              mpParm2 );
  }

  return MRFROMSHORT ( FALSE );
}

void PostFin(int onlyupdate)
{
if (onlyupdate)
   {
   WinPostMsg(hwndClient, THR_UPDATE, NULL, NULL);
   }
else
   {
   WinPostMsg(hwndClient, THR_DONE, NULL, NULL);
   }
}

void err(char* s)
{
logit(s);
ErrorBox(s);
exit(1);
}

void warn(char* s)
{
WarnBox(s);
}

static void SetTitle(char* s)
  {
  char tmp[100];
  sprintf(tmp, "SysBench %s - %s", SYSB_VER, s);
  WinSetWindowText(hwndFrame, tmp);
  }

void AddTitle(char* s)
  {
  char tmp[100], tmp1[100];
  int  i, j;
  HAB hab;
  HMQ hmq;

  strcpy(tmp1, s); /* copy passed data since C++ doesn't let me change original */

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue ( hab, 0UL );

  i = WinQueryWindowText(hwndFrame, 100L, tmp);
  i = 60-i-3;
  j = strlen(tmp1);
  if (j > i)
     {
     tmp1[i] = 0;
     }

  strcat(tmp, " (");
  strcat(tmp, tmp1);
  strcat(tmp, ")");

  WinSetWindowText(hwndFrame, tmp);

  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  }


void DelTitle(void)
  {
  char tmp[100];
  int  i;
  HAB hab;
  HMQ hmq;
  char* n1;

  hab = WinInitialize(0);
  hmq = WinCreateMsgQueue ( hab, 0UL );

  i = WinQueryWindowText(hwndFrame, 100L, tmp);

  n1 = strstr(tmp, " (");

  if (n1)
     strcpy(n1, "");

  WinSetWindowText(hwndFrame, tmp);

  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
  }


static void UpdateWindow(HPS hpsPaint, PRECTL pRect, s32 scrollValue)
  {
  static char tmp[256];
  s32 i, comp, count = 0, iterations = NUM_COMPONENTS + (data.nr_fixed_disks-1) + (data.nr_cd_drives-1);
  s32 line = 0;
  RECTL rect;
  s32 tmpn;
  s32 color;   /* &^*% American spelling is catching */
  u32 clr;

  for (i = 0; i < data.nr_cd_drives; i++)
     {
     sprintf(data.c[comp_disk+data.nr_fixed_disks+i].title,
            "CD-ROM I/O disk %d-%d: %5.0f MB - %s",
            szDisk[i+10]->Controller-1,
            i+1,
            data.cd_drive_size[i]/(KB),
            szDisk[i+10]->desc.user);
     }

  disp_lines = 0;

  // print header
  line++;
  disp_lines++;

  for (comp = 0; comp < iterations; comp++)
     {
     if(comp%2)
        {
        color = CLR_CYAN;
        }
     else
        {
        color = CLR_GREEN;
        }

     // print title
     PrintTitle(line,
               data.c[comp].bitsettings,
               data.c[comp].title,
               pRect,
               scrollValue,
               hpsPaint,
               color);

     line++;
     disp_lines++;

     if (((data.c[comp].bitsettings & (0xffffffff - GROUP_EXPANDED)) != data.c[comp].bitsettings) ||
         ((data.c[comp].bitsettings & (0xffffffff - GROUP_AUTOEXPANDED)) != data.c[comp].bitsettings) ||
         (AllExpanded))
        {
        // print lines of data
        for (i = 0; i <= data.c[comp].ndatalines-1; i++)
           {
           if (data.c[comp].datalines[i].value < 0.0)
              sprintf(tmp, "%-21s :       --.---    %s",
                     data.c[comp].datalines[i].entry,
                     data.c[comp].datalines[i].unit);
           else
              sprintf(tmp, "%-21s : %12.3f    %s",
                     data.c[comp].datalines[i].entry,
                     data.c[comp].datalines[i].value / data.c[comp].datalines[i].unit_val,
                     data.c[comp].datalines[i].unit);
           if (comp == 4)
              {
              if (i%8 == 0)
                 {
                 if ((i/8)%2 == 0)
                    clr = CLR_PALEGRAY;
                 else
                    clr = CLR_WHITE;
                 }

              Print(line,
                   3,
                   tmp,
                   pRect,
                   scrollValue,
                   hpsPaint,
                   clr,
                   color);
              }
           else
              {
              Print(line,
                   3,
                   tmp,
                   pRect,
                   scrollValue,
                   hpsPaint,
                   CLR_WHITE,
                   color);
              }
           line++;
           disp_lines++;
           }
        }

     Print(line,
          3,
          "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ",
//           0         1         2         3         4         5         6         7
//           01234567890123456789012345678901234567890123456789012345678901234567890
//          "_______________________________________________________________________",
          pRect,
          scrollValue,
          hpsPaint,
          CLR_PALEGRAY,
          color);

     line++;
     disp_lines++;

     if (data.c[comp].total < 0.0)
        sprintf(tmp, "Total                 :       --.---    %s",
               data.c[comp].unit_total);
     else
        sprintf(tmp, "Total                 : %12.3f    %s",
               data.c[comp].total,
               data.c[comp].unit_total);

     Print(line,
          3,
          tmp,
          pRect,
          scrollValue,
          hpsPaint,
          CLR_YELLOW,
          color);

     line++;
     disp_lines++;
     Print(line,
          3,
//         0         1         2         3         4         5         6         7
//         01234567890123456789012345678901234567890123456789012345678901234567890123456
//          "  îîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîîî  ",
          NULLHANDLE,
          pRect,
          scrollValue,
          hpsPaint,
          color,
          color);

     line++;
     disp_lines++;

     Print(line,
          3,
//         0         1         2         3         4         5         6         7
//         01234567890123456789012345678901234567890123456789012345678901234567890123456
          "                                 îîîî                                     ",
          pRect,
          scrollValue,
          hpsPaint,
          CLR_WHITE,
          CLR_BLACK);

     line++;
     disp_lines++;
     }

  WinQueryWindowRect(hwndClient, &rect);
  WinSendMsg( hwndVertScroll,
              SBM_SETSCROLLBAR,
              MPFROMSHORT(scroll),
              MPFROM2SHORT(0, (disp_lines*fontH)-(rect.yTop-rect.yBottom)));
  WinSendMsg( hwndVertScroll,
              SBM_SETTHUMBSIZE,
              MPFROM2SHORT(rect.yTop - rect.yBottom, disp_lines * fontH),
              NULL);
}


static void Print(s32 row,
                 s32 col,
                 char* string,
                 PRECTL pRect,
                 s32 scrollValue,
                 HPS hpsPaint,
                 s32 color,
                 s32 boxcolor)

  {
  RECTL printRect;
  POINTL ptl;

  printRect.xLeft = col * fontW;
  printRect.xRight = pRect->xRight;
  printRect.yTop = pRect->yTop - row * fontH + scrollValue;
  printRect.yBottom = printRect.yTop - fontH;

  if (string != NULLHANDLE)
     {
     WinDrawText(hpsPaint,              /* if a string is supplied, print it */
                -1,
                (PCH)string,
                &printRect,
                color,
                CLR_BLACK,
                DT_TOP |
                DT_LEFT |
                DT_ERASERECT);
     }

  ptl.x = printRect.xLeft - 8;          /* set current GPI position */
  ptl.y = printRect.yTop;

  GpiSetColor(hpsPaint, boxcolor);      /* and colour */

  GpiMove(hpsPaint, &ptl);              /* go there */

  if (string != NULLHANDLE)             /* if we have a string */
     {
     ptl.y = printRect.yBottom;         /* set to the bottom of the rect */
     }                                  /* otherwise leave it at top */

  GpiLine(hpsPaint, &ptl);              /* draw the line (does nothing if string supplied) */

  ptl.x = printRect.xRight - 20;        /* set right bound */
  if (string == NULLHANDLE)             /* if no string supplied */
     {
     GpiLine(hpsPaint, &ptl);           /* draw a line */
     }
  else
     {
     GpiMove(hpsPaint, &ptl);           /* otherwise don't draw a line */
     }
  ptl.y = printRect.yTop;
  GpiLine(hpsPaint, &ptl);              /* fill in last line of box */
  }


static void PrintTitle(s32 row,
                       ULONG bitsettings,
                       char* string,
                       PRECTL pRect,
                       s32 scrollValue,
                       HPS hpsPaint,
                       s32 color)
  {
  RECTL printRect, printRect1;
  POINTL  pointBMP, ptl;
  HBITMAP hbitmap;
  BITMAPINFOHEADER   bmpsize;
  LONG col = 1;
  int  len = 0, i, puw, work;
  char* filler;
  POINTL aptl[134];

  printRect.xLeft = col * fontW;
  printRect.xRight = pRect->xRight-25;
  printRect.yTop = pRect->yTop - row * fontH + scrollValue;
  printRect.yBottom = printRect.yTop - fontH;

  pointBMP.x = col * fontW;
  pointBMP.y = (printRect.yTop - (3*fontH/4)) - 2;

  if ((bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == bitsettings)
     {
     if (!AllExpanded)
        {
        hbitmap = WinGetSysBitmap(HWND_DESKTOP,
                                 SBMP_TREEPLUS);
        }
     else
        {
        hbitmap = WinGetSysBitmap(HWND_DESKTOP,
                                 SBMP_TREEMINUS);
        }
     }
  else
     {
     hbitmap = WinGetSysBitmap(HWND_DESKTOP,
                              SBMP_TREEMINUS);
     }

  WinDrawBitmap(hpsPaint,
               hbitmap,
               NULL,
               &pointBMP,
               0,
               CLR_BLACK,
               DBM_NORMAL);

  GpiQueryBitmapParameters(hbitmap,
                          &bmpsize);

  printRect.xLeft = printRect.xLeft + bmpsize.cx + 3;
  ptl.x = printRect.xLeft;
  ptl.y = printRect.yBottom + ((2 * (printRect.yTop-printRect.yBottom))/3);
  GpiMove(hpsPaint, &ptl);
  ptl.x = printRect.xRight;
  GpiSetColor(hpsPaint, color);
  GpiLine(hpsPaint, &ptl);

  work = strlen(string);
  work++;
  work++;
  work++;

  filler = (char*)malloc(work);

  strcpy(filler, " ");
  strcat(filler, string);
  strcat(filler, " ");

  work = strlen(filler);
  GpiQueryCharStringPos(hpsPaint, 0, work, filler, 0, aptl);
  puw = aptl[work].x - aptl[0].x;

  printRect1 = printRect;
  printRect1.xLeft = printRect1.xLeft + (((printRect.xRight-printRect.xLeft) - puw)/2);
  printRect1.xRight = printRect1.xRight - (((printRect.xRight-printRect.xLeft) - puw)/2);

  WinDrawText(hpsPaint,
             -1,
             (PCH)filler,
             &printRect1,
             color,
             CLR_BLACK,
             DT_TOP |
             DT_CENTER |
             DT_WORDBREAK |
             DT_ERASERECT);

  free (filler);

  pointBMP.x = printRect.xRight-5;

  WinDrawBitmap(hpsPaint,
               hbitmap,
               NULL,
               &pointBMP,
               0,
               CLR_BLACK,
               DBM_NORMAL);

  GpiDeleteBitmap(hbitmap);
  }


void InfoBox(char* s)
  {
  WinMessageBox(HWND_DESKTOP,
                hwndFrame,
                s,
                "Info",
                WND_MESSAGEB,
                MB_OK | MB_INFORMATION | MB_APPLMODAL | MB_MOVEABLE);
  }


void ErrorBox(char* s)
  {
  WinMessageBox(HWND_DESKTOP,
                hwndFrame,
                s,
                "Error !",
                WND_MESSAGEB,
                MB_OK | MB_ERROR | MB_APPLMODAL | MB_MOVEABLE);
  }


void WarnBox(char* s)
  {
  WinMessageBox(HWND_DESKTOP,
                hwndFrame,
                s,
                "Warning !",
                WND_MESSAGEB,
                MB_OK | MB_WARNING | MB_APPLMODAL | MB_MOVEABLE);
  }

void UpdateAll(void)
  {
  RECTL rclWindow, cliprect;
  HRGN  newHrgn, oldHrgn, dummy;
  s32 i, comp;
  bool calcav = true;

  // calculate averages
  for (comp = 0; comp < NUM_COMPONENTS+(data.nr_fixed_disks-1)+(data.nr_cd_drives-1); comp++)
     {
     calcav = true;
     if (comp != comp_alldisks)
        {
        for (i = 0; i < data.c[comp].ndatalines; i++)
           {
           calcav = calcav && (data.c[comp].datalines[i].value >= 0.0);
           }
        }
     else
        {
        calcav = false;
        for (i = 0; i < data.c[comp].ndatalines; i++)
           {
           if (data.c[comp].datalines[i].value != -1.0)
              calcav = true;
           }
        }
     if (!calcav)
        continue;
     switch (comp)
        {
        case comp_gfx:
           data.c[comp_gfx].total = CalcGfxAv();
           break;
        case comp_cpuint:
           data.c[comp_cpuint].total = CalcCPUIntAv();
           break;
        case comp_cpufloat:
           data.c[comp_cpufloat].total = CalcCPUFloatAv();
           break;
        case comp_dive:
           data.c[comp_dive].total = CalcDIVEAv();
           break;
        case comp_file:
           data.c[comp_file].total = CalcFileIOAv();
           break;
        case comp_mem:
           data.c[comp_mem].total = CalcMemAv();
           break;
        case comp_alldisks:
           data.c[comp_alldisks].total = CalcSimIOAv();
           break;
        default: /* for all disk and CD entries */
           {
           if (Initialised)
              {
              if (comp < data.nr_fixed_disks+comp_disk)
                 data.c[comp].total = CalcDiskIOAv(comp);
              else
                 if (comp < data.nr_fixed_disks+data.nr_cd_drives+comp_disk)
                    data.c[comp].total = CalcCDIOAv(comp);
              }
           }
           break;
        }
     }

  // update screen
  WinQueryWindowRect ( hwndClient, &rclWindow ); // update whole window
  cliprect.xLeft = rclWindow.xLeft;
  cliprect.xRight = rclWindow.xRight+1;
  cliprect.yBottom = rclWindow.yBottom;
  cliprect.yTop = rclWindow.yTop+1;
  newHrgn = GpiCreateRegion(mainHps, 1, &cliprect);
  GpiSetClipRegion(mainHps, newHrgn, &oldHrgn);
  if (NULLHANDLE != oldHrgn)
     GpiDestroyRegion(mainHps, oldHrgn);
  UpdateWindow(mainHps, &rclWindow, scroll);
  oldscroll = scroll;
  }

static void SetMenuState(bool active)
  { // if active == true => all items enabled
    // otherw. all items but the about and close are disabled
  int stuff = 0;

  WinEnableMenuItem(hwndMenu, MI_PROJ_SAVE, active);
  WinEnableMenuItem(hwndMenu, MI_PROJ_SAVE_HTML, active);
  WinEnableMenuItem(hwndMenu, MI_PROJ_ALL, active);
  WinEnableMenuItem(hwndMenu, MI_PROJ_BEEP, active);
  WinCheckMenuItem(hwndMenu,  MI_PROJ_BEEP, Beep);
  WinCheckMenuItem(hwndMenu,  MI_PROJ_EXPAND, AllExpanded);
  WinEnableMenuItem(hwndMenu, MI_MACHINE_DATA, active);
  WinEnableMenuItem(hwndMenu, MI_MENU_GFX, active);
  WinEnableMenuItem(hwndMenu, MI_MENU_CPUINT, active);
  WinEnableMenuItem(hwndMenu, MI_MENU_CPUFLOAT, active);
  WinEnableMenuItem(hwndMenu, MI_MENU_MEM, active);
  if (gtWarp)
     {
     WinEnableMenuItem(hwndMenu,
                      MI_MENU_DIVE,
                      active);
     }
  else
     {
     WinEnableMenuItem(hwndMenu,
                      MI_MENU_DIVE,
                      stuff);
     }
  if (data.nr_fixed_disks)
     WinEnableMenuItem(hwndMenu, MI_MENU_DISKIO, active);
  else
     WinEnableMenuItem(hwndMenu, MI_MENU_DISKIO, stuff);

  if (data.nr_cd_drives)
     WinEnableMenuItem(hwndMenu, MI_MENU_CDIO, active);
  else
     WinEnableMenuItem(hwndMenu, MI_MENU_CDIO, stuff);

  if (!fileiodisabled)
     WinEnableMenuItem(hwndMenu, MI_MENU_FILEIO, active);
}


PSZ DoScanConfigSys(PSZ Keyword, int notstart)
  {
  char CSPath[_MAX_PATH];
  FILE *fp;
  static char Buffer[1025];
  char *p;
  char* n1;
  int notstop = 1;

  CSPath[0] = (char)(BootDriveLetter + 'A'- 1);
  CSPath[1] = 0;
  strcat(CSPath, ":\\CONFIG.SYS");

  fp = fopen(CSPath, "r");
  if (!fp)
     {
     return  0;
     }

  while(fgets(Buffer, sizeof(Buffer), fp) )
     {
     if (Buffer[strlen(Buffer)-1] == '\n')
        {
        Buffer[strlen(Buffer)-1] = 0;
        }

     p = strupr(Buffer);
     n1 = strstr((char*)p, (char*)Keyword);

     if (n1)
        {
        if (n1 == p)               /* if keyword at start of line */
           {
           n1 = strstr(n1, "=");
           n1 = n1+1;
           fclose (fp);
           return (PSZ)n1;
           }
        else
           {
           if (notstart)
              {
              fclose(fp);
              return (PSZ)n1;
              }
           }
        }
     }
  fclose (fp); /* close file at end */
  return 0;
}

void GetMachineInfo(void)
{
PSZ Swappath, charsp = "SWAPPATH";
PSZ diskcache, hpfscache, jfscache;
PSZ prot;
int i, j;
char hpfs386path[260] = "";

  char* n1;
  DosDevConfig(&coproc,DEVINFO_COPROCESSOR);

  BootDriveLetter = sysinfo[QSV_BOOT_DRIVE - QSV_MAX_PATH_LENGTH];

  Swappath = DoScanConfigSys(charsp, false);

  if (Swappath)
     {
     pdisknum = Swappath[0] - 'A' +1;

     sscanf(Swappath, "%s %u %f", CSpath, &minfree, &startsize);

     if (CSpath[strlen(CSpath)-1] == '\\')  /* if last character of path \ */
        {
        CSpath[strlen(CSpath)-1] = 0;       /* back up a bit */
        }

     startsize = (((int)startsize+1023)/1024) * 1024 * 1024;

     strcat(CSpath, "\\swapper.dat");
     }
  else
     {
     startsize = 0;
     strcpy(CSpath, "\\noswap.fil");
     }

  prot = DoScanConfigSys("PROTECTONLY", false);

  if (prot)
     {
     strcpy(Protectonly, prot);
     }

  diskcache = DoScanConfigSys("DISKCACHE", false);

  if (diskcache)
     {
     char cache[12], rest[100];
     sscanf(diskcache, "%[^, ]s %s", cache, rest);
     if (strcmpi(cache, "D"))
        {
        fatcachesize = atoi(cache) * KB;          /* hard coded cache size */
        }
     else
        {
        fatcachesize = sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH] / 10;  /* cache size is 10% of RAM */
        if (fatcachesize > (4096 * KB))
           {
           fatcachesize = 4096 * KB;
           }
        if (sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH] <= (8 * MB))
           {
           fatcachesize = 512 * KB;
           }
        if (sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH] <= (6 * MB))
           {
           fatcachesize = 128 * KB;
           }
        if (sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH] <= (5 * MB))
           {
           fatcachesize = 64 * KB;
           }
        if (sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH] <= (4 * MB))
           {
           fatcachesize = 48 * KB;
           }
        }
     }
  else
     {
     fatcachesize = 0;
     }

  FILE *cp;
  char CSPath[_MAX_PATH];
  char Buffer[1025];
  char *p;

  CSPath[0] = (char)(BootDriveLetter + 'A'-1);
  CSPath[1] = 0;
  strcat(CSPath,":\\config.sys");
  printf("Opening %s...", CSPath);
  cp = fopen(CSPath, "r");
  if (cp)
     {
     printf("OK\n");
     while(fgets(Buffer, sizeof(Buffer), cp) )
        {
        if (Buffer[strlen(Buffer)-1] == '\n')
           Buffer[strlen(Buffer)-1] = 0;
        p = strupr(Buffer);
        if (strncmp(p, "IFS=", 4) == 0)
           {
           hpfscache = p; // hunt for all unREMed IFS= lines
           if (hpfscache)
              {
              printf("%s\n", hpfscache);
              n1 = strstr((char*)hpfscache, "HPFS.IFS");
              if (n1) // it's plain hpfs
                 {
                 printf("Found hpfs.ifs in use\n");
                 n1 = strstr((char*)hpfscache, "/CACHE");
                 if (!n1)
                    {
                    n1 = strstr((char*)hpfscache, "/C:");
                    }

                 if (n1)
                    {
                    char cache[12], rest[100];
                    n1 = strstr(n1, ":");
                    n1 = n1+1;
                    sscanf(n1, "%s %s", cache, rest);
                    hpfscachesize = atoi(cache) * KB;
                    }
                 else
                    {
                    hpfscachesize = sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH] / 10;  /*10% of RAM */
                    if (hpfscachesize > (2048 * KB))
                       {
                       hpfscachesize = 2048 * KB;
                       }
                    }
                 printf("hpfs.ifs cache size = %.0f\n", hpfscachesize);
                 }

              n1 = strstr((char*)hpfscache, "HPFS386.IFS");
              if (n1) // it's hpfs386
                 {
                 printf("Found hpfs386.ifs in use\n");
                 hpfscache += strlen("IFS=");
                 j = strlen(hpfscache);
                 for (i = 0; i < j; i++)
                    {
                    if (hpfscache[i] == ' ')
                       break;
                    hpfs386path[i] = hpfscache[i];
                    }
                 if (i == j)
                    {
                    hpfscachesize = 0;
                    printf("no blank betwen ifs= and end of string\n");
                    }
                 else
                    {
                    hpfs386path[i] = 0;
                    j = strlen(hpfs386path);
                    for (i = j-1; i >= 0; i--)
                       {
                       if (hpfs386path[i] == '\\')
                          {
                          FILE *fp;

                          hpfs386path[i+1] = 0;
                          strcat(hpfs386path, "hpfs386.ini");
                          printf("Opening %s now...", hpfs386path);
                          fp = fopen(hpfs386path, "r");
                          if (fp)
                             {
                             int stop = 0;

                             printf("OK\n");
                             while (!stop)
                                {
                                char *pstr;
                                int n = 0;

                                if (fgets(hpfs386path, 254, fp) == NULL)
                                   break; // on error or EOF
                                if (strnicmp(hpfs386path, "cachesize", 9) == 0)
                                   {
                                   char sizechar[255];

                                   pstr = hpfs386path + 9;
                                   j = strlen(pstr);
                                   for (i = 0; i < j; i++)
                                      {
                                      if (pstr[i] == '=')
                                         {
                                         for (i = i; i < j; i++)
                                            {
                                            if (pstr[i+1] == '\n')
                                               break;
                                            if (pstr[i+1] != ' ')
                                               {
                                               sizechar[n++] = pstr[i+1];
                                               }
                                            }
                                         if (n > 0)
                                            {
                                            sizechar[n] = 0;
                                            hpfs386cachesize = atoi(sizechar) * KB;
                                            stop = 0;
                                            }
                                         }
                                      }
                                   }
                                }
                             fclose(fp);
                             }
                          else
                             {
                             printf("failed\n");
                             hpfs386cachesize = 0;
                             }
                          }
                       }
                    }
                 printf("hpfs386 cache size = %.0f\n", hpfs386cachesize);
                 }

              n1 = strstr((char*)hpfscache, "JFS.IFS");
              if (n1)
                 {
                 printf("JFS.IFS in use\n");
                 n1 = strstr((char*)hpfscache, "/CACHE");
                 if (!n1)
                    {
                    n1 = strstr((char*)hpfscache, "/C:");
                    }

                 if (n1)
                    {
                    char cache[12], rest[100];
                    n1 = strstr(n1, ":");
                    n1 = n1+1;
                    sscanf(n1, "%s %s", cache, rest);
                    jfscachesize = atoi(cache) * KB;
                    }
                 else
                    {
                    jfscachesize = sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH] / 8;  /* 12.5% of RAM */
                    }
                 printf("JFS cache size = %.0f\n", jfscachesize);
                 }
              }
           }
        }
     fclose(cp);
     FILESIZE = 2 * max(jfscachesize, max(hpfscachesize, max(hpfs386cachesize, 64*MB))); // min 128MB file

     printf("File I/O tests will use a %uMB file\n", FILESIZE/MB);
     }
  else
     printf("Failed\n");
}


float GetSwapFileSize(void)
{
FILESTATUS3 pInfoBuf1;
if (!DosQueryPathInfo(CSpath,
                  FIL_STANDARD,
                  &pInfoBuf1,
                  sizeof(FILESTATUS3)))
     {
     return (float)pInfoBuf1.cbFile;
     }
  else
     {
     return 0;
     }
}


void _Optlink ShowWaitWindow(void* arg)
{
  HMQ         hmq = NULLHANDLE;
  HAB         hab = NULLHANDLE;      /* PM anchor block handle         */
  ERRORID erridErrorCode;
  APIRET      rc  = 0 ;

  hab = WinInitialize ( 0UL );
  if (hab == NULLHANDLE)
     {
     erridErrorCode = WinGetLastError(hab);
     printf("ShowWaitWindow WinInitialize returned %x\n", erridErrorCode);
     exit(2);
     }
  hmq = WinCreateMsgQueue ( hab, 0UL );
  if (hmq == NULLHANDLE)
     {
     erridErrorCode = WinGetLastError(hab);
     printf("ShowWaitWindow WinCreateMsgQueue returned %x\n", erridErrorCode);
     exit(2);
     }

  flashit = 1;

  rc = DosCreateEventSem(NULL,          /* Unnamed semaphore            */
                        &hevEvent3,     /* Handle of semaphore returned */
                        DC_SEM_SHARED,  /* Indicate a shared semaphore  */
                        FALSE);         /* Put in RESET state           */

  _beginthread(Flashlight, NULL, START_STACKSIZE, NULL); /* show Wait... window */

  hwndDlgB = WinLoadDlg(HWND_DESKTOP,
           HWND_DESKTOP,
           fnShowWait,
           NULLHANDLE,
           IDD_SHOWWAIT,
           NULL);

  rc = WinProcessDlg(hwndDlgB);

  flashit = 0;

  rc = DosWaitEventSem(hevEvent3, 60000);  /* wait for flashlight thread to end */

  rc = DosCloseEventSem(hevEvent3);

  WinDestroyMsgQueue ( hmq );
  WinTerminate ( hab );
  _endthread();
}


/* routine to handle messages from gathering info  box */
MRESULT APIENTRY fnShowWait (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
int rc = 0;
 switch (msg)
    {
    case WM_INITDLG:
       {
       RECTL rect, rectdlg;

       rc = WinQueryWindowRect(HWND_DESKTOP, &rect);
       rc = WinQueryWindowRect(hwnd, &rectdlg);

       rc = WinSetWindowPos(hwnd,
                      HWND_TOP,
                      ((rect.xRight-rect.xLeft)-(rectdlg.xRight-rectdlg.xLeft))/2,
                      ((rect.yTop-rect.yBottom)-(rectdlg.yTop-rectdlg.yBottom))/2,
                      0,
                      0,
                      SWP_MOVE |
                      SWP_SHOW |
                      SWP_ACTIVATE);

       rc = DosPostEventSem(hevEvent2);  /* say we're ready to take close msg */
       }
       break;

    case WM_COMMAND:
       switch (COMMANDMSG(&msg)->cmd)
          {
          case IDD_SHOWWAIT5:
             rc = WinSetFocus(HWND_DESKTOP,    /* due to an apparent bug in OS/2 that means that
                                                  the WinProcessDlg that invokes this msg handling routine
                                                  never ends if it doesn't have the focus when the
                                                  WinDismissDlg() is issued, let's give it the focus */
                             hwndDlgB);
             rc = WinDismissDlg(hwndDlgB, 0UL);  /* dismiss the dialog */
             flashit = 0;
             DosSleep(900);                      /* wait for that to happen */
             WinSetFocus(HWND_DESKTOP,
                        hwndFrame);

             return 0L;
          }
       break;

    default:
       break;
    }
 return (MRESULT)WinDefDlgProc (hwnd, msg, mp1, mp2);
}                                       /* end PromptWinProc     */


void _Optlink Flashlight(void* arg)
{
  HMQ         hmq = NULLHANDLE;
  HAB         hab = NULLHANDLE;      /* PM anchor block handle         */
  ERRORID erridErrorCode;
  BOOL i = 0;

  hab = WinInitialize ( 0UL );
  if (hab == NULLHANDLE)
     {
     erridErrorCode = WinGetLastError(hab);
     printf("Flashlight WinInitialize returned %x\n", erridErrorCode);
     exit(2);
     }
  hmq = WinCreateMsgQueue ( hab, 0UL );
  if (hmq == NULLHANDLE)
     {
     erridErrorCode = WinGetLastError(hab);
     printf("Flashlight WinCreateMsgQueue returned %x\n", erridErrorCode);
     exit(2);
     }

  while (flashit)
     {
     WinShowWindow(WinWindowFromID(hwndDlgB, IDD_SHOWICON2), /* toggle LED window visibility */
                  (i%2));
     i++;
     DosSleep(500);
     }

  DosPostEventSem(hevEvent3);    /* allow parent thread to end */

  WinDestroyMsgQueue ( hmq );
  WinTerminate ( hab );
  _endthread();
}


void _Optlink GetDriveInfo(void* arg)
{
  HWND        hwndPullDown;
  CHAR        tmp[256];
  APIRET      ulrc         = 0;
  BOOL        drivechecked = 0;
  int         i            = 0;
  MENUITEM    mi;
  ERRORID     erridErrorCode;
  HMQ         hmq          = NULLHANDLE;
  HAB         hab          = NULLHANDLE;      /* PM anchor block handle         */

  hab = WinInitialize ( 0UL );
  if (hab == NULLHANDLE)
     {
     erridErrorCode = WinGetLastError(hab);
     printf("GetDriveInfo WinInitialize returned %x\n", erridErrorCode);
     exit(2);
     }
  hmq = WinCreateMsgQueue ( hab, 0UL );
  if (hmq == NULLHANDLE)
     {
     erridErrorCode = WinGetLastError(hab);
     printf("GetDriveInfo WinCreateMsgQueue returned %x\n", erridErrorCode);
     exit(2);
     }

  AddTitle("Listing formatted drives");

  for (i = 2; i <= 25; i++)   /* do from first possible HD upwards */
     {
     if ( ((ulDriveMap1 << (31-i)) >> 31) ) /* if bit in drive array is on */
        {
        double drivesize = 300;
        char szDeviceName[3];
        FSINFOBUF VolumeInfo = {0};        /* File system info buffer */
        BYTE         fsqBuffer[sizeof(FSQBUFFER2) + (3 * CCHMAXPATH)] = {0};
        ULONG        cbBuffer   = sizeof(fsqBuffer);        /* Buffer length) */
        PFSQBUFFER2  pfsqBuffer = (PFSQBUFFER2) fsqBuffer;
        ULONG        ulOrdinal        = 0;     /* Ordinal of entry in name list      */
        PBYTE        pszFSDName       = NULL;  /* pointer to FS name                 */
        ULONG        aulFSInfoBuf[40] = {0};         /* File system info buffer     */

        ulrc = DosQueryFSInfo(i+1,
                             FSIL_VOLSER,      /* Request volume information */
                             &VolumeInfo,      /* Buffer for information     */
                             sizeof(FSINFOBUF));  /* Size of buffer          */

        if (ulrc != NO_ERROR)
           {
           if (ulrc == ERROR_BAD_NETPATH ||
               ulrc == ERROR_NOT_READY ||
               ulrc == ERROR_GEN_FAILURE ||
               ulrc == ERROR_BAD_NET_NAME)
              {
              continue;
              }
           else
              {
              sprintf(tmp, "DosQueryFSInfo error: return code = %u disk %c:\n", ulrc, 'A'+i);
              logit(tmp);
              continue;
              }
           }

        szDeviceName[0] = 'A'+i;
        szDeviceName[1] = ':';
        szDeviceName[2] = (char)NULL;

        ulrc = DosQueryFSAttach(szDeviceName,   /* Logical drive of attached FS      */
                             ulOrdinal,       /* ignored for FSAIL_QUERYNAME       */
                             FSAIL_QUERYNAME, /* Return data for a Drive or Device */
                             pfsqBuffer,      /* returned data                     */
                             &cbBuffer);      /* returned data length              */

        if (ulrc != NO_ERROR)
           {
           sprintf(tmp, "DosQueryFSAttach error: return code = %u\n", ulrc);
           logit(tmp);
           return;
           }
        else
           {
           pszFSDName = (PBYTE)(pfsqBuffer->szName + pfsqBuffer->cbName + 1);
           }

        if (!strcmp(pszFSDName, "FAT"))
           {
           curdiskFAT = 1;
           }
        else
           {
           if (!strcmp(pszFSDName, "HPFS"))
              {
              curdiskFAT = 2;
              }
           else
              {
              if (!strcmp(pszFSDName, "JFS"))
                 {
                 curdiskFAT = 3;
                 }
              else
                 {
                 curdiskFAT = 0; /*  if anything except FAT or HPFS */
                 }
              }
           }

        if (curdiskFAT)
           {
           ulrc = DosQueryFSInfo(i+1,            /* Drive number      */
                                FSIL_ALLOC,             /* Level 1 allocation info */
                                (PVOID)aulFSInfoBuf,    /* Buffer                  */
                                sizeof(aulFSInfoBuf));  /* Size of buffer          */

           if (ulrc != NO_ERROR)
              {
              sprintf(tmp, "DosQueryFSInfo error: return code = %u disk %c:\n", ulrc, 'A'+i);
              logit(tmp);
              }
           else
              {
              drivesize = (((double)aulFSInfoBuf[1] * (double)aulFSInfoBuf[2] * (USHORT)aulFSInfoBuf[4])/MB);
              /* (Sectors per allocation unit) * number of allocationunits * (Bytes per sector) */
              }

           if ((double)((double)aulFSInfoBuf[1] * (double)aulFSInfoBuf[3] * (USHORT)aulFSInfoBuf[4]) > (double)FILESIZE) /* if freespace > 10Mb */
              {
              char drive[3];
              drive[0] = 'A'+i;
              drive[1] = ':';
              drive[2] = 0;
              sprintf(tmp, "Drive % 4s % 7.0f MB", drive, drivesize);  /* print menu item inc. size */
              if (WinSendMsg(hwndMenu,
                            MM_QUERYITEM,
                            MPFROM2SHORT(MI_MENU_FILEIO_SELECT, TRUE),
                            (MPARAM)&mi))
                 {
                 hwndPullDown   = mi.hwndSubMenu;
                 mi.iPosition   = MIT_END;
                 mi.afStyle     = MIS_TEXT;
                 mi.afAttribute = (ULONG)NULL;
                 mi.id          = (ULONG)MI_MENU_FILEIO_SELECT+1+i;
                 mi.hwndSubMenu = (HWND)NULL;
                 mi.hItem       = (ULONG)NULL;
                 WinSendMsg(hwndPullDown,
                           MM_INSERTITEM,
                           (MPARAM)&mi,
                           (MPARAM)tmp);
                 if (i == ulDriveNum-1)
                    {
                    sprintf(tmp, "File I/O - Drive %c:", 'A'+ulDriveNum-1);
                    strcpy(data.c[comp_file].title, tmp);
                    drivechecked = 1;
                    WinCheckMenuItem(hwndMenu,
                                    MI_MENU_FILEIO_SELECT + ulDriveNum,
                                    drivechecked);
                    }
                 }
              else
                 {
                 erridErrorCode = WinGetLastError(hab);
                 }
              }
           else
              printf("Not enough space on drive %c: for file i/o tests\n", 'A'+i);
           }
        }
     }

  if (!drivechecked)      /* if current drive didn't have enough space */
     {
     ULONG testDrive = 0x00000001; /* bit array to reflect current drive */

     if ((ulDriveMap1 & (0xffffffff-(testDrive << ulDriveNum-1))) == ulDriveMap1) /* if current drive is not valid */
        {
        for (i = 3; i <= 26; i++)
           {
           if ((ulDriveMap1 & (0xffffffff-(testDrive << i-1))) != ulDriveMap1)
              {
              ulDriveNum = i; /* set to first drive with enough space */
              sprintf(tmp, "File I/O - Drive %c:", 'A'+ulDriveNum-1);
              strcpy(data.c[comp_file].title, tmp);
              drivechecked = 1;
              WinCheckMenuItem(hwndMenu,
                              MI_MENU_FILEIO_SELECT + ulDriveNum,
                              drivechecked);
              DosSetDefaultDisk(ulDriveNum);
              break;         /* drop out of for (i = 3; i <= 26) */
              }
           }
        }
     }

  if (!drivechecked) /* still ?! */
     {
     fileiodisabled = 1;
     }

  DelTitle();
  AddTitle("Listing available disks");

  if (data.nr_fixed_disks+data.nr_cd_drives > MAX_FIXED_DISKS+MAX_CD_DRIVES)
     {
     logit("Number of fixed disks and CD drives is too high");
     exit(1);
     }

  for (i = 0; i < data.nr_fixed_disks; i++)
     {
     data.fixed_disk_size[i] = pmb_diskio_disksize(i);
     sprintf(tmp, "Disk %d-%d: %5.0f MB", szDisk[i]->Controller-1, i+1, data.fixed_disk_size[i]/(KB));
     strcpy(data.c[comp_alldisks].datalines[i].entry, tmp);

     sprintf(tmp, "Disk %d: %5.0f MB", i+1, data.fixed_disk_size[i]/(KB));
     if (debugging)
        {
        printf("%s\n", tmp);
        }
     sprintf(data.c[comp_disk+i].title,
            "Disk I/O disk %d-%d: %5.0f MB - %s",
            szDisk[i]->Controller-1,
            i+1,
            data.fixed_disk_size[i]/(KB),
            szDisk[i]->desc.user);
     data.c[comp_disk+i].ndatalines = 7;

     strcpy(data.c[comp_disk+i].datalines[0].entry, "Avg. data access time");
     data.c[comp_disk+i].datalines[0].value = -1.0;
     data.c[comp_disk+i].datalines[0].unit_val = 1.0e-03;
     strcpy(data.c[comp_disk+i].datalines[0].unit, "milliseconds");

     strcpy(data.c[comp_disk+i].datalines[1].entry, "Cache/Bus xfer rate  ");
     data.c[comp_disk+i].datalines[1].value = -1.0;
     data.c[comp_disk+i].datalines[1].unit_val = MB;
     strcpy(data.c[comp_disk+i].datalines[1].unit, "Megabytes/second");

     strcpy(data.c[comp_disk+i].datalines[2].entry, "Track 0 xfer rate fwd");
     data.c[comp_disk+i].datalines[2].value = -1.0;
     data.c[comp_disk+i].datalines[2].unit_val = MB;
     strcpy(data.c[comp_disk+i].datalines[2].unit, "Megabytes/second");

     strcpy(data.c[comp_disk+i].datalines[3].entry, "Middle trk rate fwds.");
     data.c[comp_disk+i].datalines[3].value = -1.0;
     data.c[comp_disk+i].datalines[3].unit_val = MB;
     strcpy(data.c[comp_disk+i].datalines[3].unit, "Megabytes/second");

     strcpy(data.c[comp_disk+i].datalines[4].entry, "Last track rate bwds.");
     data.c[comp_disk+i].datalines[4].value = -1.0;
     data.c[comp_disk+i].datalines[4].unit_val = MB;
     strcpy(data.c[comp_disk+i].datalines[4].unit, "Megabytes/second");

     strcpy(data.c[comp_disk+i].datalines[5].entry, "Average Transfer rate");
     data.c[comp_disk+i].datalines[5].value = -1.0;
     data.c[comp_disk+i].datalines[5].unit_val = MB;
     strcpy(data.c[comp_disk+i].datalines[5].unit, "Megabytes/second");

     strcpy(data.c[comp_disk+i].datalines[6].entry, "Disk use CPU load    ");
     data.c[comp_disk+i].datalines[6].value = -1.0;
     data.c[comp_disk+i].datalines[6].unit_val = 1;
     strcpy(data.c[comp_disk+i].datalines[6].unit, "percent");
     if (!PerfSysSup)
        {
        strcat(data.c[comp_disk+i].datalines[6].unit, " (approx)");
        }

     data.c[comp_disk+i].total = -1.0;
     strcpy(data.c[comp_disk+i].unit_total, "Disk I/O-marks");

     if (WinSendMsg(hwndMenu,
                MM_QUERYITEM,
                MPFROM2SHORT(MI_MENU_DISKIO_SELECT, TRUE),
                (MPARAM)&mi))
        {
        hwndPullDown   = mi.hwndSubMenu;
        mi.iPosition   = MIT_END;
        mi.afStyle     = MIS_TEXT;
        mi.afAttribute = (ULONG)NULL;
        mi.id          = (ULONG)MI_MENU_DISKIO_SELECT+1+i;
        mi.hwndSubMenu = (HWND)NULL;
        mi.hItem       = (ULONG)NULL;
        WinSendMsg(hwndPullDown,
                  MM_INSERTITEM,
                  (MPARAM)&mi,
                  (MPARAM)tmp);
        }
     }

  if (!data.nr_fixed_disks)
     {
     WinSendMsg(hwndMenu,
               MM_QUERYITEM,
               MPFROM2SHORT(MI_MENU_DISKIO_SELECT, TRUE),
               (MPARAM) &mi);

    hwndPullDown   = mi.hwndSubMenu;
    mi.iPosition   = MIT_END;
    mi.afStyle     = MIS_TEXT;
    mi.afAttribute = 0;
    mi.id          = MI_MENU_DISKIO_SELECT+1;
    mi.hwndSubMenu = null;
    mi.hItem       = 0;
    WinSendMsg(hwndPullDown,
              MM_INSERTITEM,
              (MPARAM) &mi,
              (MPARAM)"No fixed disks found");
    }
 else
    {
    int stuff = 1;
    WinCheckMenuItem(hwndMenu,
                    MI_MENU_DISKIO_SELECT + 1,
                    stuff);
    data.selected_disk = 0;
    }

  DelTitle();
  AddTitle("Listing available CDs");

/*  if (data.nr_cd_drives > MAX_CD_DRIVES)
     {
     logit("Number of CD drives is too high");
     exit(1);
     }
*/
  for (i = 0; i < data.nr_cd_drives; i++)
     {
     int nDriveLetter;
     if ((nDriveLetter = CDFind(i+1)) == -1)
        {
        sprintf(tmp, "Drive %c: unavailable", nDriveLetter);
        }
     else
        {
        sprintf(tmp, "Drive %c:", nDriveLetter);
        }

     data.c[comp_disk+data.nr_fixed_disks+i].ndatalines = 4;

     strcpy(data.c[comp_disk+data.nr_fixed_disks+i].datalines[0].entry, "Avg. data access time");
     data.c[comp_disk+data.nr_fixed_disks+i].datalines[0].value = -1.0;
     data.c[comp_disk+data.nr_fixed_disks+i].datalines[0].unit_val = 1.0e-03;
     strcpy(data.c[comp_disk+data.nr_fixed_disks+i].datalines[0].unit, "milliseconds");

     strcpy(data.c[comp_disk+data.nr_fixed_disks+i].datalines[1].entry, "Inner sectors rate");
     data.c[comp_disk+data.nr_fixed_disks+i].datalines[1].value = -1.0;
     data.c[comp_disk+data.nr_fixed_disks+i].datalines[1].unit_val = KB;
     strcpy(data.c[comp_disk+data.nr_fixed_disks+i].datalines[1].unit, "Kilobytes/second");

     strcpy(data.c[comp_disk+data.nr_fixed_disks+i].datalines[2].entry, "Outer sectors rate");
     data.c[comp_disk+data.nr_fixed_disks+i].datalines[2].value = -1.0;
     data.c[comp_disk+data.nr_fixed_disks+i].datalines[2].unit_val = KB;
     strcpy(data.c[comp_disk+data.nr_fixed_disks+i].datalines[2].unit, "Kilobytes/second");

     strcpy(data.c[comp_disk+data.nr_fixed_disks+i].datalines[3].entry, "CD-ROM use CPU load ");
     data.c[comp_disk+data.nr_fixed_disks+i].datalines[3].value = -1.0;
     data.c[comp_disk+data.nr_fixed_disks+i].datalines[3].unit_val = 1;
     strcpy(data.c[comp_disk+data.nr_fixed_disks+i].datalines[3].unit, "percent");
     if (!PerfSysSup)
        {
        strcat(data.c[comp_disk+data.nr_fixed_disks+i].datalines[3].unit, " (approx)");
        }

     data.c[comp_disk+data.nr_fixed_disks+i].total = -1.0;
     strcpy(data.c[comp_disk+data.nr_fixed_disks+i].unit_total, "CD I/O-marks");

     if (WinSendMsg(hwndMenu,
               MM_QUERYITEM,
               MPFROM2SHORT(MI_MENU_CDIO_SELECT, TRUE),
               (MPARAM)&mi))
        {
        hwndPullDown   = mi.hwndSubMenu;
        mi.iPosition   = MIT_END;
        mi.afStyle     = MIS_TEXT;
        mi.afAttribute = (ULONG)NULL;
        mi.id          = (ULONG)MI_MENU_CDIO_SELECT+1+i;
        mi.hwndSubMenu = (HWND)NULL;
        mi.hItem       = (ULONG)NULL;
        WinSendMsg(hwndPullDown,
                  MM_INSERTITEM,
                  (MPARAM)&mi,
                  (MPARAM)tmp);
        }
     }

  if (!data.nr_cd_drives)
     {
     WinSendMsg(hwndMenu,
               MM_QUERYITEM,
               MPFROM2SHORT(MI_MENU_CDIO_SELECT, TRUE),
               (MPARAM) &mi);

    hwndPullDown   = mi.hwndSubMenu;
    mi.iPosition   = MIT_END;
    mi.afStyle     = MIS_TEXT;
    mi.afAttribute = 0;
    mi.id          = MI_MENU_CDIO_SELECT+1;
    mi.hwndSubMenu = null;
    mi.hItem       = 0;
    WinSendMsg(hwndPullDown,
              MM_INSERTITEM,
              (MPARAM) &mi,
              (MPARAM)"No CD drives found");
    }
 else
    {
    int stuff = 1;
    WinCheckMenuItem(hwndMenu,
                    MI_MENU_CDIO_SELECT + 1,
                    stuff);
    data.selected_cd = 0;
    }
// DosSleep(30000);

  DelTitle();

 WinSendMsg(hwndClient,
           THR_DONE,
           (MPARAM)0,
           (MPARAM)0);

 WinSendMsg(hwndClient,
           THR_UPDATE,
           (MPARAM)0,
           (MPARAM)0);

 WinDestroyMsgQueue ( hmq );
 WinTerminate ( hab );

 _endthread();
}


void GetMachineStuff(HWND hwnd)
{
  HAB         hab = NULLHANDLE;      /* PM anchor block handle         */
  ERRORID erridErrorCode;
  APIRET      rc  = 0 ;
  HWND        hwndDlgM;

  hab = WinQueryAnchorBlock( hwnd );
  if (hab == NULLHANDLE)
     {
     erridErrorCode = WinGetLastError(hab);
     printf("GetMachineInfo WinQueryAnchorBlock returned %x\n", erridErrorCode);
     exit(2);
     }

  hwndDlgM = WinLoadDlg(HWND_DESKTOP,
           hwnd,
           fnMachineStuff,
           NULLHANDLE,
           IDD_MACHINE_DATA,
           NULL);

  rc = WinProcessDlg(hwndDlgM);
}


/* routine to handle messages from machine info  box */
MRESULT APIENTRY fnMachineStuff (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
ULONG rc = 0, i, j, k;
HAB hab = WinQueryAnchorBlock(hwnd);
ULONG ipt = 0;

 switch (msg)
    {
    ULONG ulPageId;
    CHAR pszMleBuffer[512];

    case WM_INITDLG:
       {
       ulPageId = (LONG)WinSendDlgItemMsg(hwnd,
                                         IDD_MACHINE,
                                         BKM_INSERTPAGE,
                                         NULL,
                                         MPFROM2SHORT((BKA_STATUSTEXTON |
                                                       BKA_AUTOPAGESIZE |
                                                       BKA_MAJOR),
                                         BKA_LAST));

       if ( !ulPageId)
          return FALSE;

       if ( !WinSendDlgItemMsg(hwnd,
                               IDD_MACHINE,
                               BKM_SETDIMENSIONS,
                               MPFROM2SHORT(120, 30),
                               MPFROMP(BKA_MAJORTAB)))
           return FALSE;

       if ( !WinSendDlgItemMsg(hwnd,
                               IDD_MACHINE,
                               BKM_SETSTATUSLINETEXT,
                               MPFROMLONG(ulPageId),
                               MPFROMP("Page 1 of 3")))
           return FALSE;

       if ( !WinSendDlgItemMsg(hwnd,
                               IDD_MACHINE,
                               BKM_SETTABTEXT,
                               MPFROMLONG(ulPageId),
                               MPFROMP("~Machine Info")))
          return FALSE;

       hwndPage1 = WinLoadDlg(hwnd,          /* load dialog for entry fields */
                            hwnd,
                            fnMachdlgStuff,
                            0,
                            IDD_MACH_DLG_DATA,
                            NULL);

       if (!hwndPage1)
         return FALSE;

       if ( !WinSendDlgItemMsg(hwnd,
                              IDD_MACHINE,
                              BKM_SETPAGEWINDOWHWND,
                              MPFROMLONG(ulPageId),
                              MPFROMHWND(hwndPage1)))
         return FALSE;

       /*
        * Insert the second page.
        */
       ulPageId = (LONG)WinSendDlgItemMsg(hwnd,
                                         IDD_MACHINE,
                                         BKM_INSERTPAGE,
                                         NULL,
                                         MPFROM2SHORT((BKA_STATUSTEXTON |
                                                       BKA_AUTOPAGESIZE |
                                                       BKA_MAJOR),
                                         BKA_LAST));

       if (!ulPageId)
         return FALSE;

       if ( !WinSendDlgItemMsg(hwnd,
                              IDD_MACHINE,
                              BKM_SETSTATUSLINETEXT,
                              MPFROMLONG(ulPageId),
                              MPFROMP("Page 2 of 3")))
          return FALSE;

       if ( !WinSendDlgItemMsg(hwnd,
                              IDD_MACHINE,
                              BKM_SETTABTEXT,
                              MPFROMLONG(ulPageId),
                              MPFROMP("~Controllers")))
         return FALSE;

       hwndPage2 = WinCreateWindow(hwnd,                      /* parent-window handle                    */
                                 WC_LISTBOX,                 /* pointer to registered class name        */
                                 "Doubleclick item to define", /* pointer to window text                  */
                                 WS_VISIBLE,
                                 0,                          /* horizontal position of window           */
                                 0,                          /* vertical position of window             */
                                 0,                          /* window width                            */
                                 0,                          /* window height                           */
                                 hwnd,                       /* owner-window handle                     */
                                 HWND_TOP,                   /* handle to sibling window                */
                                 0,                          /* window identifier                       */
                                 NULL,                       /* pointer to buffer                       */
                                 NULL);                      /* pointer to structure with pres. params. */

       if (!hwndPage2)
         return FALSE;

      for (i = 0; i < StorageControllers; i++)
         {
         WinSendMsg(hwndPage2,
                   LM_INSERTITEM,
                   MPFROMSHORT(LIT_END),
                   MPFROMP(DiskController[i]->desc.user));
         }

       if( !WinSendDlgItemMsg(hwnd,
                             IDD_MACHINE,
                             BKM_SETPAGEWINDOWHWND,
                             MPFROMLONG(ulPageId),
                             MPFROMHWND(hwndPage2)))
         return FALSE;

       /*
        * Insert the third page.
        */
       ulPageId = (LONG)WinSendDlgItemMsg(hwnd,
                                         IDD_MACHINE,
                                         BKM_INSERTPAGE,
                                         NULL,
                                         MPFROM2SHORT((BKA_STATUSTEXTON |
                                                       BKA_AUTOPAGESIZE |
                                                       BKA_MAJOR),
                                         BKA_LAST));

       if (!ulPageId)
         return FALSE;

       if ( !WinSendDlgItemMsg(hwnd,
                              IDD_MACHINE,
                              BKM_SETSTATUSLINETEXT,
                              MPFROMLONG(ulPageId),
                              MPFROMP("Page 3 of 3")))
          return FALSE;

       if ( !WinSendDlgItemMsg(hwnd,
                              IDD_MACHINE,
                              BKM_SETTABTEXT,
                              MPFROMLONG(ulPageId),
                              MPFROMP("~Disks/CDs")))
         return FALSE;

       hwndPage3 = WinCreateWindow(hwnd,                      /* parent-window handle                    */
                                 WC_LISTBOX,                 /* pointer to registered class name        */
                                 "Doubleclick item to define", /* pointer to window text                  */
                                 WS_VISIBLE,
                                 0,                          /* horizontal position of window           */
                                 0,                          /* vertical position of window             */
                                 0,                          /* window width                            */
                                 0,                          /* window height                           */
                                 hwnd,                       /* owner-window handle                     */
                                 HWND_TOP,                   /* handle to sibling window                */
                                 0,                          /* window identifier                       */
                                 NULL,                       /* pointer to buffer                       */
                                 NULL);                      /* pointer to structure with pres. params. */

       if (!hwndPage3)
         return FALSE;

      for (i = 0; i < data.nr_fixed_disks; i++)
         {
         char disktext[80];
         sprintf(disktext, "Disk %d: % 5.0f MB - %s",
                i+1,
                data.fixed_disk_size[i]/(KB),
                szDisk[i]->desc.user);

         WinSendMsg(hwndPage3,
                   LM_INSERTITEM,
                   MPFROMSHORT(LIT_END),
                   MPFROMP(disktext));
         }

      for (i = 0; i < data.nr_cd_drives; i++)
         {
         char disktext[80];
         sprintf(disktext, "CD %d: % 7.0f MB - %s",
                i+1,
                data.cd_drive_size[i]/(KB),
                szDisk[i+10]->desc.user);

         WinSendMsg(hwndPage3,
                   LM_INSERTITEM,
                   MPFROMSHORT(LIT_END),
                   MPFROMP(disktext));
         }

       if( !WinSendDlgItemMsg(hwnd,
                             IDD_MACHINE,
                             BKM_SETPAGEWINDOWHWND,
                             MPFROMLONG(ulPageId),
                             MPFROMHWND(hwndPage3)))
         return FALSE;
       }
    break;

    case WM_CONTROL:
       {
       switch (SHORT2FROMMP(mp1))
          {
          case LN_ENTER:                            /* if list box item doubleclicked */
             {
             char *szDiskname = (char*)malloc(101);
             char* n1;
             SHORT sTextLength;
             WINPARM *winparm1;

             winparm1 = new WINPARM;

             if ((HWND)mp2 == hwndPage3) /* for disk descriptions */
                {
                i = (int)WinSendMsg(HWNDFROMMP(mp2),   /* find out which line it was... */
                                   LM_QUERYSELECTION,
                                   (MPARAM)MPFROMSHORT(LIT_CURSOR),
                                   (MPARAM)0);
                if (i == LIT_NONE)
                   {
                   i = 0;
                   }
                if (i >= data.nr_fixed_disks)
                   winparm1->number = i + 10 - data.nr_fixed_disks;       /* it's a CD */
                else
                   winparm1->number = i;            /* otherwise it's a disk */
                WinSendMsg(HWNDFROMMP(mp2),   /* find out which line it was... */
                          LM_QUERYITEMTEXT,
                          (MPARAM)MPFROM2SHORT(i, 101),
                          (MPARAM)MPFROMLONG(szDiskname));
                n1 = strstr(szDiskname, " - ");
                strcpy(szDisk[winparm1->number]->desc.user, n1+3);
                winparm1->desc = szDisk[winparm1->number]->desc.user;
                }
             else
                {                                      /* for controller descriptions */
                i = (int)WinSendMsg(HWNDFROMMP(mp2),   /* find out which line it was... */
                                   LM_QUERYSELECTION,
                                   (MPARAM)MPFROMSHORT(LIT_CURSOR),
                                   (MPARAM)0);
                if (i == LIT_NONE)
                   {
                   i = 0;
                   }
                winparm1->number = i;
                WinSendMsg(HWNDFROMMP(mp2),   /* find out which line it was... */
                          LM_QUERYITEMTEXT,
                          (MPARAM)MPFROM2SHORT(i, 101),
                          (MPARAM)MPFROMLONG(szDiskname));
                winparm1->desc = szDiskname;
                }

             winparm1->hwnd = (HWND)mp2;

             hwndDlgD = WinLoadDlg(HWND_DESKTOP,
                                  hwnd,
                                  fnDiskStuff,
                                  NULLHANDLE,
                                  IDD_DISK,
                                  &winparm1);

             rc = WinProcessDlg(hwndDlgD);

             if ((HWND)mp2 == hwndPage2)
                {
                strcpy(DiskController[winparm1->number]->desc.user, szDiskname);
                }

             free(szDiskname);
             delete winparm1;
             } /* end case LN_ENTER */
             break;

           default:
              break;
           } /* end switch */
        }    /* end case WM_CONTROL */
        break;

    case WM_COMMAND:
       switch (COMMANDMSG(&msg)->cmd)
          {
          case IDD_DID_OK:
             {
             if (NamedC)                      /* if contents of text window changed */
                {
                WinQueryWindowText(           /* get text from display window */
                                  WinWindowFromID(hwndPage1, IDD_MACH_NAMED),
                                  sizeof(Machinename),
                                  Machinename);
                 }

             if (MobodC)                      /* if contents of text window changed */
                {
                WinQueryWindowText(           /* get text from display window */
                                  WinWindowFromID(hwndPage1, IDD_MACH_MOBOD),
                                  sizeof(Moboname),
                                  Moboname);
                 }

             if (ChipdC)                      /* if contents of text window changed */
                {
                WinQueryWindowText(           /* get text from display window */
                                  WinWindowFromID(hwndPage1, IDD_MACH_CHIPD),
                                  sizeof(Chipset.desc.user),
                                  Chipset.desc.user);
                 }

             if (MakedC)                      /* if contents of text window changed */
                {
                WinQueryWindowText(           /* get text from display window */
                                  WinWindowFromID(hwndPage1, IDD_MACH_MAKED),
                                  sizeof(MachineMake),
                                  MachineMake);
                 }

             if (CachdC)                      /* if contents of text window changed */
                {
                WinQueryWindowText(           /* get text from display window */
                                  WinWindowFromID(hwndPage1, IDD_MACH_CACHD),
                                  sizeof(CacheAmount.desc.user),
                                  CacheAmount.desc.user);
                 }

             if (ProcdC)                      /* if contents of text window changed */
                {
                WinQueryWindowText(           /* get text from display window */
                                  WinWindowFromID(hwndPage1, IDD_MACH_PROCD),
                                  sizeof(Processor.desc.user),
                                  Processor.desc.user);
                 }

             if (GrapdC)                      /* if contents of text window changed */
                {
                WinQueryWindowText(           /* get text from display window */
                                  WinWindowFromID(hwndPage1, IDD_MACH_GRAPD),
                                  sizeof(Graphicscard.desc.user),
                                  Graphicscard.desc.user);
                 }

             hini = PrfOpenProfile(hab, szIniFileName); /* open our profile file */

             PrfWriteProfileData(hini,
                                pszApp,
                                pszKeyName,
                                &Machinename,
                                sizeof(Machinename));
             PrfWriteProfileData(hini,
                                pszApp,
                                pszKeyMobo,
                                &Moboname,
                                sizeof(Moboname));
             PrfWriteProfileData(hini,
                                pszApp,
                                pszKeyProcessor,
                                &Processor,
                                sizeof(Processor));
             PrfWriteProfileData(hini,
                                pszApp,
                                pszKeyMake,
                                &MachineMake,
                                sizeof(MachineMake));
             PrfWriteProfileData(hini,
                                pszApp,
                                pszKeyCache,
                                &CacheAmount,
                                sizeof(CacheAmount));
             PrfWriteProfileData(hini,
                                pszApp,
                                pszKeyChipset,
                                &Chipset,
                                sizeof(Chipset));
             PrfWriteProfileData(hini,
                                pszApp,
                                pszKeyGraphics,
                                &Graphicscard,
                                sizeof(Graphicscard));

             for (i = 0; i < StorageControllers; i++)
                {
                sprintf(pszDisk, "Controller%d", i); /* set key name */

                PrfWriteProfileData(hini,
                                   pszApp,
                                   pszDisk,
                                   DiskController[i],
                                   sizeof(DISKCONTROLLER));
                }

             for (i = 0; i < MAX_FIXED_DISKS; i++)
                {
                sprintf(pszDisk, "DiskCD%d", i); /* set key name */

                PrfWriteProfileData(hini,
                                   pszApp,
                                   pszDisk,
                                   szDisk[i],
                                   sizeof(DISKDESC));

                sprintf(data.c[comp_disk+i].title,
                       "Disk I/O disk %d-%d: %5.0f MB - %s",
                       szDisk[i]->Controller-1,
                       i+1,
                       data.fixed_disk_size[i]/(KB),
                       szDisk[i]->desc.user);
                }

             for (i = 0; i < MAX_CD_DRIVES; i++)
                {
                sprintf(pszDisk, "DiskCD%d", i+10); /* set key name */

                PrfWriteProfileData(hini,
                                   pszApp,
                                   pszDisk,
                                   szDisk[i+10],
                                   sizeof(DISKDESC));

                sprintf(data.c[comp_disk+data.nr_fixed_disks+i].title,
                       "CD-ROM I/O disk %d-%d: %5.0f MB - %s",
                       szDisk[i+10]->Controller-1,
                       i+1,
                       data.cd_drive_size[i]/(KB),
                       szDisk[i+10]->desc.user);
                }

             rc = PrfCloseProfile(hini);

             WinInvalidateRect(hwndClient,
                              NULL,
                              FALSE);

             rc = WinDismissDlg(hwnd, 0UL);  /* dismiss the dialog */
             return 0L;
             }
             break;

          case IDD_DID_CANCEL:
             rc = WinDismissDlg(hwnd, 0UL);  /* dismiss the dialog */

             for (i = 0; i < data.nr_fixed_disks; i++)
                {
                char* n1;

                n1 = strstr(data.c[comp_disk+i].title, " - ");

                strcpy(szDisk[i]->desc.user, n1+3);
                }

             for (i = 0; i < data.nr_cd_drives; i++)
                {
                char* n1;

                n1 = strstr(data.c[comp_disk+data.nr_fixed_disks+i].title, " - ");

                strcpy(szDisk[i+10]->desc.user, n1+3);
                }

             return 0L;
          }
       break;


    default:
       break;
    }
 return (MRESULT)WinDefDlgProc (hwnd, msg, mp1, mp2);
}                                       /* end fnMachineStuff     */


MRESULT APIENTRY fnMachdlgStuff (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
int rc = 0;
HAB hab = WinQueryAnchorBlock(hwnd);

 switch (msg)
    {
    case WM_INITDLG:

       WinSendDlgItemMsg(hwnd, IDD_MACH_NAMED,
                        EM_SETTEXTLIMIT,
                        (MPARAM)99,
                        (MPARAM)0);
       WinSetWindowText(WinWindowFromID(hwnd, IDD_MACH_NAMED),
                       Machinename);
       WinSendDlgItemMsg(hwnd, IDD_MACH_MOBOD,
                        EM_SETTEXTLIMIT,
                        (MPARAM)99,
                        (MPARAM)0);
       WinSetWindowText(WinWindowFromID(hwnd, IDD_MACH_MOBOD),
                       Moboname);
       WinSendDlgItemMsg(hwnd, IDD_MACH_CHIPD,
                        EM_SETTEXTLIMIT,
                        (MPARAM)99,
                        (MPARAM)0);
       WinSetWindowText(WinWindowFromID(hwnd, IDD_MACH_CHIPD),
                       Chipset.desc.user);
       WinSendDlgItemMsg(hwnd, IDD_MACH_MAKED,
                        EM_SETTEXTLIMIT,
                        (MPARAM)99,
                        (MPARAM)0);
       WinSetWindowText(WinWindowFromID(hwnd, IDD_MACH_MAKED),
                       MachineMake);
       WinSendDlgItemMsg(hwnd, IDD_MACH_CACHD,
                        EM_SETTEXTLIMIT,
                        (MPARAM)99,
                        (MPARAM)0);
       WinSetWindowText(WinWindowFromID(hwnd, IDD_MACH_CACHD),
                       CacheAmount.desc.user);
       WinSendDlgItemMsg(hwnd, IDD_MACH_PROCD,
                        EM_SETTEXTLIMIT,
                        (MPARAM)99,
                        (MPARAM)0);
       WinSetWindowText(WinWindowFromID(hwnd, IDD_MACH_PROCD),
                       Processor.desc.user);
       WinSendDlgItemMsg(hwnd, IDD_MACH_GRAPD,
                        EM_SETTEXTLIMIT,
                        (MPARAM)99,
                        (MPARAM)0);
       WinSetWindowText(WinWindowFromID(hwnd, IDD_MACH_GRAPD),
                       Graphicscard.desc.user);
       break;

    case WM_CONTROL:
        {
        switch (SHORT1FROMMP(mp1))
           {
           case IDD_MACH_NAMED:                         /* if text box event */
              {
              if (SHORT2FROMMP(mp1) == EN_CHANGE)       /* if text has changed */
                 {
                 NamedC = 1;                            /* remember that */
                 }
              if (SHORT2FROMMP(mp1) == EN_SETFOCUS)     /* if focus given to this box */
                 {
                 WinSendMsg(WinWindowFromID(hwnd, IDD_MACH_NAMED), /* select text between... */
                            EM_SETSEL,
                            (MPARAM)MPFROM2SHORT(0,99), (MPARAM)0);      /* position 0 and 99 (all of it) */
                 }
              }
              break;

           case IDD_MACH_MOBOD:                         /* if text box event */
              {
              if (SHORT2FROMMP(mp1) == EN_CHANGE)       /* if text has changed */
                 {
                 MobodC = 1;                            /* remember that */
                 }
              if (SHORT2FROMMP(mp1) == EN_SETFOCUS)     /* if focus given to this box */
                 {
                 WinSendMsg(WinWindowFromID(hwnd, IDD_MACH_MOBOD), /* select text between... */
                            EM_SETSEL,
                            (MPARAM)MPFROM2SHORT(0,99), (MPARAM)0);      /* position 0 and 99 (all of it) */
                 }
              }
              break;

           case IDD_MACH_CHIPD:                         /* if text box event */
              {
              if (SHORT2FROMMP(mp1) == EN_CHANGE)       /* if text has changed */
                 {
                 ChipdC = 1;                            /* remember that */
                 }
              if (SHORT2FROMMP(mp1) == EN_SETFOCUS)     /* if focus given to this box */
                 {
                 WinSendMsg(WinWindowFromID(hwnd, IDD_MACH_CHIPD), /* select text between... */
                            EM_SETSEL,
                            (MPARAM)MPFROM2SHORT(0,99), (MPARAM)0);      /* position 0 and 99 (all of it) */
                 }
              }
              break;

           case IDD_MACH_MAKED:                         /* if text box event */
              {
              if (SHORT2FROMMP(mp1) == EN_CHANGE)       /* if text has changed */
                 {
                 MakedC = 1;                            /* remember that */
                 }
              if (SHORT2FROMMP(mp1) == EN_SETFOCUS)     /* if focus given to this box */
                 {
                 WinSendMsg(WinWindowFromID(hwnd, IDD_MACH_MAKED), /* select text between... */
                            EM_SETSEL,
                            (MPARAM)MPFROM2SHORT(0,99), (MPARAM)0);      /* position 0 and 99 (all of it) */
                 }
              }
              break;

           case IDD_MACH_CACHD:                         /* if text box event */
              {
              if (SHORT2FROMMP(mp1) == EN_CHANGE)       /* if text has changed */
                 {
                 CachdC = 1;                            /* remember that */
                 }
              if (SHORT2FROMMP(mp1) == EN_SETFOCUS)     /* if focus given to this box */
                 {
                 WinSendMsg(WinWindowFromID(hwnd, IDD_MACH_CACHD), /* select text between... */
                            EM_SETSEL,
                            (MPARAM)MPFROM2SHORT(0,99), (MPARAM)0);      /* position 0 and 99 (all of it) */
                 }
              }
              break;

           case IDD_MACH_PROCD:                         /* if text box event */
              {
              if (SHORT2FROMMP(mp1) == EN_CHANGE)       /* if text has changed */
                 {
                 ProcdC = 1;                            /* remember that */
                 }
              if (SHORT2FROMMP(mp1) == EN_SETFOCUS)     /* if focus given to this box */
                 {
                 WinSendMsg(WinWindowFromID(hwnd, IDD_MACH_PROCD), /* select text between... */
                            EM_SETSEL,
                            (MPARAM)MPFROM2SHORT(0,99), (MPARAM)0);      /* position 0 and 99 (all of it) */
                 }
              }
              break;

           case IDD_MACH_GRAPD:                         /* if text box event */
              {
              if (SHORT2FROMMP(mp1) == EN_CHANGE)       /* if text has changed */
                 {
                 GrapdC = 1;                            /* remember that */
                 }
              if (SHORT2FROMMP(mp1) == EN_SETFOCUS)     /* if focus given to this box */
                 {
                 WinSendMsg(WinWindowFromID(hwnd, IDD_MACH_GRAPD), /* select text between... */
                            EM_SETSEL,
                            (MPARAM)MPFROM2SHORT(0,99), (MPARAM)0);      /* position 0 and 99 (all of it) */
                 }
              }
              break;
           }
        }

    default:
       break;
    }
 return (MRESULT)WinDefDlgProc (hwnd, msg, mp1, mp2);
}                                       /* end PromptWinProc     */


MRESULT APIENTRY fnDiskStuff (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
ULONG rc = 0, *k, j;
HAB hab = WinQueryAnchorBlock(hwnd);
WINPARM *winparm1;

 switch (msg)
    {
    case WM_INITDLG:

       WinSendDlgItemMsg(hwnd, IDD_DISK_NAME,
                        EM_SETTEXTLIMIT,
                        (MPARAM)100,
                        (MPARAM)0);

       k = (ULONG*)PVOIDFROMMP(mp2);

       winparm1 = (WINPARM*)*k;

       WinSetWindowText(WinWindowFromID(hwnd, IDD_DISK_NAME),
                       winparm1->desc);

       if (winparm1->hwnd == hwndPage2)
          {
          WinSetWindowText(hwnd, "Controller name...");
          }

       WinSetWindowULong(hwnd,
                         QWL_USER,
                         (ULONG)winparm1);
       break;

    case WM_CONTROL:
        {
        switch (SHORT1FROMMP(mp1))
           {
           case IDD_DISK_NAME:                         /* if text box event */
              {
              if (SHORT2FROMMP(mp1) == EN_CHANGE)       /* if text has changed */
                 {
                 DisknC = 1;                            /* remember that */
                 }
              }
              break;
           }
        }
     break;

    case WM_COMMAND:
       switch (COMMANDMSG(&msg)->cmd)
          {
          case IDD_DID_OK:
             {
             if (DisknC)                      /* if contents of text window changed */
                {
                char disktext[101];
                int i;

                WinQueryWindowText(           /* get text from display window */
                                  WinWindowFromID(hwnd, IDD_DISK_NAME),
                                  sizeof(szDiskname),
                                  szDiskname);

                winparm1 = (WINPARM*)WinQueryWindowULong(hwndDlgD,
                                           QWL_USER);

                strcpy(winparm1->desc, szDiskname);

                if (winparm1->hwnd == hwndPage3)
                   {
                   if (winparm1->number >= 10)
                      {
                      i = winparm1->number + data.nr_fixed_disks - 10;
                      sprintf(disktext, "CD %d: % 7.0f MB - %s",
                             winparm1->number-9,
                             data.cd_drive_size[winparm1->number-10]/(KB),
                             (PCSZ)winparm1->desc);
                      }
                   else
                      {
                      i = winparm1->number;
                      sprintf(disktext, "Disk %d: % 5.0f MB - %s",
                             i+1,
                             data.fixed_disk_size[i]/(KB),
                             (PCSZ)winparm1->desc);
                      }

                   WinSendMsg(hwndPage3,
                             LM_SETITEMTEXT,
                             MPFROMSHORT(i),
                             MPFROMP(disktext));
                   } /* end of if page 3 */
                else
                   {
                   WinSendMsg(winparm1->hwnd,
                             LM_SETITEMTEXT,
                             MPFROMSHORT(winparm1->number),
                             MPFROMP(szDiskname));
                   }
                }

             rc = WinDismissDlg(hwnd, 0UL);  /* dismiss the dialog */
             return 0L;
             }
             break;

          case IDD_DID_CANCEL:
             rc = WinDismissDlg(hwnd, 0UL);  /* dismiss the dialog */
             return 0L;
          }
       break;

    default:
       break;
    }
 return (MRESULT)WinDefDlgProc (hwnd, msg, mp1, mp2);
}                                       /* end PromptWinProc     */


void _Optlink WakeMeUp(void* arg)
{
HAB hab;
HMQ hmq;

while (!Initialised)
   {
   DosSleep(500);
   }
   if (!AutoBench)
      {
      hab  = WinInitialize(0);
      hmq  = WinCreateMsgQueue (hab, 0);

      WinPostMsg(hwndClient,
                WM_COMMAND,                        /* and send ourselves a message to display dialog box */
                (MPARAM)MPFROMSHORT(MI_MACHINE_DATA),  /* to be processed when we're initialised */
                (MPARAM)0);

      WinDestroyMsgQueue(hmq);
      WinTerminate ( hab );
      }

   _endthread;
}


void ToggleExpand(ULONG comp)
{
ULONG line;
int i, j;
RECTL rect;

  if ((data.c[comp].bitsettings & (0xffffffff - GROUP_AUTOEXPANDED)) != data.c[comp].bitsettings)
     {
     data.c[comp].bitsettings = data.c[comp].bitsettings & (0xffffffff - GROUP_AUTOEXPANDED);

     if ( ((data.c[comp].bitsettings & (0xffffffff - GROUP_EXPANDED)) == data.c[comp].bitsettings) &&
          (!AllExpanded))
        {
        WinQueryWindowRect(hwndClient, &rect);

        j = (rect.yTop-rect.yBottom) / fontH;      /* client window size divided by font size */

        j = j + ((scroll + (fontH-1) ) / fontH);         /* plus number of lines missing off top, rounded up */

        j = j - (disp_lines - data.c[comp].ndatalines);   /* is # of surplus lines */

        if (j > 0)
           {
           scroll = scroll - (j * fontH);
           }
        }
     }
  else
     {
     data.c[comp].bitsettings = data.c[comp].bitsettings | GROUP_AUTOEXPANDED;
     }

  if (!AllExpanded)
     {
     WinInvalidateRect(hwndClient,
                      NULL,
                      FALSE);
     }
}


void GetVerNum(void)
{
HPIPE hpR, hpW;
RESULTCODES resc;
ULONG ulRead, ulWritten;
CHAR achBuf[PIPESIZE],
     szFailName[CCHMAXPATH];

HFILE hfSave = -1,
      hfNew = HF_STDOUT;

char cmdparms[50] = "cmd.exe\0/c ver /r\0\0";
char* n1 = 0;
char* n2;

     DosDupHandle(HF_STDOUT,
                  &hfSave); /* Saves standard output handle      */

     DosCreatePipe(&hpR,
                   &hpW,
                   PIPESIZE); /* Creates pipe                      */

     DosDupHandle(hpW,
                  &hfNew); /* Duplicates standard output handle */


     DosExecPgm(szFailName,
                sizeof(szFailName), /* Starts child process      */
                EXEC_ASYNC,
                (PSZ) &cmdparms,
                (PSZ) NULL,
                &resc,
                "CMD.EXE");

     DosClose(hpW);                /* Closes write handle to ensure     */
                                   /* Notification at child termination */

     DosDupHandle(hfSave,
                  &hfNew);     /* Brings stdout back                */

     /*
      * Read from the pipe and write to the screen
      * as long as there are bytes to read.
      *
      */

     do {
        DosRead(hpR,
                achBuf,
                sizeof(achBuf),
                &ulRead);

        if (!n1) /* In English */
           {
           n1 = strstr(achBuf, "Revision ");

           if (n1)
              {
              n1 = n1+strlen("Revision ");
              n2 = strstr(n1, " ");
              n2[0] = 0;
              strcpy(version, n1);
              }
           else                 /* try German */
              {
              n1 = strstr(achBuf, "šberarbeitungsversion ");

              if (n1)
                 {
                 n1 = n1+strlen("šberarbeitungsversion ");
                 n2 = strstr(n1, " ");
                 n2[0] = 0;
                 strcpy(version, n1);
                 }
              else                              /* and Spanish */
                 {
                 n1 = strstr(achBuf, "Revisi¢n ");
                 if (n1)
                    {
                    n1 = n1+strlen("Revisi¢n ");
                    n2 = strstr(n1, " ");
                    n2[0] = 0;
                    strcpy(version, n1);
                    }
                 }
              }
           }
        } while(ulRead);
}


/* routine to handle messages from About  box */
MRESULT APIENTRY fnAboutBox      (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
 switch (msg)
    {
    case WM_INITDLG:
       {
       char SysbenchVer[60];

       sprintf(SysbenchVer, "Sysbench %s - %s, %s", SYSB_VER, __DATE__, __TIME__);

       WinSetDlgItemText(hwnd,
                        IDT_PRODINFO_TEXT1,
                        SysbenchVer);
       }
       break;

    default:
       break;
    }

 return (MRESULT)WinDefDlgProc (hwnd, msg, mp1, mp2);
}                                       /* end fnAboutBox  */



