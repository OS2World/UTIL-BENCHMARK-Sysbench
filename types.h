
#ifndef TYPES_H
#define TYPES_H

#define true  1
#define false 0
#define null  0L

typedef char  	      	      	i8;
typedef signed char             s8;
typedef unsigned char           u8;
typedef short int     	      	i16;
typedef signed short int        s16;
typedef unsigned short int      u16;
typedef long int      	      	i32;
typedef signed long int         s32;
typedef unsigned long int       u32;
typedef signed short            bool;

typedef struct _desc
     {
     char  detected[100];
     char  user[100];
     } DESC;

typedef struct _diskdesc
     {
     ULONG Controller;
     ULONG Disknum;
     DESC  desc;
     } DISKDESC;

typedef struct _diskcontroller
     {
     DESC desc;
     } DISKCONTROLLER;

typedef struct _winparm
       {
       HWND hwnd;
       ULONG number;
       char  *desc;
       } WINPARM;

typedef struct _pcidev
     {
     ULONG Classcode;
     USHORT VID;
     USHORT DID;
     UCHAR DevFunc;
     } PCIDEV;


typedef struct _FSINFOBUF
     {
     ULONG        ulVolser;   /* Volume serial number */
     VOLUMELABEL  vol;        /* Volume label         */
     } FSINFOBUF;

typedef FSINFOBUF *PFSINFOBUF;


#pragma pack(1)
typedef struct  pciparms
     {
     UCHAR Command;
     ULONG ClassCode;
     UCHAR Index;
     } PCIPARMS;

typedef struct  pciparms1
     {
     UCHAR Command;
     UCHAR BusNum;
     UCHAR DevFuncNum;
     UCHAR CfgReg;
     UCHAR Size;
     } PCIPARMS1;

typedef struct pcibus
     {
     UCHAR rc;
     UCHAR hwmech;
     UCHAR majver;
     UCHAR minver;
     UCHAR lastbus;
     } PCIBUS;

typedef struct pcibusdata
     {
     UCHAR rc;
     ULONG data;
     } PCIBUSDATA;

typedef struct PCIcfg
     {
     USHORT vendorID;
     USHORT deviceID;
     USHORT command_reg;
     USHORT status_reg;
     BYTE revisionID;
     BYTE progIF;
     BYTE subclass;
     BYTE classcode;
     BYTE cacheline_size;
     BYTE latency;
     BYTE header_type;
     BYTE BIST;
     union
        {
        struct
          {
          ULONG base_address0 ;
          ULONG base_address1 ;
          ULONG base_address2 ;
          ULONG base_address3 ;
          ULONG base_address4 ;
          ULONG base_address5 ;
          ULONG CardBus_CIS ;
          USHORT  subsystem_vendorID ;
          USHORT  subsystem_deviceID ;
          ULONG expansion_ROM ;
          BYTE  cap_ptr ;
          BYTE  reserved1[3] ;
          ULONG reserved2[1] ;
          BYTE  interrupt_line ;
          BYTE  interrupt_pin ;
          BYTE  min_grant ;
          BYTE  max_latency ;
          ULONG device_specific[48] ;
          } nonbridge ;
        struct
          {
          ULONG base_address0 ;
          ULONG base_address1 ;
          BYTE  primary_bus ;
          BYTE  secondary_bus ;
          BYTE  subordinate_bus ;
          BYTE  secondary_latency ;
          BYTE  IO_base_low ;
          BYTE  IO_limit_low ;
          USHORT  secondary_status ;
          USHORT  memory_base_low ;
          USHORT  memory_limit_low ;
          USHORT  prefetch_base_low ;
          USHORT  prefetch_limit_low ;
          ULONG prefetch_base_high ;
          ULONG prefetch_limit_high ;
          USHORT  IO_base_high ;
          USHORT  IO_limit_high ;
          ULONG reserved2[1] ;
          ULONG expansion_ROM ;
          BYTE  interrupt_line ;
          BYTE  interrupt_pin ;
          USHORT  bridge_control ;
          ULONG device_specific[48] ;
          } bridge ;
        struct
          {
          ULONG ExCa_base ;
          BYTE  cap_ptr ;
          BYTE  reserved05 ;
          USHORT  secondary_status ;
          BYTE  PCI_bus ;
          BYTE  CardBus_bus ;
          BYTE  subordinate_bus ;
          BYTE  latency_timer ;
          ULONG memory_base0 ;
          ULONG memory_limit0 ;
          ULONG memory_base1 ;
          ULONG memory_limit1 ;
          USHORT  IObase_0low ;
          USHORT  IObase_0high ;
          USHORT  IOlimit_0low ;
          USHORT  IOlimit_0high ;
          USHORT  IObase_1low ;
          USHORT  IObase_1high ;
          USHORT  IOlimit_1low ;
          USHORT  IOlimit_1high ;
          BYTE  interrupt_line ;
          BYTE  interrupt_pin ;
          USHORT  bridge_control ;
          USHORT  subsystem_vendorID ;
          USHORT  subsystem_deviceID ;
          ULONG legacy_baseaddr ;
          ULONG cardbus_reserved[14] ;
          ULONG vendor_specific[32] ;
          } cardbus ;
       } ;
     } PCICFG;

typedef struct
     {
     int nHandle;
     int nData;
     ULONG nSides;
     ULONG nSectors;
     ULONG nTracks;
     void *pBuffer;
     } THREADPARMS;

typedef struct
     {
     BYTE   bCommand;
     USHORT usHead;
     USHORT usCylinder;
     USHORT usFirstSector;
     USHORT cSectors;
     struct
        {
        USHORT usSectorNumber;
        USHORT usSectorSize;
        }
     TrackTable[256];
     } TRACK;

#ifndef min
#define min(a,b) ((a) > (b) ? (b) : (a))
#endif
#ifndef max
#define max(a,b) ((a) < (b) ? (b) : (a))
#endif
#define clamp(low, x, high) ((x) < (low) ? (low) : ((x) > (high) ? (high) : (x)))

#endif
