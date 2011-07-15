/** @file
  Ext2 Driver Reference EDKII Module  

  Copyright (c) 2011, Alin-Florin Rus-Rebreanu <alin@softwareliber.ro>

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

int bread (EXT2_DEV *Private, INTN Sblock, INTN SbSize, INTN a, INTN b, buf_t **Buffer)
{
    EFI_DISK_IO_PROTOCOL *DiskIo = Private->DiskIo;
    EFI_BLOCK_IO_PROTOCOL *BlockIo = Private->BlockIo;
    UINT32 MediaId = BlockIo->Media->MediaId;

    EFI_STATUS Status;
    struct buf *TempBuf;

    TempBuf = AllocateZeroPool (sizeof (struct buf));
    TempBuf->b_data = AllocateZeroPool (sizeof (SbSize));

    if (TempBuf->b_data == NULL) {
      DEBUG ((EFI_D_INFO, "bread: error AllocateZeroPool\n"));
      return -1;
    }

    Status = DiskIo->ReadDisk (DiskIo, MediaId, Sblock, SbSize, TempBuf->b_data);

    if (EFI_ERROR (Status)) {
     DEBUG ((EFI_D_INFO, "bread: error reading ext2 superblock\n"));
     return Status;
    }

    *Buffer = TempBuf;
    return EFI_SUCCESS;
}

void brelse (buf_t *Buffer, INTN a)
{
  struct buf *b = Buffer;
  FreePool (b->b_data);
}


