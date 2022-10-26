# sysbench.mak
# Created by IBM WorkFrame/2 MakeMake at 13:26:34 on 28 June 1998
#
# The actions included in this make file are:
#  Compile::C++ Compiler
#  Link::Linker
#  Bind::Resource Bind
#  Compile::Resource Compiler

!IFDEF DEBUG
CCDEBUG = /Ti+ /O+
LLDEBUG = /debug
!ELSE
CCDEBUG = /Ti- /O+
LLDEBUG =
!ENDIF

.SUFFIXES:

.SUFFIXES: \
    .cpp .obj .rc .res .asm

.asm.obj:
    @echo " Assembler::Assembler "
    alp.exe -SV:ALP %s

.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe -DOS2TMR /Tl10 $(CCDEBUG) /Ss /Gm /Gl+ /C %s

{F:\sysbench\src}.cpp.obj:
    @echo " Compile::C++ Compiler "
    icc.exe -DOS2TMR /Tl10 $(CCDEBUG) /Ss /Gm /Gl+ /C %s

.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

{F:\sysbench\src}.rc.res:
    @echo " Compile::Resource Compiler "
    rc.exe -r %s .\%|fF.RES

all: \
    \sysbench\sysbench.exe

.\sysbench.map \sysbench\sysbench.exe: \
    F:\sysbench\src\pmcpu3a.obj \
    .\pmrmqry.obj \
    .\dhry_1.obj \
    .\dhry_2.obj \
    .\diskacc2.obj \
    .\pmbbench.obj \
    .\pmbdskfe.obj \
    .\pmbdskio.obj \
    .\pmbfilio.obj \
    .\pmbgbios.obj \
    .\pmbmemsp.obj \
    .\pmbsaver.obj \
    .\pmb_dive.obj \
    .\pmb_gfx.obj \
    .\pmb_main.obj \
    .\pmcpu3b.obj \
    .\pmb.res \
    {$(LIB)}dive.lib
    @echo " Link::Linker "
    @echo " Bind::Resource Bind "
    icc.exe @<<
     /B" /exepack:2 /st:65536 /pmtype:pm /optfunc /packcode $(LLDEBUG)"
     /Fe\sysbench\sysbench.exe
     /Fm"sysbench.map"
     dive.lib
     F:\sysbench\src\pmcpu3a.obj
     .\pmrmqry.obj
     .\dhry_1.obj
     .\dhry_2.obj
     .\diskacc2.obj
     .\pmbbench.obj
     .\pmbdskfe.obj
     .\pmbdskio.obj
     .\pmbfilio.obj
     .\pmbgbios.obj
     .\pmbmemsp.obj
     .\pmbsaver.obj
     .\pmb_dive.obj
     .\pmb_gfx.obj
     .\pmb_main.obj
     .\pmcpu3b.obj
<<
    rc.exe .\pmb.res \sysbench\sysbench.exe

.\pmcpu3b.obj: \
    F:\sysbench\src\pmcpu3b.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h

.\pmcpu3a.obj: \
    F:\sysbench\src\pmcpu3a.asm

.\pmb_main.obj: \
    F:\sysbench\src\pmb_main.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}pmbdatat.h \
    {F:\sysbench\src;$(INCLUDE);}diskacc2.h \
    {F:\sysbench\src;$(INCLUDE);}pmbbench.h \
    {F:\sysbench\src;$(INCLUDE);}pmb.h

.\pmb_gfx.obj: \
    F:\sysbench\src\pmb_gfx.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}pmb.h

.\pmb_dive.obj: \
    F:\sysbench\src\pmb_dive.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}pmb.h \
    {F:\sysbench\src;$(INCLUDE);}mmioos2.h \
    {F:\sysbench\src;$(INCLUDE);}dive.h \
    {F:\sysbench\src;$(INCLUDE);}fourcc.h

.\pmbsaver.obj: \
    F:\sysbench\src\pmbsaver.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}pmbdatat.h \
    {F:\sysbench\src;$(INCLUDE);}pmbbench.h \
    {F:\sysbench\src;$(INCLUDE);}pmb.h

.\pmbmemsp.obj: \
    F:\sysbench\src\pmbmemsp.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h

.\pmbgbios.obj: \
    F:\sysbench\src\pmbgbios.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}pmbbench.h \
    {F:\sysbench\src;$(INCLUDE);}pmb.h

.\pmbfilio.obj: \
    F:\sysbench\src\pmbfilio.cpp

.\pmbdskio.obj: \
    F:\sysbench\src\pmbdskio.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}diskacc2.h

.\pmbdskfe.obj: \
    F:\sysbench\src\pmbdskfe.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}pmbdatat.h \
    {F:\sysbench\src;$(INCLUDE);}diskacc2.h \
    {F:\sysbench\src;$(INCLUDE);}perfutil.h

.\pmbbench.obj: \
    F:\sysbench\src\pmbbench.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}pmbdatat.h \
    {F:\sysbench\src;$(INCLUDE);}pmbbench.h \
    {F:\sysbench\src;$(INCLUDE);}pmb.h

.\diskacc2.obj: \
    F:\sysbench\src\diskacc2.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}diskacc2.h

.\dhry_2.obj: \
    F:\sysbench\src\dhry_2.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}dhry.h

.\dhry_1.obj: \
    F:\sysbench\src\dhry_1.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}dhry.h

.\pmrmqry.obj: \
    F:\sysbench\src\pmrmqry.cpp \
    {F:\sysbench\src;$(INCLUDE);}types.h \
    {F:\sysbench\src;$(INCLUDE);}pmbdatat.h

.\pmb.res: \
    F:\sysbench\src\pmb.rc \
    {F:\sysbench\src;$(INCLUDE)}sysbench.ico \
    {F:\sysbench\src;$(INCLUDE)}syslight.ico \
    {F:\sysbench\src;$(INCLUDE)}pmb.h
