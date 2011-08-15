#include "Ext2.h"

EFI_STATUS EFIAPI Ext2SimpleFileSystemOpen (
  IN EFI_FILE_PROTOCOL * This,
  OUT EFI_FILE_PROTOCOL ** NewHandle,
  IN CHAR16 * FileName,
  IN UINT64 OpenMode,
  IN UINT64 Attributes
){

return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemClose (
  IN EFI_FILE_PROTOCOL * This
){

return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemDelete (
  IN EFI_FILE_PROTOCOL * This
){

return EFI_SUCCESS;
}


EFI_STATUS EFIAPI Ext2SimpleFileSystemRead (
  IN EFI_FILE_PROTOCOL * This,
  IN OUT UINTN * BufferSize,
  OUT VOID * Buffer
){

return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemWrite (
  IN EFI_FILE_PROTOCOL * This,
  IN OUT UINTN * BufferSize,
  IN VOID * Buffer
){

return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemSetPosition (
  IN EFI_FILE_PROTOCOL * This,
  IN UINT64 Position
){

return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemGetPosition (
  IN EFI_FILE_PROTOCOL * This,
  OUT UINT64 * Position
){

return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemGetInfo (
  IN EFI_FILE_PROTOCOL * This,
  IN EFI_GUID * InformationType,
  IN OUT UINTN * BufferSize,
  OUT VOID * Buffer
){

return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemSetInfo (
  IN EFI_FILE_PROTOCOL * This,
  IN EFI_GUID * InformationType,
  IN UINTN BufferSize,
  IN VOID * Buffer
){

return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemFlush (
  IN EFI_FILE_PROTOCOL * This
){

return EFI_SUCCESS;
}
