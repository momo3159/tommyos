bits 64
section .text
global IoOut32 ; void IoOut32(uint16_t addr, uint32_t data)
IoOut32:
  mov dx, di 
  mov eax, esi
  out dx, eax
  ret

global IoIn32 ; uint32_t IoIn32(uint16_t addr)

IoIn32:
  mov dx, di 
  in  eax, dx
  ret

global GetCS
GetCS:
  xor eax, eax
  mov ax, cs
  ret

global LoadIDT
LoadIDT:
  ; 定形処理
  push rbp
  mov rbp, rsp
  sub rsp, 10
  ; ここまで 
  mov [rsp], di
  mov [rsp + 2], rsi
  lidt [rsp]
  ; 定形処理
  mov rsp, rbp 
  pop rbp
  ; ここまで
  ret

extern kernel_main_stack
extern KernelMainNewStack

global KernelMain
KernelMain:
  mov rsp, kernel_main_stack + 1024 * 1024 ; スタック切り替え. ※ スタックの始点は末尾から
  call KernelMainNewStack
.fin:
  hlt 
  jmp .fin 
  
global LoadGDT
LoadGDT:
  push rbp
  mov rbp, rsp
  sub rsp, 10
  mov [rsp], di
  mov [rsp + 2], rsi;
  lgdt [rsp]
  mov rsp, rbp
  pop rbp
  ret

global SetDSAll
SetDSAll:
  ; 4つのデータセグメントレジスタに0 (== ヌルディスクリプタ)を設定
  mov ds, di
  mov es, di
  mov fs, di
  mov gs, di
  ret

global SetCSSS
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
  
global SetCR3
SetCR3:
  mov cr3, rdi
  ret