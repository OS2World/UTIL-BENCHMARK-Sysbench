/* Filename:        pmcpu3b.cpp                                  */
/* This file incorporates code copyright Intel Corporation       */
/* Additional code copyright Trevor Hemsley 1998                 */
/*                                                               */
/*                                                               */

#define FPU_FLAG        0x00000001  /* 0 */
#define VME_FLAG        0x00000002  /* 1 */
#define DEBUG_FLAG      0x00000004  /* 2 */
#define PSE_FLAG        0x00000008  /* 3 */
#define TIME_FLAG       0x00000010  /* 4 */
#define K86_FLAG        0x00000020  /* 5 */
#define PAE_FLAG        0x00000040  /* 6 */
#define MCE_FLAG        0x00000080  /* 7 */
#define CMPXCHG8B_FLAG  0x00000100  /* 8 */
#define APIC_FLAG       0x00000200  /* 9 */
#define K86_RSV1        0x00000400  /* 10*/
#define SEP_FLAG        0x00000800  /* 11*/
#define MTRR_FLAG       0x00001000  /* 12*/
#define GLOBPAG_FLAG    0x00002000  /* 13*/
#define K86_RSV3        0x00004000  /* 14*/
#define CMOV_FLAG       0x00008000  /* 15*/
#define PAT_FLAG        0x00010000  /* 16 */
#define PSE36_FLAG      0x00020000  /* 17 */
#define MMX_FLAG        0x00800000  /* 23 */
#define FXSR_FLAG       0x01000000  /* 24 */
#define BT25_FLAG       0x02000000  /* 25 */
#define BT26_FLAG       0x04000000  /* 26 */
#define BT27_FLAG       0x08000000  /* 27 */
#define BT28_FLAG       0x10000000  /* 28 */
#define BT29_FLAG       0x20000000  /* 29 */
#define BT30_FLAG       0x40000000  /* 30 */
#define D3NOW_FLAG      0x80000000  /* 31 */

#define INCL_DOS
#define INCL_BASE
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_DOSPROCESS
#define INCL_DOSNLS     /* National Language Support values */

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bsedev.h>
#include "types.h"

extern char  _cpu_type;
extern char  _fpu_type;
extern char  _cpuid_flag;
extern char  _intel_CPU;
extern char  vendor_id[12];
extern long  _cpu_signature;
extern long  features_ecx;
extern long  _features_edx;
extern long  features_ebx;
extern "C" int   _get_cpu_type(void);
extern "C" int   _get_fpu_type(void);
extern short _inst_cache;
extern char  _inst_cway;
extern char  _inst_lines;
extern short _data_cache;
extern char  _data_cway;
extern char  _data_lines;
extern long  _lvl2_cache;
extern char  _cache_info;
extern char  _amd_model;
extern DISKCONTROLLER Processor;
extern DISKCONTROLLER CacheAmount;
extern ULONG numcpus;

char   tmp[100];
char   cpumult[10];
extern char   registered[4];


void print(void);
void pmcpu3b(void);


void pmcpu3b(void)
{
    _get_cpu_type();
    _get_fpu_type();
    print();
}

void print(void)
{
    if (numcpus > 1)
       {
       sprintf(cpumult, "%d x ", numcpus);
       }
    else
       {
       sprintf(cpumult, "");
       }
    if (_cpuid_flag == 0)
       {
       switch (_cpu_type)
          {
          case 0:
             sprintf(Processor.desc.detected, "%s8086/8088", cpumult);
             if (_fpu_type)
                {
                sprintf(tmp, " and an 8087");
                strcat(Processor.desc.detected,tmp);
                }
             break;
          case 2:
             sprintf(Processor.desc.detected, "%s80286", cpumult);
             if (_fpu_type)
                {
                strcat(Processor.desc.detected,tmp);
                sprintf(tmp, " and an 80287");
                }
             break;
          case 3:
             sprintf(Processor.desc.detected, "%s80386", cpumult);
             if (_fpu_type == 2)
                {
                sprintf(tmp, " and an 80287");
                strcat(Processor.desc.detected,tmp);
                }
             else
                {
                if (_fpu_type)
                   {
                   sprintf(tmp, " and an 80387");
                   strcat(Processor.desc.detected, tmp);
                   }
                }
             break;
          case 4:
             if (_fpu_type)
                {
                sprintf(Processor.desc.detected, "%s80486DX, 80486DX2 or 80486SX/80487", cpumult);
                }
             else
                {
                sprintf(Processor.desc.detected, "%s80486SX", cpumult);
                }
             break;
          default:
             sprintf(Processor.desc.detected, "%sunknown", cpumult);
          }  /* end switch */
       }     /* end cpuid_flag == 0 */
    else
       {
       char cpumake[2];
       char cpunum[4];
       char cpucache[5];
       char* cpumaketable[] = {"no cpu", "Intel", "AMD", "Cyrix", "IDT", "NexGen", "UMC"};
       FILE *fp;
       char input[133],
            *inpcpumake  = "unknown",
            *inpcpunum   = "unknown",
            *inpcpucache = "unknown",
            *inpdesc     = "unknown",
            *n;
       BOOL found = 0;
       ULONG ucache;

       /* using cpuid instruction */
       sprintf(cpumake, "%d", _intel_CPU);     // 1 = Intel, 2 = AMD, 3 = Cyrix, 4 = IDT, 5 = NexGen, 6 = UMC
       sprintf(cpunum, "%x%x%x", _cpu_type, ((_cpu_signature>>4)&0x0000000f), ((_cpu_signature)&0x0000000f));
       sprintf(cpucache, "%d", _lvl2_cache);

       fp = fopen("cpuid.dat", "rb");
       if (!fp)
          {
          sprintf(Processor.desc.detected, "%s%s model %s processor found but no cpuid.dat file available",
                  cpumult, cpumaketable[_intel_CPU], cpunum);
          }
       else
          {
          while (!feof(fp))
             {
             fgets(input, 132, fp);
             if (strncmp(input, "#", 1) == 0)
                continue;
             if (strncmp(input, ";", 1) == 0)
                continue;

             inpcpumake  = strtok(input, " \t");
             inpcpunum   = strtok(NULL, " \t");
             inpcpucache = strtok(NULL, " \t");
             inpdesc     = strtok(NULL, "\n");

             if (strcmp(inpcpumake, cpumake) < 0)
                continue;                         // not our make of cpu yet
             if (strcmp(inpcpumake, cpumake) > 0)
                break;                            // past it and not found
             // so this one must be ours then
             if (strcmp(inpcpunum, cpunum) < 0)
                continue;                         // not our model number of cpu yet
             if (strcmp(inpcpunum, cpunum) > 0)
                {
                n = strchr(inpcpunum, '?');
                if (n == NULL)                    // if not a generic line
                   break;
                }
             n = strchr(inpcpucache, '?');
             if (n != NULL)
                {
                found = TRUE;
                break;                            // this one will have to do
                }
             ucache = atoi(inpcpucache);          // convert to decimal
             if (ucache < _lvl2_cache)            // if not ours
                continue;                         // keep going, it might come round yet
             if (ucache > _lvl2_cache)            // if too much
                break;
             found = TRUE;
             break;
             }
          fclose(fp);
          if (!found)
             {
             sprintf(Processor.desc.detected, "%s%s model %s found (not in cpu table)",
                    cpumult, cpumaketable[_intel_CPU], cpunum);
             }
          else
             {
             inpdesc[strlen(inpdesc)-1] = 0;      // stomp on CR at end
             if (strlen(inpdesc) > 66)
                inpdesc[66] = 0;                  // maximum allowed is 100 chars
             sprintf(Processor.desc.detected, "%s%s %s (stepping %s)",
                    cpumult, cpumaketable[_intel_CPU], inpdesc, cpunum);
             }
          }
       }

    if (_cpu_signature & 0x1000)
       {
       sprintf(tmp, " OverDrive%s", registered);
       if (strlen(Processor.desc.detected) < 100 - strlen(tmp))
          strcat(Processor.desc.detected, tmp);
       }
    else
       {
       if (_cpu_signature & 0x2000)
          {
          sprintf(tmp, " dual processor upgrade");
          if (strlen(Processor.desc.detected) < 100 - strlen(tmp))
             strcat(Processor.desc.detected, tmp);
          }
       }

    if (_features_edx & MMX_FLAG)
       {
       sprintf(tmp, " with MMX");
       if (strlen(Processor.desc.detected) < 100 - strlen(tmp))
          strcat(Processor.desc.detected, tmp);
       }

    if (_cache_info)
       {
       sprintf(CacheAmount.desc.detected, "%dKb internal instruction cache",
              _inst_cache);
       sprintf(tmp, ", %dKb internal data cache",
              _data_cache);
       strcat(CacheAmount.desc.detected, tmp);

       if (_lvl2_cache)
          {
          sprintf(tmp, ", %dKb unified level2 cache",
                 _lvl2_cache);
          strcat(CacheAmount.desc.detected, tmp);
          }
       }
    else
       {
       sprintf(CacheAmount.desc.detected, "Unable to determine cache (cpuid instruction not supported).");
       }
    }


