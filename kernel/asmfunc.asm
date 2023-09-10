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