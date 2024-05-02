#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>

struct MemoryMap {
  UINTN buffer_size; // 格納用バッファの大きさ
  VOID* buffer;
  UINTN map_size; // 実際のメモリマップの大きさ
  UINTN map_key;
  UINTN descriptor_size;
  UINTN descriptor_version;
};

EFI_STATUS GetMemorymap(struct MemoryMap* map) {
  if (map->buffer == NULL) {
    return EFI_BUFFER_TOO_SMALL;
  }

  map->map_size = map->buffer_size; 
  return gBS->GetMemoryMap(
    &map->map_size,
    (EFI_MEMORY_DESCRIPTOR*)map->buffer,
    &map->map_key,
    &map->descriptor_size,
    &map->descriptor_version
  );
}


EFI_STATUS EFIAPI UefiMain(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
  Print(L"Hello World!\n");

  CHAR8 memmap_buf[4096 * 4]; // 4ページ分の領域を確保
  struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
  GetMemorymap(&memmap);

  
  while (1);
  return EFI_SUCCESS;
}
