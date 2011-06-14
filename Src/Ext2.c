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

  @param  This                Protocol instance pointer.
  @param  ControllerHandle    Handle of device to test.
  @param  RemainingDevicePath Not used.

  @retval EFI_SUCCESS         This driver supports this device.
  @retval EFI_ALREADY_STARTED This driver is already running on this device.
  @retval other               This driver does not support this device.

**/
EFI_STATUS
EFIAPI
Ext2Supported (
	       IN EFI_DRIVER_BINDING_PROTOCOL  *This,
	       IN EFI_HANDLE                   ControllerHandle,
	       IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
	       )
{
  EFI_STATUS                Status;
  EFI_DISK_IO_PROTOCOL      *DiskIo;

  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
			      ControllerHandle,
			      &gEfiDiskIoProtocolGuid,
			      (VOID **) &DiskIo,
			      This->DriverBindingHandle,
			      ControllerHandle,
			      EFI_OPEN_PROTOCOL_BY_DRIVER
			      );
  if (Status == EFI_ALREADY_STARTED) {
    return EFI_SUCCESS;
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }
  //
  // Close the I/O Abstraction(s) used to perform the supported test
  //
  gBS->CloseProtocol (
		      ControllerHandle,
		      &gEfiDiskIoProtocolGuid,
		      This->DriverBindingHandle,
		      ControllerHandle
		      );
  //
  // Open the IO Abstraction(s) needed to perform the supported test
  //
  Status = gBS->OpenProtocol (
			      ControllerHandle,
			      &gEfiBlockIoProtocolGuid,
			      NULL,
			      This->DriverBindingHandle,
			      ControllerHandle,
			      EFI_OPEN_PROTOCOL_TEST_PROTOCOL
			      );

  return Status;
}


/**
  Start this driver on ControllerHandle by opening a Block IO and Disk IO
  protocol.

  @param  This                 Protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to.
  @param  RemainingDevicePath  Not used.

  @retval EFI_SUCCESS          This driver is added to ControllerHandle.
  @retval EFI_ALREADY_STARTED  This driver is already running on ControllerHandle.
  @retval other                This driver does not support this device.

**/
EFI_STATUS
EFIAPI
Ext2Start (
  IN EFI_DRIVER_BINDING_PROTOCOL  *This,
  IN EFI_HANDLE                   ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
  )
{
  EFI_STATUS                Status;
  EFI_STATUS                OpenStatus;
  EFI_BLOCK_IO_PROTOCOL     *BlockIo;
  EFI_DISK_IO_PROTOCOL      *DiskIo;
  BOOLEAN                   MediaPresent;
  EFI_TPL                   OldTpl;

  OldTpl = gBS->RaiseTPL (TPL_CALLBACK);

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiBlockIoProtocolGuid,
                  (VOID **) &BlockIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_GET_PROTOCOL
                  );
  if (EFI_ERROR (Status)) {
    goto Exit;
  }

  Status = gBS->OpenProtocol (
                  ControllerHandle,
                  &gEfiDiskIoProtocolGuid,
                  (VOID **) &DiskIo,
                  This->DriverBindingHandle,
                  ControllerHandle,
                  EFI_OPEN_PROTOCOL_BY_DRIVER
                  );
 if (EFI_ERROR (Status)) {
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

    // install ext2 handle if supported by the media
    // still need to code

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
  if (EFI_ERROR (Status)          &&
      !EFI_ERROR (OpenStatus)     &&
      Status != EFI_MEDIA_CHANGED &&
      !(MediaPresent && Status == EFI_NO_MEDIA)) {
    gBS->CloseProtocol (
          ControllerHandle,
          &gEfiDiskIoProtocolGuid,
          This->DriverBindingHandle,
          ControllerHandle
          );
  }

Exit:
  gBS->RestoreTPL (OldTpl);
  return Status;
}


/**
  Stop this driver on ControllerHandle.

  @param  This              Protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on.
  @param  NumberOfChildren  Not used.
  @param  ChildHandleBuffer Not used.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle.
  @retval other             This driver was not removed from this device.

**/
EFI_STATUS
EFIAPI
Ext2Stop (
	  IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
	  IN  EFI_HANDLE                    ControllerHandle,
	  IN  UINTN                         NumberOfChildren,
	  IN  EFI_HANDLE                    *ChildHandleBuffer
	  )
{
  EFI_STATUS              Status;
  UINTN                   Index;
  EFI_BLOCK_IO_PROTOCOL   *BlockIo;
  EFI_DISK_IO_PROTOCOL    *DiskIo;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EXT2_DEV *Private;

  Status = gBS->OpenProtocol (
			      ControllerHandle,
			      &gEfiSimpleFileSystemProtocolGuid,
			      (VOID **) &FileSystem,
			      This->DriverBindingHandle,
			      ControllerHandle,
			      EFI_OPEN_PROTOCOL_GET_PROTOCOL
			      );
  if (!EFI_ERROR (Status)) {
    
    Private = EXT2_FILESYSTEM_FROM_DEV (FileSystem);
    BlockIo = EXT2_DEV_FROM_THIS (FileSystem);

    //
    // All Software protocols have be freed from the handle so remove it.
    //
    BlockIo->FlushBlocks (BlockIo);

    Status = gBS->UninstallMultipleProtocolInterfaces (
						       Private->Handle,
						       &gEfiSimpleFileSystemProtocolGuid,
						       &Private->Filesystem,
						       NULL
						       );
    if (!EFI_ERROR (Status)) {
      FreePool (Private); 
    } else {
      return Status;
    }

    Status = gBS->CloseProtocol (
			ControllerHandle,
			&gEfiDiskIoProtocolGuid,
			This->DriverBindingHandle,
			ControllerHandle
			);
    
    return Status;
  
}
  

/**
   The user Entry Point for module Ext2. The user code start with this function.

   @param[in] ImageHandle   The firmware allocated handle for the EFI image.
   @param[in] SystemTable   A pointer to the EFI System Table.

   @retval EFI_SUCCESS      The entry point is executed successfully.
   @retval other            Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
Ext2EntryPoint (
		IN EFI_HANDLE ImageHandle,
		IN EFI_SYSTEM_TABLE *SystemTable
		)
{
  EFI_STATUS Status;

  //
  // Install driver model protocol(s).
  //

  Status = EfiLibInstallDriverBindingComponentName2 (
						     ImageHandle,
						     SystemTable,
						     &gExt2DriverBinding,
						     ImageHandle,
						     &gExt2ComponentName,
						     &gExt2ComponentName2
						     );
  ASSERT_EFI_ERROR (Status);

  return Status;
}


