/** @file
  Ext2 Driver Reference EDKII Module  

  Copyright (c) 2011, Alin-Florin Rus-Rebreanu <alin.codejunkie@gmail.com>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Ext2.h"

EFI_STATUS EFIAPI
Ext2OpenVolume (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * This,
  OUT EFI_FILE_PROTOCOL ** Root
)
{
  return EFI_SUCCESS;
}

/**
  Checks if the volume contains a valid ext2 partition

  @param  Private[in]       A pointer to the volume to check.              

  @retval EFI_SUCCESS       This volume contains a valid ext2 partition.
  @retval other             This volume does not contain a valid ext2 partition.

**/
EFI_STATUS
Ext2CheckSB (
  IN EFI_DISK_IO_PROTOCOL *DiskIo,
  IN UINT32 MediaId,
  OUT struct ext2fs *e2fs
)
{
  EFI_STATUS      Status;

  Status = DiskIo->ReadDisk (DiskIo, MediaId, 1024, sizeof (struct ext2fs), e2fs);

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ext2CheckSB: error reading ext2 superblock\n"));
    return Status;
  }

  if (e2fs->e2fs_magic != E2FS_MAGIC) {
    DEBUG ((EFI_D_INFO, "Ext2CheckSB: error not ext2 partition\n"));
    return EFI_UNSUPPORTED;
  }

  DEBUG ((EFI_D_INFO, "Ext2CheckSB: Success\n"));
  return EFI_SUCCESS;
}
