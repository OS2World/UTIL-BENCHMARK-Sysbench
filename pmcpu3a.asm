;     Filename:       cpuid3a.asm
;      Copyright 1993, 1994 by Intel Corp.
;
;      This program has been developed by Intel Corporation.
;   Intel has intellectual property
;      rights which it may assert if another manufacturer's processor
;      mis-identifies itself as being "GenuineIntel" when the CPUID
;      instruction is executed.
;
;      Intel specifically disclaims all warranties, express or
;      implied, and all liability, including consequential and other
;      indirect damages, for the use of this code, including
;      liability for infringement of any proprietary rights, and
;      including the warranties of merchantability and fitness for a
;      particular purpose.  Intel does not assume any responsibility
;      for any errors which may appear in this code nor any
;      responsibility to update it.
;
;      This code contains two procedures:
;      _get_cpu_type: Identifies processor type in _cpu_type:
;              0=8086/8088 processor
;              2=Intel 286 processor
;              3=Intel386(TM) family processor
;              4=Intel486(TM) family processor
;              5=Pentium(TM) family processor
;              6=Pentium Pro(TM) family processor
;
;      _get_fpu_type: Identifies FPU type in _fpu_type:
;              0=FPU not present
;              1=FPU present
;              2=287 present (only if _cpu_type=3)
;              3=387 present (only if _cpu_type=3)
;

       TITLE   cpuid3
       DOSSEG
       .MODEL FLAT,C,OS_OS2
       .386p                   ; it is safe to use 386 instructions

CPU_ID MACRO
       db      0fh             ; Hardcoded CPUID instruction
       db      0a2h
ENDM

CODE32      SEGMENT Dword USE32 PUBLIC 'CODE'
            ASSUME SS:FLAT,DS:FLAT,ES:FLAT

_DATA    SEGMENT dword public 'DATA' USE32; All code and data segments must be named
       public  _cpu_type
       public  _fpu_type
       public  _cpuid_flag
       public  _intel_CPU
       public  _vendor_id
       public  _cpu_signature
       public  _features_ecx
       public  _features_edx
       public  _features_ebx
       public  _inst_cache
       public  _inst_cway
       public  _inst_lines
       public  _data_cache
       public  _data_cway
       public  _data_lines
       public  _lvl2_cache
       public  _cache_info
       public  _amd_model
_cpu_type      db      0
_fpu_type      db      0
_cpuid_flag    db      0
_intel_CPU     db      0
_vendor_id     db      "------------"
intel_id       db      "GenuineIntel"
AMD_id         db      "AuthenticAMD"
Cyrix_id       db      "CyrixInstead"
NexGen_id      db      "NexGenDriven"
IDT_id         db      "CentaurHauls"
UMC_id         db      "UMC UMC UMC "
_inst_cway     db      0
_inst_lines    db      0
_data_cway     db      0
_data_lines    db      0
workarea       db      "workworkworkwork"
_amd_model     db      "123456789012345678901234567890123456789012345678"
_cache_info    db      0
_cpu_signature dd      0
_features_ecx  dd      0
_features_edx  dd      0
_features_ebx  dd      0
_lvl2_cache    dd      0
_inst_cache    dw      0
_data_cache    dw      0
fp_status      dw      0
cpuid_high     dd      0
ecpuid_high    dw      0
_DATA ENDS

       ;.CODE

;*********************************************************************

       public  _get_cpu_type
_get_cpu_type  proc

;      This procedure determines the type of processor in a system
;      and sets the _cpu_type variable with the appropriate
;      value.  If the CPUID instruction is available, it is used
;      to determine more specific details about the processor.
;      All registers are used by this procedure, none are preserved.
;      To avoid AC faults, the AM bit in CR0 must not be set.

;      Intel386 processor check
;      The AC bit, bit #18, is a new bit introduced in the EFLAGS
;      register on the Intel486 processor to generate alignment
;      faults.
;      This bit cannot be set on the Intel386 processor.

check_80386:
       pushfd                  ; push original EFLAGS
       pop     eax             ; get original EFLAGS
       mov     ecx, eax        ; save original EFLAGS
       xor     eax, 40000h     ; flip AC bit in EFLAGS
       push    eax             ; save new EFLAGS value on stack
       popfd                   ; replace current EFLAGS value
       pushfd                  ; get new EFLAGS
       pop     eax             ; store new EFLAGS in EAX
       xor     eax, ecx        ; can't toggle AC bit, processor=80386
       mov     _cpu_type, 3    ; turn on 80386 processor flag
       jz      end_cpu_type    ; jump if 80386 processor

       push    ecx
       popfd                   ; restore AC bit in EFLAGS first

;      Intel486 processor check
;      Checking for ability to set/clear ID flag (Bit 21) in EFLAGS
;      which indicates the presence of a processor with the CPUID
;      instruction.

;      .486p                   ; it is safe to use 486 instructions
check_80486:
       mov     _cpu_type, 4    ; turn on 80486 processor flag
       mov     eax, ecx        ; get original EFLAGS
       xor     eax, 200000h    ; flip ID bit in EFLAGS
       push    eax             ; save new EFLAGS value on stack
       popfd                   ; replace current EFLAGS value
       pushfd                  ; get new EFLAGS
       pop     eax             ; store new EFLAGS in EAX
       xor     eax, ecx        ; can't toggle ID bit,
       jne     cpuid_supp      ; processor=80486

       xor     eax,eax         ;clear eax
       sahf                    ;clear flags
       mov     eax, 5
       mov     ebx, 2
       div     bl              ;do an operation that does not change flags
       lahf                    ;get flags
       cmp     ah, 2           ;check for change in flags
       jne     end_cpu_type    ;not Cyrix, go away
       mov     _intel_CPU, 3   ;show Cyrix
       mov     _cpuid_flag, 1  ;flag indicating use of CPUID inst. (lies)
       mov     _cpu_signature, 00000fffh
       jmp     end_cpu_type    ;get out
cpuid_supp:
;      Execute CPUID instruction to determine vendor, family,
;      model, stepping and features.  For the purpose of this
;      code, only the initial set of CPUID information is saved.

       mov     _cpuid_flag, 1  ; flag indicating use of CPUID inst.
       push    ebx             ; save registers
       push    esi
       push    edi
       mov     eax, 0          ; set up for CPUID instruction
       CPU_ID                  ; get and save vendor ID
       mov     cpuid_high, eax ;save eax as highest CPUID func allowed
       mov     dword ptr _vendor_id, ebx
       mov     dword ptr _vendor_id[+4], edx
       mov     dword ptr _vendor_id[+8], ecx

       mov     esi, offset FLAT:_vendor_id
       mov     edi, offset FLAT:intel_id
       mov     ecx, 12         ; should be length intel_id
       cld                     ; set direction flag
       repe    cmpsb           ; compare vendor ID to "GenuineIntel"
       jne     amd_cpuid_type  ; if not equal, not an Intel processor

       mov     _intel_CPU, 1   ; indicate an Intel processor
       cmp     cpuid_high, 1   ; make sure 1 is valid input for CPUID
       jl      end_cpuid_type  ; if not, jump to end
       mov     eax, 1
       CPU_ID                  ; get family/model/stepping/features
       mov     _cpu_signature, eax
       mov     _features_ebx, ebx
       mov     _features_edx, edx
       mov     _features_ecx, ecx

       shr     eax, 8          ; isolate family
       and     eax, 0fh
       mov     _cpu_type, al   ; set _cpu_type with family
       cmp     cpuid_high, 2   ;cache info available?
       jl      end_cpuid_type  ;exit if not
       mov     _cache_info, 1  ;show cache info avail.
       mov     eax, 2          ;set up to ask for cache details
       CPU_ID                  ;get cache details

       mov     dword ptr workarea, eax ;save all registers returned
       mov     dword ptr workarea[+4], ebx
       mov     dword ptr workarea[+8], ecx
       mov     dword ptr workarea[+12], edx
       mov     ecx, 16
cache_loop:
       mov     al, byte ptr workarea[+0+ecx]   ;use next byte
       cmp     al, 00          ;check for nil byte
       je      cache_end       ;skip this one then
       cmp     al, 06          ;check for 8kb inst, 4 way, 32 byte
       jne     cache1          ;not found, jump
       mov     byte ptr _inst_cache, 8         ;8kb
       mov     byte ptr _inst_cway, 4          ;4 way
       mov     byte ptr _inst_lines, 32        ;32 byte lines
       jmp     cache_end       ;get next
cache1:        cmp     al, 08          ;check for 16kb inst, 4 way, 32 byte
       jne     cache2          ;not found, jump
       mov     byte ptr _inst_cache, 16        ;16kb
       mov     byte ptr _inst_cway, 4          ;4 way
       mov     byte ptr _inst_lines, 32        ;32 byte lines
       jmp     cache_end       ;get next
cache2:        cmp     al, 0ah         ;check for 8kb data, 2 way, 32 byte
       jne     cache3          ;not found, jump
       mov     byte ptr _data_cache, 8         ;8kb
       mov     byte ptr _data_cway, 2          ;2 way
       mov     byte ptr _data_lines, 32        ;32 byte lines
       jmp     cache_end       ;get next
cache3:        cmp     al, 0ch         ;check for 16kb data, 4 way, 32 byte
       jne     cache3a          ;not found, jump
       mov     byte ptr _data_cache, 16        ;16kb
       mov     byte ptr _data_cway, 4          ;4 way
       mov     byte ptr _data_lines, 32        ;32 byte lines
       jmp     cache_end       ;get next
cache3a:       cmp     al, 2ch         ;check for 32kb data, 8 way, 64 byte
       jne     cache3b
       mov     byte ptr _data_cache, 32
       mov     byte ptr _data_cway, 8
       mov     byte ptr _data_lines, 64
       jmp     cache_end
cache3b:       cmp     al, 30h         ;check for 32kb inst, 8 way, 64 byte
       jne     cache3c
       mov     byte ptr _inst_cache, 32
       mov     byte ptr _inst_cway, 8
       mov     byte ptr _inst_lines, 64
       jmp     cache_end
cache3c:       cmp     al, 60h         ;check for 16kb data, 8 way, 64 byte
       jne     cache3d
       mov     byte ptr _inst_cache, 16
       mov     byte ptr _inst_cway, 8
       mov     byte ptr _inst_lines, 64
       jmp     cache_end
cache3d:       cmp     al, 66h         ;check for 8kb data, 4 way, 64 byte
       jne     cache3e
       mov     byte ptr _inst_cache, 8
       mov     byte ptr _inst_cway, 4
       mov     byte ptr _inst_lines, 64
       jmp     cache_end
cache3e:       cmp     al, 67h         ;check for 16kb data, 4 way, 64 byte
       jne     cache3f
       mov     byte ptr _inst_cache, 16
       mov     byte ptr _inst_cway, 4
       mov     byte ptr _inst_lines, 64
       jmp     cache_end
cache3f:       cmp     al, 68h         ;check for 32kb data, 4 way, 64 byte
       jne     cache4
       mov     byte ptr _inst_cache, 32
       mov     byte ptr _inst_cway, 4
       mov     byte ptr _inst_lines, 64
       jmp     cache_end

cache4:        cmp     al, 40h         ;check for no L2 cache
       jne     cache5          ;not found, jump
       mov     dword ptr _lvl2_cache, 0        ;0kb
       jmp     cache_end       ;get next
cache5:        cmp     al, 41h         ;check for 128kb L2 cache
       jne     cache5a          ;not found, jump
       mov     dword ptr _lvl2_cache, 128      ;128kb
       jmp     cache_end       ;get next
cache5a:        cmp     al, 79h         ;check for 128kb L2 cache
       jne     cache6          ;not found, jump
       mov     dword ptr _lvl2_cache, 128      ;128kb
       jmp     cache_end       ;get next
cache6:        cmp     al, 42h         ;check for 256Kb L2 cache
       jne     cache6a          ;not found, jump
       mov     dword ptr _lvl2_cache, 256      ;256kb
       jmp     cache_end       ;get next
cache6a:        cmp     al, 7ah         ;check for 256Kb L2 cache
       jne     cache7          ;not found, jump
       mov     dword ptr _lvl2_cache, 256      ;256kb
       jmp     cache_end       ;get next
cache7:        cmp     al, 43h         ;check for 512Kb L2 cache
       jne     cache7a          ;not found, jump
       mov     dword ptr _lvl2_cache, 512      ;512kb
       jmp     cache_end       ;get next
cache7a:        cmp     al, 7bh         ;check for 512Kb L2 cache
       jne     cache8          ;not found, jump
       mov     dword ptr _lvl2_cache, 512      ;512kb
       jmp     cache_end       ;get next
cache8:        cmp     al, 44h         ;check for 1024Kb L2 cache?
       jne     cache8a          ;not found, jump
       mov     dword ptr _lvl2_cache, 1024     ;1024kb
       jmp     cache_end       ;get next
cache8a:        cmp     al, 7ch         ;check for 1024Kb L2 cache?
       jne     cache9          ;not found, jump
       mov     dword ptr _lvl2_cache, 1024     ;1024kb
       jmp     cache_end       ;get next
cache9:        cmp     al, 45h         ;check for 2048Kb L2 cache?
       jne     cache9a       ;not found, jump
       mov     dword ptr _lvl2_cache, 2048     ;2048kb
       jmp     cache_end       ;get next
cache9a:        cmp     al, 7dh         ;check for 2048Kb L2 cache?
       jne     cache10       ;not found, jump
       mov     dword ptr _lvl2_cache, 2048     ;2048kb
       jmp     cache_end       ;get next
cache10: cmp     al, 82h         ;check for 256Kb/8way L2 cache?
       jne     cache11       ;not found, jump
       mov     dword ptr _lvl2_cache, 256     ;256 kb
       jmp     cache_end       ;get next
cache11: cmp     al, 83h         ;check for 512Kb/8way L2 cache?
       jne     cache12     ;not found, jump
       mov     dword ptr _lvl2_cache, 512     ;512 kb
       jmp     cache_end       ;get next
cache12: cmp     al, 84h         ;check for 1MB/8way L2 cache?
       jne     cache13     ;not found, jump
       mov     dword ptr _lvl2_cache, 1024    ;1024kb
       jmp     cache_end       ;get next
cache13: cmp     al, 85h         ;check for 2MB/8way L2 cache?
       jne     cache14     ;not found, jump
       mov     dword ptr _lvl2_cache, 2048    ;2048kb
       jmp     cache_end       ;get next
cache14: cmp     al, 86h         ;check for 512Kb/8way L2 cache?
       jne     cache15     ;not found, jump
       mov     dword ptr _lvl2_cache, 512     ;512 kb
       jmp     cache_end       ;get next
cache15: cmp     al, 87h         ;check for 1MB/8way L2 cache?
       jne     cache_end   ;not found, jump
       mov     dword ptr _lvl2_cache, 1024    ;1024kb
       jmp     cache_end       ;get next
cache_end:
       dec     ecx             ;decrement loop counter
       jnz     cache_loop      ;loop
       jmp     end_cpuid_type  ;exit

amd_cpuid_type:
       mov     esi, offset FLAT:_vendor_id
       mov     edi, offset FLAT:AMD_id
       mov     ecx,12          ;length of AuthenticAMD
       cld                     ;set direction
       repe    cmpsb           ;compare vendor id to AuthenticAMD
       jne     cyrix_cpuid_type ;exit checking
       mov     _intel_CPU, 2   ;indicate AMD cpu
       cmp     cpuid_high, 1   ;check cpuid func 1 allowed
       jl      end_cpuid_type  ;exit if not
       mov     eax, 1          ;CPUID function code 1
       CPU_ID
       mov     _cpu_signature, eax
       mov     _features_ebx, ebx
       mov     _features_edx, edx
       mov     _features_ecx, ecx
       shr     eax,8           ;isolate family and model
       mov     _cpu_type, al   ;set _cpu_type with it
       mov     eax, 80000000h  ;call to find highest poss EAX value
       CPU_ID
       cmp     eax, 00000000h  ;extended calls supported?
       je      end_cpuid_type  ;no, exit
       cmp     ax,  0005h      ;our specific call OK?
       mov     ecpuid_high, ax ;save for later use
       jl      end_cpuid_type  ;no, bog off then
       mov     eax, 80000001h  ;extended version of type 1 info
       CPU_ID
       and     _features_edx, 00001000h ; preserve MTRR bit from standard call
       or      _features_edx, edx ; merge extended feature list
       mov     eax, 80000002h  ;first info
       CPU_ID
       mov     dword ptr _amd_model, eax
       mov     dword ptr _amd_model[+4], ebx
       mov     dword ptr _amd_model[+8], ecx
       mov     dword ptr _amd_model[+12], edx
       mov     eax, 80000003h  ;first info
       CPU_ID
       mov     dword ptr _amd_model[+16], eax
       mov     dword ptr _amd_model[+20], ebx
       mov     dword ptr _amd_model[+24], ecx
       mov     dword ptr _amd_model[+28], edx
       mov     eax, 80000004h  ;first info
       CPU_ID
       mov     dword ptr _amd_model[+32], eax
       mov     dword ptr _amd_model[+36], ebx
       mov     dword ptr _amd_model[+40], ecx
       mov     dword ptr _amd_model[+44], edx
       mov     eax, 80000005h  ;call for cache info
       CPU_ID
       mov     _cache_info, 1  ;show info avail.
       mov     eax, ecx        ;save it
       shr     eax, 24         ;shift it right 24 bits
       mov     _data_cache, ax ;data cache info
       mov     eax, ecx
       shl     eax, 8          ;shift left 8 bits
       shr     eax, 24         ;shift right 24
       mov     _data_cway, al  ;set cache associativity
       mov     _data_lines, cl ;set bytes per line
       mov     eax, edx
       shr     eax, 24         ;shift right 24
       mov     _inst_cache, ax ;instruction cache info
       mov     eax, edx
       shl     eax, 8          ;shift left 8
       shr     eax, 24         ;shift right 24
       mov     _inst_cway, al  ;set value
       mov     _inst_lines, dl ;set bytes per line
       cmp     ecpuid_high, 6  ;extended cpuid func 6 supported?
       jl      end_cpuid_type
       mov     eax, 80000006h  ; function 6
       CPU_ID
       mov     eax, ecx        ;move to work reg
       shr     eax, 16         ;lose least significant 16 bits
       mov     _lvl2_cache, ax ; save it
       jmp     end_cpuid_type  ;fuck off then

cyrix_cpuid_type:
       mov     esi, offset FLAT:_vendor_id
       mov     edi, offset FLAT:Cyrix_id
       mov     ecx,12          ;length of CyrixInstead
       cld                     ;set direction
       repe    cmpsb           ;compare vendor id to CyrixInstead
       jne     IDT_cpuid_type ;exit checking
       mov     _intel_CPU, 3   ;indicate Cyrix cpu
       cmp     cpuid_high, 1   ;check max func allowed
       jl      end_cpuid_type  ;exit if not allowed
       mov     eax, 1          ;CPUID function code 1
       CPU_ID
       mov     _cpu_signature, eax
       mov     _features_ebx, ebx
       mov     _features_edx, edx
       mov     _features_ecx, ecx
       shr     eax,4           ;isolate family and model
       mov     _cpu_type, al   ;set _cpu_type with it
       cmp     cpuid_high, 2   ;check max func allowed
       jl      end_cpuid_type  ;exit if not
       mov     eax, 2          ;set func 2
       CPU_ID
       mov     eax, 4          ;4 times round loop
cyrix_loop:
       cmp     dl, 80h         ;check for 16kb cache, 4 way, 16 bytes/line
       jne     cyr_loop_dec    ;no, branch to decrement count
       mov     _inst_cache, 16
       mov     _inst_cway, 4
       mov     _inst_lines, 16
       mov     _data_cache, 16
       mov     _data_cway, 4
       mov     _data_lines, 16
       jmp     end_cyrix_loop
cyr_loop_dec:
       rol     edx, 8          ;rotate edx left 1 byte
       dec     eax
       jnz     cyrix_loop
end_cyrix_loop:
       mov     eax, 80000000h  ;call to find highest poss EAX value
       CPU_ID
       cmp     eax, 00000000h  ;extended calls supported?
       je      end_cpuid_type  ;no, exit
       cmp     ax,  0005h      ;our specific call OK?
       jl      end_cpuid_type  ;no, bog off then
       mov     eax, 80000002h  ;first info
       CPU_ID
       mov     dword ptr _amd_model, eax
       mov     dword ptr _amd_model[+4], ebx
       mov     dword ptr _amd_model[+8], ecx
       mov     dword ptr _amd_model[+12], edx
       mov     eax, 80000003h  ;first info
       CPU_ID
       mov     dword ptr _amd_model[+16], eax
       mov     dword ptr _amd_model[+20], ebx
       mov     dword ptr _amd_model[+24], ecx
       mov     dword ptr _amd_model[+28], edx
       mov     eax, 80000004h  ;first info
       CPU_ID
       mov     dword ptr _amd_model[+32], eax
       mov     dword ptr _amd_model[+36], ebx
       mov     dword ptr _amd_model[+40], ecx
       mov     dword ptr _amd_model[+44], edx
IDT_cpuid_type:
       mov     esi, offset FLAT:_vendor_id
       mov     edi, offset FLAT:IDT_id
       mov     ecx,12          ;length of CentaurHauls
       cld                     ;set direction
       repe    cmpsb           ;compare vendor id to CentaurHauls
       jne     NxG_cpuid_type  ;exit checking
       mov     _intel_CPU, 4   ;indicate IDT cpu
       cmp     cpuid_high, 1   ;check max func allowed
       jl      end_cpuid_type  ;exit if not allowed
       mov     eax, 1          ;CPUID function code 1
       CPU_ID
       mov     _cpu_signature, eax
       mov     _features_ebx, ebx
       mov     _features_edx, edx
       mov     _features_ecx, ecx
       shr     eax,4           ;isolate family and model
       mov     _cpu_type, al   ;set _cpu_type with it
       jmp     end_cpuid_type
NxG_cpuid_type:
       mov     esi, offset FLAT:_vendor_id
       mov     edi, offset FLAT:NexGen_id
       mov     ecx,12          ;length of NexGenDriven
       cld                     ;set direction
       repe    cmpsb           ;compare vendor id to NexGenDriven
       jne     UMC_cpuid_type ;exit checking
       mov     _intel_CPU, 5   ;indicate NexGen cpu
       cmp     cpuid_high, 1   ;check max func allowed
       jl      end_cpuid_type  ;exit if not allowed
       mov     eax, 1          ;CPUID function code 1
       CPU_ID
       mov     _cpu_signature, eax
       mov     _features_ebx, ebx
       mov     _features_edx, edx
       mov     _features_ecx, ecx
       shr     eax,4           ;isolate family and model
       mov     _cpu_type, al   ;set _cpu_type with it
       jmp     end_cpuid_type
UMC_cpuid_type:
       mov     esi, offset FLAT:_vendor_id
       mov     edi, offset FLAT:UMC_id
       mov     ecx,12          ;length of UMC UMC UMC
       cld                     ;set direction
       repe    cmpsb           ;compare vendor id to UMC UMC UMC
       jne     end_cpuid_type ;exit checking
       mov     _intel_CPU, 6   ;indicate UMC cpu
       cmp     cpuid_high, 1   ;check max func allowed
       jl      end_cpuid_type  ;exit if not allowed
       mov     eax, 1          ;CPUID function code 1
       CPU_ID
       mov     _cpu_signature, eax
       mov     _features_ebx, ebx
       mov     _features_edx, edx
       mov     _features_ecx, ecx
       shr     eax,4           ;isolate family and model
       mov     _cpu_type, al   ;set _cpu_type with it

end_cpuid_type:
       pop     edi             ; restore registers
       pop     esi
       pop     ebx

;      .8086
end_cpu_type:
       ret
_get_cpu_type  endp

;*********************************************************************

       public  _get_fpu_type
_get_fpu_type  proc

;      This procedure determines the type of FPU in a system
;      and sets the _fpu_type variable with the appropriate value.
;      All registers are used by this procedure, none are preserved.

;      Coprocessor check
;      The algorithm is to determine whether the floating-point
;      status and control words are present.  If not, no
;      coprocessor exists.  If the status and control words can
;      be saved, the correct coprocessor is then determined
;      depending on the processor type.  The Intel386 processor can
;      work with either an Intel287 NDP or an Intel387 NDP.
;      The infinity of the coprocessor must be checked to determine
;      the correct coprocessor type.

       fninit                  ; reset FP status word
       mov     fp_status, 5a5ah; initialize temp word to non-zero
       fnstsw  fp_status       ; save FP status word
       mov     ax, fp_status   ; check FP status word
       cmp     al, 0           ; was correct status written
       mov     _fpu_type, 0    ; no FPU present
       jne     end_fpu_type

check_control_word:
       fnstcw  fp_status       ; save FP control word
       mov     ax, fp_status   ; check FP control word
       and     ax, 103fh       ; selected parts to examine
       cmp     ax, 3fh         ; was control word correct
       mov     _fpu_type, 0
       jne     end_fpu_type    ; incorrect control word, no FPU
       mov     _fpu_type, 1

;      80287/80387 check for the Intel386 processor

check_infinity:
       cmp     _cpu_type, 3
       jne     end_fpu_type
       fld1                    ; must use default control from FNINIT
       fldz                    ; form infinity
       fdiv                    ; 8087/Intel287 NDP say +inf = -inf
       fld     st              ; form negative infinity
       fchs                    ; Intel387 NDP says +inf <> -inf
       fcompp                  ; see if they are the same
       fstsw   fp_status       ; look at status from FCOMPP
       mov     ax, fp_status
       mov     _fpu_type, 2    ; store Intel287 NDP for FPU type
       sahf                    ; see if infinities matched
       jz      end_fpu_type    ; jump if 8087 or Intel287 is present
       mov     _fpu_type, 3    ; store Intel387 NDP for FPU type
end_fpu_type:
       ret
_get_fpu_type  endp

CODE32 ENDS

       end
