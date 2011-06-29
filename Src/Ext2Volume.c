#include "Ext2.h"
#include "Ext2FS.h"

EFI_STATUS
EFIAPI
Ext2OpenVolume (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  OUT EFI_FILE_PROTOCOL                **Root
  )
{
return EFI_SUCCESS;
}

EFI_STATUS
Ext2CheckSB (
    IN EFI_DISK_IO_PROTOCOL *DiskIo,
    IN UINT32 MediaId
    )
{
    EFI_STATUS Status;
    struct ext2fs *Ext2SuperBlock = AllocateZeroPool (sizeof (struct ext2fs));

    if (Ext2SuperBlock == NULL) {
      DEBUG((EFI_D_INFO, "Ext2CheckSB: error AllocateZeroPool\n"));
      return EFI_OUT_OF_RESOURCES;
    }
    
    Status = DiskIo->ReadDisk (DiskIo, MediaId, 0, sizeof (struct ext2fs), Ext2SuperBlock);

    if (EFI_ERROR (Status)) {
	DEBUG ((EFI_D_INFO, "Ext2CheckSB: error reading ext2 superblock\n"));
	return Status;
    }
    
    if (Ext2SuperBlock->e2fs_magic != E2FS_MAGIC) {
	DEBUG ((EFI_D_INFO, "Ext2CheckSB: error not ext2 partition\n"));
	return EFI_UNSUPPORTED;
    }

    DEBUG ((EFI_D_INFO, "Ext2CheckSB: Success\n"));
    return EFI_SUCCESS;
}
