/* Get BIOS Information */
#define INCL_DOS
#define INCL_BASE
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSERRORS

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "pmb.h"
#include "pmbbench.h"
#include <bsedev.h>

extern ULONG VideoMem;
extern char  VideoMan[15];
extern char  VideoType[15];
extern USHORT VideoAdapter, VideoChip;
extern char  BIOSname[50];
extern char  BusName[9];



void GetBIOSInfo(void)
{
  HFILE   DevHandle        = NULLHANDLE;   /* Handle for device */
  ULONG   ulCategory       = IOCTL_TESTCFG_SYS;         /* Device category */
  ULONG   ulFunction       = TESTCFG_SYS_GETBIOSADAPTER; /* Device-specific function */
  PUCHAR  pBIOSCopy;
  char*   bodge;
  ULONG   u;
  ULONG   uBusArch;
  char*   Bustype[]        = {"ISA", "MCA", "EISA"};
  BYTE    bZero            = 0;

  struct  biosparms {
        ULONG Command;
        ULONG Addr;
        USHORT Numbytes;
        };
  struct biosparms uchParms  = {0, 0x000f0000, 0x0064};

  struct  viddata {
        USHORT Adapter;
        USHORT Chiptype;
        ULONG  Memory;
        };
  struct viddata uvidData  = {0, 0, 0};

  ULONG   ulParmLen        = 0;            /* Input and output parameter size */
  ULONG   ulDataLen        = 0;            /* Input and output data size */
  ULONG   ulAction         = 0;
  APIRET  rc               = NO_ERROR;     /* Return code */

  char    Award[6]        = "Award";
  char    AMI[8]          = "AMIBIOS";
  char    Phoenix[8]      = "Phoenix";
  char    MRBIOS[8]       = "MR BIOS";
  char    Compaq[8]       = "Compaq ";
  char    IBM[10]          = "GHT IBM C";
  BOOL    BIOSided         = 0;

  struct vidtype {
        char name[19];
        };
#define MAX_MANUFACTURERS 33
#define MAX_MODELS        20

  struct mantab {
        USHORT uEntries;
        char   Manufacturer[12];
        struct vidtype type[MAX_MODELS];
        };

   struct vidtab {
        USHORT uEntries;
        struct mantab VidMan[MAX_MANUFACTURERS];
        };

   struct vidtab VidTable = {MAX_MANUFACTURERS,
                               {{1, "Unknown",    /* #0 */
                                  {"type",
                                  "Undef.",
                                  "Undef.",
                                  "Undef.",
                                  "Undef.",
                                  "Undef.",
                                  "Undef.",
                                  "Undef.",
                                  "Undef."}
                               },
                              {3, "Headland",          /* #1 */
                                 {"HT205",
                                 "HT208",
                                 "HT209"}
                               },
                              {2, "Trident",          /* #2 */
                                 {"8800",
                                 "8900"}
                               },
                              {11, "Tseng",           /* #3 */
                                 {"ET3000",
                                 "ET4000",
                                 "ET4000W32",
                                 "ET4000W32I",
                                 "ET4000W32IB",
                                 "ET4000W32IC",
                                 "ET4000W32PA",
                                 "ET4000W32PB",
                                 "ET4000W32PC",
                                 "ET4000W32ID",
                                 "ET4000W32PD"}
                               },
                              {9, "WD",              /* #4 */
                                 {"PVGA1A",
                                 "WD9000",
                                 "WD9011",
                                 "WD9030",
                                 "WD9026",
                                 "WD9027",
                                 "WD9031",
                                 "WD9024",
                                 "WD9033"}
                               },
                              {6, "ATI",            /* #5 */
                                 {"18800",
                                 "28800",
                                 "38800",
                                 "68800",
                                 "88800",
                                 "88800CT"}
                               },
                              {2, "IBM",            /* #6 */
                                 {"VGA-256C",
                                 "IBMSVGA"}
                               },
                              {11, "CL",              /* #7 */
                                 {"5420", /* 1 */
                                 "5422",  /* 2 */
                                 "5424",  /* 3 */
                                 "5426",  /* 4 */
                                 "5428",  /* 5 */
                                 "5429",  /* 6 */
                                 "543x",  /* 7 */
                                 "5434",  /* 8 */
                                 "xxxx",  /* 9 */
                                 "xxxx",  /* 10 */
                                 "5436"}  /* 11 */
                               },
                              {20, "S3",                /* #8 */
                                 {"86C805",  /* 1 */
                                 "86C928",   /* 2 */
                                 "86C911",   /* 3 */
                                 "86C864",   /* 4 */
                                 "86C964",   /* 5 */
                                 "86C868",    /* 6 */
                                 "86C968",    /* 7 */
                                 "Trio32",    /* 8 */
                                 "Trio64",    /* #9 */
                                 "Trio64V+",  /* 10 */
                                 "Aurora64V+", /* 11 */
                                 "ViRGE",     /* 12 */
                                 "ViRGE/vX",   /* 13 */
                                 "Trio64UV+",  /* 14 */
                                 "Trio64V+Compatible", /* 15 */
                                 "Trio64V2",           /* 16 */
                                 "Yosemite",         /* 17 */
                                 "ViRGE/DX-GX",      /* 18 */
                                 "ViRGE/GX2",        /* 19 */
                                 "ViRGE+Compatible"} /* 20 */
                               },
                              {1, "C&T",               /* #9 */
                                 {"type"}
                               },
                              {3, "Weitek",            /* #10 */
                                 {"P9000",
                                 "W5186",
                                 "W5286"}
                               },
                              {1, "#9",                /* #11 */
                                 {"type"}
                               },
                              {1, "Generic",           /* #12 */
                                 {"type"}
                               },
                              {1, "Oak",               /* #13 */
                                 {"type"}
                               },
                              {1, "Matrox",       /* #14 */
                                 {"type"}
                               },
                              {1, "Brooktree",    /* #15 */
                                 {"type"}
                               },
                              {1, "NVidea",       /* #16 */
                                 {"type"}
                               },
                              {1, "Alliance",     /* #17 */
                                 {"type"}
                               },
                              {1, "Avance",       /* #18 */
                                 {"type"}
                               },
                              {1, "MediaVision",  /* #19 */
                                 {"type"}
                               },
                              {1, "Ark",          /* #20 */
                                 {"type"}
                               },
                              {1, "Radius",       /* #21 */
                                 {"type"}
                               },
                              {1, "3D Labs",      /* #22 */
                                 {"type"}
                               },
                              {1, "NCR",          /* #23 */
                                 {"type"}
                               },
                              {1, "IIT",          /* #24 */
                                 {"type"}
                               },
                              {1, "Appian",       /* #25 */
                                 {"type"}
                               },
                              {1, "Sierra",       /* #26 */
                                 {"type"}
                               },
                              {1, "Cornerstone",  /* #27 */
                                 {"type"}
                               },
                              {1, "Digital",      /* #28 */
                                 {"type"}
                               },
                              {1, "Compaq",       /* #29 */
                                 {"type"}
                               },
                              {1, "Infotronic",   /* #30 */
                                 {"type"}
                               },
                              {1, "Opti",         /* #31 */
                                 {"type"}
                               },
                              {1, "Undef.",       /* #32 */
                                 {"type"}
                               }
                            }
                           };

   uchParms.Command  = 0;
   uchParms.Addr     = 0x000f0000;
   uchParms.Numbytes = 65535;

   DosAllocMem((PPVOID)&pBIOSCopy,
              65536,
              PAG_COMMIT |
              PAG_READ   |
              PAG_WRITE);

   ulParmLen = sizeof(ULONG)+sizeof(ULONG)+sizeof(USHORT);   /* Length of input parameters */

   rc = DosOpen("TESTCFG$",
               &DevHandle,
               &ulAction,
               (ULONG)NULL,
               (ULONG)NULL,
               1,
               0x40,
               0);

   rc = DosDevIOCtl(DevHandle,           /* Handle to device */
                    ulCategory,          /* Category of request */
                    TESTCFG_SYS_GETBUSARCH,   /* Function being requested */
                    (void*)&uchParms,    /* Input/Output parameter list */
                    sizeof(uchParms),    /* Maximum output parameter size */
                    &ulParmLen,          /* Input:  size of parameter list */
                                         /* Output: size of parameters returned */
                    &uBusArch,           /* Input/Output data area */
                    4,                   /* Maximum output data size */
                    &ulDataLen);         /* Input:  size of input data area */
                                         /* Output: size of data returned   */

   rc = DosDevIOCtl(DevHandle,           /* Handle to device */
                    ulCategory,          /* Category of request */
                    ulFunction,          /* Function being requested */
                    (void*)&uchParms,    /* Input/Output parameter list */
                    sizeof(uchParms),    /* Maximum output parameter size */
                    &ulParmLen,          /* Input:  size of parameter list */
                                         /* Output: size of parameters returned */
                    pBIOSCopy,            /* Input/Output data area */
                    65535,               /* Maximum output data size */
                    &ulDataLen);         /* Input:  size of input data area */
                                         /* Output: size of data returned   */

   DosClose(DevHandle);

   if (uBusArch > 2)
      uBusArch = 0;

   strcpy(BusName, Bustype[uBusArch]);     /* set bus architecture name */

   if (!rc)
      {
      int i;
      for (u = 0; u < 0xffffL-(sizeof(Phoenix)-1); u++)
         {
         bodge = (char*)&pBIOSCopy[u];
         if (!memicmp(bodge, Award, sizeof(Award)-1))
            {
            strcpy(BIOSname, "Award");
            BIOSided = TRUE;
            break;
            }
         if (!memicmp(bodge, Phoenix, sizeof(Phoenix)-1))
            {
            strcpy(BIOSname, "Phoenix");
            BIOSided = TRUE;
            break;
            }
         if (!memicmp(bodge, MRBIOS, sizeof(MRBIOS)-1))
            {
            strcpy(BIOSname, "MR BIOS");
            BIOSided = TRUE;
            break;
            }
         if (!memicmp(bodge, AMI, sizeof(AMI)-1))
            {
            strcpy(BIOSname, "AMI");
            BIOSided = TRUE;
            break;
            }
         if (!memicmp(bodge, Compaq, sizeof(Compaq)-1))
            {
            strcpy(BIOSname, "Compaq");
            BIOSided = TRUE;
            break;
            }
         if (!memicmp(bodge, IBM, sizeof(IBM)-1))
            {
            strcpy(BIOSname, "IBM");
            BIOSided = TRUE;
            break;
            }
         }
      }

   DosFreeMem(pBIOSCopy);

   if (!BIOSided)
      strcpy(BIOSname, "Unknown");

   ulCategory        = IOCTL_OEMHLP;         /* Device category */
   ulFunction        = OEMHLP_GETSVGAINFO;   /* Device-specific function */
   ulDataLen = sizeof(uvidData);

   rc = DosOpen("OEMHLP$",
               &DevHandle,
               &ulAction,
               (ULONG)NULL,
               (ULONG)NULL,
               1,
               0x40,
               0);

   if (!rc)
      {
      rc = DosDevIOCtl(DevHandle,           /* Handle to device */
                      ulCategory,          /* Category of request */
                      ulFunction,          /* Function being requested */
                      (void*)&uchParms,    /* Input/Output parameter list */
                      sizeof(uchParms),    /* Maximum output parameter size */
                      &ulParmLen,          /* Input:  size of parameter list */
                                           /* Output: size of parameters returned */
                      (void*)&uvidData,    /* Input/Output data area */
                      sizeof(uvidData),    /* Maximum output data size */
                      &ulDataLen);         /* Input:  size of input data area */
                                           /* Output: size of data returned   */
      if (!rc)
         {
         VideoMem = uvidData.Memory;
         VideoAdapter = uvidData.Adapter;
         VideoChip    = uvidData.Chiptype;

         if (VidTable.uEntries > uvidData.Adapter)
            {
            strcpy(VideoMan, VidTable.VidMan[uvidData.Adapter].Manufacturer);

            if ((VidTable.VidMan[uvidData.Adapter].uEntries >= uvidData.Chiptype) &&
                (uvidData.Chiptype))
               {
               strcpy(VideoType, VidTable.VidMan[uvidData.Adapter].type[uvidData.Chiptype-1].name);
               }
            else
               {
               strcpy(VideoType, "Unknown");
               }
            }
         else
            {
            strcpy(VideoMan, "Unknown");
            strcpy(VideoType, "Unknown");
            }

         if (VideoAdapter == 0 && VideoChip == 4139)  /* Matrox Millenium Bodge */
            {
            strcpy(VideoMan, "Matrox");
            strcpy(VideoType, "Mill./Myst.");
            VideoMem = VideoMem * 4;
            }
         }
      else
         {
         printf("DosDevIOCtl for video info error: return code = %u\n", rc);
         }
      rc = DosDevIOCtl(DevHandle,           /* Handle to device */
                      ulCategory,          /* Category of request */
                      OEMHLP_PCI,          /* Function being requested */
                      (void*)&uchParms,    /* Input/Output parameter list */
                      sizeof(uchParms),    /* Maximum output parameter size */
                      &ulParmLen,          /* Input:  size of parameter list */
                                           /* Output: size of parameters returned */
                      (void*)&uvidData,    /* Input/Output data area */
                      sizeof(uvidData),    /* Maximum output data size */
                      &ulDataLen);         /* Input:  size of input data area */
                                           /* Output: size of data returned   */
      if (!memcmp((const void*)&uvidData, (const void*)&bZero, 1))
         {
         strcat(BusName, "/PCI");
         }
      }
   else
      {
      printf("DosOpen OEMHLP$ error: return code = %u\n", rc);
      }

   DosClose(DevHandle);

   if (rc != NO_ERROR) {
       printf("DosDevIOCtl error: return code = %u\n", rc);
       return;
   }
}



