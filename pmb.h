#define SYSB_VER            "0.9.5d"

// last used number 5309

#define WND_MAIN              5000
#define WND_MCCANVAS          5001
#define WND_VIEWPORT          5003
#define WND_MESSAGEB          5004
#define ID_WINDOW             5005

#define MI_MENU_PROJ          5100
#define MI_PROJ_SAVE          5101
#define MI_PROJ_ALL           5102
#define MI_PROJ_ABOUT         5103
#define MI_PROJ_KILL          5104
#define MI_PROJ_QUIT          5105
#define MI_PROJ_SAVE_HTML     5106
#define MI_PROJ_BEEP          5107
#define MI_PROJ_EXPAND        5108

#define MI_MENU_GFX           5110
#define MI_GFX_BITBLIT_SS     5111
#define MI_GFX_BITBLIT_MS     5112
#define MI_GFX_FILLRECT       5113
#define MI_GFX_PATFIL         5114
#define MI_GFX_VLINES         5115
#define MI_GFX_HLINES         5116
#define MI_GFX_DLINES         5117
#define MI_GFX_TEXTRENDER     5118
#define MI_GFX_ALL            5119

#define MI_MENU_CPUINT        5120
#define MI_CPUINT_DHRY        5121
#define MI_CPUINT_HANOI       5122
#define MI_CPUINT_HEAPS       5123
#define MI_CPUINT_SIEVE       5124
#define MI_CPUINT_ALL         5125

#define MI_MENU_CPUFLOAT      5130
#define MI_CPUFLOAT_LINPACK   5131
#define MI_CPUFLOAT_FLOPS     5132
#define MI_CPUFLOAT_FFT       5133
#define MI_CPUFLOAT_ALL       5134

#define MI_MENU_MEM           5140
#define MI_MEM_5              5141
#define MI_MEM_10             5142
#define MI_MEM_20             5143
#define MI_MEM_40             5144
#define MI_MEM_80             5145
#define MI_MEM_160            5146
#define MI_MEM_320            5147
#define MI_MEM_640            5148
#define MI_MEM_1280           5149

#define MI_MENU_DIVE          5150
#define MI_DIVE_VIDEO_BW      5151
#define MI_DIVE_ROTATE_SCREEN 5152
#define MI_DIVE_MS_11         5153
#define MI_DIVE_ALL           5154

#define MI_MENU_DISKIO        5160 /* reserve next 10 numbers for disks */
#define MI_MENU_DISKIO_SELECT 5161
#define MI_DISKIO_AVSEEK      5175
#define MI_DISKIO_CBXFER      5176
#define MI_DISKIO_CPU_USAGE   5177
#define MI_DISKIO_TRANS_SPEED 5178
#define MI_DISKIO_ALL         5179
#define MI_DISKIO_ALL_DISKS   5189
#define MI_ALL_DISKS          5190

#define MI_MEMR_5             5180
#define MI_MEMR_10            5181
#define MI_MEMR_20            5182
#define MI_MEMR_40            5183
#define MI_MEMR_80            5184
#define MI_MEMR_160           5185
#define MI_MEMR_320           5186
#define MI_MEMR_640           5187
#define MI_MEMR_1280          5188

#define MI_MEMW_5             5191
#define MI_MEMW_10            5192
#define MI_MEMW_20            5193
#define MI_MEMW_40            5194
#define MI_MEMW_80            5195
#define MI_MEMW_160           5196
#define MI_MEMW_320           5197
#define MI_MEMW_640           5198
#define MI_MEMW_1280          5199
#define MI_MEM_ALL            5200

#define MI_MENU_REF           5203
#define MI_REF_1_LOAD         5204
#define MI_REF_2_LOAD         5205
#define MI_REF_1_INFO         5206
#define MI_REF_2_INFO         5207

#define MI_MENU_FILEIO        5208
#define MI_MENU_FILEIO_SELECT 5209
#define MI_FILEIO_4KB         5236
#define MI_FILEIO_8KB         5237
#define MI_FILEIO_16KB        5238
#define MI_FILEIO_32KB        5239
#define MI_FILEIO_64KB        5240
#define MI_FILEIO_ALL         5241

#define IDD_SHOWWAIT          5250
#define IDD_SHOWWAIT1         5251
#define IDD_SHOWWAIT2         5252
#define IDD_SHOWICON          5253
#define IDD_SHOWLITE          5254
#define IDD_SHOWICON2         5255
#define IDD_SHOWWAIT3         5256
#define IDD_SHOWWAIT4         5257
#define IDD_SHOWWAIT5         5258

#define MI_MACHINE_DATA       5260

#define IDD_MACHINE_DATA      5261
#define IDD_MACHINE           5262

#define IDD_MACH_NAMET        5263
#define IDD_MACH_NAMED        5264
#define IDD_MACH_MOBOT        5265
#define IDD_MACH_MOBOD        5266
#define IDD_MACH_CHIPT        5267
#define IDD_MACH_CHIPD        5268
#define IDD_MACH_MAKET        5269
#define IDD_MACH_MAKED        5270
#define IDD_MACH_CACHT        5271
#define IDD_MACH_CACHD        5272
#define IDD_MACH_PROCT        5273
#define IDD_MACH_PROCD        5274
#define IDD_MACH_GRAPT        5275
#define IDD_MACH_GRAPD        5276
#define IDD_MACH_DISKT        5277
#define IDD_MACH_DISKD        5278
#define IDD_DID_OK            5279
#define IDD_DID_CANCEL        5280
#define IDD_MACH_DLG          5281
#define IDD_MACH_DLG_DATA     5282

#define MI_MENU_CDIO          5290 /* reserve next 10 numbers for CDs */
#define MI_MENU_CDIO_SELECT   5291
#define MI_CDIO_AVSEEK        5305
#define MI_CDIO_TRANS_INNER   5306
#define MI_CDIO_TRANS_OUTER   5307
#define MI_CDIO_CPU_USAGE     5308
#define MI_CDIO_ALL           5309
#define MI_CDIO_ALL_DRIVES    5310

#define IDD_DISK              5320
#define IDD_DISK_DLG          5321
#define IDD_DISK_NAME         5322

#define IDD_PROD_INFO         5350
#define IDI_PMICON            5351
#define IDT_PRODINFO_TEXT1    5352
#define IDT_PRODINFO_TEXT2    5353
#define IDT_PRODINFO_TEXT3    5354
#define IDT_PRODINFO_TEXT4    5355
#define IDT_PRODINFO_TEXT5    5356
#define IDT_PRODINFO_TEXT6    5357
#define IDT_PRODINFO_TEXT7    5358
#define IDT_PRODINFO_TEXT8    5359
#define IDT_PRODINFO_TEXT9    5360
#define IDT_BUTTON            5361

//#define QSV_NUMPROCESSORS       QSV_FOREGROUND_PROCESS+1
//#define QSV_MAXHPRMEM           QSV_FOREGROUND_PROCESS+2
//#define QSV_MAXHSHMEM           QSV_FOREGROUND_PROCESS+3
//#define QSV_MAXPROCESSES        QSV_FOREGROUND_PROCESS+4
//#define QSV_VIRTUALADDRESSLIMIT QSV_FOREGROUND_PROCESS+5
#define QSV_MAXREAL             (QSV_MAX+10)

#define GROUP_EXPANDED          0x00000001
#define GROUP_AUTOEXPANDED      0x00000002
#define GROUP_ALLEXPANDED       0x00000004

#define MB                  (1024*1024)
#define KB                  1024
#define MN                  1000000

