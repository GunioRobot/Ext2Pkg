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
  Ext2DriverBindingSupported,
  Ext2DriverBingingStart,
  Ext2DriverBindingStop,
  0xa,
  NULL,
  NULL
};


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
