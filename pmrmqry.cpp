
/* Extract info from Resource Manager */
/* command line version adapted from code in Sysbench - TPH */
#define INCL_DOSMISC
#define INCL_DOSDEVICES
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS

#include <os2.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <rmbase.h>
#include <rmioctl.h>
#include <bsedev.h>
#include "types.h"
#include "pmbdatat.h"

// internal functions in this file
void AddHD(char*);
void AddCD(char*);
int DecodePCIclass(void);
int GetPCIData(void);
void EnumeratePCIDevices(void);

#define FUNC_GET_CONTROLLER_NUMBERS 0
#define FUNC_GET_CONTROLLER_NAMES   1
#define FUNC_GET_DISK_AND_CD_NAMES  2

PCIDEV *pcidev[40];

// the following externs are in pmb_main.cpp
extern ULONG debugging;
extern DISKDESC *szDisk[40];
extern char Chipset[100];
extern char Graphicscard[100];
//extern char DiskController[100];
extern DISKCONTROLLER *DiskController[40];
extern int  StorageControllers;

extern void logit(char*);


char   tempdevicedesc[500];

int pcibusnum = 0, PCI = 0, searchdone = 0;

PCIPARMS uchPCI   = {0, 0, 0};

PCIPARMS1 uchPCI1  = {0, 0, 0, 0, 0};

PCIBUS ubusData    = {0, 0, 0, 0, 0};

PCIBUSDATA ubusData1    = {0, 0};

PCICFG pcicfg = {0};

int i = 0, index, found;

ULONG   hdnum = 0, cdnum = 0, Temp = 0;

RM_ENUMNODES_DATA* res;
RM_GETNODE_DATA*   rmnode;
RM_GETNODE_PARM     rmgnp;

HFILE   DevHandPCI       = NULLHANDLE;   /* Handle for device */
ULONG   ulCatPCI         = IOCTL_OEMHLP; /* Device category */
ULONG   ulFuncPCI        = OEMHLP_PCI;   /* Device-specific function */
ULONG   ulParmLPCI       = 0;            /* Input and output parameter size */
ULONG   ulDataLPCI       = 0;            /* Input and output data size */
ULONG   ulActPCI         = 0;
int     Function1        = 0;
int rmqinit = 0;


void  pmrmqry(int Function)
{
int     i = 0, index, found = 0, j;
char    *pointer, *pointer1;
HFILE   DevHandle        = NULLHANDLE;   /* Handle for device */
ULONG   ulCategory       = CAT_RM;         /* Device category */
ULONG   ulFunction       = FUNC_RM_ENUM_NODES; /* Device-specific function */
PUCHAR  pRMCopy, pRMStruct;
BYTE    bZero            = 0;
ULONG   ulParmLen        = 0;            /* Input and output parameter size */
UCHAR   uchDataArea[200] = {0};          /* Input and output data area */
ULONG   ulDataLen        = 0;            /* Input and output data size */
ULONG   ulAction         = 0;
UCHAR   uchParms[10]     = {0};
APIRET  rc               = NO_ERROR;     /* Return code */
ULONG   rmhandle, adapter;

   if (Function == FUNC_GET_DISK_AND_CD_NAMES) /* if request for disk and CD names */
      {
      hdnum = cdnum = Temp = 0;                /* initialise numbers to zero all round */
      }
   else
      {
      StorageControllers = 0;                  /* otherwise initialise storage controller #'s */
      }

   if (!rmqinit)                               /* if this is the first entry to this routine */
      {
      for (i = 0; i < 40; i++)                 /* loop 40 times */
         {
         pcidev[i] = (PCIDEV*)malloc(sizeof(PCIDEV));    /* grab storage */
         pcidev[i]->Classcode = 0;              /* and initialise it */
         pcidev[i]->VID = 0;
         pcidev[i]->DID = 0;
         }
      rmqinit = 1;                             /* show we've done this */
      }

   Function1 = Function;                       /* save the function call # */

   rc = DosAllocMem((PPVOID)&pRMCopy,          /* grab enough memory for the RM Query returns */
                   65536,
                   PAG_COMMIT |
                   PAG_READ   |
                   PAG_WRITE);
   if (rc)
      {
      printf("DosAllocMem failed rc = %d\n", rc);  /* and print error if fails */
      exit;
      }
   rc = DosAllocMem((PPVOID)&pRMStruct,          /* same thing again for more RM stuff */
                   32767,
                   PAG_COMMIT |
                   PAG_READ   |
                   PAG_WRITE);
   if (rc)
      {
      printf("DosAllocMem failed rc = %d\n", rc);
      exit;
      }

   ulCatPCI   = IOCTL_OEMHLP;         /* Device category */
   ulFuncPCI  = OEMHLP_PCI;           /* Device-specific function */
   ulDataLPCI = sizeof(ubusData);     /* length of returned data */

   rc = DosOpen("OEMHLP$",            /* DosOpen for OEMHLP device driver */
               &DevHandPCI,
               &ulActPCI,
               (ULONG)NULL,
               (ULONG)NULL,
               1,
               0x40,
               0);
   uchPCI.Command = 0;
   if (!rc)                           /* if opened OK ask PCI bus data */
      {
      rc = DosDevIOCtl(DevHandPCI,         /* Handle to device */
                      ulCatPCI,            /* Category of request */
                      ulFuncPCI,           /* Function being requested */
                      (void*)&uchPCI,      /* Input/Output parameter list */
                      sizeof(uchPCI),      /* Maximum output parameter size */
                      &ulParmLen,          /* Input:  size of parameter list */
                                           /* Output: size of parameters returned */
                      (void*)&ubusData,    /* Input/Output data area */
                      sizeof(ubusData),    /* Maximum output data size */
                      &ulDataLen);         /* Input:  size of input data area */
                                           /* Output: size of data returned   */
      if (!rc)                             /* if rc is zero */
         {
         pcibusnum = ubusData.lastbus;     /* number of PCI buses in machine */
         if (debugging)                    /* if /debug set */
            {
            printf("\nThis system has %d PCI bus", /* print debug info */
                  pcibusnum+1);
            if (pcibusnum)                 /* plural if more than one */
               {
               printf("es");
               }
            printf(", PCI version %x.%x\n", /* and version */
                  ubusData.majver,
                  ubusData.minver);
            }
         PCI = 1;                           /* show this is a PCI machine */
         }
      else
         {
         PCI = 0;                           /* otherwise show it isn't PCI */
         }
      }

   ulParmLen = sizeof(ULONG)+sizeof(ULONG)+sizeof(USHORT);   /* Length of input parameters */

   rc = DosOpen("RESMGR$",              /* device name of resource manager */
               &DevHandle,              /* open conversation with him */
               &ulAction,
               (ULONG)NULL,
               (ULONG)NULL,
               1,
               0x40,
               0);

   rc = DosDevIOCtl(DevHandle,           /* Handle to device */
                    ulCategory,          /* Category of request */
                    ulFunction,          /* Function being requested */
                    (void*)&uchParms,    /* Input/Output parameter list */
                    sizeof(uchParms),    /* Maximum output parameter size */
                    &ulParmLen,          /* Input:  size of parameter list */
                                         /* Output: size of parameters returned */
                    pRMCopy,             /* Input/Output data area */
                    65535,               /* Maximum output data size */
                    &ulDataLen);         /* Input:  size of input data area */
                                         /* Output: size of data returned   */

   if (rc)
      {
      printf("DosDevIOCtl rc = 0x%08x\n", rc);  /* print return code if nonzero */
      }

   res = (RM_ENUMNODES_DATA*)pRMCopy;    /* point at returned data */

   for (i = 0; i < res->NumEntries; i++) /* and loop for number of entries in list */
      {
      ULONG j, k, l;

      ulCategory = CAT_RM;                 /* setup for next rm call */
      ulFunction = FUNC_RM_GET_NODEINFO;   /* ask for info on this node */
      rmgnp.RMHandle = res->NodeEntry[i].RMHandle; /* RM parms point to RM handle */
      rmgnp.Linaddr  = (ULONG)pRMStruct;           /* base address of returned data */
      ulParmLen  = sizeof(rmgnp);          /* size of this area */
      ulDataLen  = 0;

      rc = DosDevIOCtl(DevHandle,           /* Handle to device */
                       ulCategory,          /* Category of request */
                       ulFunction,          /* Function being requested */
                       (void*)&rmgnp,       /* Input/Output parameter list */
                       8,                   /* Maximum output parameter size */
                       &ulParmLen,          /* Input:  size of parameter list */
                                            /* Output: size of parameters returned */
                       pRMStruct,           /* Input/Output data area */
                       32767,               /* Maximum output data size */
                       &ulDataLen);         /* Input:  size of input data area */
                                            /* Output: size of data returned   */

      rmnode = (RM_GETNODE_DATA*)pRMStruct; /* coerce returned data into rmnode format */

      switch (rmnode->RMNode.NodeType)      /* decide on node type (adapter/device etc) */
         {
         case RMTYPE_ADAPTER:               /* if this node is for an adapter */
            {
            switch (rmnode->RMNode.pAdapterNode->BaseType) /* decide on type of adapter */
               {
               case AS_BASE_MSD:            /* if mass storage device adapter */
               case AS_BASE_DISPLAY:        /* or display adapter */
                  {
                  if (PCI)                  /* and it's a PCI machine */
                     {
                     UCHAR SubT;
                     int   remembered = 0;  /* temp area for controller # */

                     ulDataLPCI     = sizeof(ubusData1);
                     uchPCI.Command = 2;          /* subfunction 2, find PCI class code */

                     if (rmnode->RMNode.pAdapterNode->pAdjunctList) /* if we have an adjunct list */
                        {
                        uchPCI.Index   = rmnode->RMNode.pAdapterNode->pAdjunctList->Adapter_Number; /* set controller number in query to ours */
                        }
                     else                 /* if we don't have an adjunct list */
                        {
                        uchPCI.Index = 0;   /* query on controller zero instead */
                        }

                     SubT = rmnode->RMNode.pAdapterNode->SubType; /* temp subtype is PCI subtype */
                     SubT--;                                      /* decrement by 1 */

                     memcpy((char*)&uchPCI.ClassCode+sizeof(UCHAR), /* fill in in PCI query parms */
                           (const void*)&SubT,
                           sizeof(UCHAR));
                     memcpy((char*)&uchPCI.ClassCode+sizeof(USHORT), /* fill in base type in PCI query parms */
                           (const void*)&rmnode->RMNode.pAdapterNode->BaseType,
                           sizeof(USHORT));

                     sprintf(tempdevicedesc, "%s",              /* set to default name */
                           rmnode->RMNode.pAdapterNode->AdaptDescriptName);

                     if (debugging)                 /* if /debug requested */
                        {
                        printf("Adapter name = %s\n",   /* print this stuff */
                              rmnode->RMNode.pAdapterNode->AdaptDescriptName);
                        }

                     remembered = StorageControllers; /* remember count of controllers */

                     GetPCIData(); /* overwrite and copy tempdevicedesc if we can */

                     if (remembered == StorageControllers) /* if we didn't find a controller in the scan */
                        {                                  /* let's try to force scan for it instead */
                        switch (rmnode->RMNode.pAdapterNode->BaseType) /* decide on PCI base type */
                           {
                           case AS_BASE_MSD:                 /* if mass storage device adapter */
                              {
                              switch (rmnode->RMNode.pAdapterNode->SubType)
                                 {
                                 case AS_SUB_IDE:            /* subtype IDE */
                                 case AS_SUB_SCSI:           /* or subtype SCSI */
                                    {
                                    if (Function == FUNC_GET_CONTROLLER_NAMES) /* if controller names requested */
                                       {
                                       if (!searchdone)      /* and we have not done this before */
                                          {
                                          EnumeratePCIDevices();  /* go scan all PCI devices */
                                          searchdone = 1;         /* stop us doing this more than once */
                                          }
                                       for (j = 0; j < 40; j++)   /* search up to 20 entries from force scan */
                                          {
                                          if (uchPCI.ClassCode == pcidev[j]->Classcode) /* if our adapter classcode */
                                             {
                                             pcicfg.vendorID = pcidev[j]->VID; /* set vendor ID from it */
                                             pcicfg.deviceID = pcidev[j]->DID; /* set device ID from it */

                                             DecodePCIclass();  /* go decode this PCI classcode */

                                             if (remembered == StorageControllers) /* and if we still didn't find it */
                                                {
                                                strcpy(DiskController[StorageControllers]->desc.detected, tempdevicedesc);
                                                /* set controller name to RM name for it instead */
                                                break;  /* and exit loop */
                                                }
                                             else
                                                {
                                                break;  /* if we did find it in decodePCIclass() then exit loop */
                                                }
                                             }
                                          }
                                       }

                                    if (Function == FUNC_GET_CONTROLLER_NAMES ||
                                        Function == FUNC_GET_CONTROLLER_NUMBERS)
                                       {
                                       StorageControllers++; /* increase controller number */
                                       }
                                    else           /* if get disk and CD names */
                                       {
                                       Temp++;     /* temp increment */
                                       }
                                    }
                                 }
                              }
                           }
                        }
                     }
                  }
               }
            }
         break;

         case RMTYPE_DEVICE:                       /* if RM node type for device */
            {
            switch (rmnode->RMNode.pDeviceNode->DevType) /* decide on dev type */
               {
               case DS_TYPE_DISK:                 /* if a disk */
                  {
                  switch (rmnode->RMNode.pDeviceNode->DevFlags) /* decide on flags */
                     {
                     case DS_REMOVEABLE_MEDIA:    /* if disk removeable */
                        break;                    /* skip */

                     default:                /* if not removeable */
                        {
                        if (rmnode->RMNode.pDeviceNode->DevDescriptName)
                           {
                           AddHD(rmnode->RMNode.pDeviceNode->DevDescriptName); /* add HD entry from RM dev descript */
                           }
                        else
                           {
                           AddHD("Thisisan Untitled device found");
                           }
                        }
                     break;
                     }
                  }
               break;

               case DS_TYPE_WORM:      /* if WORM device */
                  {
                  if (rmnode->RMNode.pDeviceNode->DevDescriptName)
                     {
                     AddHD(rmnode->RMNode.pDeviceNode->DevDescriptName); /* add HD entry from RM dev descript */
                     }
                  else
                     {
                     AddHD("Thisisan Untitled device found");
                     }
                  }
               break;

               case DS_TYPE_OPT_MEM:   /* if optical memory */
                  {
                  if (rmnode->RMNode.pDeviceNode->DevDescriptName)
                     {
                     AddHD(rmnode->RMNode.pDeviceNode->DevDescriptName); /* add HD entry */
                     }
                  else
                     {
                     AddHD("Thisisan Untitled device found");
                     }
                  }
               break;

               case DS_TYPE_CDROM:     /* if CD ROM drive */
                  {
                  if (rmnode->RMNode.pDeviceNode->DevDescriptName)
                     {
                     AddCD(rmnode->RMNode.pDeviceNode->DevDescriptName); /* add CD entry */
                     }
                  else
                     {
                     AddHD("Thisisan Untitled device found");
                     }
                  }
               break;
               }
            }
         break;
         }
      }

   if (PCI) /* if on a PCI machine, query for PCI class "bridge" and type PCI/CPU to get mobo chipset */
      {
      UCHAR SubT, BaseT;

      ulDataLPCI     = sizeof(ubusData1);
      uchPCI.Command = 2;          /* subfunction 2, find PCI class code */

      SubT = 0;        /* setup query for PCI/CPU */
      BaseT = 6;       /* and PCI Class Bridge */
      uchPCI.Index = 0;

      memcpy((char*)&uchPCI.ClassCode+sizeof(UCHAR),
            (const void*)&SubT,
            sizeof(UCHAR));
      memcpy((char*)&uchPCI.ClassCode+sizeof(USHORT),
            (const void*)&BaseT,
            sizeof(USHORT));

      GetPCIData(); /* overwrite and copy tempdevicedesc if we can */
      }

   DosClose(DevHandle);  /* free up resources */
   DosClose(DevHandPCI);  /* free up resources */

   DosFreeMem(pRMCopy);  /* and get out */
   DosFreeMem(pRMStruct);
}


void AddHD(char* pointer) /* add HD entry */
  {
  char *pointer1;         /* working data */
  int index, k;

  pointer1 = strstr(pointer, " ");  /* find first blank in description */
  if (pointer1)                     /* if we find a blank */
     {
     index    = strspn(pointer1, " ");  /* find the next one */
     pointer1 += index;                 /* jump to it */

     if (strlen(pointer1))
        {
        for (k = strlen(pointer1)-1; k > 0; k--) /* scan description backwards */
           {
           if (pointer1[k] == ' ') /* for blanks */
              {
              pointer1[k] = 0;     /* and eliminate trailing blanks */
              }
           else                    /* otherwise break out of loop */
              {
              break;
              }
           }

        for (k = 0; k < strlen(pointer1)-1; k++) /* index through remaining description */
           {
           if (pointer1[k] == ' ' && /* and we have two consecutive blanks */
               pointer1[k+1] == ' ')
              {
              strcpy(pointer1+k, pointer1+k+1); /* shuffle remaining data left one place */
              k--;                              /* make sure we scan the entire string */
              }
           }
        }

     if (Function1 == FUNC_GET_DISK_AND_CD_NAMES) /* if we asked for disk and CD names */
        {
        sprintf(szDisk[hdnum]->desc.detected, "%s", pointer1); /* set relevant HD description */
        if (Temp)
           {
           szDisk[hdnum]->Controller = Temp;         /* and controller number */
           }
        else
           {
           szDisk[hdnum]->Controller = MAX_CONTROLLERS;         /* and controller number */
           }
        szDisk[hdnum]->Disknum = hdnum;           /* and device number */
        }
     if (debugging)       /* if /debug asked for */
        {
        printf("HD %d = %s\n", hdnum, pointer1);   /* print disk number and desc */
        }
     hdnum++;                                      /* add one to curent count */
     }
  }


void AddCD(char* pointer)                   /* add CD description */
  {
  char *pointer1;
  int index, k;

  pointer1 = strstr(pointer, " ");  /* find first blank in description */
  if (pointer1)                     /* if we find a blank */
     {
     index    = strspn(pointer1, " "); /* and next blank */
     pointer1 += index;                /* jump to it */

     if (strlen(pointer1))
        {
        for (k = strlen(pointer1)-1; k > 0; k--) /* scan description backwards */
           {
           if (pointer1[k] == ' ') /* for blanks */
              {
              pointer1[k] = 0;     /* and eliminate trailing blanks */
              }
           else                    /* otherwise break out of loop */
              {
              break;
              }
           }

        for (k = 0; k < strlen(pointer1)-1; k++) /* index through remaining description */
           {
           if (pointer1[k] == ' ' && /* and we have two consecutive blanks */
               pointer1[k+1] == ' ')
              {
              strcpy(pointer1+k, pointer1+k+1); /* shuffle remaining data left one place */
              k--;                              /* make sure we scan the entire string */
              }
           }
        }

     if (Function1 == FUNC_GET_DISK_AND_CD_NAMES)  /* if function disk and CD names */
        {
        sprintf(szDisk[cdnum+10]->desc.detected, "%s", pointer1); /* set up description */
        if (Temp)
           {
           szDisk[cdnum+10]->Controller = Temp;       /* and controller number */
           }
        else
           {
           szDisk[cdnum+10]->Controller = MAX_CONTROLLERS;       /* and controller number */
           }
        szDisk[cdnum+10]->Disknum = cdnum;         /* and device number */
        }
     if (debugging)                               /* if /debug */
        {
        printf("CD %d = %s\n", cdnum, pointer1);  /* print it */
        }
     cdnum++;
     }
  }


int DecodePCIclass(void)  /* look up pcicfg.classcode in table */
  {
  char    *bodge, *p, *n1;
  FILE    *fp;
  static char Buffer[1025];
  char    vendor[255], device[250], odevice[7], rdevice[5], vdevice[7], ddevice[7], sdevice[7];
  char  invocationpath[CCHMAXPATH] = ".\\pcidevs.txt";  /* path to data file */
  int   go, warndone;

  sprintf(vdevice, "V\t%04X", pcicfg.vendorID);
  sprintf(ddevice, "D\t%04X", pcicfg.deviceID);
  sprintf(rdevice, "R\t%02X", pcicfg.revisionID);
  sprintf(odevice, "O\t%04X", pcicfg.nonbridge.subsystem_vendorID);
  sprintf(sdevice, "S\t%04X", pcicfg.nonbridge.subsystem_deviceID);

  go = 0;
  fp = fopen(invocationpath, "r");       /* open pcicfg.dat for read */
  if (!fp)                               /* if it didn't open */
     {
     if (!warndone)
        {
        warndone = 1;
        if (debugging)
           {
           printf("Unable to open file \"%s\".\nNo vendor/device translation available.\n",
                 invocationpath);
           }
        }
     }
  else    /* if pcicfg.dat did open */
     {
     while(fgets(Buffer, sizeof(Buffer), fp) && go == 0 )
        {
        if (Buffer[0] == ';')
           continue;
        if (Buffer[0] == '\n')
           continue;
        if (Buffer[strlen(Buffer)-1] == '\n')
           {
           Buffer[strlen(Buffer)-1] = 0;
           }
        n1 = strstr((char*)Buffer, vdevice);

        if (n1)
           {
           n1 = n1+strlen(vdevice)+1;
           sprintf(vendor, "%s", n1);

           while(fgets(Buffer, sizeof(Buffer), fp) && go == 0 )
              {
              if (Buffer[0] == ';')
                 continue;
              if (Buffer[0] == '\n')
                 continue;
              if (Buffer[strlen(Buffer)-1] == '\n')
                 {
                 Buffer[strlen(Buffer)-1] = 0;
                 }

              n1 = strstr((char*)Buffer, ddevice);
              if (n1)
                 {
                 n1 = n1 + strlen(ddevice) + 1;

                 sprintf(device, "%s", n1);
                 go = 1;
                 while (fgets(Buffer, sizeof(Buffer), fp) && go == 1)
                    {
                    if (Buffer[0] == ';')
                       continue;
                    if (Buffer[0] == '\n')
                       continue;
                    if (Buffer[strlen(Buffer)-1] == '\n')
                       {
                       Buffer[strlen(Buffer)-1] = 0;
                       }
                    n1 = strstr(Buffer, rdevice); // look for R\trevision

                    if (n1)
                       {
                       n1 = n1 + strlen(rdevice) +1;
                       sprintf(device, "%s", n1);
                       go = 2;
                       }
                    else
                       {
                       if (Buffer[0] != 'R')
                          break;
                       }
                    }

                 if ((pcicfg.nonbridge.subsystem_vendorID != 0) && (pcicfg.nonbridge.subsystem_deviceID != 0) && go > 0)
                    {
                    while (fgets(Buffer, sizeof(Buffer), fp) && go < 5)
                       {
                       if (Buffer[0] == ';')
                          continue;
                       if (Buffer[0] == '\n')
                          continue;
                       if (Buffer[strlen(Buffer)-1] == '\n')
                          {
                          Buffer[strlen(Buffer)-1] = 0;
                          }
                       n1 = strstr(Buffer, odevice); // look for O\toemid
                       if (n1)
                          {
                          n1 = n1 + strlen(odevice) +1;
                          sprintf(device, "%s", n1);
                          go = 3;
                          while (fgets(Buffer, sizeof(Buffer), fp) && go < 5)
                             {
                             if (Buffer[0] == ';')
                                continue;
                             if (Buffer[0] == '\n')
                                continue;
                             if (Buffer[strlen(Buffer)-1] == '\n')
                                {
                                Buffer[strlen(Buffer)-1] = 0;
                                }
                             n1 = strstr(Buffer, sdevice); // look for S\toemid

                             if (n1)
                                {
                                n1 = n1 + strlen(sdevice) +1;
                                sprintf(device, "%s", n1);
                                go = 4;
                                break;
                                }
                             else
                                {
                                if ((Buffer[0] == 'V') || (Buffer[0] == 'D'))
                                   {
                                   go = 5;
                                   }
                                }
                             }
                          if ((pcicfg.vendorID != pcicfg.nonbridge.subsystem_vendorID) &&
                              (!fseek(fp, 0, SEEK_SET)))   // point back to start of file
                             {
                             char oemdevice[7];
                             sprintf(oemdevice, "V\t%04X", pcicfg.nonbridge.subsystem_vendorID);
                             while(fgets(Buffer, sizeof(Buffer), fp) && go == 0 )
                                {
                                if (Buffer[0] == ';')
                                   continue;
                                if (Buffer[0] == '\n')
                                   continue;
                                if (Buffer[strlen(Buffer)-1] == '\n')
                                   {
                                   Buffer[strlen(Buffer)-1] = 0;
                                   }
                                n1 = strstr((char*)Buffer, oemdevice);

                                if (n1)
                                   {
                                   n1 = n1+strlen(oemdevice)+1;
                                   sprintf(vendor, "%s", n1);
                                   }
                                }
                             }
                          break;
                          }
                       else
                          {
                          if ((Buffer[0] == 'V') || (Buffer[0] == 'D'))
                             {
                             go = 5;
                             }
                          }
                       }
                    }
                 }
              else
                 {
                 n1 = strstr((char*)Buffer, "V\t");

                 if (n1)
                    {
                    if (go == 0)
                       {
                       sprintf(device, "Unknown device 0x%04X", pcicfg.deviceID);
                       break;
                       }
                    }
                 }
              }
           }
        }
     fclose (fp); /* close file at end */
     sprintf(tempdevicedesc, "%s - %s", vendor, device);
     printf("%s - %s ", vendor, device);
     }

  if (go)             /* if we found the classcode in the file */
     {
     if (debugging)
        {
        printf("PCI Class = ");
        }
     switch (pcicfg.classcode-1) /* decide on classcode */
        {
        case 0:                  /* if storage class */
           {
           if (debugging)
              {
              printf("(Storage - ");
              }
           switch (pcicfg.subclass)   /* decide on subclass */
              {
              case 0:                 /* if SCSI */
                 {
                 if (debugging)
                    {
                    printf("SCSI)\n");
                    }
                 if (Function1 == FUNC_GET_CONTROLLER_NAMES) /* if controller anmes requested */
                    {
                    strcpy(DiskController[StorageControllers]->desc.detected, tempdevicedesc); /* fill it in */
                    }
                 if (Function1 == FUNC_GET_CONTROLLER_NAMES ||
                     Function1 == FUNC_GET_CONTROLLER_NUMBERS) /* if names or numbers */
                    {
                    StorageControllers++;  /* increase count */
                    }
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("IDE)\n");
                    }
                 if (Function1 == FUNC_GET_CONTROLLER_NAMES)
                    {
                    strcpy(DiskController[StorageControllers]->desc.detected, tempdevicedesc);
                    }
                 if (Function1 == FUNC_GET_CONTROLLER_NAMES ||
                     Function1 == FUNC_GET_CONTROLLER_NUMBERS)
                    {
                    StorageControllers++;
                    }
                 }
              break;
              case 2:
                 {
                 if (debugging)
                    {
                    printf("Floppy)\n");
                    }
                 }
              break;
              case 3:
                 {
                 if (debugging)
                    {
                    printf("IPI)\n");
                    }
                 }
              break;
              case 4:
                 {
                 if (debugging)
                    {
                    printf("RAID)\n");
                    }
                 if (Function1 == FUNC_GET_CONTROLLER_NAMES)
                    {
                    strcpy(DiskController[StorageControllers]->desc.detected, tempdevicedesc);
                    }
                 if (Function1 == FUNC_GET_CONTROLLER_NAMES ||
                     Function1 == FUNC_GET_CONTROLLER_NUMBERS)
                    {
                    StorageControllers++;
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           } /* end switch class 0 */
        break;

        case 1:
           {
           if (debugging)
              {
              printf("(Network - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("Ethernet)\n");
                    }
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("Tokenring)\n");
                    }
                 }
              break;
              case 2:
                 {
                 if (debugging)
                    {
                    printf("FDDI)\n");
                    }
                 }
              break;
              case 3:
                 {
                 if (debugging)
                    {
                    printf("ATM)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }   /* end switch class 1 */
        break;

        case 2:
           {
           if (debugging)
              {
              printf("(Display - ");
              }
           switch(pcicfg.subclass)
              {
              case 0:
                 {
                 char* caplist = (char*)&pcicfg.nonbridge.base_address1;
                 char  agpchar[80];
                 int x;
                 if (debugging)
                    {
                    printf("VGA)\n");
                    }

                 caplist += pcicfg.nonbridge.cap_ptr;
                 x = (int)*(caplist+2);       // AGP version
                 if (x != 0)                  // if AGP version == 0 then don't do this
                    {
                    sprintf(agpchar, " (AGP v%d.%d", (x & 0xf0) >> 4, (x & 0x0f));
                    strcat(tempdevicedesc, agpchar);
                    x = (int)*(caplist+9);
                    sprintf(agpchar, " %sabled,", (x & 1) ? "en" : "dis");
                    strcat(tempdevicedesc, agpchar);
                    x = (int)*(caplist+4);
                    sprintf(agpchar, "%s%s%s capable, ", (x & 1) ? " 1x" : "",
                            (x & 2) ? " 2x" : "", (x & 4) ? " 4x" : "" );
                    strcat(tempdevicedesc, agpchar);
                    x = (int)*(caplist+8);
                    sprintf(agpchar, "using%s%s%s%s)", (x & 1) ? " 1x" : "",
                            (x & 2) ? " 2x" : "", (x & 4) ? " 4x" : "", (x & 7) ? "" : " None");
                    strcat(tempdevicedesc, agpchar);
                    if (strlen(tempdevicedesc) > 100)
                       tempdevicedesc[99] = 0;
                    printf("%s\n", tempdevicedesc);
                    }
                 strcpy(Graphicscard, tempdevicedesc);
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("SVGA)\n");
                    }
                 strcpy(Graphicscard, tempdevicedesc);
                 }
              break;
              case 2:
                 {
                 if (debugging)
                    {
                    printf("XGA)\n");
                    }
                 strcpy(Graphicscard, tempdevicedesc);
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 strcpy(Graphicscard, tempdevicedesc);
                 }
              }
           }
        break;

        case 3:
           {
           if (debugging)
              {
              printf("(Multimedia - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("Video)\n");
                    }
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("Audio)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }
        break;

        case 4:
           {
           if (debugging)
              {
              printf("(Memory - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("RAM)\n");
                    }
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("Flash Memory)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }
        break;

        case 5:
           {
           if (debugging)
              {
              printf("(Bridge - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("CPU/PCI)\n");
                    }
                 strcpy(Chipset, tempdevicedesc);
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("PCI/ISA)\n");
                    }
                 }
              break;
              case 2:
                 {
                 if (debugging)
                    {
                    printf("PCI/EISA)\n");
                    }
                 }
              break;
              case 3:
                 {
                 if (debugging)
                    {
                    printf("PCI/MCA)\n");
                    }
                 }
              break;
              case 4:
                 {
                 if (debugging)
                    {
                    printf("PCI/PCI)\n");
                    }
                 }
              break;
              case 5:
                 {
                 if (debugging)
                    {
                    printf("PCI/PCMCIA)\n");
                    }
                 }
              break;
              case 6:
                 {
                 if (debugging)
                    {
                    printf("PCI/NuBus)\n");
                    }
                 }
              break;
              case 7:
                 {
                 if (debugging)
                    {
                    printf("PCI/CardBus)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }
        break;

        case 6:
           {
           if (debugging)
              {
              printf("(Communication - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("Serial)\n");
                    }
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("Parallel)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }
        break;

        case 7:
           {
           if (debugging)
              {
              printf("(System Peripheral - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("PIC)\n");
                    }
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("DMAC)\n");
                    }
                 }
              break;
              case 2:
                 {
                 if (debugging)
                    {
                    printf("Timer)\n");
                    }
                 }
              break;
              case 3:
                 {
                 if (debugging)
                    {
                    printf("RTC)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }
        break;

        case 8:
           {
           if (debugging)
              {
              printf("(Input - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("Keyboard)\n");
                    }
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("Digitizer)\n");
                    }
                 }
              break;
              case 2:
                 {
                 if (debugging)
                    {
                    printf("Mouse)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }
        break;

        case 9:
           {
           if (debugging)
              {
              printf("(Docking Station - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("Generic)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }
        break;

        case 10:
           {
           if (debugging)
              {
              printf("(CPU - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("386)\n");
                    }
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("486)\n");
                    }
                 }
              break;
              case 2:
                 {
                 if (debugging)
                    {
                    printf("Pentium)\n");
                    }
                 }
              break;
              case 3:
                 {
                 if (debugging)
                    {
                    printf("Pentium Pro)\n");
                    }
                 }
              break;
              case 16:
                 {
                 if (debugging)
                    {
                    printf("Alpha)\n");
                    }
                 }
              break;
              case 64:
                 {
                 if (debugging)
                    {
                    printf("Coprocessor)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }
        break;

        case 11:
           {
           if (debugging)
              {
              printf("(Serial Bus - ");
              }
           switch (pcicfg.subclass)
              {
              case 0:
                 {
                 if (debugging)
                    {
                    printf("Firewire)\n");
                    }
                 }
              break;
              case 1:
                 {
                 if (debugging)
                    {
                    printf("Access Bus)\n");
                    }
                 }
              break;
              case 2:
                 {
                 if (debugging)
                    {
                    printf("SSA)\n");
                    }
                 }
              break;
              case 3:
                 {
                 if (debugging)
                    {
                    printf("USB)\n");
                    }
                 }
              break;
              case 4:
                 {
                 if (debugging)
                    {
                    printf("Fiber Channel)\n");
                    }
                 }
              break;
              default:
                 {
                 if (debugging)
                    {
                    printf("Other 0x%02x)\n", pcicfg.subclass);
                    }
                 }
              }
           }
        break;
        }
     } /* end if go */
  else
     {
     return 1;
     }
  return 0;
  }


int GetPCIData(void) /* query PCI device by classcode basetype/subtype */
   {
   ULONG   ulParmLen        = 0;            /* Input and output parameter size */
   APIRET  rc               = NO_ERROR;     /* Return code */

   ulParmLen = sizeof(uchPCI);

   rc = DosDevIOCtl(DevHandPCI,         /* Handle to device */
                   ulCatPCI,            /* Category of request */
                   ulFuncPCI,           /* Function being requested */
                   (void*)&uchPCI,      /* Input/Output parameter list */
                   sizeof(uchPCI),      /* Maximum output parameter size */
                   &ulParmLen,          /* Input:  size of parameter list */
                                        /* Output: size of parameters returned */
                   (void*)&ubusData,    /* Input/Output data area */
                   sizeof(ubusData),    /* Maximum output data size */
                   &ulDataLPCI);        /* Input:  size of input data area */
                                        /* Output: size of data returned   */
   if (!rc)
      {
      char* bodge;

      ulDataLPCI = sizeof(ubusData1);
      uchPCI1.Command = 3;              /* subfunction 3, read config */
      uchPCI1.BusNum  = ubusData.hwmech;
      uchPCI1.DevFuncNum = ubusData.majver;
      uchPCI1.Size = 4;
      uchPCI1.CfgReg = 0;

      rc = DosDevIOCtl(DevHandPCI,      /* Handle to device */
                      ulCatPCI,         /* Category of request */
                      ulFuncPCI,        /* Function being requested */
                      (void*)&uchPCI1,  /* Input/Output parameter list */
                      sizeof(uchPCI1),  /* Maximum output parameter size */
                      &ulParmLen,       /* Input:  size of parameter list */
                                        /* Output: size of parameters returned */
                      (void*)&ubusData1, /* Input/Output data area */
                      sizeof(ubusData1), /* Maximum output data size */
                      &ulDataLPCI);     /* Input:  size of input data area */
                                        /* Output: size of data returned   */
      bodge = (char*)&pcicfg;
      memcpy(bodge+uchPCI1.CfgReg, &ubusData1.data, 4);

      if (!rc && !ubusData1.rc && ubusData1.data != -1)
         {
         for (uchPCI1.CfgReg = 4; uchPCI1.CfgReg < 252; uchPCI1.CfgReg += 4)
            {
            rc = DosDevIOCtl(DevHandPCI,          /* Handle to device */
                            ulCatPCI,          /* Category of request */
                            ulFuncPCI,          /* Function being requested */
                            (void*)&uchPCI1,    /* Input/Output parameter list */
                            sizeof(uchPCI1),    /* Maximum output parameter size */
                            &ulParmLen,          /* Input:  size of parameter list */
                                                 /* Output: size of parameters returned */
                            (void*)&ubusData1,   /* Input/Output data area */
                            sizeof(ubusData1),   /* Maximum output data size */
                            &ulDataLPCI);         /* Input:  size of input data area */
                                                 /* Output: size of data returned   */
            bodge = (char*)&pcicfg;
            memcpy(bodge+uchPCI1.CfgReg, &ubusData1.data, 4);
            if (uchPCI1.CfgReg == 64)
               if (pcicfg.classcode != 3 || pcicfg.subclass != 0)
                  break;
            }
         printf("\n");
         printf("Vendor = %04X DeviceID = %04X cmd_reg = %04X status_reg = %04X\n",
               pcicfg.vendorID, pcicfg.deviceID, pcicfg.command_reg, pcicfg.status_reg);
         printf("Revision = %02X subclass = %02X classcode = %02X\n",
               pcicfg.revisionID, pcicfg.subclass, pcicfg.classcode);
         switch (pcicfg.header_type & 0x7f)
            {
            case 0:  // nonbridge
               if (pcicfg.nonbridge.base_address0 != 0)
                  printf("Base Address 0 (%s) = %08X\n", (pcicfg.nonbridge.base_address0 & 0x00000001) ? "i/o": "mem",
                        pcicfg.nonbridge.base_address0 & 0xffffff00);
               if (pcicfg.nonbridge.base_address1 != 0)
                  printf("Base Address 1 (%s) = %08X\n", (pcicfg.nonbridge.base_address1 & 0x00000001) ? "i/o": "mem",
                        pcicfg.nonbridge.base_address1 & 0xffffff00);
               if (pcicfg.nonbridge.base_address2 != 0)
                  printf("Base Address 2 (%s) = %08X\n", (pcicfg.nonbridge.base_address2 & 0x00000001) ? "i/o": "mem",
                        pcicfg.nonbridge.base_address2 & 0xffffff00);
               if (pcicfg.nonbridge.base_address3 != 0)
                  printf("Base Address 3 (%s) = %08X\n", (pcicfg.nonbridge.base_address3 & 0x00000001) ? "i/o": "mem",
                        pcicfg.nonbridge.base_address3 & 0xffffff00);
               if (pcicfg.nonbridge.base_address4 != 0)
                  printf("Base Address 4 (%s) = %08X\n", (pcicfg.nonbridge.base_address4 & 0x00000001) ? "i/o": "mem",
                        pcicfg.nonbridge.base_address4 & 0xffffff00);
               if (pcicfg.nonbridge.base_address5 != 0)
                  printf("Base Address 5 (%s) = %08X\n", (pcicfg.nonbridge.base_address5 & 0x00000001) ? "i/o": "mem",
                        pcicfg.nonbridge.base_address5 & 0xffffff00);
               if (pcicfg.nonbridge.CardBus_CIS != 0)
                  printf("CardBus CIS    = %08X\n", pcicfg.nonbridge.CardBus_CIS);
               if (pcicfg.nonbridge.subsystem_vendorID != 0)
                  printf("Subsys VendorID= %04X\n", pcicfg.nonbridge.subsystem_vendorID);
               if (pcicfg.nonbridge.subsystem_deviceID != 0)
                  printf("Subsys DeviceID= %04X\n", pcicfg.nonbridge.subsystem_deviceID);
               if (pcicfg.nonbridge.expansion_ROM != 0)
                  printf("Expansion ROM  = %08X\n", pcicfg.nonbridge.expansion_ROM);
               if (pcicfg.nonbridge.interrupt_line != 0)
                  printf("Interrupt Line = %02X\n", pcicfg.nonbridge.interrupt_line);
               if (pcicfg.nonbridge.interrupt_pin != 0)
                  printf("Interrupt Pin  = %c\n", 'A'-1+pcicfg.nonbridge.interrupt_pin);
               if (pcicfg.classcode == 3 && pcicfg.subclass == 0)
                  {
                  char* caplist = (char*)&pcicfg.nonbridge.base_address1;
                  int x;
                  caplist += pcicfg.nonbridge.cap_ptr;
                  x = (int)*(caplist+2);
                  if (x != 0)
                     {
                     printf("AGP capabilities, Version %d.%d\n", (x & 0xf0) >> 4, (x & 0x0f));
                     x = (int)*(caplist+4);
                     printf("AGP Speeds supported : %s %s %s\n", (x & 1) ? "1x" : "",
                             (x & 2) ? "2x" : "", (x & 4) ? "4x" : "" );
                     printf("FW transfers supported : %s\n", (x & 0x10) ? "Yes" : "No");
                     printf(">4GB Address space supported : %s\n", (x & 0x20) ? "Yes" : "No");
                     x = (int)*(caplist+5);
                     printf("Sideband addressing supported : %s\n", (x & 2) ? "Yes" : "No");
                     x = (int)*(caplist+7);
                     printf("Maximum command queue length : %d byte%s\n", (x + 1), (x ? "s" : ""));
                     x = (int)*(caplist+8);
                     printf("AGP speed selected : %s %s %s %s\n", (x & 1) ? "1x" : "",
                             (x & 2) ? "2x" : "", (x & 4) ? "4x" : "", (x & 7) ? "" : "None");
                     printf("FW transfers enabled : %s\n", (x & 0x10) ? "Yes" : "No");
                     printf(">4GB Address space enabled : %s\n", (x & 0x20) ? "Yes" : "No");
                     x = (int)*(caplist+9);
                     printf("AGP Enabled : %s\n", (x & 1) ? "Yes" : "No");
                     printf("Sideband Addressing enabled : %s\n", (x & 2) ? "Yes" : "No");
                     x = (int)*(caplist+11);
                     printf("Current command queue length : %d byte%s\n", x+1, (x ? "s" :""));
                     }
                  }
               break;

            case 1:  // bridge
               if (pcicfg.bridge.base_address0 != 0)
                  printf("Base Address 0 (%s) = %08X\n", (pcicfg.bridge.base_address0 & 0x00000001) ? "i/o": "mem",
                        pcicfg.bridge.base_address0 & 0xffffff00);
               if (pcicfg.bridge.base_address1 != 0)
                  printf("Base Address 1 (%s) = %08X\n", (pcicfg.bridge.base_address1 & 0x00000001) ? "i/o": "mem",
                        pcicfg.bridge.base_address1 & 0xffffff00);
               if (pcicfg.bridge.primary_bus != 0)
                  printf("Primary Bus    = %02X\n", pcicfg.bridge.primary_bus);
               if (pcicfg.bridge.secondary_bus != 0)
                  printf("Secondary Bus  = %02X\n", pcicfg.bridge.secondary_bus);
               if (pcicfg.bridge.subordinate_bus != 0)
                  printf("Subordinate Bus= %02X\n", pcicfg.bridge.subordinate_bus);
               if (pcicfg.bridge.expansion_ROM != 0)
                  printf("Expansion ROM  = %08X\n", pcicfg.bridge.expansion_ROM);
               if (pcicfg.bridge.interrupt_line != 0)
                  printf("Interrupt Line = %02X\n", pcicfg.bridge.interrupt_line);
               if (pcicfg.bridge.interrupt_pin != 0)
                  printf("Interrupt Pin  = %c\n", 'A'-1+pcicfg.bridge.interrupt_pin);
               break;

            case 2:  // cardbus
               printf("CardBus device\n");
               break;
            }
         }

      return DecodePCIclass();
      }
   else
      {
      return 1;
      }
   }

/* build a list of PCI classcodes and devfunc numbers in machine */
/* bypass a bug in IBM's code that doesn't id IDE controllers by */
/* PCI classcode 0x00010100.                                     */
void EnumeratePCIDevices(void)
 {
 int bus, device, func, i;
 char *bodge;
 ULONG   ulCategory       = CAT_RM;         /* Device category */
 ULONG   ulFunction       = FUNC_RM_ENUM_NODES; /* Device-specific function */
 ULONG   ulParmLen        = 0;            /* Input and output parameter size */
 ULONG   ulDataLen        = 0;            /* Input and output data size */
 ULONG   ulAction         = 0;
 APIRET  rc               = NO_ERROR;     /* Return code */

 i = 0;

 for (bus = 0; bus <= pcibusnum; bus++) // for each PCI bus
    {
    for (device = 0; device < 32; device++) // for each device on each bus
       {
       for (func = 0; func < 8; func++) // for each subdevice on each device on each bus!
          {
//          printf("Querying PCI device %d:%d.%d\n", bus, device, func);
          ulCategory        = IOCTL_OEMHLP;         /* Device category */
          ulFunction        = OEMHLP_PCI;           /* Device-specific function */
          ulDataLen = sizeof(ubusData1);
          uchPCI1.Command = 3;                     /* subfunction 3, read PCI config space */
          uchPCI1.BusNum  = bus;
          uchPCI1.DevFuncNum = (device<<3) | (func & 0x07);
          uchPCI1.Size = 4;
          uchPCI1.CfgReg = 0;
          ulParmLen = sizeof(uchPCI1);

          // read first 4 bytes of PCI config space for bus:device:func
          rc = DosDevIOCtl(DevHandPCI,           /* Handle to device */
                          ulCategory,          /* Category of request */
                          ulFunction,          /* Function being requested */
                          (void*)&uchPCI1,    /* Input/Output parameter list */
                          sizeof(uchPCI1),    /* Maximum output parameter size */
                          &ulParmLen,          /* Input:  size of parameter list */
                                               /* Output: size of parameters returned */
                          (void*)&ubusData1,   /* Input/Output data area */
                          sizeof(ubusData1),   /* Maximum output data size */
                          &ulDataLen);         /* Input:  size of input data area */
                                               /* Output: size of data returned   */

          //printf("Device %d:%d.%d rc = %d\n", bus, device, func, rc);
          if (!rc && !ubusData1.rc && ubusData1.data != -1) // if we got the first 4 bytes
             {
             bodge = (char*)&pcicfg;
             memcpy(bodge+uchPCI1.CfgReg, &ubusData1.data, 4);

             for (uchPCI1.CfgReg = 4; uchPCI1.CfgReg < 20; uchPCI1.CfgReg += 4) // now get rest of PCI config
                {                                                               // space in 4 byte chunks
                ulDataLen = sizeof(ubusData1);
                ulParmLen = sizeof(uchPCI1);
                uchPCI1.Size = 4;
                rc = DosDevIOCtl(DevHandPCI,          /* Handle to device */
                                ulCategory,          /* Category of request */
                                ulFunction,          /* Function being requested */
                                (void*)&uchPCI1,    /* Input/Output parameter list */
                                sizeof(uchPCI1),    /* Maximum output parameter size */
                                &ulParmLen,          /* Input:  size of parameter list */
                                                     /* Output: size of parameters returned */
                                (void*)&ubusData1,   /* Input/Output data area */
                                sizeof(ubusData1),   /* Maximum output data size */
                                &ulDataLen);         /* Input:  size of input data area */
                                                     /* Output: size of data returned   */
                if (!rc && !ubusData1.rc && ubusData1.data != -1) // if it worked
                   {
                   bodge = (char*)&pcicfg;
                   memcpy(bodge+uchPCI1.CfgReg, &ubusData1.data, 4);
                   }
                else
                   {
                   char tmp[100];
                   sprintf(tmp, "DosDevIOCtl rc = %d disp %02x", rc, uchPCI1.CfgReg);
                   logit(tmp);
                   }
                }
             // we have first 16 bytes of PCI config space
             bodge = (char*)&pcidev[i]->Classcode;

             memset(bodge, 0, 4);
             memcpy(bodge+2, &pcicfg.classcode, 1);
             memcpy(bodge+1, &pcicfg.subclass, 1);
             pcidev[i]->VID = pcicfg.vendorID;
             pcidev[i]->DID = pcicfg.deviceID;
             pcidev[i]->DevFunc = uchPCI1.DevFuncNum;

             i++;
             if (i > 39)
                {
                logit("More than 40 PCI devices!\n");
                bus = pcibusnum;
                device = 32;
                func = 8;
                break;
                }
             if (!(pcicfg.header_type & 0x80))
                break;
             }
          }
       }
    }
 }


