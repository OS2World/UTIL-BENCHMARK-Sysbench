
void _Optlink DoAll(void* p);
void _Optlink DoAllCPUFloat(void* p);
void _Optlink DoAllCPUInt(void* p);
void _Optlink DoAllDIVE(void* p);
void _Optlink DoAllDiskIO(void* p);
void _Optlink DoAllDiskIOAll(void* p);
void _Optlink DoAllCDIO(void* p);
void _Optlink DoAllCDIOAll(void*);
void _Optlink DoAllGraphics(void* p);
void _Optlink DoAllMem(void* p);

void _Optlink DoCPUFloatFFT(void* p);
void _Optlink DoCPUFloatFlops(void* p);
void _Optlink DoCPUFloatLinpack(void* p);

void _Optlink DoCPUIntDhry(void* p);
void _Optlink DoCPUIntHanoi(void* p);
void _Optlink DoCPUIntHeaps(void* p);
void _Optlink DoCPUIntSieve(void* p);

void _Optlink DoDiskIOAvSeek(void* p);
void _Optlink DoDiskCacheXfer(void* p);
void _Optlink DoDiskIOTransSpeed(void* p);
void _Optlink DoDiskIOCPUUsage(void* p);

void _Optlink DoFileIO4(void*);
void _Optlink DoFileIO8(void*);
void _Optlink DoFileIO16(void*);
void _Optlink DoFileIO32(void*);
void _Optlink DoFileIO64(void*);
void    DoFileIOBuf(ULONG);
void _Optlink DoFileIOAll(void*);

void _Optlink DoCDIOAvSeek(void* p);
void _Optlink DoCDIOInnerSpeed(void* p);
void _Optlink DoCDIOOuterSpeed(void* p);
void _Optlink DoCDIOCPUUsage(void* p);

void _Optlink DoDiveRot(void* p);
void _Optlink DoDiveVBW(void* p);
void _Optlink DoDiveMS11(void* p);

void _Optlink DoGfxBlitBlitSS(void* p);
void _Optlink DoGfxBlitBlitMS(void* p);
void _Optlink DoGfxFillRect(void* p);
void _Optlink DoGfxDLines(void* p);
void _Optlink DoGfxHLines(void* p);
void _Optlink DoGfxPatFil(void* p);
void _Optlink DoGfxTextRender(void* p);
void _Optlink DoGfxVLines(void* p);

void _Optlink DoMem5(void* p);
void _Optlink DoMem10(void* p);
void _Optlink DoMem20(void* p);
void _Optlink DoMem40(void* p);
void _Optlink DoMem80(void* p);
void _Optlink DoMem160(void* p);
void _Optlink DoMem320(void* p);
void _Optlink DoMem640(void* p);
void _Optlink DoMem1280(void* p);

void _Optlink DoMemR5(void* p);
void _Optlink DoMemR10(void* p);
void _Optlink DoMemR20(void* p);
void _Optlink DoMemR40(void* p);
void _Optlink DoMemR80(void* p);
void _Optlink DoMemR160(void* p);
void _Optlink DoMemR320(void* p);
void _Optlink DoMemR640(void* p);
void _Optlink DoMemR1280(void* p);

void _Optlink DoMemW5(void* p);
void _Optlink DoMemW10(void* p);
void _Optlink DoMemW20(void* p);
void _Optlink DoMemW40(void* p);
void _Optlink DoMemW80(void* p);
void _Optlink DoMemW160(void* p);
void _Optlink DoMemW320(void* p);
void _Optlink DoMemW640(void* p);
void _Optlink DoMemW1280(void* p);

void   _Optlink DoSimDiskIO(void*);

void    DoRef1Info(void);
void    DoRef1Load(void);
void    DoRef2Info(void);
void    DoRef2Load(void);

double    CalcGfxAv(void);
double    CalcCPUIntAv(void);
double    CalcCPUFloatAv(void);
double    CalcMemAv(void);
double    CalcDIVEAv(void);
double    CalcDiskIOAv(s32);
double    CalcCDIOAv(s32);
double    CalcFileIOAv(void);

void      GetMachineInfo(void);
float     GetSwapFileSize(void);

#define START_STACKSIZE 65536
