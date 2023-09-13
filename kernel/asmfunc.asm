; asmfunc.asm
;
; System V AMD64 Calling Convention
; Registers: RDI, RSI, RDX, RCX, R8, R9

bits 64
section .text

global IoOut32 ; void IoOut32(uint16_t addr, uint32_t data)
IoOut32:
  mov dx, di
  mov eax, esi
  out dx, eax ; addr対してdataを書き込む
  ret

global IoIn32 ; uint32_t IoIn32(uint16_t addr)
IoIn32:
  mov dx, di 
  in eax, dx ; RAXレジスタに設定した値が戻り値になる
  ret

global GetCS ; uint16_t GetCS(void)
GetCS:
  xor eax, eax
  mov ax, cs
  ret 


global LoadIDT ; void LoadIDT(uint16_t limit, uint64_t offset);
LoadIDT:
  push rbp 
  mov rbp, rsp 
  sub rsp, 10 ; スタックポインタ - 10→メモリ領域を10バイト確保
  mov [rsp], di ; limit
  mov [rsp + 2], rsi ; offset 
  lidt [rsp];
  mov rsp, rbp 
  pop rbp
  ret

; エントリポイント
extern kernel_main_stack
extern KernelMainNewStack
global KernelMain
KernelMain:
  mov rsp, kernel_main_stack + 1024 * 1024 ; スタックポインタをkernel_main_stackに設定している
  call KernelMainNewStack
.fin:
  hlt 
  jmp .fin

global LoadGDT ; void LoadGDT(uint16_t limit, uint64_t offset);
LoadGDT:
  push rbp
  mov rbp, rsp
  sub rsp, 10 ; スタックに10バイト分確保する
  mov [rsp], di ; limit
  mov [rsp + 2], rsi
  lgdt [rsp]
  mov rsp, rbp
  pop rbp 
  ret

global SetDSAll ; void SetDSAll(uint16_t value);
SetDSAll:
  mov ds, di
  mov es, di
  mov fs, di 
  mov gs, di
  ret

; https://babyron64.hatenablog.com/entry/2017/12/22/210124
global SetCSSS ; void SetCSSS(uint16_t cs, uint16_t ss);
SetCSSS:
  push rbp
  mov rbp, rsp
  mov ss, si 
  mov rax, .next
  push rdi ; CS 
  push rax ; RIP 
  o64 retf
.next:
  mov rsp, rbp
  pop rbp 
  ret

global SetCR3 ; void SetCR3(uint64_t value)
SetCR3:
  mov cr3, rdi 
  ret


