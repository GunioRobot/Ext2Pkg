#include "Ext2.h"
#include "Ext2File.h"
#include <Library/PrintLib.h>
#include <sys/dirent.h>

EXT2_EFI_FILE_PRIVATE mExt2File = {
    EXT2_FILE_PRIVATE_DATA_SIGNATURE,
    NULL,
    {
        EFI_FILE_PROTOCOL_REVISION,
        Ext2SimpleFileSystemOpen,
        Ext2SimpleFileSystemClose,
        Ext2SimpleFileSystemDelete,
        Ext2SimpleFileSystemRead,
        Ext2SimpleFileSystemWrite,
        Ext2SimpleFileSystemGetPosition,
        Ext2SimpleFileSystemSetPosition,
        Ext2SimpleFileSystemGetInfo,
        Ext2SimpleFileSystemSetInfo,
        Ext2SimpleFileSystemFlush
    },
    FALSE,
    VNON,
    NULL,
    NULL,
    NULL,
    0
};

/**
Removes the last directory or file entry in a path by changing the last
L'\' to a CHAR_NULL.

@param[in,out] Path The pointer to the path to modify.

@retval FALSE Nothing was found to remove.
@retval TRUE A directory or file was removed.
**/
BOOLEAN
EFIAPI
PathRemoveLastItem(
    IN OUT CHAR16 *Path
) {
    CHAR16 *Walker;
    CHAR16 *LastSlash;
    //
    // get directory name from path... ('chop' off extra)
    //
    for ( Walker = Path, LastSlash = NULL
                                     ; Walker != NULL && *Walker != CHAR_NULL
            ; Walker++
        ) {
        if (*Walker == L'\\' && *(Walker + 1) != CHAR_NULL) {
            LastSlash = Walker+1;
        }
    }
    if (LastSlash != NULL) {
        *LastSlash = CHAR_NULL;
        return (TRUE);
    }
    return (FALSE);
}

/**
Function to clean up paths.
- Single periods in the path are removed.
- Double periods in the path are removed along with a single parent directory.
- Forward slashes L'/' are converted to backward slashes L'\'.

This will be done inline and the existing buffer may be larger than required
upon completion.

@param[in] Path The pointer to the string containing the path.

@retval NULL An error occured.
@return Path in all other instances.
**/
CHAR16*
EFIAPI
PathCleanUpDirectories(
    IN CHAR16 *Path
) {
    CHAR16 *TempString;
    UINTN TempSize;
    if (Path==NULL) {
        return(NULL);
    }

    //
    // Fix up the '/' vs '\'
    //
    for (TempString = Path ; TempString != NULL && *TempString != CHAR_NULL ; TempString++) {
        if (*TempString == L'/') {
            *TempString = L'\\';
        }
    }

    //
    // Fix up the ..
    //
    while ((TempString = StrStr(Path, L"\\..\\")) != NULL) {
        *TempString = CHAR_NULL;
        TempString += 4;
        PathRemoveLastItem(Path);
        TempSize = StrSize(TempString);
        CopyMem(Path+StrLen(Path), TempString, TempSize);
    }
    if ((TempString = StrStr(Path, L"\\..")) != NULL && *(TempString + 3) == CHAR_NULL) {
        *TempString = CHAR_NULL;
        PathRemoveLastItem(Path);
    }

    //
    // Fix up the .
    //
    while ((TempString = StrStr(Path, L"\\.\\")) != NULL) {
        *TempString = CHAR_NULL;
        TempString += 2;
        TempSize = StrSize(TempString);
        CopyMem(Path+StrLen(Path), TempString, TempSize);
    }
    if ((TempString = StrStr(Path, L"\\.")) != NULL && *(TempString + 2) == CHAR_NULL) {
        *TempString = CHAR_NULL;
    }

    return (Path);
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemOpen (
    IN EFI_FILE_PROTOCOL * This,
    OUT EFI_FILE_PROTOCOL ** NewHandle,
    IN CHAR16 * FileName,
    IN UINT64 OpenMode,
    IN UINT64 Attributes
) {

    EFI_STATUS Status;
    EXT2_EFI_FILE_PRIVATE *PrivateFile, *NewPrivateFile;
    CHAR16 *CleanPath;
    EXT2_DEV *Private;

    Status = EFI_SUCCESS;

    //
    // Check for a valid OpenMode parameter. Since this is a read-only filesystem
    // it must not be EFI_FILE_MODE_WRITE or EFI_FILE_MODE_CREATE. Additionally,
    // ensure that the file name to be accessed isn't empty.
    //
    if ((OpenMode & (EFI_FILE_MODE_READ | EFI_FILE_MODE_CREATE)) != EFI_FILE_MODE_READ) {
        DEBUG ((EFI_D_INFO, "Ext2SimpleFileSystemOpen: OpenMode must be Read\n"));
        Status = EFI_WRITE_PROTECTED;
        goto OpenDone;
    } else if (FileName == NULL || StrCmp (FileName, L"") == 0) {
        DEBUG ((EFI_D_INFO, "Ext2SimpleFileSystemOpen: Missing FileName!\n"));
        Status = EFI_NOT_FOUND;
        goto OpenDone;
    }

    DEBUG ((EFI_D_INFO, "Ext2SimpleFileSystemOpen: Opening: %s\n", FileName));
    PrivateFile = EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);
    CleanPath = PathCleanUpDirectories (FileName);
    DEBUG ((EFI_D_INFO, "Ext2SimpleFileSystemOpen: Path reconstructed as: %s\n", CleanPath));

    DEBUG ((EFI_D_INFO, "Ext2SimpleFileSystemOpen: Relative to: %s\n", PrivateFile->Filename));

    if (StrCmp (CleanPath, L"\\") == 0 ||
            (StrCmp (CleanPath, L".") == 0 && PrivateFile->v_type == VDIR)) {
        DEBUG ((EFI_D_INFO, "Open root\n"));

        Private = EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(PrivateFile->Filesystem);

        DEBUG ((EFI_D_INFO, "open null root\n"));

        Ext2OpenVolume(&Private->Filesystem, NewHandle);

        NewPrivateFile = EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS(*NewHandle);

        NewPrivateFile->Filesystem = PrivateFile->Filesystem;
        NewPrivateFile->v_type = VDIR;
        NewPrivateFile->Filename = AllocateZeroPool(2*sizeof(CHAR16));
        NewPrivateFile->Filename[0] = '\\';
        NewPrivateFile->Filename[1] = CHAR_NULL;
        NewPrivateFile->Position = 0;

        Status = EFI_SUCCESS;
    } else {
        Status = EFI_NOT_FOUND;
        DEBUG ((EFI_D_INFO, "PLM\n"));
    }

    DEBUG ((EFI_D_INFO, "Ext2SimpleFileSystemOpen: End of func\n"));

OpenDone:

    return Status;

}


EFI_STATUS EFIAPI Ext2SimpleFileSystemClose (
    IN EFI_FILE_PROTOCOL * This
) {

    EXT2_EFI_FILE_PRIVATE *PrivateFile;

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemClose: Start of func ***\n"));

    //
    // Grab the associated private data.
    //
    PrivateFile = EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);

    FreePool (PrivateFile);

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemClose: End of func ***\n"));

    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemDelete (
    IN EFI_FILE_PROTOCOL * This
) {

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemDelete: Unsupported ***\n"));
    return EFI_UNSUPPORTED;;
}


EFI_STATUS EFIAPI Ext2SimpleFileSystemRead (
    IN EFI_FILE_PROTOCOL * This,
    IN OUT UINTN * BufferSize,
    OUT VOID * Buffer
) {
    EFI_STATUS Status;

    EXT2_EFI_FILE_PRIVATE *PrivateFile = EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS(This);

    EFI_FILE_INFO *FileInfo;
    CHAR16 *FileName;
    CHAR8 data[1024];
    struct uio uio;
    struct iovec uio_iov;

    Status = EFI_SUCCESS;

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemRead: Unsupported ***\n"));

    if (PrivateFile->v_type == VDIR) {
        DEBUG((EFI_D_INFO, "*** Ext2SimpleFileSystemRead: Called on directory ***\n"));

        if (*BufferSize < SIZE_OF_EFI_FILE_INFO + 255) {
            DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemRead: buffer too small ***\n"));
            *BufferSize = SIZE_OF_EFI_FILE_INFO + 255;
            Status = EFI_BUFFER_TOO_SMALL;
            goto ReadDone;
        }

        DEBUG ((EFI_D_INFO, "FileName %s, Position %d\n", PrivateFile->Filename, PrivateFile->Position));

        struct vop_readdir_args ap;
        int a_eofflag;
        off_t *a_cookies;
        struct dirent *dir;
        void *c;
        UINTN size;

        uio_iov.iov_base = &data;
        c = uio_iov.iov_base;
        uio_iov.iov_len = 1024;
        uio.uio_iov = &uio_iov;
        uio.uio_offset = 0;
        uio.uio_resid = 1024;
        uio.uio_rw = UIO_READ;
        ap.a_uio = &uio;
        ap.a_eofflag = &a_eofflag;
        ap.a_cookies = &a_cookies;
        ap.a_cred = 0;
        ap.a_vp = PrivateFile;

        size = ext2fs_readdir(&ap);

        if (PrivateFile->Position >= (1024 - uio.uio_resid)){
            *BufferSize = 0;
            goto ReadDone;
        }
        dir = (struct dirent *) (c + PrivateFile->Position);

       // Ext2DebugDirent(dir);

        size = SIZE_OF_EFI_FILE_INFO + ((dir->d_namlen + 1) * sizeof(CHAR16));
        FileInfo = AllocateZeroPool (size);

        FileInfo->Size             = size;
        memset(&FileInfo->CreateTime,0, sizeof(EFI_TIME)); //(EFI_TIME)PrivateFile->File->i_din.e2fs_din->e2di_ctime;
        memset(&FileInfo->LastAccessTime,0, sizeof(EFI_TIME)); //(EFI_TIME)PrivateFile->File->i_din.e2fs_din->e2di_atime;
        memset(&FileInfo->ModificationTime,0, sizeof(EFI_TIME));//(EFI_TIME)PrivateFile->File->i_din.e2fs_din->e2di_mtime;
        FileInfo->Attribute        = EFI_FILE_READ_ONLY;

        //
        // Copy in the file name from private data.
        //
        FileName = AllocateZeroPool ((dir->d_namlen + 1) * sizeof(CHAR16));
        AsciiStrToUnicodeStr(dir->d_name, FileName);

        DEBUG ((EFI_D_INFO, "Filename : %s\n", FileName));

        StrCpy (FileInfo->FileName, FileName);

        FreePool (FileName);

        PrivateFile->Position += dir->d_reclen;
        DEBUG ((EFI_D_INFO, "FileName %s, Position %d\n", PrivateFile->Filename, PrivateFile->Position));
        CopyMem (Buffer, FileInfo, size);
        *BufferSize = size;

    } else {
        DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemRead: file ***\n"));

        struct vop_read_args v;

        uio_iov.iov_len = *BufferSize;
        uio_iov.iov_base = Buffer;

        uio.uio_iov = &uio_iov;
        uio.uio_iovcnt = 1;
        uio.uio_offset = PrivateFile->Position;
//    uio.uio_resid = size;
        uio.uio_rw = UIO_READ;

        v.a_vp = PrivateFile;
        v.a_uio = &uio;
        v.a_ioflag = 0;

        ext2fs_read(&v);
        *BufferSize -= uio.uio_resid;

        PrivateFile->Position += *BufferSize;

    }

ReadDone:

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemRead: end ***\n"));
    return EFI_SUCCESS;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemWrite (
    IN EFI_FILE_PROTOCOL * This,
    IN OUT UINTN * BufferSize,
    IN VOID * Buffer
) {

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemWrite: Unsupported ***\n"));
    return EFI_ACCESS_DENIED;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemSetPosition (
    IN EFI_FILE_PROTOCOL * This,
    IN UINT64 Position
) {

    EFI_STATUS Status;

    EXT2_EFI_FILE_PRIVATE *PrivateFile = EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS(This);
    EXT2_DEV *Private = EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(PrivateFile->Filesystem);
    struct m_ext2fs *fs = Private->fs;

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemSetPosition: Start of func ***\n"));
    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemSetPosition: %s, %d\n", PrivateFile->Filename, Position));

    //
    // Check for the invalid condition that This is a directory and the position
    // is non-zero. This has the effect of only allowing directory reads to be
    // restarted.
    //
    if ((PrivateFile->v_type == VDIR) && Position != 0) {
        Status = EFI_UNSUPPORTED;
        goto SetPosDone;
    }

    if (Position == EXT2_MAX_FILE_SIZE(fs->e2fs_bsize)) {
        //
        // Set to the end-of-file position.
        //
        PrivateFile->Position = PrivateFile->File->i_din.e2fs_din->e2di_size * 8;

    } else {
        //
        // Set the position normally.
        //
        PrivateFile->Position = Position;
    }

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemSetPosition: End of func ***\n"));

SetPosDone:

    return Status;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemGetPosition (
    IN EFI_FILE_PROTOCOL * This,
    OUT UINT64 * Position
) {

    EFI_STATUS Status;
    EXT2_EFI_FILE_PRIVATE *PrivateFile;

    Status = EFI_SUCCESS;
    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemGetPosition: Start of func ***\n"));

    //
    // Grab the private data associated with This.
    //
    PrivateFile = EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);
    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemSetPosition: Start of func ***\n"));
    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemSetPosition: %s\n", PrivateFile->Filename));

    //
    // Ensure that this function is not called on a directory.
    //
    if (PrivateFile->v_type == VDIR) {
        Status = EFI_UNSUPPORTED;
        goto GetPosDone;
    }

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemGetPosition: End of func ***\n"));
    *Position = PrivateFile->Position;

GetPosDone:

    return Status;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemGetInfo (
    IN EFI_FILE_PROTOCOL * This,
    IN EFI_GUID * InformationType,
    IN OUT UINTN * BufferSize,
    OUT VOID * Buffer
) {
    EFI_STATUS Status;
    EXT2_EFI_FILE_PRIVATE *PrivateFile;
    EXT2_DEV *Private;
    UINTN DataSize;
    EFI_FILE_INFO *FileInfo;
    EFI_FILE_SYSTEM_INFO *FsInfo;
    CHAR16	*VolumeLabel, *FileName;

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemGetInfo: Start of func ***\n"));

    //
    // Grab the associated private data.
    //
    PrivateFile = EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS (This);
    Private = EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS (PrivateFile->Filesystem);

    //
    // Check InformationType to determine what kind of data to return.
    //
    if (CompareGuid (InformationType, &gEfiFileInfoGuid)) {
        DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemGetInfo: EFI_FILE_INFO request %s ***\n", PrivateFile->Filename));

        //
        // Determine if the size of Buffer is adequate, and if not, break so we can
        // return an error and calculate the needed size;
        //
        DataSize = SIZE_OF_EFI_FILE_INFO + 255;

        if (*BufferSize < DataSize) {
            //
            // Error condition. The buffer size is too small.
            //
            *BufferSize = DataSize;
            Status = EFI_BUFFER_TOO_SMALL;
            DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemGetInfo: EFI_BUFFER_TOO_SMALL ***\n"));
        } else {
            //
            // Allocate and fill out an EFI_FILE_INFO instance for this file.
            //
            FileInfo                   = AllocateZeroPool (DataSize);
            FileInfo->Size             = DataSize;
            memset(&FileInfo->CreateTime,0, sizeof(EFI_TIME)); //(EFI_TIME)PrivateFile->File->i_din.e2fs_din->e2di_ctime;
            memset(&FileInfo->LastAccessTime,0, sizeof(EFI_TIME)); //(EFI_TIME)PrivateFile->File->i_din.e2fs_din->e2di_atime;
            memset(&FileInfo->ModificationTime,0, sizeof(EFI_TIME));//(EFI_TIME)PrivateFile->File->i_din.e2fs_din->e2di_mtime;
            FileInfo->Attribute        = EFI_FILE_READ_ONLY;

            //
            // Copy in the file name from private data.
            //
            FileName = AllocateZeroPool (255);
            UnicodeSPrint (FileName,
                           255,
                           L"%s",
                           PrivateFile->Filename);
            StrCpy (FileInfo->FileName, FileName);
            FreePool (FileName);

            //
            // Set the next params based on whether the file is a directory or not.
            //
            if (PrivateFile->v_type == VDIR) {
                //
                // Calculate size of the directory by summing the filesizes of each
                // file.
                //
                FileInfo->FileSize = PrivateFile->File->i_din.e2fs_din->e2di_size;

                //
                // Update the Attributes field to reflect that this file is also a
                // directory.
                //
                FileInfo->Attribute |= EFI_FILE_DIRECTORY;
            } else {
                FileInfo->FileSize = PrivateFile->File->i_din.e2fs_din->e2di_size;
            }

            //
            // Use the same value for PhysicalSize as FileSize calculated beforehand.
            // PhysicalSize is just an analogue of FileSize.
            //
            FileInfo->PhysicalSize = FileInfo->FileSize;

            //
            // Copy the memory to Buffer, set the output value of BufferSize, and
            // free the temporary data structure.
            //
            CopyMem (Buffer, FileInfo, DataSize);
            *BufferSize = DataSize;
            FreePool (FileInfo);
            Status = EFI_SUCCESS;
        }
    } else if (CompareGuid (InformationType, &gEfiFileSystemInfoGuid)) {
        DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemGetInfo: EFI_FILE_SYSTEM_INFO request ***\n"));

        //
        // Determine if the size of Buffer is adequate, and if not, break so we can
        // return an error and calculate the needed size.
        //
        DataSize = SIZE_OF_EFI_FILE_SYSTEM_INFO + 255;

        if (*BufferSize < DataSize) {
            //
            // Error condition. The buffer passed in is too small to be used by this
            // function. Set the required minimal buffer size and return.
            //
            *BufferSize = DataSize;
            Status = EFI_BUFFER_TOO_SMALL;
        } else {
            //
            // Allocate and fill out an EFI_FILE_INFO instance for this file.
            //
            FsInfo = AllocateZeroPool (DataSize);
            FsInfo->Size = DataSize;
            FsInfo->ReadOnly = TRUE;
            FsInfo->VolumeSize = Private->fs->e2fs_bsize * Private->fs->e2fs.e2fs_bcount;
            FsInfo->FreeSpace = Private->fs->e2fs_bsize * Private->fs->e2fs.e2fs_fbcount;
            FsInfo->BlockSize = Private->fs->e2fs_bsize;

            //
            // Generate the volume name. This is of the format "FV2@0x...", where the
            // location in memory of the Fv2 instance replaces "...".
            //
            VolumeLabel = AllocateZeroPool (255);
            UnicodeSPrint (VolumeLabel,
                           255,
                           L"EXT2%x",
                           &(PrivateFile->Filesystem));
            StrCpy (FsInfo->VolumeLabel, VolumeLabel);
            FreePool (VolumeLabel);

            //
            // Copy the memory to Buffer, set the output value of BufferSize, and
            // free the temporary data structure.
            //
            CopyMem (Buffer, FsInfo, DataSize);
            *BufferSize = DataSize;
            FreePool (FsInfo);
            Status = EFI_SUCCESS;
        }
    } else {
        //
        // Invalid InformationType GUID, return that the call is unsupported.
        //
        DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemGetInfo: Invalid request ***\n"));
        Status = EFI_UNSUPPORTED;
    }

    if (Status == EFI_SUCCESS) {
        DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemGetInfo: EFI_SUCCESS ***\n"));
    } else {
        DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemGetInfo: EFI_WHATEVER ***\n"));
    }

    return Status;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemSetInfo (
    IN EFI_FILE_PROTOCOL * This,
    IN EFI_GUID * InformationType,
    IN UINTN BufferSize,
    IN VOID * Buffer
) {

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemSetInfo: Unsupported ***\n"));
    return EFI_WRITE_PROTECTED;
}

EFI_STATUS EFIAPI Ext2SimpleFileSystemFlush (
    IN EFI_FILE_PROTOCOL * This
) {

    DEBUG ((EFI_D_INFO, "*** Ext2SimpleFileSystemFlush: Unsupported ***\n"));
    return EFI_ACCESS_DENIED;
}

