#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/DiskIo2.h>
#include <Protocol/BlockIo.h>
#include <Guid/FileInfo.h>
#include "elf.hpp"
#include "frame_buffer_config.hpp"

struct MemoryMap {
  UINTN buffer_size; // 格納用バッファの大きさ
  VOID* buffer;
  UINTN map_size; // 実際のメモリマップの大きさ
  UINTN map_key;
  UINTN descriptor_size;
  UINT32 descriptor_version;
};

EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL** root) {
  EFI_STATUS status;
  EFI_LOADED_IMAGE_PROTOCOL* loaded_image; 
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;

  status = gBS->OpenProtocol(
    image_handle, 
    &gEfiLoadedImageProtocolGuid, 
    (VOID**)&loaded_image, 
    image_handle, 
    NULL, 
    EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
  );
  if (EFI_ERROR(status)) return status;

  status = gBS->OpenProtocol(
    loaded_image->DeviceHandle,
    &gEfiSimpleFileSystemProtocolGuid,
    (VOID**)&fs,
    image_handle,
    NULL,
    EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
  );
  if (EFI_ERROR(status)) return status;

  return fs->OpenVolume(fs, root);
}

const CHAR16* GetMemoryTypeUnicode(EFI_MEMORY_TYPE type) {
  switch (type) {
    case EfiReservedMemoryType: return L"EfiReservedMemoryType";
    case EfiLoaderCode: return L"EfiLoaderCode";
    case EfiLoaderData: return L"EfiLoaderData";
    case EfiBootServicesCode: return L"EfiBootServicesCode";
    case EfiBootServicesData: return L"EfiBootServicesData";
    case EfiRuntimeServicesCode: return L"EfiRuntimeServicesCode";
    case EfiRuntimeServicesData: return L"EfiRuntimeServicesData";
    case EfiConventionalMemory: return L"EfiConventionalMemory";
    case EfiUnusableMemory: return L"EfiUnusableMemory";
    case EfiACPIReclaimMemory: return L"EfiACPIReclaimMemory";
    case EfiACPIMemoryNVS: return L"EfiACPIMemoryNVS";
    case EfiMemoryMappedIO: return L"EfiMemoryMappedIO";
    case EfiMemoryMappedIOPortSpace: return L"EfiMemoryMappedIOPortSpace";
    case EfiPalCode: return L"EfiPalCode";
    case EfiPersistentMemory: return L"EfiPersistentMemory";
    case EfiMaxMemoryType: return L"EfiMaxMemoryType";
    default: return L"InvalidMemoryType";
  }
}

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

EFI_STATUS SaveMemoryMap(struct MemoryMap* map, EFI_FILE_PROTOCOL* file) {
  EFI_STATUS status;
  CHAR8 buf[256];
  UINTN len;

  CHAR8* header = "Index, Type, Type(name), PhysicalStart, NumberOfPages, Attribute\n";
  len = AsciiStrLen(header);
  status = file->Write(file, &len, header);
  if (EFI_ERROR(status)) {
    return status;
  }
  Print(L"map->buffer = %08lx, map->map_size = %08lx\n", map->buffer, map->map_size);

  EFI_PHYSICAL_ADDRESS iter;
  int i;
  for (
    iter = (EFI_PHYSICAL_ADDRESS)map->buffer, i = 0;
    iter < (EFI_PHYSICAL_ADDRESS)map->buffer + map->map_size;
    iter += map->descriptor_size, i++
  ) {
    EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)iter;
    
    len = AsciiSPrint(
        buf, 
        sizeof(buf),
        "%u, %x, %-ls, %08lx, %lx, %lx\n",
        i,
        desc->Type,
        GetMemoryTypeUnicode(desc->Type),
        desc->PhysicalStart,
        desc->NumberOfPages,
        desc->Attribute & 0xffffflu
      );
    status = file->Write(file, &len, buf);
    if (EFI_ERROR(status)) {
      return status;
    }
  }
  return EFI_SUCCESS;
}

EFI_STATUS ExitBootServicesWithRetry(EFI_HANDLE image_handle, struct MemoryMap* memmap) {
  EFI_STATUS status;
  status = gBS->ExitBootServices(image_handle, memmap->map_key);
  if(!EFI_ERROR(status)) return status;

  // retry 
  status = GetMemorymap(memmap);
  if (EFI_ERROR(status)) {
    Print(L"failed to get memory map: %r\n", status);
    return status;
  }

  status = gBS->ExitBootServices(image_handle, memmap->map_key);
  return status;
}


EFI_STATUS OpenGOP(EFI_HANDLE image_handle, EFI_GRAPHICS_OUTPUT_PROTOCOL** gop) {
  EFI_STATUS status;
  UINTN num_gop_handles = 0;
  EFI_HANDLE* gop_handles = NULL;

  status = gBS->LocateHandleBuffer(
      ByProtocol,
      &gEfiGraphicsOutputProtocolGuid,
      NULL,
      &num_gop_handles,
      &gop_handles);
  if (EFI_ERROR(status)) return status;

  status = gBS->OpenProtocol(
      gop_handles[0],
      &gEfiGraphicsOutputProtocolGuid,
      (VOID**)gop,
      image_handle,
      NULL,
      EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL);
  if (EFI_ERROR(status)) return status;

  FreePool(gop_handles);
  return EFI_SUCCESS;
}

const CHAR16* GetPixelFormatUnicode(EFI_GRAPHICS_PIXEL_FORMAT fmt) {
  switch (fmt) {
    case PixelRedGreenBlueReserved8BitPerColor:
      return L"PixelRedGreenBlueReserved8BitPerColor";
    case PixelBlueGreenRedReserved8BitPerColor:
      return L"PixelBlueGreenRedReserved8BitPerColor";
    case PixelBitMask:
      return L"PixelBitMask";
    case PixelBltOnly:
      return L"PixelBltOnly";
    case PixelFormatMax:
      return L"PixelFormatMax";
    default:
      return L"InvalidPixelFormat";
  }
}

void CalcLoadAddressRange(Elf64_Ehdr* ehdr, UINT64* first_addr, UINT64* last_addr) {
  *first_addr = MAX_UINT64;
  *last_addr = 0;

  Elf64_Phdr* phdr_table = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);
  for (Elf64_Half i=0;i<ehdr->e_phnum;i++) {
    Elf64_Phdr phdr = phdr_table[i];
    if (phdr.p_type != PT_LOAD) {
      continue;
    }

    *first_addr = MIN(*first_addr, phdr.p_vaddr);
    *last_addr = MAX(*last_addr, phdr.p_vaddr + phdr.p_memsz); 
  }
}

void CopyLoadSegments(Elf64_Ehdr* ehdr) {
  Elf64_Phdr* phdr_table = (Elf64_Phdr*)((UINT64)ehdr + ehdr->e_phoff);
  
  for (Elf64_Half i=0;i<ehdr->e_phnum;i++) {
    Elf64_Phdr phdr = phdr_table[i];
    if (phdr.p_type != PT_LOAD) {
      continue;
    }

    UINT64 segm_in_file = (UINT64)ehdr + phdr.p_offset;
    CopyMem((VOID*)phdr.p_vaddr, (VOID*)segm_in_file, phdr.p_filesz);

    UINTN remain_bytes = phdr.p_memsz - phdr.p_filesz;
    if (remain_bytes > 0) {
      SetMem((VOID*)(phdr.p_vaddr + phdr.p_filesz), remain_bytes, 0);
    }
  }
}

void Halt(void) {
  while (1) __asm__("hlt");
}

void HaltIfError(CHAR16* message, EFI_STATUS status) {
  if (EFI_ERROR(status)) {
    Print(message, status); // TODO: formatをこっちでやるの気持ち悪いので変更したい
    Halt();
  }
}

EFI_STATUS EFIAPI UefiMain(EFI_HANDLE image_handle, EFI_SYSTEM_TABLE* system_table) {
  Print(L"Hello World!\n");
  
  EFI_STATUS status;
  
  CHAR8 memmap_buf[4096 * 4]; // 4ページ分の領域を確保
  struct MemoryMap memmap = {sizeof(memmap_buf), memmap_buf, 0, 0, 0, 0};
  status = GetMemorymap(&memmap);
  HaltIfError(L"failed to get memory map: %r\n", status);
  
  EFI_FILE_PROTOCOL* root_dir;
  status = OpenRootDir(image_handle, &root_dir);
  HaltIfError(L"failed to open root directory: %r\n", status);

  EFI_FILE_PROTOCOL* memmap_file;
  status = root_dir->Open(
    root_dir, &memmap_file, L"\\memmap", 
    EFI_FILE_MODE_CREATE | EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 
    0
  );
  if (EFI_ERROR(status)) {
    Print(L"failed to open file '\\memmap': %r\n", status);
    Print(L"Ignored.\n");
  } else {
    status = SaveMemoryMap(&memmap, memmap_file);
    HaltIfError(L"failed to save memory map: %r\n", status);

    status = memmap_file->Close(memmap_file);
    HaltIfError(L"failed to close memory map: %r\n", status);
  }

  // Graphics Output Protocolの情報取得
  EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
  status = OpenGOP(image_handle, &gop);
  HaltIfError(L"failed to open GOP: %r\n", status);

  struct FrameBufferConfig config = {
    (UINT8*)gop->Mode->FrameBufferBase,
    gop->Mode->Info->PixelsPerScanLine,
    gop->Mode->Info->HorizontalResolution,
    gop->Mode->Info->VerticalResolution,
    0
  };
  switch (gop->Mode->Info->PixelFormat) {
    case PixelBlueGreenRedReserved8BitPerColor:
      config.pixel_format = kPixelBGRResv8BitPerColor;
      break;
    case PixelRedGreenBlueReserved8BitPerColor:
      config.pixel_format = kPixelRGBResv8BitPerColor;
      break;
    default:
      Print(L"Unimplemented pixel format: %d\n", gop->Mode->Info->PixelFormat);
      Halt(); 
  }

  Print(
    L"Resolution: %ux%u, Pixel Format: %s, %u pixels/line\n", 
    gop->Mode->Info->HorizontalResolution,
    gop->Mode->Info->VerticalResolution,
    GetPixelFormatUnicode(gop->Mode->Info->PixelFormat),
    gop->Mode->Info->PixelsPerScanLine
  );
  Print(
    L"Frame Buffer: 0x%0lx = 0x%0lx, Size: %lu bytes\n",
    gop->Mode->FrameBufferBase,
    gop->Mode->FrameBufferBase + gop->Mode->FrameBufferSize,
    gop->Mode->FrameBufferSize
  );


  // カーネルファイルを読み込む  
  EFI_FILE_PROTOCOL* kernel_file;
  status = root_dir->Open(root_dir, &kernel_file, L"\\kernel.elf", EFI_FILE_MODE_READ, 0);
  HaltIfError(L"failed to open file '\\kernel.elf': %r\n", status);

  // カーネルファイルの情報取得
  UINTN file_info_size = sizeof(EFI_FILE_INFO) + sizeof(CHAR16) * 12; // \kernel.elf 分だけ余分に容量を確保
  UINT8 file_info_buffer[file_info_size];
  status = kernel_file->GetInfo(kernel_file, &gEfiFileInfoGuid, &file_info_size, file_info_buffer);
  HaltIfError(L"failed to get file information: %r\n", status);

  EFI_FILE_INFO* file_info = (EFI_FILE_INFO*)file_info_buffer;
  UINTN kernel_file_size = file_info->FileSize;

  // カーネルのファイルサイズ分だけ一時的なメモリ領域を確保
  VOID* kernel_temp_buffer;
  status = gBS->AllocatePool(EfiLoaderData, kernel_file_size, &kernel_temp_buffer);
  HaltIfError(L"failed to allocate pool: %r\n", status);

  status = kernel_file->Read(kernel_file, &kernel_file_size, kernel_temp_buffer);
  HaltIfError(L"error: %r", status);

  // LOADセグメントをメモリに配置するために、アドレスの範囲を計算
  Elf64_Ehdr* kernel_ehdr = (Elf64_Ehdr*)kernel_temp_buffer;
  UINT64 kernel_first_addr, kernel_last_addr;
  CalcLoadAddressRange(kernel_ehdr, &kernel_first_addr, &kernel_last_addr);
  Print(L"Kernel: 0x%0lx - 0x%0lx\n", kernel_first_addr, kernel_last_addr);


  // LOADセグメントをコピー、一時領域を削除
  UINTN num_pages = (kernel_last_addr - kernel_first_addr + 0xfff) / 0x1000;  
  status = gBS->AllocatePages(AllocateAddress, EfiLoaderData, num_pages, &kernel_first_addr);
  HaltIfError(L"failed to allocate pages: %r\n", status);

  CopyLoadSegments(kernel_ehdr);
  Print(L"kernel: 0x%lx - 0x%0lx\n", kernel_first_addr, kernel_last_addr);

  status = gBS->FreePool(kernel_temp_buffer);
  HaltIfError(L"failed to free pool: %r\n", status);

  // ブートサービスを止める
  status = ExitBootServicesWithRetry(image_handle, &memmap);
  HaltIfError(L"failed to exit boot services: %r\n", status);
  
  // ELF形式のファイルにはファイルの先頭から24バイトの位置にエントリーポイントのアドレスが書かれている
  // LOADセグメントの1つはファイルオフセット0から始まっている。これが仕様として定まっているかは不明。
  UINT64 entry_addr = *(UINT64*)(kernel_first_addr + 24); 

  // 関数ポインタ型似キャストして実行 
  typedef void EntryPointType(const struct FrameBufferConfig*);
  EntryPointType* entry_point = (EntryPointType*)entry_addr;
  entry_point(&config);

  Print(L"All done\n");
  Halt();
  return EFI_SUCCESS;
}



