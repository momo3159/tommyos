#include <Uefi.h>
#include <Library/UefiLib.h>

EFI_STATUS EFIAPI UefiMain(
  EFI_HANDLE image_handle,
  EFI_SYSTEM_TABLE *system_table
) {
  Print(L"Hello, TommyOS World!\n");
  
  while (1); // NOTE: return で関数から抜けるとqemuが終了する（？）
  return EFI_SUCCESS;
}