/* Save test reports */
//#define INCL_DOS
//#define INCL_BASE
//#define INCL_DOSMISC
//#define INCL_DOSDEVICES
//#define INCL_DOSERRORS
//#define INCL_DOSFILEMGR
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dive.h>
#include <fourcc.h>
#include <math.h>

#include "types.h"
#include "pmb.h"
#include "pmbbench.h"
#include "pmbdatat.h"
#include <bsedev.h>

extern struct glob_data data;
extern char pszFullFile[CCHMAXPATH]; /* File filter string       */
extern char pszFullFileHTML[CCHMAXPATH]; /* File filter string       */
extern HWND hwndClient;
extern int AutoBench;
extern void WarnBox(char*);
extern char Machinename[100];
extern char Moboname[100];
extern DISKCONTROLLER Processor;
extern char MachineMake[100];
extern DISKCONTROLLER CacheAmount;
extern DISKCONTROLLER Chipset;
extern DISKCONTROLLER Graphicscard;
extern DISKCONTROLLER *DiskController[40];
extern int  StorageControllers;
extern BYTE   coproc;
extern char   CSDlevel[20];
extern char   FIXlevel[20];
extern char    version[10];
extern int gtWarp;
extern ULONG numcpus;
extern ULONG sysinfo[QSV_MAXREAL];
extern ULONG     vernum[2];
extern char   Protectonly[10];
extern float  startsize,
              fatcachesize,
              hpfscachesize;
extern void err(char*);
extern ULONG VideoMem;
extern char  VideoMan[15];
extern char  VideoType[15];
extern USHORT VideoAdapter, VideoChip;
extern char  BIOSname[50];
extern char  BusName[9];

extern DISKDESC *szDisk[20];

// this one in pmcpu3b.cpp
extern char   registered[4];

void      PrintFile(s32, s32, char*, FILE*);
void      SaveResults(void);
void      SaveResultsHtml(void);


void SaveResults(void)
  {
  FILE* fp;
  static char tmp[256];
  s32 i, comp, iterations = NUM_COMPONENTS + (data.nr_fixed_disks-1) + (data.nr_cd_drives-1);
  struct tm *newtime;
  time_t ltime;

  FILEDLG fild;            /* File dialog info structure           */
  char pszTitle[24] = "Save Results to file..."; /* Title of dialog              */
  HWND hwndDlg;            /* File dialog window */

  APIRET rc = 0UL, ret;
  double meg = MB;
  static DIVE_CAPS dc;
  float size;
  char  IDrive[2];

  memset(&fild, 0, sizeof(FILEDLG)); /* set fields in file dlg to zero */

  fild.cbSize   = sizeof(FILEDLG);       /* Size of structure        */
  fild.fl       = FDS_CENTER | FDS_SAVEAS_DIALOG | FDS_ENABLEFILELB      ;
                                    /* FDS_* flags              */
  fild.pszTitle = pszTitle;         /* Dialog title string         */
  if (pszFullFile[1] == ':')
     {
     fild.pszIDrive = (char*)&IDrive;
     IDrive[0] = pszFullFile[0];
     IDrive[1] = pszFullFile[1];
     IDrive[2] = 0;
     strcpy(fild.szFullFile, pszFullFile+2);  /* Initial path,file name, or file filter */
     }
  else
     {
     strcpy(fild.szFullFile, pszFullFile);  /* Initial path,file name, or file filter */
     }

  if (!AutoBench)
     {
     hwndDlg = WinFileDlg(HWND_DESKTOP, hwndClient, &fild);

     if (hwndDlg && (fild.lReturn == DID_OK))
        {
        fp = fopen(fild.szFullFile, "wb");
        }
     else
        {
        return;
        }
     }
  else
     {
     fp = fopen(pszFullFile, "wb");
     }

  if (!fp)
     {
     WarnBox("Cannot open output file");
     return;
     }

  time(&ltime);
  newtime = localtime(&ltime);
  fprintf(fp, "\n\nSysbench " SYSB_VER " result file created %s\n", asctime(newtime));

  fprintf(fp, "Machine name       - %s\n"
              "Manufacturer       - %s\n"
              "Motherboard        - %s\n"
              "Chipset            - %s\n"
              "Processor          - %s\n"
              "External cache     - %s\n"
              "Graphics card      - %s\n",
              Machinename,
              MachineMake,
              Moboname,
              Chipset.desc.user,
              Processor.desc.user,
              CacheAmount.desc.user,
              Graphicscard.desc.user);

  for (i = 0; i < StorageControllers; i++)
     {
     fprintf(fp, "Storage Controller - %s\n",
            DiskController[i]->desc.user);
     }

  fprintf(fp, "Machine data\n"
              "Coprocessor        = %s\n"
              "Processors         = %d\n"
              "RAM                = %3.2f MB\n",
               (coproc ? "Yes" : "No"),
               numcpus,
               (double)((double)sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH]+(double)(384*KB))/meg);

  fprintf(fp, "\nOperating System data\n"
              "OS/2 version       = %d.%d\n"
              "CSDLevel           = %s\n"
              "FIXLevel           = %s\n"
              "Revision number    = %s\n"
              "Priority           = %s\n"
              "Maxwait            = %d\n"
              "Timeslice          = (%d,%d)\n"
              "Protectonly        = %s\n",
               vernum[0],vernum[1],
               CSDlevel,
               FIXlevel,
               version,
               (sysinfo[QSV_DYN_PRI_VARIATION - QSV_MAX_PATH_LENGTH] ? "Dynamic" : "Absolute"),
               sysinfo[QSV_MAX_WAIT - QSV_MAX_PATH_LENGTH],
               sysinfo[QSV_MIN_SLICE - QSV_MAX_PATH_LENGTH],
               sysinfo[QSV_MAX_SLICE - QSV_MAX_PATH_LENGTH],
               Protectonly);

  size = GetSwapFileSize();

  if (size)
     {
     fprintf(fp, "Swap file size     = %4.2fMB\n"
                 "  ...initially     = %4.2fMB\n",
                 (float)size/MB,
                 (float)startsize/MB);
     }
  else
     {
     fprintf(fp, "Unable to determine current swapfile size\n"
                 "   ...initially = %4.2fMB\n",
                 (float)startsize/MB);
     }

   if (gtWarp)
     {
     memset(&dc, 0, sizeof(dc));
     dc.ulStructLen    = sizeof(dc);
     dc.ulFormatLength = 0;
     ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN);
     if (DIVE_SUCCESS != ret)
        {
        if (ret == DIVE_ERR_INSUFFICIENT_LENGTH)
           {
           dc.pFormatData = calloc(dc.ulFormatLength,1);
           ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN); // let's try again
           if (DIVE_SUCCESS != ret)
              {
              err("Error in call to DiveQueryCaps()");
              }
           }
        else
           {
           err("Error in call to DiveQueryCaps()");
           }
        }

     fprintf(fp, "\nVideo data\n"
                 "Resolution         = %dx%dx%u bits/pixel\n",
            dc.ulHorizontalResolution,
            dc.ulVerticalResolution,
            dc.ulDepth);

     fprintf(fp, "Number planes      = %d\n"
                 "Screen Access      = %s\n"
                 "Bank Switched      = %s\n"
                 "Bytes/scanline     = %d\n"
                 "Aperture size      = %d\n",
            dc.ulPlaneCount,
            (dc.fScreenDirect ? "Direct" : "Not Direct"),
            (dc.fBankSwitched ? "Yes" : "No"),
            dc.ulScanLineBytes,
            dc.ulApertureSize);
     fprintf(fp, "Manufact. code     = %d\n"
                 "Chipset code       = %d\n\n",
                 VideoAdapter,
                 VideoChip);
     }
  else
     {
     fprintf(fp, "Video data not listed because OS/2 release less than Warp\n");
     }

  for (comp = 0; comp < iterations; comp++)
     {
     // print title
     PrintFile(1, 1, data.c[comp].title, fp);
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

        PrintFile(1, 3, tmp, fp);
        }
//  PrintFile(1, 3, "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴", fp);
     PrintFile(1, 3, "-----------------------------------------------------------------------", fp);
     if (data.c[comp].total < 0.0)
        sprintf(tmp, "Total                 :       --.---    %s",
                data.c[comp].unit_total);
     else
        sprintf(tmp, "Total                 : %12.3f    %s",
               data.c[comp].total,
               data.c[comp].unit_total);
     PrintFile(2, 3, tmp, fp);
     }

  fclose(fp);

  if (AutoBench)
     {
     SaveResultsHtml();
     WinPostMsg(hwndClient,
               WM_COMMAND,
               (MPARAM)MPFROMSHORT(MI_PROJ_QUIT),
               (MPARAM)MPFROM2SHORT(CMDSRC_MENU, TRUE));
     }
}


void SaveResultsHtml(void)
  {
  FILE* fp;
  static char tmp[256];
  s32 i, comp, iterations = NUM_COMPONENTS + (data.nr_fixed_disks-1) + (data.nr_cd_drives-1);

  FILEDLG fild;            /* File dialog info structure           */
  char pszTitle[24] = "Save Results to file..."; /* Title of dialog              */
  HWND hwndDlg;            /* File dialog window */

  APIRET rc = 0UL, ret;
  double meg = MB, d;
  static DIVE_CAPS dc;
  float size;
  ULONG colours;
  char  IDrive[2], vidres[20], proc[120], *n1;

  memset(&fild, 0, sizeof(FILEDLG)); /* set fields in file dlg to zero */

  fild.cbSize   = sizeof(FILEDLG);       /* Size of structure        */
  fild.fl       = FDS_CENTER | FDS_SAVEAS_DIALOG | FDS_ENABLEFILELB      ;
                                    /* FDS_* flags              */
  fild.pszTitle = pszTitle;         /* Dialog title string         */
  if (pszFullFile[1] == ':')
     {
     fild.pszIDrive = (char*)&IDrive;
     IDrive[0] = pszFullFileHTML[0];
     IDrive[1] = pszFullFileHTML[1];
     IDrive[2] = 0;
     strcpy(fild.szFullFile, pszFullFileHTML+2);  /* Initial path,file name, or file filter */
     }
  else
     {
     strcpy(fild.szFullFile, pszFullFileHTML);  /* Initial path,file name, or file filter */
     }

  if (!AutoBench)
     {
     hwndDlg = WinFileDlg(HWND_DESKTOP, hwndClient, &fild);

     if (hwndDlg && (fild.lReturn == DID_OK))
        {
        fp = fopen(fild.szFullFile, "wb");
        }
     else
        {
        return;
        }
     }
  else
     {
     fp = fopen(pszFullFileHTML, "wb");
     }

  if (!fp)
     {
     WarnBox("Cannot open output file");
     return;
     }

  if (gtWarp)
     {
     memset(&dc, 0, sizeof(dc));
     dc.ulStructLen    = sizeof(dc);
     dc.ulFormatLength = 0;
     ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN);
     if (DIVE_SUCCESS != ret)
        {
        if (ret == DIVE_ERR_INSUFFICIENT_LENGTH)
           {
           dc.pFormatData = calloc(dc.ulFormatLength,1);
           ret = DiveQueryCaps(&dc, DIVE_BUFFER_SCREEN); // let's try again
           if (DIVE_SUCCESS != ret)
              {
              err("Error in call to DiveQueryCaps()");
              }
           }
        else
           {
           err("Error in call to DiveQueryCaps()");
           }
        }

     colours = pow(2, dc.ulDepth);

     sprintf(vidres, "%dx%dx%u",
            dc.ulHorizontalResolution,
            dc.ulVerticalResolution,
            colours);
     }
  else
     {
     sprintf(vidres, "Unknown");
     }

  strcpy(proc, Processor.desc.user);
  n1 = strstr(proc, registered);
  if (n1)
     {
     strcpy(n1, "&reg;");
     n1 = strstr(Processor.desc.user, registered);
     n1 = strstr(n1, " ");
     strcat(proc, n1);
     }

  fprintf(fp, "<HTML>\n<HEAD>\n<TITLE>Sysbench " SYSB_VER " results</TITLE>\n</HEAD>\n");

  fprintf(fp, "<BODY BACKGROUND=\"../../DataBackGround.GIF\" TEXT=\"#000000\""
              " BGCOLOR=\"#FFFFFF\" LINK=\"#0000FF\" VLINK=\"#0000BB\""
              " ALINK=\"#00FFFF\">\n<BLOCKQUOTE><BLOCKQUOTE>\n\n<H3>Sysbench " SYSB_VER " results"
              " - HTML | <A HREF=\"./result.txt\">Text</A></H3>\n");

  fprintf(fp, "<H4>Machine Data </H4>\n"
              "<TABLE BORDER>\n"
              "<TR>\n"
              "<TH>Coprocessor</TH>\n"
              "<TH>Processors</TH>\n"
              "<TH>RAM</TH>\n"
              "<TH>OS/2 Version</TH>\n"
              "<TH>CSD Level</TH>\n"
              "<TH>Fix Level</TH>\n"
              "<TH>Revision #</TH>\n"
              "<TH>Priority</TH>\n"
              "<TH>Maxwait</TH>\n"
              "<TH>Timeslice</TH>\n"
              "<TH>Protectonly</TH>\n"
              "</TR>\n\n");

  d = (double)sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH];
  d = d + (double)(384*KB);
  d = d / meg;

  fprintf(fp, "<TR>\n"
              "<TD ALIGN=CENTER>%s</TD>\n" /* Co-proc */
              "<TD ALIGN=CENTER>%d</TD>\n" /* Processors */
              "<TD ALIGN=CENTER>%3.2f</TD>\n" /*RAM */
              "<TD ALIGN=CENTER>%d.%d</TD>\n"   /* OS/2 version */
              "<TD ALIGN=CENTER>%s</TD>\n" /* CSD Level */
              "<TD ALIGN=CENTER>%s</TD>\n"  /* Fix level */
              "<TD ALIGN=CENTER>%s</TD>\n"  /* rev # */
              "<TD ALIGN=CENTER>%s</TD>\n"  /* priority */
              "<TD ALIGN=CENTER>%d</TD>\n"  /* maxwait */
              "<TD ALIGN=CENTER>(%d,%d)</TD>\n"  /* timeslice */
              "<TD ALIGN=CENTER>%s</TD>\n"  /* protectonly */
              "</TR></TABLE>\n\n",
              (coproc ? "Yes" : "No"),
              numcpus,
              ((double)sysinfo[QSV_TOTPHYSMEM - QSV_MAX_PATH_LENGTH]+(double)(384*KB))/meg,
              vernum[0],vernum[1],
              CSDlevel,
              FIXlevel,
              version,
              (sysinfo[QSV_DYN_PRI_VARIATION - QSV_MAX_PATH_LENGTH] ? "Dynamic" : "Absolute"),
              sysinfo[QSV_MAX_WAIT - QSV_MAX_PATH_LENGTH],
              sysinfo[QSV_MIN_SLICE - QSV_MAX_PATH_LENGTH],
              sysinfo[QSV_MAX_SLICE - QSV_MAX_PATH_LENGTH],
              Protectonly);

  fprintf(fp, "<H4>CPU Integer tests </H4>\n"
              "<TABLE BORDER>\n"
              "<TR>\n"
              "<TH>CPU</TH>\n"
              "<TH>Brand/Board</TH>\n"
              "<TH>Chipset</TH>\n"
              "<TH>BIOS</TH>\n"
              "<TH>Cache/Type</TH>\n"
              "<TH>Dhrystone</TH>\n"
              "<TH>Hanoi</TH>\n"
              "<TH>Heapsort</TH>\n"
              "<TH>Sieve</TH>\n"
              "<TH>CpuI-marks</TH>\n"
              "</TR>\n\n");

  fprintf(fp, "<TR>\n"
              "<TD ALIGN=CENTER>%s</TD>\n" /* CPU */
              "<TD ALIGN=CENTER>%s/%s</TD>\n" /* mobo */
              "<TD ALIGN=CENTER>%s</TD>\n" /*Chipset */
              "<TD ALIGN=CENTER>%s</TD>\n"   /* BIOS */
              "<TD ALIGN=CENTER>%s</TD>\n" /* Cache */
              "<TD ALIGN=CENTER> %12.3f </TD>\n"  /* dhrystone */
              "<TD ALIGN=CENTER> %12.3f </TD>\n"  /* Hanoi */
              "<TD ALIGN=CENTER> %12.3f </TD>\n"  /* heapsort */
              "<TD ALIGN=CENTER> %12.3f </TD>\n"  /* sieve */
              "<TD ALIGN=CENTER><B> %12.3f </B></TD>\n"  /* CPUint */
              "</TR></TABLE>\n\n",
              proc,
              MachineMake,Moboname,
              Chipset.desc.user,
              BIOSname,
              CacheAmount.desc.user,
              data.c[comp_cpuint].datalines[cpuint_dhrystone].value / data.c[comp_cpuint].datalines[cpuint_dhrystone].unit_val,
              data.c[comp_cpuint].datalines[cpuint_hanoi].value / data.c[comp_cpuint].datalines[cpuint_hanoi].unit_val,
              data.c[comp_cpuint].datalines[cpuint_heapsort].value / data.c[comp_cpuint].datalines[cpuint_heapsort].unit_val,
              data.c[comp_cpuint].datalines[cpuint_sieve].value / data.c[comp_cpuint].datalines[cpuint_sieve].unit_val,
              data.c[comp_cpuint].total);

  fprintf(fp, "<H4>CPU Floating Point tests </H4>\n"
              "<TABLE BORDER>\n"
              "<TR>\n"
              "<TH>CPU</TH>\n"
              "<TH>Brand/Board</TH>\n"
              "<TH>Chip set</TH>\n"
              "<TH>Bios</TH>\n"
              "<TH>Cache/Type</TH>\n"
              "<TH>LinPack</TH>\n"
              "<TH>Flops</TH>\n"
              "<TH>FFT</TH>\n"
              "<TH>CpuF-marks</TH>\n"
              "</TR>\n\n");

  fprintf(fp, "<TR>\n"
              "<TD ALIGN=CENTER>%s</TD>\n" /* CPU */
              "<TD ALIGN=CENTER>%s/%s</TD>\n" /* mobo */
              "<TD ALIGN=CENTER>%s</TD>\n" /*Chipset */
              "<TD ALIGN=CENTER>%s</TD>\n"   /* BIOS */
              "<TD ALIGN=CENTER>%s</TD>\n" /* Cache */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* linpack */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* Flops */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* FFT */
              "<TD ALIGN=CENTER><B>%12.3f</B></TD>\n"  /* CPUfloat */
              "</TR></TABLE>\n\n",
              proc,
              MachineMake,
              Moboname,
              Chipset.desc.user,
              BIOSname,
              CacheAmount.desc.user,
              data.c[comp_cpufloat].datalines[cpufloat_linpack].value / data.c[comp_cpufloat].datalines[cpufloat_linpack].unit_val,
              data.c[comp_cpufloat].datalines[cpufloat_flops].value / data.c[comp_cpufloat].datalines[cpufloat_flops].unit_val,
              data.c[comp_cpufloat].datalines[cpufloat_fft].value / data.c[comp_cpufloat].datalines[cpufloat_fft].unit_val,
              data.c[comp_cpufloat].total);

  fprintf(fp, "<H4>DIVE tests </H4>\n"
              "<TABLE BORDER>\n"
              "<TR>\n"
              "<TH>CPU</TH>\n"
              "<TH>Vid Bus</TH>\n"
              "<TH>Vid Card</TH>\n"
              "<TH>Chip Set</TH>\n"
              "<TH>Vid Mem</TH>\n"
              "<TH>Video Bus Bandwidth</TH>\n"
              "<TH>DIVE fun</TH>\n"
              "<TH>M->S DD 1.00:1</TH>\n"
              "<TH>Dive-marks</TH>\n"
              "</TR>\n\n");

  fprintf(fp, "<TR>\n"
              "<TD ALIGN=CENTER>%s</TD>\n" /* CPU */
              "<TD ALIGN=CENTER>%s</TD>\n"   /* video bus */
              "<TD ALIGN=CENTER>%s</TD>\n" /* video card */
              "<TD ALIGN=CENTER>%s %s</TD>\n"   /* video chipset */
              "<TD ALIGN=CENTER> %d MB</TD>\n"   /* video memory */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* Video Bus Bandwidth */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* DIVE fun */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* M->S DD 1.00:1 */
              "<TD ALIGN=CENTER><B>%12.3f</B></TD>\n"  /* Dive-marks */
              "</TR></TABLE>\n\n",
              proc,
              BusName,
              Graphicscard.desc.user,
              VideoMan,
              VideoType,
              VideoMem / MB,
              data.c[comp_dive].datalines[dive_videobw].value / data.c[comp_dive].datalines[dive_videobw].unit_val,
              data.c[comp_dive].datalines[dive_rotate].value / data.c[comp_dive].datalines[dive_rotate].unit_val,
              data.c[comp_dive].datalines[dive_ms_11].value / data.c[comp_dive].datalines[dive_ms_11].unit_val,
              data.c[comp_dive].total);

  fprintf(fp, "<H4>Video tests </H4>\n"
              "<H4>%s</H4>\n"
              "<TABLE BORDER>\n"
              "<TR>\n"
              "<TH>CPU</TH>\n"
              "<TH>Vid Bus</TH>\n"
              "<TH>Vid Card</TH>\n"
              "<TH>Chip Set</TH>\n"
              "<TH>Vid Mem</TH>\n"
              "<TH>BitBlt S->S Copy</TH>\n"
              "<TH>BitBlt M->S Copy</TH>\n"
              "<TH>Filled Rect.</TH>\n"
              "<TH>Pattern Fill</TH>\n"
              "<TH>Vert. Lines</TH>\n"
              "<TH>Horiz. Lines</TH>\n"
              "<TH>Diag. Lines</TH>\n"
              "<TH>Text Render</TH>\n"
              "<TH>PM-marks</TH>\n"
              "</TR>\n\n",
              vidres);

  fprintf(fp, "<TR>\n"
              "<TD ALIGN=CENTER>%s</TD>\n" /* CPU */
              "<TD ALIGN=CENTER>%s</TD>\n"   /* video bus */
              "<TD ALIGN=CENTER>%s</TD>\n" /* video card */
              "<TD ALIGN=CENTER>%s %s</TD>\n"   /* video chipset */
              "<TD ALIGN=CENTER> %d MB</TD>\n"   /* video memory */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* BitBLT S->S */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* BitBLT M->S */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* Filled rect */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* pattern fill */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* Vert lines */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* Horiz lines */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* diag lines */
              "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* text render */
              "<TD ALIGN=CENTER><B>%12.3f</B></TD>\n"  /* PM-marks */
              "</TR></TABLE>\n\n",
              proc,
              BusName,
              Graphicscard.desc.user,
              VideoMan,
              VideoType,
              VideoMem / MB,
              data.c[comp_gfx].datalines[gfx_bitblt_SS].value / data.c[comp_gfx].datalines[gfx_bitblt_SS].unit_val,
              data.c[comp_gfx].datalines[gfx_bitblt_MS].value / data.c[comp_gfx].datalines[gfx_bitblt_MS].unit_val,
              data.c[comp_gfx].datalines[gfx_filled_rect].value / data.c[comp_gfx].datalines[gfx_filled_rect].unit_val,
              data.c[comp_gfx].datalines[gfx_patt_fill].value / data.c[comp_gfx].datalines[gfx_patt_fill].unit_val,
              data.c[comp_gfx].datalines[gfx_vlines].value / data.c[comp_gfx].datalines[gfx_vlines].unit_val,
              data.c[comp_gfx].datalines[gfx_hlines].value / data.c[comp_gfx].datalines[gfx_hlines].unit_val,
              data.c[comp_gfx].datalines[gfx_dlines].value / data.c[comp_gfx].datalines[gfx_dlines].unit_val,
              data.c[comp_gfx].datalines[gfx_textrender].value / data.c[comp_gfx].datalines[gfx_textrender].unit_val,
              data.c[comp_gfx].total);

  fprintf(fp, "<H4>Disk tests</H4>\n"
              "<TABLE BORDER>\n"
              "<TR>\n"
              "<TH>CPU</TH>\n"
              "<TH>Brand/Board</TH>\n"
              "<TH>Bus</TH>\n"
              "<TH>Controller</TH>\n"
              "<TH>Drive</TH>\n"
              "<TH>Avg. Data Access time</TH>\n"
              "<TH>Cache/Bus xfer</TH>\n"
              "<TH>Avg. Transfer</TH>\n"
              "<TH>Disk use CPU load</TH>\n"
              "<TH>DiskIO-marks</TH>\n"
              "</TR>\n\n");

  for (i = 0; i < data.nr_fixed_disks; i++)
     {
     char* n1 = strstr(data.c[comp_disk+i].title, " -");

     fprintf(fp, "<TR>\n"
                 "<TD ALIGN=CENTER>%s</TD>\n" /* CPU */
                 "<TD ALIGN=CENTER>%s/%s</TD>\n" /* mobo */
                 "<TD ALIGN=CENTER>%s</TD>\n"   /* Bus */
                 "<TD ALIGN=CENTER>%s</TD>\n" /* Controller */
                 "<TD ALIGN=CENTER>%s</TD>\n"   /* Drive */
                 "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* data access time */
                 "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* cache/bus */
                 "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* xfer rate */
                 "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* CPU % */
                 "<TD ALIGN=CENTER><B>%12.3f</B></TD>\n"  /* CD mark */
                 "</TR>\n\n",
                 proc,
                 MachineMake,
                 Moboname,
                 BusName,
                 (szDisk[i]->Controller <= 40) ? DiskController[szDisk[i]->Controller-1]->desc.user : "Unknown",
                 n1+2,
                 data.c[comp_disk+i].datalines[disk_avseek].value / data.c[comp_disk+i].datalines[disk_avseek].unit_val,
                 data.c[comp_disk+i].datalines[disk_busxfer].value / data.c[comp_disk+i].datalines[disk_busxfer].unit_val,
                 data.c[comp_disk+i].datalines[disk_transf].value / data.c[comp_disk+i].datalines[disk_transf].unit_val,
                 data.c[comp_disk+i].datalines[disk_cpupct].value / data.c[comp_disk+i].datalines[disk_cpupct].unit_val,
                 data.c[comp_disk+i].total);
     }

  fprintf(fp, "</TABLE>\n\n");

  if (data.nr_cd_drives)
     {
     fprintf(fp, "<H4>CD-ROM tests</H4>\n"
                 "<TABLE BORDER>\n"
                 "<TR>\n"
                 "<TH>CPU</TH>\n"
                 "<TH>Brand/Board</TH>\n"
                 "<TH>Bus</TH>\n"
                 "<TH>Controller</TH>\n"
                 "<TH>Drive</TH>\n"
                 "<TH>Avg. Data Access time</TH>\n"
                 "<TH>Transfer speed (inner)</TH>\n"
                 "<TH>Transfer speed (outer)</TH>\n"
                 "<TH>CD use CPU load</TH>\n"
                 "<TH>CD I/O-marks</TH>\n"
                 "</TR>\n\n");

     for (i = 0; i < data.nr_cd_drives; i++)
        {
        char* n1 = strstr(data.c[comp_disk+data.nr_fixed_disks+i].title, " -");
        fprintf(fp, "<TR>\n"
                    "<TD ALIGN=CENTER>%s</TD>\n" /* CPU */
                    "<TD ALIGN=CENTER>%s/%s</TD>\n" /* mobo */
                    "<TD ALIGN=CENTER>%s</TD>\n"   /* Bus */
                    "<TD ALIGN=CENTER>%s</TD>\n" /* Controller */
                    "<TD ALIGN=CENTER>%s</TD>\n"   /* Drive */
                    "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* data access time */
                    "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* inner xfer rate */
                    "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* outer xfer rate */
                    "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* CPU % */
                    "<TD ALIGN=CENTER><B>%12.3f</B></TD>\n"  /* CD mark */
                    "</TR>\n\n",
                    proc,
                    MachineMake,
                    Moboname,
                    BusName,
                    (szDisk[i+10]->Controller <= 40) ? DiskController[szDisk[i+10]->Controller-1]->desc.user : "Unknown",
                    n1+2,
                    data.c[comp_disk+data.nr_fixed_disks+i].datalines[cdio_avseek].value /
                          data.c[comp_disk+data.nr_fixed_disks+i].datalines[cdio_avseek].unit_val,
                    data.c[comp_disk+data.nr_fixed_disks+i].datalines[cdio_inner].value /
                          data.c[comp_disk+data.nr_fixed_disks+i].datalines[cdio_inner].unit_val,
                    data.c[comp_disk+data.nr_fixed_disks+i].datalines[cdio_outer].value /
                          data.c[comp_disk+data.nr_fixed_disks+i].datalines[cdio_outer].unit_val,
                    data.c[comp_disk+data.nr_fixed_disks+i].datalines[cdio_cpupct].value /
                          data.c[comp_disk+data.nr_fixed_disks+i].datalines[cdio_cpupct].unit_val,
                    data.c[comp_disk+data.nr_fixed_disks+i].total);
        }
     fprintf(fp, "</TABLE>\n\n");
     }


  if (data.nr_fixed_disks > 1)
     {
     fprintf(fp, "<H4>Simultaneous I/O tests</H4>\n"
                 "<TABLE BORDER>\n"
                 "<TR>\n"
                 "<TH>CPU</TH>\n"
                 "<TH>Brand/Board</TH>\n"
                 "<TH>Bus</TH>\n"
                 "<TH>Controller</TH>\n"
                 "<TH>Drive</TH>\n"
                 "<TH>Transfer rate - track 0</TH>\n"
                 "</TR>\n\n");

     for (i = 0; i < data.nr_fixed_disks; i++)
        {
        char* n1 = strstr(data.c[comp_disk+i].title, " -");
        fprintf(fp, "<TR>\n"
                    "<TD ALIGN=CENTER>%s</TD>\n" /* CPU */
                    "<TD ALIGN=CENTER>%s/%s</TD>\n" /* mobo */
                    "<TD ALIGN=CENTER>%s</TD>\n"   /* Bus */
                    "<TD ALIGN=CENTER>%s</TD>\n" /* Controller */
                    "<TD ALIGN=CENTER>%s</TD>\n"   /* Drive */
                    "<TD ALIGN=CENTER>%12.3f</TD>\n"  /* xfer rate */
                    "</TR>\n\n",
                    proc,
                    MachineMake,
                    Moboname,
                    BusName,
                    (szDisk[i]->Controller <= 40) ? DiskController[szDisk[i]->Controller-1]->desc.user : "Unknown",
                    n1+2,
                    data.c[comp_alldisks].datalines[i].value / data.c[comp_alldisks].datalines[i].unit_val);
        }

     fprintf(fp, "<TR>\n"
                 "<TD ALIGN=CENTER>Total</TD>\n" /* CPU */
                 "<TD ALIGN=CENTER></TD>\n" /* mobo */
                 "<TD ALIGN=CENTER></TD>\n"   /* Bus */
                 "<TD ALIGN=CENTER></TD>\n" /* Controller */
                 "<TD ALIGN=CENTER></TD>\n"   /* Drive */
                 "<TD ALIGN=CENTER><B>%12.3f</B></TD>\n"  /* xfer rate */
                 "</TR>\n\n",
                 data.c[comp_alldisks].total);
     fprintf(fp, "</TABLE>\n\n");
     }

  fprintf(fp, "<BR>\nBack to Sysbench <A HREF=\"../index.html\">" SYSB_VER "</A> Results Main Page.\n"
              "<BR>\n<BR>\n\n</BLOCKQUOTE></BLOCKQUOTE></BODY>\n</HTML>\n");

  fclose(fp);
}


void PrintFile(s32 newlines, s32 col, char* string, FILE* fp)
  {
  s32 i;
  for (i = 0; i < col; i++)
      fprintf(fp, " ");

  fprintf(fp, "%s", string);

  for (i = 0; i < newlines; i++)
      fprintf(fp, "\n");
  }


