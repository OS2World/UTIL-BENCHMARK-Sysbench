
/* Main benchmark file */

#define INCL_DOSMISC
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES   /* Semaphore values */
#include <os2.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>
#include <string.h>

#include "types.h"
#include "pmbbench.h"
#include "pmbdatat.h"
#include "pmb.h"


#define d1 (12345678.901*(((double)rand())/RAND_MAX+0.5))
#define d2 (12.345678901*(((double)rand())/RAND_MAX+0.5))
#define MIN_DHRY_TIME 10.0
#define MIN_MEASURE 0.1
#define MARGINAL 1.1

static bool mult;  // true -> runs multiple tests in one thread
static void* p;
double mh, mv, md, mm, mc;
struct    cpuresults {
          double rc;
          ULONG func;
          };

extern void PostFin(int);
double dtime(void);
static void EndBench(void);
extern double pmb_fft(void);
extern double pmb_flops(void);
extern double pmb_linpack(void);
extern double pmb_dhry(int);
extern double pmb_hanoi(void);
extern double pmb_heaps(void);
extern double pmb_sieve(void);
extern double pmb_diskio_avseek(int);
extern double pmb_buscache_xfer(int);
extern double pmb_diskio_transfer0(int);
extern double pmb_diskio_transfern2(int disknr);
extern double pmb_diskio_transfern(int disknr);
extern double pmb_diskio_cpupct(int);
extern double pmb_cdio_avseek(int);
extern double pmb_cdio_inner(int);
extern double pmb_cdio_outer(int);
extern double pmb_cdio_cpupct(int);
extern double pmb_dive_bw(void);
extern double pmb_dive_rot(void);
extern double pmb_dive_ms11(void);
extern double pmb_gfx_bitblitss(void);
extern double pmb_gfx_bitblitms(void);
extern double pmb_gfx_dlines(void);
extern double pmb_gfx_hlines(void);
extern double pmb_gfx_patrect(void);
extern double pmb_gfx_fillrect(void);
extern double pmb_gfx_textrender(void);
extern double pmb_gfx_vlines(void);
extern double pmb_memspeed(s32);
extern double pmb_memspeedr(s32);
extern double pmb_memspeedw(s32);
extern void _Optlink TimeThread(void*);
extern double DoFileIO(ULONG, BOOL, BOOL, BOOL);
extern void  AddTitle(char* s);
extern void  DelTitle(void);
extern HEV   hevFileIO;
extern ULONG ulPostCount2;
extern void WarnBox(char*);
extern void ToggleExpand(ULONG);

extern struct glob_data data;
extern int gtWarp;
extern ULONG numcpus;
extern int Beep;
extern ULONG swapfilegrown;
extern ULONG maxswapfilesize;
extern float startsize;
extern volatile int Timeout;
extern BOOL fileiodisabled;
extern char invocationpath[CCHMAXPATH];

double rt;

void   MultiDisk(char*);
double CalcSimIOAv(void);

void _Optlink DoAll(void* p)
{
  mult = true;
  DoAllGraphics(p);
  DoAllCPUInt(p);
  DoAllCPUFloat(p);
  DoAllDIVE(p);
  DoFileIOAll(p);
  DoAllMem(p);
  DoSimDiskIO(p);
  DoAllDiskIOAll(p);
  if (data.nr_cd_drives)
     {
     DoAllCDIOAll(p);
     }
  mult = false;
  PostFin(false);
  if (Beep)
     {
     DosBeep(440, 100);
     DosBeep(550, 75);
     DosBeep(687, 75);
     DosBeep(860, 250);
     }
}


void _Optlink DoAllCPUFloat(void* p)
{
  if (!mult)
     {
     mult = true;
     ToggleExpand(comp_cpufloat);
     AddTitle("Linpack");
     DoCPUFloatLinpack(p);
     DelTitle();
     AddTitle("Flops");
     DoCPUFloatFlops(p);
     DelTitle();
     AddTitle("FFT");
     DoCPUFloatFFT(p);
     DelTitle();
     ToggleExpand(comp_cpufloat);
     mult = false;
     PostFin(false);
     }
  else
     {
     ToggleExpand(comp_cpufloat);
     AddTitle("Linpack");
     DoCPUFloatLinpack(p);
     DelTitle();
     AddTitle("Flops");
     DoCPUFloatFlops(p);
     DelTitle();
     AddTitle("FFT");
     DoCPUFloatFFT(p);
     DelTitle();
     ToggleExpand(comp_cpufloat);
     }
}


void _Optlink DoAllCPUInt(void* p)
{
  if (!mult)
     {
     mult = true;
     ToggleExpand(comp_cpuint);
     AddTitle("Dhrystone");
     DoCPUIntDhry(p);
     DelTitle();
     AddTitle("Hanoi");
     DoCPUIntHanoi(p);
     DelTitle();
     AddTitle("Heapsort");
     DoCPUIntHeaps(p);
     DelTitle();
     AddTitle("Sieve");
     DoCPUIntSieve(p);
     DelTitle();
     mult = false;
     ToggleExpand(comp_cpuint);
     PostFin(false);
     }
  else
     {
     ToggleExpand(comp_cpuint);
     AddTitle("Dhrystone");
     DoCPUIntDhry(p);
     DelTitle();
     AddTitle("Hanoi");
     DoCPUIntHanoi(p);
     DelTitle();
     AddTitle("Heapsort");
     DoCPUIntHeaps(p);
     DelTitle();
     AddTitle("Sieve");
     DoCPUIntSieve(p);
     DelTitle();
     ToggleExpand(comp_cpuint);
     }
}


void _Optlink DoAllDIVE(void* p)
{
  if (!mult)
     {
     mult = true;
     ToggleExpand(comp_dive);
     AddTitle("DIVE Bandwidth");
     DoDiveVBW(p);
     DelTitle();
     AddTitle("DIVE Fun");
     DoDiveRot(p);
     DelTitle();
     AddTitle("DIVE M->S");
     DoDiveMS11(p);
     DelTitle();
     mult = false;
     ToggleExpand(comp_dive);
     PostFin(false);
     }
  else
     {
     ToggleExpand(comp_dive);
     AddTitle("DIVE Bandwidth");
     DoDiveVBW(p);
     DelTitle();
     AddTitle("DIVE Fun");
     DoDiveRot(p);
     DelTitle();
     AddTitle("DIVE M->S");
     DoDiveMS11(p);
     DelTitle();
     ToggleExpand(comp_dive);
     }
  }


void _Optlink DoAllDiskIO(void* p)
{
  if (!mult)
     {
     mult = true;
     ToggleExpand(comp_disk+data.selected_disk);
     AddTitle("Disk Avg time");
     DoDiskIOAvSeek(p);
     DelTitle();
     AddTitle("Cache/bus");
     DoDiskCacheXfer(p);
     DelTitle();
     AddTitle("Xfer speed");
     DoDiskIOTransSpeed(p);
     DelTitle();
     AddTitle("CPU %");
     DoDiskIOCPUUsage(p);
     DelTitle();
     ToggleExpand(comp_disk+data.selected_disk);
     mult = false;
     PostFin(false);
     }
  else
     {
     ToggleExpand(comp_disk+data.selected_disk);
     AddTitle("Disk Avg time");
     DoDiskIOAvSeek(p);
     DelTitle();
     AddTitle("Cache/bus");
     DoDiskCacheXfer(p);
     DelTitle();
     AddTitle("Xfer speed");
     DoDiskIOTransSpeed(p);
     DelTitle();
     AddTitle("CPU %");
     DoDiskIOCPUUsage(p);
     DelTitle();
     ToggleExpand(comp_disk+data.selected_disk);
     }
}


void _Optlink DoAllCDIOAll(void* p)
{
 int i, saved;

 saved = data.selected_cd;

 if (!mult)
    {
    mult = true;
    for (i = 0; i < data.nr_cd_drives; i++)
       {
       data.selected_cd = i;
       DoAllCDIO((void*)i);
       }
    mult = false;
    PostFin(false);
    }
 else
    {
    for (i = 0; i < data.nr_cd_drives; i++)
       {
       data.selected_cd = i;
       DoAllCDIO((void*)i);
       }
    }
 data.selected_cd = saved;
 }


void _Optlink DoAllDiskIOAll(void* p)
{
 int i, saved;

 saved = data.selected_disk;

 if (!mult)
    {
    mult = true;
    for (i = 0; i < data.nr_fixed_disks; i++)
       {
       data.selected_disk = i;
       DoAllDiskIO((void*)i);
       }
    mult = false;
    PostFin(false);
    }
 else
    {
    for (i = 0; i < data.nr_fixed_disks; i++)
       {
       data.selected_disk = i;
       DoAllDiskIO((void*)i);
       }
    }
 data.selected_disk = saved;
 }


void _Optlink DoSimDiskIO(void* p)
{
ULONG i;
TID ttid[MAX_FIXED_DISKS];
int index  = 0;
int r;
char exe[14] = "diskit.exe";

 ToggleExpand(comp_alldisks);

 if (!mult)
    {
    mult = true;
    AddTitle("Simultaneous I/O");
    MultiDisk(exe);
    DelTitle();
    mult = false;
    PostFin(false);
    }
 else
    {
    AddTitle("Simultaneous I/O");
    MultiDisk(exe);
    DelTitle();
    }

 ToggleExpand(comp_alldisks);
 EndBench();
 }


void MultiDisk(char* file2exe)
    {
    int                       i = 0;
    ULONG                result = 0;
    APIRET                   rc = 0;
    UCHAR LoadError[CCHMAXPATH] = {0};
    RESULTCODES  resultcode[16] = {0};
    PID            pidChild[16] = {0};
    UCHAR   exefile[CCHMAXPATH] = {0};
    UCHAR   args[CCHMAXPATH];
    int bodge = 0;

    strcpy((char*)exefile, invocationpath);
    strcat((char*)exefile, "\\");
    strcat((char*)exefile, file2exe);

    for (i = 0; i <= data.nr_fixed_disks-1; i++)
       {
       sprintf((char*)args, "%s  %d \0", file2exe, i+1);
       bodge = strlen(file2exe);
       args[bodge] = 0;

       rc = DosExecPgm((PCHAR)LoadError,
                      sizeof(LoadError),
                      EXEC_ASYNCRESULT,
                      (PCSZ)args,
                      0,
                      &resultcode[i],
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
          printf("%s\n", tmp);
          return;
          }
       }

    for (i = 0; i <= data.nr_fixed_disks-1; i++)
       {
       rc = DosWaitChild(DCWA_PROCESS,
                        DCWW_WAIT,
                        &resultcode[i],
                        &pidChild[i],
                        resultcode[i].codeTerminate);
       if (rc)
          {
          printf("DosWaitChild rc = %x\n", rc);
          return;
          }
       }

    for (i = 0; i <= data.nr_fixed_disks-1; i++)
       {
       printf("disk %d codeResult=%d codeTerminate=%d\n", i, resultcode[i].codeResult * 100, resultcode[i].codeTerminate);
       if (resultcode[i].codeResult >= 0 && resultcode[i].codeResult <= 65534)
          {
          data.c[comp_alldisks].datalines[i].value = (double)resultcode[i].codeResult * 100;
          }
       else
          {
          data.c[comp_alldisks].datalines[i].value = -1.0;
          }
       }

    return;
    }


void _Optlink DoAllCDIO(void* p)
{
  if (!mult)
     {
     mult = true;
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     AddTitle("CD Seek time");
     DoCDIOAvSeek(p);
     DelTitle();
     AddTitle("CD inner speed");
     DoCDIOInnerSpeed(p);
     DelTitle();
     AddTitle("CD outer speed");
     DoCDIOOuterSpeed(p);
     DelTitle();
     AddTitle("CD CPU %");
     DoCDIOCPUUsage(p);
     DelTitle();
     mult = false;
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     PostFin(false);
     }
  else
     {
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     AddTitle("CD Seek time");
     DoCDIOAvSeek(p);
     DelTitle();
     AddTitle("CD inner speed");
     DoCDIOInnerSpeed(p);
     DelTitle();
     AddTitle("CD outer speed");
     DoCDIOOuterSpeed(p);
     DelTitle();
     AddTitle("CD CPU %");
     DoCDIOCPUUsage(p);
     DelTitle();
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     }
}



void _Optlink DoAllGraphics(void* p)
{
  if (!mult)
     {
     mult = true;
     ToggleExpand(comp_gfx);
     AddTitle("BitBltS->S");
     DoGfxBlitBlitSS(p);
     DelTitle();
     AddTitle("BitBltM->S");
     DoGfxBlitBlitMS(p);
     DelTitle();
     AddTitle("Filled Rectangle");
     DoGfxFillRect(p);
     DelTitle();
     AddTitle("Pattern Fill");
     DoGfxPatFil(p);
     DelTitle();
     AddTitle("Vertical Lines");
     DoGfxVLines(p);
     DelTitle();
     AddTitle("Horizontal Lines");
     DoGfxHLines(p);
     DelTitle();
     AddTitle("Diagonal Lines");
     DoGfxDLines(p);
     DelTitle();
     AddTitle("Text Render");
     DoGfxTextRender(p);
     DelTitle();
     mult = false;
     ToggleExpand(comp_gfx);
     PostFin(false);
     }
  else
     {
     ToggleExpand(comp_gfx);
     AddTitle("BitBltS->S");
     DoGfxBlitBlitSS(p);
     DelTitle();
     AddTitle("BitBltM->S");
     DoGfxBlitBlitMS(p);
     DelTitle();
     AddTitle("Filled Rectangle");
     DoGfxFillRect(p);
     DelTitle();
     AddTitle("Pattern Fill");
     DoGfxPatFil(p);
     DelTitle();
     AddTitle("Vertical Lines");
     DoGfxVLines(p);
     DelTitle();
     AddTitle("Horizontal Lines");
     DoGfxHLines(p);
     DelTitle();
     AddTitle("Diagonal Lines");
     DoGfxDLines(p);
     DelTitle();
     AddTitle("Text Render");
     DoGfxTextRender(p);
     DelTitle();
     ToggleExpand(comp_gfx);
     }
}


void _Optlink DoAllMem(void* p)
{
  if (!mult)
     {
     mult = true;
     ToggleExpand(comp_mem);
     AddTitle("Mem Copy 5Kb");
     DoMem5(p);
     DelTitle();
     AddTitle("Mem Copy 10Kb");
     DoMem10(p);
     DelTitle();
     AddTitle("Mem Copy 20Kb");
     DoMem20(p);
     DelTitle();
     AddTitle("Mem Copy 40Kb");
     DoMem40(p);
     DelTitle();
     AddTitle("Mem Copy 80Kb");
     DoMem80(p);
     DelTitle();
     AddTitle("Mem Copy 160Kb");
     DoMem160(p);
     DelTitle();
     AddTitle("Mem Copy 320Kb");
     DoMem320(p);
     DelTitle();
     AddTitle("Mem Copy 640Kb");
     DoMem640(p);
     DelTitle();
     AddTitle("Mem Copy 1280Kb");
     DoMem1280(p);
     DelTitle();
     AddTitle("Mem Read 5Kb");
     DoMemR5(p);
     DelTitle();
     AddTitle("Mem Read 10Kb");
     DoMemR10(p);
     DelTitle();
     AddTitle("Mem Read 20Kb");
     DoMemR20(p);
     DelTitle();
     AddTitle("Mem Read 40Kb");
     DoMemR40(p);
     DelTitle();
     AddTitle("Mem Read 80Kb");
     DoMemR80(p);
     DelTitle();
     AddTitle("Mem Read 160Kb");
     DoMemR160(p);
     DelTitle();
     AddTitle("Mem Read 320Kb");
     DoMemR320(p);
     DelTitle();
     AddTitle("Mem Read 640Kb");
     DoMemR640(p);
     DelTitle();
     AddTitle("Mem Read 1280Kb");
     DoMemR1280(p);
     DelTitle();
     AddTitle("Mem Write 5Kb");
     DoMemW5(p);
     DelTitle();
     AddTitle("Mem Write 10Kb");
     DoMemW10(p);
     DelTitle();
     AddTitle("Mem Write 20Kb");
     DoMemW20(p);
     DelTitle();
     AddTitle("Mem Write 40Kb");
     DoMemW40(p);
     DelTitle();
     AddTitle("Mem Write 80Kb");
     DoMemW80(p);
     DelTitle();
     AddTitle("Mem Write 160Kb");
     DoMemW160(p);
     DelTitle();
     AddTitle("Mem Write 320Kb");
     DoMemW320(p);
     DelTitle();
     AddTitle("Mem Write 640Kb");
     DoMemW640(p);
     DelTitle();
     AddTitle("Mem Write 1280Kb");
     DoMemW1280(p);
     DelTitle();
     mult = false;
     ToggleExpand(comp_mem);
     PostFin(false);
     }
  else
     {
     ToggleExpand(comp_mem);
     AddTitle("Mem Copy 5Kb");
     DoMem5(p);
     DelTitle();
     AddTitle("Mem Copy 10Kb");
     DoMem10(p);
     DelTitle();
     AddTitle("Mem Copy 20Kb");
     DoMem20(p);
     DelTitle();
     AddTitle("Mem Copy 40Kb");
     DoMem40(p);
     DelTitle();
     AddTitle("Mem Copy 80Kb");
     DoMem80(p);
     DelTitle();
     AddTitle("Mem Copy 160Kb");
     DoMem160(p);
     DelTitle();
     AddTitle("Mem Copy 320Kb");
     DoMem320(p);
     DelTitle();
     AddTitle("Mem Copy 640Kb");
     DoMem640(p);
     DelTitle();
     AddTitle("Mem Copy 1280Kb");
     DoMem1280(p);
     DelTitle();
     AddTitle("Mem Read 5Kb");
     DoMemR5(p);
     DelTitle();
     AddTitle("Mem Read 10Kb");
     DoMemR10(p);
     DelTitle();
     AddTitle("Mem Read 20Kb");
     DoMemR20(p);
     DelTitle();
     AddTitle("Mem Read 40Kb");
     DoMemR40(p);
     DelTitle();
     AddTitle("Mem Read 80Kb");
     DoMemR80(p);
     DelTitle();
     AddTitle("Mem Read 160Kb");
     DoMemR160(p);
     DelTitle();
     AddTitle("Mem Read 320Kb");
     DoMemR320(p);
     DelTitle();
     AddTitle("Mem Read 640Kb");
     DoMemR640(p);
     DelTitle();
     AddTitle("Mem Read 1280Kb");
     DoMemR1280(p);
     DelTitle();
     AddTitle("Mem Write 5Kb");
     DoMemW5(p);
     DelTitle();
     AddTitle("Mem Write 10Kb");
     DoMemW10(p);
     DelTitle();
     AddTitle("Mem Write 20Kb");
     DoMemW20(p);
     DelTitle();
     AddTitle("Mem Write 40Kb");
     DoMemW40(p);
     DelTitle();
     AddTitle("Mem Write 80Kb");
     DoMemW80(p);
     DelTitle();
     AddTitle("Mem Write 160Kb");
     DoMemW160(p);
     DelTitle();
     AddTitle("Mem Write 320Kb");
     DoMemW320(p);
     DelTitle();
     AddTitle("Mem Write 640Kb");
     DoMemW640(p);
     DelTitle();
     AddTitle("Mem Write 1280Kb");
     DoMemW1280(p);
     DelTitle();
     ToggleExpand(comp_mem);
     }
}


void _Optlink DoFileIOAll(void* p)
{
if (!fileiodisabled)
   {
   if (!mult)
      {
      mult = true;
      ToggleExpand(comp_file);
      AddTitle("4Kb File");
      DoFileIO4(p);
      DelTitle();
      AddTitle("8Kb File");
      DoFileIO8(p);
      DelTitle();
      AddTitle("16Kb File");
      DoFileIO16(p);
      DelTitle();
      AddTitle("32Kb File");
      DoFileIO32(p);
      DelTitle();
      AddTitle("64Kb File");
      DoFileIO64(p);
      DelTitle();
      mult = false;
      ToggleExpand(comp_file);
      PostFin(false);
      }
   else
      {
      ToggleExpand(comp_file);
      AddTitle("4Kb File");
      DoFileIO4(p);
      DelTitle();
      AddTitle("8Kb File");
      DoFileIO8(p);
      DelTitle();
      AddTitle("16Kb File");
      DoFileIO16(p);
      DelTitle();
      AddTitle("32Kb File");
      DoFileIO32(p);
      DelTitle();
      AddTitle("64Kb File");
      DoFileIO64(p);
      DelTitle();
      ToggleExpand(comp_file);
      }
   }
}


double MultiTask(char* file2exe)
    {
    int                       i = 0;
    ULONG                result = 0;
    APIRET                   rc = 0;
    UCHAR LoadError[CCHMAXPATH] = {0};
    RESULTCODES  resultcode[16] = {0};
    PID            pidChild[16] = {0};
    UCHAR   exefile[CCHMAXPATH] = {0};
    char*	cmdparams[16];

    strcpy((char*)exefile, invocationpath);
    strcat((char*)exefile, "\\");
    strcat((char*)exefile, file2exe);

    for (i = 0; i <= numcpus-1; i++)
       {
	cmdparams[i] = (char*)malloc(CCHMAXPATH);
	memset(cmdparams[i], 0, CCHMAXPATH);
	sprintf(cmdparams[i],"%s /osb%06d\0\0", exefile, i);
	cmdparams[i][strlen((char*)exefile)] = 0;
       rc = DosExecPgm((PCHAR)LoadError,
                      sizeof(LoadError),
                      EXEC_ASYNCRESULT,
                      (PSZ) cmdparams[i],
                      (PSZ) NULL,
                      &resultcode[i],
                      (PCSZ)exefile);
       free(cmdparams[i]);
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
       }

    for (i = 0; i <= numcpus-1; i++)
       {
       rc = DosWaitChild(DCWA_PROCESS,
                        DCWW_WAIT,
                        &resultcode[i],
                        &pidChild[i],
                        resultcode[i].codeTerminate);
       if (rc)
          {
          printf("DosWaitChild rc = %x\n", rc);
          return -1;
          }
       }

    for (i = 0; i <= numcpus-1; i++)
       {
	char tmp[CCHMAXPATH];
	char input[40];
	FILE* fin;

	sprintf(tmp, "sb%06d", i);
	fin = fopen(tmp, "r");
	if (fin)
		{
		memset(input, 0, 40);
		fread(input, 40, 1, fin);
		fclose(fin);
		unlink(tmp);
		result += atol(input);
		}
//       result = result + resultcode[i].codeResult;
       }

    return (double)result;
    }


void _Optlink DoCPUFloatFFT(void* p)
{
char pmbfft[14] = "pmb_fft.exe";

  if ( (data.c[comp_cpufloat].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_cpufloat].bitsettings)
     {
     ToggleExpand(comp_cpufloat);
     data.c[comp_cpufloat].datalines[cpufloat_fft].value = MultiTask(pmbfft);
     ToggleExpand(comp_cpufloat);
     }
  else
     {
     data.c[comp_cpufloat].datalines[cpufloat_fft].value = MultiTask(pmbfft);
     }

  EndBench();
}


void _Optlink DoCPUFloatFlops(void* p)
{
char pmbflops[14] = "pmbflops.exe";

  if ((data.c[comp_cpufloat].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_cpufloat].bitsettings)
     {
     ToggleExpand(comp_cpufloat);
     data.c[comp_cpufloat].datalines[cpufloat_flops].value = MultiTask(pmbflops);
     ToggleExpand(comp_cpufloat);
     }
  else
     {
     data.c[comp_cpufloat].datalines[cpufloat_flops].value = MultiTask(pmbflops);
     }

  EndBench();
}


void _Optlink DoCPUFloatLinpack(void* p)
{
char pmblinpk[14] = "pmblinpk.exe";

  if ((data.c[comp_cpufloat].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_cpufloat].bitsettings)
     {
     ToggleExpand(comp_cpufloat);
     data.c[comp_cpufloat].datalines[cpufloat_linpack].value = MultiTask(pmblinpk);
     ToggleExpand(comp_cpufloat);
     }
  else
     {
     data.c[comp_cpufloat].datalines[cpufloat_linpack].value = MultiTask(pmblinpk);
     }

  EndBench();
}


void _Optlink DoCPUIntDhry(void* p)
{
char pmbdhry[14] = "pmb_dhry.exe";
  if ((data.c[comp_cpuint].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_cpuint].bitsettings)
     {
     ToggleExpand(comp_cpuint);
     data.c[comp_cpuint].datalines[cpuint_dhrystone].value = MultiTask(pmbdhry);
     ToggleExpand(comp_cpuint);
     }
  else
     {
     data.c[comp_cpuint].datalines[cpuint_dhrystone].value = MultiTask(pmbdhry);
     }

  EndBench();
}


void _Optlink DoCPUIntHanoi(void* p)
{
char pmbhanoi[14] = "pmbhanoi.exe";

  if ((data.c[comp_cpuint].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_cpuint].bitsettings)
     {
     ToggleExpand(comp_cpuint);
     data.c[comp_cpuint].datalines[cpuint_hanoi].value = MultiTask(pmbhanoi);
     ToggleExpand(comp_cpuint);
     }
  else
     {
     data.c[comp_cpuint].datalines[cpuint_hanoi].value = MultiTask(pmbhanoi);
     }

  EndBench();
}


void _Optlink DoCPUIntHeaps(void* p)
{
char pmbheaps[14] = "pmbheaps.exe";

  if ((data.c[comp_cpuint].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_cpuint].bitsettings)
     {
     ToggleExpand(comp_cpuint);
     data.c[comp_cpuint].datalines[cpuint_heapsort].value = MultiTask(pmbheaps);
     ToggleExpand(comp_cpuint);
     }
  else
     {
     data.c[comp_cpuint].datalines[cpuint_heapsort].value = MultiTask(pmbheaps);
     }

  EndBench();
}


void _Optlink DoCPUIntSieve(void* p)
{
char pmbsieve[14] = "pmbsieve.exe";

  if ((data.c[comp_cpuint].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_cpuint].bitsettings)
     {
     ToggleExpand(comp_cpuint);
     data.c[comp_cpuint].datalines[cpuint_sieve].value = MultiTask(pmbsieve);
     ToggleExpand(comp_cpuint);
     }
  else
     {
     data.c[comp_cpuint].datalines[cpuint_sieve].value = MultiTask(pmbsieve);
     }

  EndBench();
}


void _Optlink DoDiskIOAvSeek(void* p)
{
  if ((data.c[comp_disk+data.selected_disk].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_disk+data.selected_disk].bitsettings)
     {
     ToggleExpand(comp_disk+data.selected_disk);
     data.c[comp_disk+data.selected_disk].datalines[disk_avseek].value = (pmb_diskio_avseek(data.selected_disk)/(10*1000));
     ToggleExpand(comp_disk+data.selected_disk);
     }
  else
     {
     data.c[comp_disk+data.selected_disk].datalines[disk_avseek].value = (pmb_diskio_avseek(data.selected_disk)/(10*1000));
     }

  EndBench();
}


void _Optlink DoDiskCacheXfer(void* p)
{
  if ((data.c[comp_disk+data.selected_disk].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_disk+data.selected_disk].bitsettings)
     {
     ToggleExpand(comp_disk+data.selected_disk);
     data.c[comp_disk+data.selected_disk].datalines[disk_busxfer].value = (pmb_buscache_xfer(data.selected_disk)*KB);
     ToggleExpand(comp_disk+data.selected_disk);
     }
  else
     {
     data.c[comp_disk+data.selected_disk].datalines[disk_busxfer].value = (pmb_buscache_xfer(data.selected_disk)*KB);
     }

  EndBench();
}


void _Optlink DoDiskIOTransSpeed(void* p)
{
  if ((data.c[comp_disk+data.selected_disk].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_disk+data.selected_disk].bitsettings)
     {
     ToggleExpand(comp_disk+data.selected_disk);
     data.c[comp_disk+data.selected_disk].datalines[disk_transf0].value = (pmb_diskio_transfer0(data.selected_disk)*KB);
     PostFin(TRUE);
     data.c[comp_disk+data.selected_disk].datalines[disk_transfn2].value = (pmb_diskio_transfern2(data.selected_disk)*KB);
     PostFin(TRUE);
     data.c[comp_disk+data.selected_disk].datalines[disk_transfn].value = (pmb_diskio_transfern(data.selected_disk)*KB);
     PostFin(TRUE);
     data.c[comp_disk+data.selected_disk].datalines[disk_transf].value =
         (data.c[comp_disk+data.selected_disk].datalines[disk_transf0].value +
         data.c[comp_disk+data.selected_disk].datalines[disk_transfn2].value +
         data.c[comp_disk+data.selected_disk].datalines[disk_transfn].value)/3;
     ToggleExpand(comp_disk+data.selected_disk);
     }
  else
     {
     data.c[comp_disk+data.selected_disk].datalines[disk_transf0].value = (pmb_diskio_transfer0(data.selected_disk)*KB);
     PostFin(TRUE);
     data.c[comp_disk+data.selected_disk].datalines[disk_transfn2].value = (pmb_diskio_transfern2(data.selected_disk)*KB);
     PostFin(TRUE);
     data.c[comp_disk+data.selected_disk].datalines[disk_transfn].value = (pmb_diskio_transfern(data.selected_disk)*KB);
     PostFin(TRUE);
     data.c[comp_disk+data.selected_disk].datalines[disk_transf].value =
         (data.c[comp_disk+data.selected_disk].datalines[disk_transf0].value +
         data.c[comp_disk+data.selected_disk].datalines[disk_transfn2].value +
         data.c[comp_disk+data.selected_disk].datalines[disk_transfn].value)/3;
     }

  EndBench();
}


void _Optlink DoDiskIOCPUUsage(void* p)
{
  if ((data.c[comp_disk+data.selected_disk].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_disk+data.selected_disk].bitsettings)
     {
     ToggleExpand(comp_disk+data.selected_disk);
     data.c[comp_disk+data.selected_disk].datalines[disk_cpupct].value = (pmb_diskio_cpupct(data.selected_disk));
     ToggleExpand(comp_disk+data.selected_disk);
     }
  else
     {
     data.c[comp_disk+data.selected_disk].datalines[disk_cpupct].value = (pmb_diskio_cpupct(data.selected_disk));
     }

  EndBench();
}


void _Optlink DoCDIOAvSeek(void* p)
{
  if ((data.c[comp_cd+(data.nr_fixed_disks-1)+data.selected_cd].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_cd+(data.nr_fixed_disks-1)+data.selected_cd].bitsettings)
     {
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_avseek].value = (pmb_cdio_avseek(data.selected_cd)/(10*1000));
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     }
  else
     {
     data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_avseek].value = (pmb_cdio_avseek(data.selected_cd)/(10*1000));
     }

  EndBench();
}


void _Optlink DoCDIOSpeed(void* p)
{
  AddTitle("CD inner speed");
  DoCDIOInnerSpeed(p);
  DelTitle();
  AddTitle("CD outer speed");
  DoCDIOOuterSpeed(p);
  DelTitle();
}


void _Optlink DoCDIOInnerSpeed(void* p)
{
char circaspeed[11];
  if ((data.c[comp_cd+(data.nr_fixed_disks-1)+data.selected_cd].bitsettings &
      (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) ==
      data.c[comp_cd+(data.nr_fixed_disks-1)+data.selected_cd].bitsettings)
     {
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_inner].value = (pmb_cdio_inner(data.selected_cd)*KB);
     strcpy(data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_inner].unit, "Kilobytes/second");
     sprintf(circaspeed, " (~%3.1fX)",
            (data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_inner].value/(150.0*KB)));
     strcat(data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_inner].unit, circaspeed);
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     }
  else
     {
     data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_inner].value = (pmb_cdio_inner(data.selected_cd)*KB);
     strcpy(data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_inner].unit, "Kilobytes/second");
     sprintf(circaspeed, " (~%3.1fX)",
            (data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_inner].value/(150.0*KB)));
     strcat(data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_inner].unit, circaspeed);
     }
  EndBench();
}


void _Optlink DoCDIOOuterSpeed(void* p)
{
char circaspeed[11];
  if ((data.c[comp_cd+(data.nr_fixed_disks-1)+data.selected_cd].bitsettings &
      (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) ==
      data.c[comp_cd+(data.nr_fixed_disks-1)+data.selected_cd].bitsettings)
     {
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_outer].value = (pmb_cdio_outer(data.selected_cd)*KB);
     strcpy(data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_outer].unit, "Kilobytes/second");
     sprintf(circaspeed, " (~%3.1fX)",
            (data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_outer].value/(150.0*KB)));
     strcat(data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_outer].unit, circaspeed);
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     }
  else
     {
     data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_outer].value = (pmb_cdio_outer(data.selected_cd)*KB);
     strcpy(data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_outer].unit, "Kilobytes/second");
     sprintf(circaspeed, " (~%3.1fX)",
            (data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_outer].value/(150.0*KB)));
     strcat(data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_outer].unit, circaspeed);
     }
  EndBench();
}


void _Optlink DoCDIOCPUUsage(void* p)
{
  if ((data.c[comp_cd+(data.nr_fixed_disks-1)+data.selected_cd].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_cd+(data.nr_fixed_disks-1)+data.selected_cd].bitsettings)
     {
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_cpupct].value = (pmb_cdio_cpupct(data.selected_cd));
     ToggleExpand(comp_cd+(data.nr_fixed_disks-1)+data.selected_cd);
     }
  else
     {
     data.c[comp_disk+data.nr_fixed_disks+data.selected_cd].datalines[cdio_cpupct].value = (pmb_cdio_cpupct(data.selected_cd));
     }

  EndBench();
}


void _Optlink DoDiveVBW(void* p)
  {
  double r = -1;
  if (gtWarp)
     {
     if ((data.c[comp_dive].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_dive].bitsettings)
        {
        ToggleExpand(comp_dive);
        r = pmb_dive_bw();
        ToggleExpand(comp_dive);
        }
     else
        {
        r = pmb_dive_bw();
        }

     if (r < 0)
        {
        r = -1.0;
        }
     }
  data.c[comp_dive].datalines[dive_videobw].value = r;
  EndBench();
}


void _Optlink DoDiveRot(void* p)
{
  double r = -1;
  if (gtWarp)
     {
     if ((data.c[comp_dive].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_dive].bitsettings)
        {
        ToggleExpand(comp_dive);
        r = pmb_dive_rot();
        ToggleExpand(comp_dive);
        }
     else
        {
        r = pmb_dive_rot();
        }

     if (r < 0)
        {
        r = -1.0;
        }
     }
  data.c[comp_dive].datalines[dive_rotate].value = r;
  EndBench();
}


void _Optlink DoDiveMS11(void* p)
{
  double r = 1;
  if (gtWarp)
     {
     if ((data.c[comp_dive].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_dive].bitsettings)
        {
        ToggleExpand(comp_dive);
        r = pmb_dive_ms11();
        ToggleExpand(comp_dive);
        }
     else
        {
        r = pmb_dive_ms11();
        }

     if (r < 0)
        {
        r = -1.0;
        }
     }
  data.c[comp_dive].datalines[dive_ms_11].value = r;
  EndBench();
}


void _Optlink DoGfxBlitBlitSS(void* p)
{
  if ((data.c[comp_gfx].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_gfx].bitsettings)
     {
     ToggleExpand(comp_gfx);
     data.c[comp_gfx].datalines[gfx_bitblt_SS].value = pmb_gfx_bitblitss();
     ToggleExpand(comp_gfx);
     }
  else
     {
     data.c[comp_gfx].datalines[gfx_bitblt_SS].value = pmb_gfx_bitblitss();
     }

  EndBench();
}


void _Optlink DoGfxBlitBlitMS(void* p)
{
  if ((data.c[comp_gfx].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_gfx].bitsettings)
     {
     ToggleExpand(comp_gfx);
     data.c[comp_gfx].datalines[gfx_bitblt_MS].value = pmb_gfx_bitblitms();
     ToggleExpand(comp_gfx);
     }
  else
     {
     data.c[comp_gfx].datalines[gfx_bitblt_MS].value = pmb_gfx_bitblitms();
     }

  EndBench();
}


void _Optlink DoGfxDLines(void* p)
{
  if ((data.c[comp_gfx].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_gfx].bitsettings)
     {
     ToggleExpand(comp_gfx);
     data.c[comp_gfx].datalines[gfx_dlines].value = pmb_gfx_dlines();
     ToggleExpand(comp_gfx);
     }
  else
     {
     data.c[comp_gfx].datalines[gfx_dlines].value = pmb_gfx_dlines();
     }

  EndBench();
}


void _Optlink DoGfxHLines(void* p)
{
  if ((data.c[comp_gfx].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_gfx].bitsettings)
     {
     ToggleExpand(comp_gfx);
     data.c[comp_gfx].datalines[gfx_hlines].value = pmb_gfx_hlines();
     ToggleExpand(comp_gfx);
     }
  else
     {
     data.c[comp_gfx].datalines[gfx_hlines].value = pmb_gfx_hlines();
     }

  EndBench();
}


void _Optlink DoGfxPatFil(void* p)
{
  if ((data.c[comp_gfx].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_gfx].bitsettings)
     {
     ToggleExpand(comp_gfx);
     data.c[comp_gfx].datalines[gfx_patt_fill].value = pmb_gfx_patrect();
     ToggleExpand(comp_gfx);
     }
  else
     {
     data.c[comp_gfx].datalines[gfx_patt_fill].value = pmb_gfx_patrect();
     }

  EndBench();
}

void _Optlink DoGfxFillRect(void* p)
{
  if ((data.c[comp_gfx].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_gfx].bitsettings)
     {
     ToggleExpand(comp_gfx);
     data.c[comp_gfx].datalines[gfx_filled_rect].value = pmb_gfx_fillrect();
     ToggleExpand(comp_gfx);
     }
  else
     {
     data.c[comp_gfx].datalines[gfx_filled_rect].value = pmb_gfx_fillrect();
     }

  EndBench();
}


void _Optlink DoGfxTextRender(void* p)
{
  if ((data.c[comp_gfx].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_gfx].bitsettings)
     {
     ToggleExpand(comp_gfx);
     data.c[comp_gfx].datalines[gfx_textrender].value = pmb_gfx_textrender();
     ToggleExpand(comp_gfx);
     }
  else
     {
     data.c[comp_gfx].datalines[gfx_textrender].value = pmb_gfx_textrender();
     }

  EndBench();
}


void _Optlink DoGfxVLines(void* p)
{
  if ((data.c[comp_gfx].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_gfx].bitsettings)
     {
     ToggleExpand(comp_gfx);
     data.c[comp_gfx].datalines[gfx_vlines].value = pmb_gfx_vlines();
     ToggleExpand(comp_gfx);
     }
  else
     {
     data.c[comp_gfx].datalines[gfx_vlines].value = pmb_gfx_vlines();
     }

  EndBench();
}


void _Optlink DoMem5(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[mem_5].value = pmb_memspeed(5*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[mem_5].value = pmb_memspeed(5*KB);
     }

  EndBench();
}


void _Optlink DoMem10(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[mem_10].value = pmb_memspeed(10*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[mem_10].value = pmb_memspeed(10*KB);
     }

  EndBench();
}


void _Optlink DoMem20(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[mem_20].value = pmb_memspeed(20*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[mem_20].value = pmb_memspeed(20*KB);
     }

  EndBench();
}


void _Optlink DoMem40(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[mem_40].value = pmb_memspeed(40*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[mem_40].value = pmb_memspeed(40*KB);
     }

  EndBench();
}


void _Optlink DoMem80(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[mem_80].value = pmb_memspeed(80*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[mem_80].value = pmb_memspeed(80*KB);
     }

  EndBench();
}


void _Optlink DoMem160(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[mem_160].value = pmb_memspeed(160*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[mem_160].value = pmb_memspeed(160*KB);
     }

  EndBench();
}


void _Optlink DoMem320(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[mem_320].value = pmb_memspeed(320*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[mem_320].value = pmb_memspeed(320*KB);
     }

  EndBench();
}


void _Optlink DoMem640(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[mem_640].value = pmb_memspeed(640*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[mem_640].value = pmb_memspeed(640*KB);
     }

  EndBench();
}


void _Optlink DoMem1280(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[mem_1280].value = pmb_memspeed(1280*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[mem_1280].value = pmb_memspeed(1280*KB);
     }

  EndBench();
}


void _Optlink DoMemR5(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memr_5].value = pmb_memspeedr(5*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memr_5].value = pmb_memspeedr(5*KB);
     }

  EndBench();
}


void _Optlink DoMemR10(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memr_10].value = pmb_memspeedr(10*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memr_10].value = pmb_memspeedr(10*KB);
     }

  EndBench();
}


void _Optlink DoMemR20(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memr_20].value = pmb_memspeedr(20*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memr_20].value = pmb_memspeedr(20*KB);
     }

  EndBench();
}


void _Optlink DoMemR40(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memr_40].value = pmb_memspeedr(40*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memr_40].value = pmb_memspeedr(40*KB);
     }

  EndBench();
}


void _Optlink DoMemR80(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memr_80].value = pmb_memspeedr(80*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memr_80].value = pmb_memspeedr(80*KB);
     }

  EndBench();
}


void _Optlink DoMemR160(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memr_160].value = pmb_memspeedr(160*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memr_160].value = pmb_memspeedr(160*KB);
     }

  EndBench();
}


void _Optlink DoMemR320(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memr_320].value = pmb_memspeedr(320*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memr_320].value = pmb_memspeedr(320*KB);
     }

  EndBench();
}


void _Optlink DoMemR640(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memr_640].value = pmb_memspeedr(640*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memr_640].value = pmb_memspeedr(640*KB);
     }

  EndBench();
}


void _Optlink DoMemR1280(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memr_1280].value = pmb_memspeedr(1280*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memr_1280].value = pmb_memspeedr(1280*KB);
     }

  EndBench();
}


void _Optlink DoMemW5(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memw_5].value = pmb_memspeedw(5*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memw_5].value = pmb_memspeedw(5*KB);
     }

  EndBench();
}


void _Optlink DoMemW10(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memw_10].value = pmb_memspeedw(10*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memw_10].value = pmb_memspeedw(10*KB);
     }

  EndBench();
}


void _Optlink DoMemW20(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memw_20].value = pmb_memspeedw(20*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memw_20].value = pmb_memspeedw(20*KB);
     }

  EndBench();
}


void _Optlink DoMemW40(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memw_40].value = pmb_memspeedw(40*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memw_40].value = pmb_memspeedw(40*KB);
     }

  EndBench();
}


void _Optlink DoMemW80(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memw_80].value = pmb_memspeedw(80*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memw_80].value = pmb_memspeedw(80*KB);
     }

  EndBench();
}


void _Optlink DoMemW160(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memw_160].value = pmb_memspeedw(160*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memw_160].value = pmb_memspeedw(160*KB);
     }

  EndBench();
}


void _Optlink DoMemW320(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memw_320].value = pmb_memspeedw(320*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memw_320].value = pmb_memspeedw(320*KB);
     }
  EndBench();
}


void _Optlink DoMemW640(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memw_640].value = pmb_memspeedw(640*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memw_640].value = pmb_memspeedw(640*KB);
     }
  EndBench();
}


void _Optlink DoMemW1280(void* p)
{
  if ((data.c[comp_mem].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_mem].bitsettings)
     {
     ToggleExpand(comp_mem);
     data.c[comp_mem].datalines[memw_1280].value = pmb_memspeedw(1280*KB);
     ToggleExpand(comp_mem);
     }
  else
     {
     data.c[comp_mem].datalines[memw_1280].value = pmb_memspeedw(1280*KB);
     }
  EndBench();
}


void _Optlink DoFileIO4(void* p)
{
 if ((data.c[comp_file].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_file].bitsettings)
    {
    ToggleExpand(comp_file);

    if (!mult)
       {
       mult = true;
       DoFileIOBuf(4096);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(4096);
       }

    ToggleExpand(comp_file);
    }
 else
    {
    if (!mult)
       {
       mult = true;
       DoFileIOBuf(4096);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(4096);
       }
    }

}


void _Optlink DoFileIO8(void* p)
{
 if ((data.c[comp_file].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_file].bitsettings)
    {
    ToggleExpand(comp_file);
    if (!mult)
       {
       mult = true;
       DoFileIOBuf(8192);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(8192);
       }
    ToggleExpand(comp_file);
    }
 else
    {
    if (!mult)
       {
       mult = true;
       DoFileIOBuf(8192);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(8192);
       }
    }
}


void _Optlink DoFileIO16(void* p)
{
 if ((data.c[comp_file].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_file].bitsettings)
    {
    ToggleExpand(comp_file);
    if (!mult)
       {
       mult = true;
       DoFileIOBuf(16384);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(16384);
       }
    ToggleExpand(comp_file);
    }
 else
    {
    if (!mult)
       {
       mult = true;
       DoFileIOBuf(16384);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(16384);
       }
    }
}


void _Optlink DoFileIO32(void* p)
{
 if ((data.c[comp_file].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_file].bitsettings)
    {
    ToggleExpand(comp_file);
    if (!mult)
       {
       mult = true;
       DoFileIOBuf(32768);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(32768);
       }
    ToggleExpand(comp_file);
    }
 else
    {
    if (!mult)
       {
       mult = true;
       DoFileIOBuf(32768);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(32768);
       }
    }
}


void _Optlink DoFileIO64(void* p)
{
 if ((data.c[comp_file].bitsettings & (0xffffffff-(GROUP_AUTOEXPANDED+GROUP_EXPANDED))) == data.c[comp_file].bitsettings)
    {
    ToggleExpand(comp_file);
    if (!mult)
       {
       mult = true;
       DoFileIOBuf(65536);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(65536);
       }
    ToggleExpand(comp_file);
    }
 else
    {
    if (!mult)
       {
       mult = true;
       DoFileIOBuf(65536);
       mult = false;
       EndBench();
       }
    else
       {
       DoFileIOBuf(65536);
       }
    }
}


void DoFileIOBuf(ULONG buffersize)
{
int n, linenum;
BOOL cache, random, reading;
APIRET rc = 0;

rc = DosCreateEventSem(NULL,           /* Unnamed semaphore            */
                       &hevFileIO,     /* Handle of semaphore returned */
                       DC_SEM_SHARED,  /* Indicate a shared semaphore  */
                       FALSE);         /* Put in RESET state           */

for (cache = 0; cache <= 1; cache++)        /* toggle cache off then on for each */
   {
   for (random = 0; random <= 1; random++)     /* toggle sequential or random access */
      {
      for (reading = 0; reading <= 1; reading++)  /* toggle writing or reading */
         {
         rc = DosResetEventSem(hevFileIO,        /* Reset the semaphore         */
                              &ulPostCount2);
         n = frexp(buffersize/4096, &linenum);
         linenum = ((linenum - 1) * 8) + (cache * 4) + (random * 2) + (reading);
//         _beginthread(TimeThread, NULL, 8192, NULL);    /* start timer thread */
         data.c[comp_file].datalines[linenum].value = DoFileIO(buffersize, cache, reading, random); /* go do current test */
         EndBench();                                                             /* update display */
         }
      }
   }
rc = DosCloseEventSem(hevFileIO);      /* Get rid of semaphore       */
}


void DoRef1Info(void)
{
}

void DoRef1Load(void)
{
}


void DoRef2Info(void)
{
}


void DoRef2Load(void)
{
}


double CalcGfxAv(void)
   {
/*   return (
          (data.c[comp_gfx].datalines[gfx_bitblt_SS].value * 10) +
          (data.c[comp_gfx].datalines[gfx_bitblt_MS].value * 8) +
          (data.c[comp_gfx].datalines[gfx_filled_rect].value * 7) +
          (data.c[comp_gfx].datalines[gfx_patt_fill].value * 4) +
          (data.c[comp_gfx].datalines[gfx_vlines].value * 5) +
          (data.c[comp_gfx].datalines[gfx_hlines].value * 5) +
          (data.c[comp_gfx].datalines[gfx_dlines].value * 4) +
          (data.c[comp_gfx].datalines[gfx_textrender].value * 8)
         ) / 51 / 1.0e6;
*/
   return pow((data.c[comp_gfx].datalines[gfx_bitblt_SS].value *
               data.c[comp_gfx].datalines[gfx_bitblt_MS].value *
               data.c[comp_gfx].datalines[gfx_filled_rect].value *
               data.c[comp_gfx].datalines[gfx_patt_fill].value *
               data.c[comp_gfx].datalines[gfx_vlines].value *
               data.c[comp_gfx].datalines[gfx_hlines].value *
               data.c[comp_gfx].datalines[gfx_dlines].value *
               data.c[comp_gfx].datalines[gfx_textrender].value), 1.0/8.0)/1.0e6;
   }


double CalcCPUIntAv(void)
{
  return (
          (data.c[comp_cpuint].datalines[cpuint_dhrystone].value * 12 * 1.0e5) +
          (data.c[comp_cpuint].datalines[cpuint_hanoi].value * 5 * 1.0e5)+
          (data.c[comp_cpuint].datalines[cpuint_heapsort].value * 6 * 1.0e5) +
          (data.c[comp_cpuint].datalines[cpuint_sieve].value * 6 * 1.0e5)
         ) / 29 / 1.0e6;
}


double CalcCPUFloatAv(void)
{
  return (
          (data.c[comp_cpufloat].datalines[cpufloat_linpack].value * 100000) +
          (data.c[comp_cpufloat].datalines[cpufloat_flops].value * 200000) +
          (data.c[comp_cpufloat].datalines[cpufloat_fft].value * 9 * 1.0e4)
         ) / 39 / 1.0e6;
}


double CalcMemAv(void)
{
  return (
          (data.c[comp_mem].datalines[mem_5].value * 7) +
          (data.c[comp_mem].datalines[mem_10].value * 8) +
          (data.c[comp_mem].datalines[mem_20].value * 7) +
          (data.c[comp_mem].datalines[mem_40].value * 7) +
          (data.c[comp_mem].datalines[mem_80].value * 6) +
          (data.c[comp_mem].datalines[mem_160].value * 5) +
          (data.c[comp_mem].datalines[mem_320].value * 4) +
          (data.c[comp_mem].datalines[mem_640].value * 4) +
          (data.c[comp_mem].datalines[mem_1280].value * 3) +
          (data.c[comp_mem].datalines[memr_5].value * 7) +
          (data.c[comp_mem].datalines[memr_10].value * 8) +
          (data.c[comp_mem].datalines[memr_20].value * 7) +
          (data.c[comp_mem].datalines[memr_40].value * 7) +
          (data.c[comp_mem].datalines[memr_80].value * 6) +
          (data.c[comp_mem].datalines[memr_160].value * 5) +
          (data.c[comp_mem].datalines[memr_320].value * 4) +
          (data.c[comp_mem].datalines[memr_640].value * 4) +
          (data.c[comp_mem].datalines[memr_1280].value * 3) +
          (data.c[comp_mem].datalines[memw_5].value * 7) +
          (data.c[comp_mem].datalines[memw_10].value * 8) +
          (data.c[comp_mem].datalines[memw_20].value * 7) +
          (data.c[comp_mem].datalines[memw_40].value * 7) +
          (data.c[comp_mem].datalines[memw_80].value * 6) +
          (data.c[comp_mem].datalines[memw_160].value * 5) +
          (data.c[comp_mem].datalines[memw_320].value * 4) +
          (data.c[comp_mem].datalines[memw_640].value * 4) +
          (data.c[comp_mem].datalines[memw_1280].value * 3)
         ) / (51*3.0) / MB;
}


double CalcDIVEAv(void)
{
  return (
          (data.c[comp_dive].datalines[dive_videobw].value * 10) +
          (data.c[comp_dive].datalines[dive_rotate].value * 3 * 1.0e6) +
          (data.c[comp_dive].datalines[dive_ms_11].value * 10 * 1.0e6)
         ) / 43 / 1.0e6;
}


double CalcDiskIOAv(s32 comp)
{
  double x;
  x =     ((1/(data.c[comp].datalines[disk_avseek].value / 1.0e-3)) * 1.0e9 / 20.0) +
          (data.c[comp].datalines[disk_busxfer].value * (1.0/10)) +
          (data.c[comp].datalines[disk_transf].value * 5);

  return (x + ( x / (data.c[comp].datalines[disk_cpupct].value * 5) ) ) / 1.0e6;
}


double CalcSimIOAv(void)
{
int i;
double r = 0;
 for (i = 0; i < data.nr_fixed_disks; i++)
    {
    if (data.c[comp_alldisks].datalines[i].value >= 0)
       r = r + data.c[comp_alldisks].datalines[i].value;
    }
 r = r/KB;
 return r;
}


double CalcCDIOAv(s32 comp)
{
  double x;
  x =     ((1/(data.c[comp].datalines[cdio_avseek].value / 1.0e-3)) * 1.0e8 / 20.0) +
          ((data.c[comp].datalines[cdio_inner].value * 50));

  return (x + ( x / (data.c[comp].datalines[cdio_cpupct].value * 5) ) ) / 1.0e6;
}



double CalcFileIOAv(void)
{
  double x = 0;
  int linenum, i, j, k, l;

  for (i = 0; i <= 4; i++) /* for number of buffer sizes */
     {
     for (j = 0; j <= 1; j++) /* cached or not */
        {
        for (k = 0; k <= 1; k++) /* seq or random */
           {
           for (l = 0; l <= 1; l++) /* writing or reading */
              {
              linenum = (i * 8) + (j * 4) + (k * 2) + (l);
              x = x + (data.c[comp_file].datalines[linenum].value / KB);
              }
           }
        }
     }
  return (x / 40);   /* total divided by # of tests */
}


double dtime(void)
{
   return ((double)clock())/CLOCKS_PER_SEC;
/*
   ULONG value[QSV_MAX] = {0};
   APIRET rc = 0UL;

   rc = DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, (PVOID)value, sizeof(ULONG));

   return((double)*value/CLOCKS_PER_SEC); */
}


double rtime(void)
{
   return ((double)clock())/CLOCKS_PER_SEC;
/*
   ULONG value[QSV_MAX] = {0};
   APIRET rc = 0UL;

   rc = DosQuerySysInfo(QSV_MS_COUNT, QSV_MS_COUNT, (PVOID)value, sizeof(ULONG));

   return((double)*value); */
}


void logit(char* s)
{
  FILE* f;
  f = fopen("sb_error.log", "a+");
  if (!f)
     {
     exit(234);
     }
  fprintf(f, "%s\n", s);
  fclose(f);
}


static void EndBench(void)
{
  float size = GetSwapFileSize();

  if (startsize < size)
     {
     swapfilegrown++;
     if (size > maxswapfilesize)
        {
        maxswapfilesize = size;
        }
     }

  if (!mult)
     {
     PostFin(true);
     PostFin(false);
     _heapmin();        /* free off memory allocated by previous test */
     }
  else
     {
     PostFin(true);
     DosSleep(500); // wait for the window thread to update the screen
     _heapmin();        /* free off memory allocated by previous test */
     }
}


