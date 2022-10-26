#define MAX_FIXED_DISKS 20
#define MAX_CD_DRIVES   10
#define MAX_CONTROLLERS 40

enum components_enum {
  comp_gfx,
  comp_cpuint,
  comp_cpufloat,
  comp_dive,
  comp_file,
  comp_mem,
  comp_alldisks,
  comp_disk,
  comp_cd
};

enum gfx_enum {
  gfx_bitblt_SS,
  gfx_bitblt_MS,
  gfx_filled_rect,
  gfx_patt_fill,
  gfx_vlines,
  gfx_hlines,
  gfx_dlines,
  gfx_textrender
};

enum cpuint_enum {
  cpuint_dhrystone,
  cpuint_hanoi,
  cpuint_heapsort,
  cpuint_sieve
};

enum cpufloat_enum {
  cpufloat_linpack,
  cpufloat_flops,
  cpufloat_fft
};

enum mem_enum {
  mem_5,
  mem_10,
  mem_20,
  mem_40,
  mem_80,
  mem_160,
  mem_320,
  mem_640,
  mem_1280,
  memr_5,
  memr_10,
  memr_20,
  memr_40,
  memr_80,
  memr_160,
  memr_320,
  memr_640,
  memr_1280,
  memw_5,
  memw_10,
  memw_20,
  memw_40,
  memw_80,
  memw_160,
  memw_320,
  memw_640,
  memw_1280
};

enum dive_enum {
  dive_videobw,
  dive_rotate,
  dive_ms_11
};

enum disk_enum {
  disk_avseek,
  disk_busxfer,
  disk_transf0,
  disk_transfn2,
  disk_transfn,
  disk_transf,
  disk_cpupct
};

enum cd_enum {
  cdio_avseek,
  cdio_inner,
  cdio_outer,
  cdio_cpupct
  };


struct component {
  ULONG bitsettings;
  s32 nrepeatcount;
  char title[80];
  s32 ndatalines;
  struct {
    char entry[30];       // Entry title, Vertical Lines
    double value;         // -1 == not defined
    double unit_val;      // example: if unit-string is MB/s, then this is 1024*1024
    char unit[40];        // unit-string              exaple: MB/s
  } datalines[40];
  double total;
  char unit_total[40];
};


#define NUM_COMPONENTS 9

struct glob_data {
  s32 selected_disk;
  s32 nr_fixed_disks;
  double fixed_disk_size[MAX_FIXED_DISKS];
  s32 selected_cd;
  s32 nr_cd_drives;
  double cd_drive_size[MAX_CD_DRIVES];
  struct component c[NUM_COMPONENTS+MAX_FIXED_DISKS+MAX_CD_DRIVES];
};


