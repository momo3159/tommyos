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