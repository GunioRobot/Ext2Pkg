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

#ifndef _EXT2_H_
#define _EXT2_H_

#include <Uefi.h>
#include <Protocol/BlockIo.h>
#include <Protocol/BlockIo2.h>
#include <Guid/Gpt.h>
#include <Protocol/ComponentName.h>
#include <Protocol/DevicePath.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/DiskIo.h>
#include <Library/DebugLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/DevicePathLib.h>

#define EXT2_PRIVATE_DATA_SIGNATURE SIGNATURE_32 ('E','x','t','2')

typedef struct {
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL Filesystem;

  UINT32 Signature;

  EFI_HANDLE Handle;

  EFI_DISK_IO_PROTOCOL DiskIo;
  EFI_BLOCK_IO_PROTOCOL BlockIo;

  EFI_UNICODE_STRING_TABLE *ControllerNameTable;

} EXT2_DEV;

#define EXT2_DEV_FROM_THIS(a) CR (a, EXT2_DEV, BlockIo, EXT2_PRIVATE_DATA_SIGNATURE)
#define EXT2_DEV_FROM_FILESYSTEM(a) CR (a, EXT2_DEV, Filesystem, EXT2_PRIVATE_DATA_SIGNATURE)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gExt2DriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gExt2ComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gPartitionComponentName2;

//


//
// Function Prototypes
//
/**
  Test to see if this driver supports ControllerHandle. Any ControllerHandle
  than contains a BlockIo and DiskIo protocol can be supported.

  @param  This                Protocol instance pointer.
  @param  ControllerHandle    Handle of device to test
  @param  RemainingDevicePath Optional parameter use to pick a specific child
  device to start.

  @retval EFI_SUCCESS         This driver supports this device
  @retval EFI_ALREADY_STARTED This driver is already running on this device
  @retval other               This driver does not support this device

**/
EFI_STATUS
EFIAPI
Ext2DriverBindingSupported (
			    IN EFI_DRIVER_BINDING_PROTOCOL  *This,
			    IN EFI_HANDLE                   ControllerHandle,
			    IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
			    );

/**
  Start this driver on ControllerHandle by opening a Block IO and Disk IO
  protocol, reading Device Path, and creating a child handle with a
  Disk IO and device path protocol.

  @param  This                 Protocol instance pointer.
  @param  ControllerHandle     Handle of device to bind driver to
  @param  RemainingDevicePath  Optional parameter use to pick a specific child
  device to start.

  @retval EFI_SUCCESS          This driver is added to ControllerHandle
  @retval EFI_ALREADY_STARTED  This driver is already running on ControllerHandle
  @retval other                This driver does not support this device

**/
EFI_STATUS
EFIAPI
Ext2DriverBindingStart (
			IN EFI_DRIVER_BINDING_PROTOCOL  *This,
			IN EFI_HANDLE                   ControllerHandle,
			IN EFI_DEVICE_PATH_PROTOCOL     *RemainingDevicePath
			);

/**
  Stop this driver on ControllerHandle. Support stopping any child handles
  created by this driver.

  @param  This              Protocol instance pointer.
  @param  ControllerHandle  Handle of device to stop driver on
  @param  NumberOfChildren  Number of Handles in ChildHandleBuffer. If number of
  children is zero stop the entire bus driver.
  @param  ChildHandleBuffer List of Child Handles to Stop.

  @retval EFI_SUCCESS       This driver is removed ControllerHandle
  @retval other             This driver was not removed from this device

**/
EFI_STATUS
EFIAPI
Ext2DriverBindingStop (
		      IN  EFI_DRIVER_BINDING_PROTOCOL   *This,
		      IN  EFI_HANDLE                    ControllerHandle,
		      IN  UINTN                         NumberOfChildren,
		      IN  EFI_HANDLE                    *ChildHandleBuffer
		      );

//
// EFI Component Name Functions
//
/**
  Retrieves a Unicode string that is the user readable name of the driver.

  This function retrieves the user readable name of a driver in the form of a
  Unicode string. If the driver specified by This has a user readable name in
  the language specified by Language, then a pointer to the driver name is
  returned in DriverName, and EFI_SUCCESS is returned. If the driver specified
  by This does not support the language specified by Language,
  then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
  EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
  array indicating the language. This is the
  language of the driver name that the caller is
  requesting, and it must match one of the
  languages specified in SupportedLanguages. The
  number of languages supported by a driver is up
  to the driver writer. Language is specified
  in RFC 4646 or ISO 639-2 language code format.

  @param  DriverName[out]       A pointer to the Unicode string to return.
  This Unicode string is the name of the
  driver specified by This in the language
  specified by Language.

  @retval EFI_SUCCESS           The Unicode string for the Driver specified by
  This and the language specified by Language was
  returned in DriverName.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER DriverName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
  the language specified by Language.
**/
EFI_STATUS
EFIAPI
Ext2ComponentNameGetDriverName (
				IN  EFI_COMPONENT_NAME_PROTOCOL  *This,
				IN  CHAR8                        *Language,
				OUT CHAR16                       **DriverName
				);


/**
  Retrieves a Unicode string that is the user readable name of the controller
  that is being managed by a driver.

  This function retrieves the user readable name of the controller specified by
  ControllerHandle and ChildHandle in the form of a Unicode string. If the
  driver specified by This has a user readable name in the language specified by
  Language, then a pointer to the controller name is returned in ControllerName,
  and EFI_SUCCESS is returned.  If the driver specified by This is not currently
  managing the controller specified by ControllerHandle and ChildHandle,
  then EFI_UNSUPPORTED is returned.  If the driver specified by This does not
  support the language specified by Language, then EFI_UNSUPPORTED is returned.

  @param  This[in]              A pointer to the EFI_COMPONENT_NAME2_PROTOCOL or
  EFI_COMPONENT_NAME_PROTOCOL instance.

  @param  ControllerHandle[in]  The handle of a controller that the driver
  specified by This is managing.  This handle
  specifies the controller whose name is to be
  returned.

  @param  ChildHandle[in]       The handle of the child controller to retrieve
  the name of.  This is an optional parameter that
  may be NULL.  It will be NULL for device
  drivers.  It will also be NULL for a bus drivers
  that wish to retrieve the name of the bus
  controller.  It will not be NULL for a bus
  driver that wishes to retrieve the name of a
  child controller.

  @param  Language[in]          A pointer to a Null-terminated ASCII string
  array indicating the language.  This is the
  language of the driver name that the caller is
  requesting, and it must match one of the
  languages specified in SupportedLanguages. The
  number of languages supported by a driver is up
  to the driver writer. Language is specified in
  RFC 4646 or ISO 639-2 language code format.

  @param  ControllerName[out]   A pointer to the Unicode string to return.
  This Unicode string is the name of the
  controller specified by ControllerHandle and
  ChildHandle in the language specified by
  Language from the point of view of the driver
  specified by This.

  @retval EFI_SUCCESS           The Unicode string for the user readable name in
  the language specified by Language for the
  driver specified by This was returned in
  DriverName.

  @retval EFI_INVALID_PARAMETER ControllerHandle is not a valid EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER ChildHandle is not NULL and it is not a valid
  EFI_HANDLE.

  @retval EFI_INVALID_PARAMETER Language is NULL.

  @retval EFI_INVALID_PARAMETER ControllerName is NULL.

  @retval EFI_UNSUPPORTED       The driver specified by This is not currently
  managing the controller specified by
  ControllerHandle and ChildHandle.

  @retval EFI_UNSUPPORTED       The driver specified by This does not support
  the language specified by Language.

**/
EFI_STATUS
EFIAPI
Ext2ComponentNameGetControllerName (
				    IN  EFI_COMPONENT_NAME_PROTOCOL                     *This,
				    IN  EFI_HANDLE                                      ControllerHandle,
				    IN  EFI_HANDLE                                      ChildHandle        OPTIONAL,
				    IN  CHAR8                                           *Language,
				    OUT CHAR16                                          **ControllerName
				    );


