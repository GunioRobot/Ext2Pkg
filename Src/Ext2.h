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
#include <Protocol/SimpleFileSystem.h>
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
#include <Guid/FileSystemInfo.h>
#include <Guid/FileInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>

#include "ext2fs.h"

#define EXT2_PRIVATE_DATA_SIGNATURE SIGNATURE_32 ('E','x','t','2')

typedef struct {
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL Filesystem;

  UINT32          Signature;

  EFI_HANDLE      Handle;

  EFI_DISK_IO_PROTOCOL *DiskIo;
  EFI_BLOCK_IO_PROTOCOL *BlockIo;

  EFI_UNICODE_STRING_TABLE *ControllerNameTable;

  struct m_ext2fs fs;

} EXT2_DEV;

#define EXT2_DEV_FROM_THIS(a) CR (a, EXT2_DEV, BlockIo, EXT2_PRIVATE_DATA_SIGNATURE)
#define EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(a) \
	CR (a, \
	EXT2_DEV, \
	Filesystem, \
	EXT2_PRIVATE_DATA_SIGNATURE\
	)

#define EXT2_FILE_PRIVATE_DATA_SIGNATURE SIGNATURE_32 ('E','x','t','f')

typedef struct {
  UINTN           Signature;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Filesystem;
  EFI_FILE_PROTOCOL EfiFile;
  INTN            fd;
//  DIR            *Dir;
  BOOLEAN         IsRootDirectory;
  BOOLEAN         IsDirectoryPath;
  BOOLEAN         IsOpenedByRead;
  char           *FileName;
  struct dirent  *Dirent;
} EXT2_EFI_FILE_PRIVATE;

#define EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS(a) \
	CR (a, \
	EXT2_EFI_FILE_PRIVATE, \
	EfiFile, \
	EXT2_FILE_PRIVATE_DATA_SIGNATURE SIGNATURE_32 \
	)

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL gExt2DriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL gExt2ComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL gExt2ComponentName2;

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
EFI_STATUS EFIAPI Ext2Supported (
  IN EFI_DRIVER_BINDING_PROTOCOL * This,
  IN EFI_HANDLE ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL * RemainingDevicePath
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
EFI_STATUS EFIAPI Ext2Start (
  IN EFI_DRIVER_BINDING_PROTOCOL * This,
  IN EFI_HANDLE ControllerHandle,
  IN EFI_DEVICE_PATH_PROTOCOL * RemainingDevicePath
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
EFI_STATUS EFIAPI Ext2Stop (
  IN EFI_DRIVER_BINDING_PROTOCOL * This,
  IN EFI_HANDLE ControllerHandle,
  IN UINTN NumberOfChildren,
  IN EFI_HANDLE * ChildHandleBuffer
);

/**
  Open the root directory on a volume.

  @param  This              A pointer to the volume to open.
  @param  Root              A pointer to storage for the returned opened file handle of the root directory.

  @retval EFI_SUCCESS           The volume was opened.
  @retval EFI_UNSUPPORTED       The volume does not support the requested file system type.
  @retval EFI_NO_MEDIA          The device has no media.
  @retval EFI_DEVICE_ERROR      The device reported an error.
  @retval EFI_VOLUME_CORRUPTED  The file system structures are corrupted.
  @retval EFI_ACCESS_DENIED     The service denied access to the file.
  @retval EFI_OUT_OF_RESOURCES  The file volume could not be opened due to lack of resources.
  @retval EFI_MEDIA_CHANGED     The device has new media or the media is no longer supported.
**/

EFI_STATUS EFIAPI Ext2OpenVolume (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * This,
  OUT EFI_FILE_PROTOCOL ** File
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
EFI_STATUS EFIAPI Ext2ComponentNameGetDriverName (
  IN EFI_COMPONENT_NAME_PROTOCOL * This,
  IN CHAR8 * Language,
  OUT CHAR16 ** DriverName
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
EFI_STATUS EFIAPI Ext2ComponentNameGetControllerName (
  IN EFI_COMPONENT_NAME_PROTOCOL * This,
  IN EFI_HANDLE ControllerHandle,
  IN EFI_HANDLE ChildHandle OPTIONAL,
  IN CHAR8 * Language,
  OUT CHAR16 ** ControllerName
);


/**
  Checks if the volume contains a valid ext2 partition

  @param  Private[in]       A pointer to the volume to check.              

  @retval EFI_SUCCESS       This volume contains a valid ext2 partition.
  @retval other             This volume does not contain a valid ext2 partition.

**/
EFI_STATUS      Ext2CheckSB (
  IN EFI_DISK_IO_PROTOCOL * DiskIo,
  IN UINT32 MediaId,
  OUT struct ext2fs *e2fs
);


/**
  Open a file relative to the source file location.

  @param  This        A pointer to the source file location.
  @param  NewHandle   Pointer to storage for the new file handle.
  @param  FileName    Pointer to the file name to be opened.
  @param  OpenMode    File open mode information.
  @param  Attributes  File creation attributes.

  @retval  EFI_SUCCESS           The file was opened.
  @retval  EFI_NOT_FOUND         The file could not be found in the volume.
  @retval  EFI_NO_MEDIA          The device has no media.
  @retval  EFI_MEDIA_CHANGED     The device has new media or the media is no longer supported.
  @retval  EFI_DEVICE_ERROR      The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED  The file system structures are corrupted.
  @retval  EFI_WRITE_PROTECTED   The volume or file is write protected.
  @retval  EFI_ACCESS_DENIED     The service denied access to the file.
  @retval  EFI_OUT_OF_RESOURCES  Not enough resources were available to open the file.
  @retval  EFI_VOLUME_FULL       There is not enough space left to create the new file.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemOpen (
  IN EFI_FILE_PROTOCOL * This,
  OUT EFI_FILE_PROTOCOL ** NewHandle,
  IN CHAR16 * FileName,
  IN UINT64 OpenMode,
  IN UINT64 Attributes
);

/**
  Close the specified file handle.

  @param  This  Pointer to a returned opened file handle.

  @retval EFI_SUCCESS   The file handle has been closed.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemClose (
  IN EFI_FILE_PROTOCOL * This
);

/**
  Close and delete a file.

  @param  This  Pointer to a returned opened file handle.

  @retval EFI_SUCCESS             The file handle was closed and deleted.
  @retval EFI_WARN_DELETE_FAILURE The handle was closed but could not be deleted.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemDelete (
  IN EFI_FILE_PROTOCOL * This
);

/**
  Read data from a file.

  @param  This        Pointer to a returned open file handle.
  @param  BufferSize  On input, the size of the Buffer.  On output, the number of bytes stored in the Buffer.
  @param  Buffer      Pointer to the first byte of the read Buffer.

  @retval  EFI_SUCCESS           The data was read.
  @retval  EFI_NO_MEDIA          The device has no media.
  @retval  EFI_DEVICE_ERROR      The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED  The file system structures are corrupted.
  @retval  EFI_BUFFER_TOO_SMALL  The supplied buffer size was too small to store the current directory entry.
                          *BufferSize has been updated with the size needed to complete the request.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemRead (
  IN EFI_FILE_PROTOCOL * This,
  IN OUT UINTN * BufferSize,
  OUT VOID * Buffer
);

/**
  Write data to a file.

  @param  This        Pointer to an opened file handle.
  @param  BufferSize  On input, the number of bytes in the Buffer to write to the file.  On output, the number of bytes
                of data written to the file.
  @param  Buffer      Pointer to the first by of data in the buffer to write to the file.

  @retval  EFI_SUCCESS           The data was written to the file.
  @retval  EFI_UNSUPPORTED       Writes to an open directory are not supported.
  @retval  EFI_NO_MEDIA          The device has no media.
  @retval  EFI_DEVICE_ERROR      The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED  The file system structures are corrupt.
  @retval  EFI_WRITE_PROTECTED   The file, directory, volume, or device is write protected.
  @retval  EFI_ACCESS_DENIED     The file was opened read-only.
  @retval  EFI_VOLUME_FULL       The volume is full.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemWrite (
  IN EFI_FILE_PROTOCOL * This,
  IN OUT UINTN * BufferSize,
  IN VOID * Buffer
);

/**
  Set a file's current position.

  @param  This      Pointer to an opened file handle.
  @param  Position  The byte position from the start of the file to set.

  @retval  EFI_SUCCESS     The file position has been changed.
  @retval  EFI_UNSUPPORTED The seek request for non-zero is not supported for directories.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemSetPosition (
  IN EFI_FILE_PROTOCOL * This,
  IN UINT64 Position
);

/**
  Get a file's current position.

  @param  This      Pointer to an opened file handle.
  @param  Position  Pointer to storage for the current position.

  @retval  EFI_SUCCESS     The file position has been reported.
  @retval  EFI_UNSUPPORTED Not valid for directories.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemGetPosition (
  IN EFI_FILE_PROTOCOL * This,
  OUT UINT64 * Position
);

/**
  Return information about a file or volume.

  @param  This            Pointer to an opened file handle.
  @param  InformationType GUID describing the type of information to be returned.
  @param  BufferSize      On input, the size of the information buffer.  On output, the number of bytes written to the
                          information buffer.
  @param  Buffer          Pointer to the first byte of the information buffer.

  @retval  EFI_SUCCESS           The requested information has been written into the buffer.
  @retval  EFI_UNSUPPORTED       The InformationType is not known.
  @retval  EFI_NO_MEDIA          The device has no media.
  @retval  EFI_DEVICE_ERROR      The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED  The file system structures are corrupt.
  @retval  EFI_BUFFER_TOO_SMALL  The buffer size was too small to contain the requested information.  The buffer size has
                                 been updated with the size needed to complete the requested operation.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemGetInfo (
  IN EFI_FILE_PROTOCOL * This,
  IN EFI_GUID * InformationType,
  IN OUT UINTN * BufferSize,
  OUT VOID * Buffer
);

/**
  Set information about a file or volume.

  @param  This            Pointer to an opened file handle.
  @param  InformationType GUID identifying the type of information to set.
  @param  BufferSize      Number of bytes of data in the information buffer.
  @param  Buffer          Pointer to the first byte of data in the information buffer.

  @retval  EFI_SUCCESS           The file or volume information has been updated.
  @retval  EFI_UNSUPPORTED       The information identifier is not recognised.
  @retval  EFI_NO_MEDIA          The device has no media.
  @retval  EFI_DEVICE_ERROR      The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED  The file system structures are corrupt.
  @retval  EFI_WRITE_PROTECTED   The file, directory, volume, or device is write protected.
  @retval  EFI_ACCESS_DENIED     The file was opened read-only.
  @retval  EFI_VOLUME_FULL       The volume is full.
  @retval  EFI_BAD_BUFFER_SIZE   The buffer size is smaller than the type indicated by InformationType.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemSetInfo (
  IN EFI_FILE_PROTOCOL * This,
  IN EFI_GUID * InformationType,
  IN UINTN BufferSize,
  IN VOID * Buffer
);

/**
  Flush all modified data to the media.

  @param  This  - Pointer to an opened file handle.

  @retval  EFI_SUCCESS           The data has been flushed.
  @retval  EFI_NO_MEDIA          The device has no media.
  @retval  EFI_DEVICE_ERROR      The device reported an error.
  @retval  EFI_VOLUME_CORRUPTED  The file system structures have been corrupted.
  @retval  EFI_WRITE_PROTECTED   The file, directory, volume, or device is write protected.
  @retval  EFI_ACCESS_DENIED     The file was opened read-only.
  @retval  EFI_VOLUME_FULL       The volume is full.
**/
EFI_STATUS EFIAPI Ext2SimpleFileSystemFlush (
  IN EFI_FILE_PROTOCOL * This
);
#endif
