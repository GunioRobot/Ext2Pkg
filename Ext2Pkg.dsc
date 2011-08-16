## @file
#  Ext2 Driver Reference EDKII Module  
#
#  Copyright (c) 2011, Alin-Florin Rus-Rebreanu <alin@softwareliber.ro>
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution. The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#
##

[Defines]
  PLATFORM_NAME                  = Ext2
  PLATFORM_GUID                  = 16692bf7-089d-4a63-993d-e0570d65207a
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  SUPPORTED_ARCHITECTURES        = IA32|X64|IPF|EBC
  OUTPUT_DIRECTORY               = Build/Ext2
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  #
  # Entry Point Libraries
  #
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  #
  # Common Libraries
  #
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf

[Components]
  Ext2Pkg/Src/Ext2.inf

[BuildOptions]
  INTEL:*_*_*_CC_FLAGS          = /D _NETBSD_SOURCE /D _POSIX_SOURCE /D _EXT2_TIANOCORE_SOURCE
   MSFT:*_*_*_CC_FLAGS          = /D _NETBSD_SOURCE /D _POSIX_SOURCE /D _EXT2_TIANOCORE_SOURCE
    GCC:*_*_*_CC_FLAGS          = -D_NETBSD_SOURCE -D_POSIX_SOURCE -D_EXT2_TIANOCORE_SOURCE
