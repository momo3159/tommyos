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