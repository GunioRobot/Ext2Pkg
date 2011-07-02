## @file
#  Ext2 Driver Reference EDKII Module  
#
#  Copyright (c) 2011, Alin-Florin Rus-Rebreanu <alin.codejunkie@gmail.com>
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
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.infx
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

  #
  # C Standard Libraries
  #
  LibC|StdLib/LibC/LibC.inf
  LibStdLib|StdLib/LibC/StdLib/StdLib.inf
  LibString|StdLib/LibC/String/String.inf
  LibWchar|StdLib/LibC/Wchar/Wchar.inf
  LibCType|StdLib/LibC/Ctype/Ctype.inf
  LibTime|StdLib/LibC/Time/Time.inf
  LibStdio|StdLib/LibC/Stdio/Stdio.inf
  LibGdtoa|StdLib/LibC/gdtoa/gdtoa.inf
  LibLocale|StdLib/LibC/Locale/Locale.inf
  LibUefi|StdLib/LibC/Uefi/Uefi.inf
  LibMath|StdLib/LibC/Math/Math.inf
  LibSignal|StdLib/LibC/Signal/Signal.inf
  LibNetUtil|StdLib/LibC/NetUtil/NetUtil.inf

[Components]
  MdePkg/Library/BaseLib/BaseLib.inf {
    <BuildOptions>
      MSFT:*_*_*_CC_FLAGS    = /X /Zc:wchar_t /GL-
  }

  MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf {
    <BuildOptions>
      MSFT:*_*_*_CC_FLAGS    = /X /Zc:wchar_t /GL-
  }

  Ext2Pkg/Src/Ext2.inf

[BuildOptions]
  INTEL:*_*_*_CC_FLAGS      = /Qfreestanding
   MSFT:*_*_*_CC_FLAGS      = /X /Zc:wchar_t
    GCC:*_*_*_CC_FLAGS      = -ffreestanding -nostdinc -nostdlib

