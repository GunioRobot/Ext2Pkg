/** @file
  Ext2 Driver Reference EDKII Module  

  Based on MdeModulePkg/Universal/Disk/PartitionDxe/

  Copyright (c) 2011, Alin-Florin Rus-Rebreanu <alin@softwareliber.ro>

  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution. The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Ext2.h"

//
// Driver binding protocol implementation for Ext2 driver.
//

EFI_DRIVER_BINDING_PROTOCOL gExt2DriverBinding = {
  Ext2Supported,
  Ext2Start,
  Ext2Stop,
  0xa,
  NULL,
  NULL
};


/**
  Test to see if this driver supports ControllerHandle. Any ControllerHandle
  than contains a BlockIo and DiskIo protocol can be supported.

  @param[in]  This                Protocol instance pointer.
  @param[in]  ControllerHandle    Handle of device to test.
  @param[in]  RemainingDevicePath Not used.

  @retval EFI_SUCCESS         This driver supports this device.
  @retval EFI_ALREADY_STARTED This driver is already running on this device.
  @retval other               This driver does not support this device.

**/
EFI_STATUS EFIAPI
Ext2Supported (
  IN EFI_DRIVER_BINDING_PROTOCOL * This,
  IN EFI_HANDLE ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL * RemainingDevicePath
)
{
  EFI_STATUS      Status;
  EFI_DISK_IO_PROTOCOL *DiskIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (ControllerHandle,
			      &gEfiDiskIoProtocolGuid,
			      (VOID **) & DiskIo,
			      This->DriverBindingHandle,
			      ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER);

  if (Status == EFI_ALREADY_STARTED) {
    DEBUG ((EFI_D_INFO, "Ext2Supported: EFI_ALREADY_STARTED\n"));
    return EFI_SUCCESS;
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ext2Supported: Error open protocol DiskIo\n"));
    DEBUG ((EFI_D_ERROR, "Ext2Supported: Error %r\n", Status));
    return Status;
  }
  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (ControllerHandle,
		      &gEfiDiskIoProtocolGuid,
		      This->DriverBindingHandle, ControllerHandle);
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (ControllerHandle,
			      &gEfiBlockIoProtocolGuid,
			      NULL,
			      This->DriverBindingHandle,
			      ControllerHandle,
			      EFI_OPEN_PROTOCOL_TEST_PROTOCOL);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ext2Supported: Error open protocol BlockIo\n"));
    return Status;
  }

  DEBUG ((EFI_D_INFO, "Ext2Supported\n"));
  return EFI_SUCCESS;
}


/**
  Start this driver on ControllerHandle by opening a Block IO and Disk IO
  protocol.

  @param[in]  This                 Protocol instance pointer.
  @param[in]  ControllerHandle     Handle of device to bind driver to.
  @param[in]  RemainingDevicePath  Not used.

  @retval EFI_SUCCESS          This driver is added to ControllerHandle.
  @retval EFI_ALREADY_STARTED  This driver is already running on ControllerHandle.
  @retval other                This driver does not support this device.

**/
EFI_STATUS EFIAPI
Ext2Start (
  IN EFI_DRIVER_BINDING_PROTOCOL * This,
  IN EFI_HANDLE ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL * RemainingDevicePath
)
{
  EFI_STATUS      Status;
  EFI_STATUS      OpenStatus;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;
  EFI_DISK_IO_PROTOCOL *DiskIo;
  BOOLEAN         MediaPresent;
  EFI_TPL         OldTpl;
  EXT2_DEV       *Private;

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  Status = gBS->OpenProtocol (ControllerHandle,
			      &gEfiBlockIoProtocolGuid,
			      (VOID **) & BlockIo,
			      This->DriverBindingHandle,
			      ControllerHandle, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ext2Start: Error open protocol BlockIo\n"));
    goto Exit;
  }

  Status = gBS->OpenProtocol (ControllerHandle,
			      &gEfiDiskIoProtocolGuid,
			      (VOID **) & DiskIo,
			      This->DriverBindingHandle,
			      ControllerHandle, EFI_OPEN_PROTOCOL_BY_DRIVER);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ext2Start: Error open protocol BlockIo\n"));
    goto Exit;
  }

  OpenStatus = Status;

  //
  // Try to read blocks when there's media or it is removable physical partition.
  //
  // Status       = EFI_UNSUPPORTED;

  Status = EFI_SUCCESS;

  MediaPresent = BlockIo->Media->MediaPresent;
  if (BlockIo->Media->MediaPresent ||
      (BlockIo->Media->RemovableMedia && !BlockIo->Media->LogicalPartition)) {

    Private = AllocateZeroPool (sizeof (EXT2_DEV));
    if (Private == NULL) {
      DEBUG ((EFI_D_INFO, "Ext2Start: error AllocateZeroPool\n"));
      return EFI_OUT_OF_RESOURCES;
    }	

    Private->BlockIo = BlockIo;
    Private->DiskIo = DiskIo;

    ext2fs_mountroot (Private);

    Private->Signature = EXT2_PRIVATE_DATA_SIGNATURE;
    Private->Filesystem.Revision = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
    Private->Filesystem.OpenVolume = Ext2OpenVolume;

    Status = gBS->InstallMultipleProtocolInterfaces (
			&Private->Handle,
			&gEfiSimpleFileSystemProtocolGuid,
			&Private->FileSystem,	
			NULL
			);

  }
  //
  // In the case that the driver is already started (OpenStatus == EFI_ALREADY_STARTED),
  // the DiskIoProtocol are not actually opened by the driver. So don't try to close it.
  // Otherwise, we will break the dependency between the controller and the driver set 
  // up before.
  //
  // In the case that when the media changes on a device it will Reinstall the 
  // BlockIo interaface. This will cause a call to our Stop(), and a subsequent
  // reentrant call to our Start() successfully. We should leave the device open
  // when this happen. The "media change" case includes either the status is
  // EFI_MEDIA_CHANGED or it is a "media" to "no media" change. 
  //  
  if (EFI_ERROR (Status) &&
      !EFI_ERROR (OpenStatus) &&
      Status != EFI_MEDIA_CHANGED &&
      !(MediaPresent && Status == EFI_NO_MEDIA)) {

    DEBUG ((EFI_D_INFO, "Ext2Start: Error EFI_MEDIA_CHANGED\n"));

    gBS->CloseProtocol (ControllerHandle,
			&gEfiDiskIoProtocolGuid,
			This->DriverBindingHandle, ControllerHandle);
  }

Exit:
  gBS->RestoreTPL (OldTpl);
  DEBUG ((EFI_D_ERROR, "Ext2Start: %r\n", Status));
  return Status;
}


/**
  Stop this driver on ControllerHandle.

  @param[in]  This              Protocol instance pointer.
  @param[in]  ControllerHandle  Handle of device to stop driver on.
  @param[in]  NumberOfChildren  Not used.
  @param[in]  ChildHandleBuffer Not used.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle.
  @retval other             This driver was not removed from this device.

**/
EFI_STATUS EFIAPI
Ext2Stop (
  IN EFI_DRIVER_BINDING_PROTOCOL * This,
  IN EFI_HANDLE ControllerHandle,
  IN UINTN NumberOfChildren,
  IN EFI_HANDLE * ChildHandleBuffer
)
{
  EFI_STATUS      Status;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EXT2_DEV       *Private;

  BlockIo = NULL;
  Private = NULL;

  //
  // Get our context back
  //
  Status = gBS->OpenProtocol (ControllerHandle,
			      &gEfiSimpleFileSystemProtocolGuid,
			      (VOID **) & FileSystem,
			      This->DriverBindingHandle,
			      ControllerHandle, EFI_OPEN_PROTOCOL_GET_PROTOCOL);
  if (!EFI_ERROR (Status)) {

    DEBUG ((EFI_D_INFO, "Ext2Stop: Error open protocol SimpleFileSystem\n"));

    Private = EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS (FileSystem);
    BlockIo = Private->BlockIo;

    //
    // All Software protocols have be freed from the handle so remove it.
    //
    BlockIo->FlushBlocks (BlockIo);

    Status = gBS->UninstallMultipleProtocolInterfaces (Private->Handle,
						       &gEfiSimpleFileSystemProtocolGuid,
						       &Private->Filesystem,
						       NULL);
    if (!EFI_ERROR (Status)) {

      DEBUG ((EFI_D_INFO,
	      "Ext2Stop: Error uninstall multiple protocol interfaces\n"));

      Status = gBS->CloseProtocol (ControllerHandle,
				   &gEfiBlockIoProtocolGuid,
				   This->DriverBindingHandle, ControllerHandle);
      if (!EFI_ERROR (Status)) {

	DEBUG ((EFI_D_INFO, "Ext2Stop: Error close protocol BlockIo\n"));

	Status = gBS->CloseProtocol (ControllerHandle,
				     &gEfiDiskIoProtocolGuid,
				     This->DriverBindingHandle,
				     ControllerHandle);
      }

      FreePool (Private);
    }
  }

  DEBUG ((EFI_D_ERROR, "Ext2Stop: %r\n", Status));

  return Status;
}

/**
   The user Entry Point for module Ext2. The user code start with this function.

   @param[in] ImageHandle   The firmware allocated handle for the EFI image.
   @param[in] SystemTable   A pointer to the EFI System Table.

   @retval EFI_SUCCESS      The entry point is executed successfully.
   @retval other            Some error occurs when executing this entry point.

**/
EFI_STATUS EFIAPI
Ext2EntryPoint (
  IN EFI_HANDLE ImageHandle,
  IN EFI_SYSTEM_TABLE * SystemTable
)
{
  EFI_STATUS      Status;

  //
  // Install driver model protocol(s).
  //

  Status = EfiLibInstallDriverBindingComponentName2 (ImageHandle,
						     SystemTable,
						     &gExt2DriverBinding,
						     ImageHandle,
						     &gExt2ComponentName,
						     &gExt2ComponentName2);
  ASSERT_EFI_ERROR (Status);

  return Status;
}
