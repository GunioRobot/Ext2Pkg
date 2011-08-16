#ifndef _EXT2FILE_H_
#define _EXT2FILE_H_

#include <Protocol/SimpleFileSystem.h>
#include "inode.h"
#include "CompatibilityLayer.h"

#define EXT2_FILE_PRIVATE_DATA_SIGNATURE SIGNATURE_32 ('E','x','t','f')

enum vtype      { VNON, VREG, VDIR, VBLK, VCHR, VLNK, VSOCK, VFIFO, VBAD,
               VMARKER };

typedef struct {
  UINTN           Signature;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *Filesystem;
  EFI_FILE_PROTOCOL EfiFile;

  BOOLEAN         IsRootDirectory;

  enum vtype v_type;

  CHAR16           *FileName;

  struct inode *File;
  struct dirent *Dir;

} EXT2_EFI_FILE_PRIVATE;

#define EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS(a) \
	CR (a, \
	EXT2_EFI_FILE_PRIVATE, \
	EfiFile, \
	EXT2_FILE_PRIVATE_DATA_SIGNATURE \
	)

struct iovec {
    void    *iov_base;      /* Base address. */
    size_t   iov_len;       /* Length. */
};

enum    uio_rw { UIO_READ, UIO_WRITE };

struct uio {
    struct iovec *uio_iov;
    int uio_iovcnt;
    off_t uio_offset;
    size_t uio_resid;
    enum uio_rw uio_rw;
    void *uio_vmspace;
};

struct vop_read_args {
    EXT2_EFI_FILE_PRIVATE *a_vp;
    struct uio *a_uio;
    int a_ioflag;
};

struct vop_bmap_args {
    EXT2_EFI_FILE_PRIVATE *a_vp;
    daddr_t a_bn;
    EXT2_EFI_FILE_PRIVATE *a_vpp;
    daddr_t *a_bnp;
    int *a_runp;
};

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

uint64_t ext2fs_size(struct inode *ip);
int ext2fs_setsize(struct inode *ip, uint64_t size);

int
ext2_ubc_uiomove(void *uobj, struct uio *uio, vsize_t todo,
	 int advice, int flags);
int uiomove(void *buf, size_t n, struct uio *uio);
int ext2fs_read(void *v);
int ext2fs_bmap(void *v);
#endif