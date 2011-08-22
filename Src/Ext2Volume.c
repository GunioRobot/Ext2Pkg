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
#include "inode.h"

EFI_STATUS EFIAPI
Ext2OpenVolume (
  IN EFI_SIMPLE_FILE_SYSTEM_PROTOCOL * This,
  OUT EFI_FILE_PROTOCOL ** Root
)
{
  EXT2_DEV *Private;
  EXT2_EFI_FILE_PRIVATE *PrivateFile;
//  EFI_FILE_PROTOCOL *fot;
  EXT2_EFI_FILE_PRIVATE *pfile;
 // char buffer[1024];
  //struct m_ext2fs *fs;
  int error;
  int size = 100;
  char *str = NULL;
  
  Private = EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(This);

  DEBUG((EFI_D_INFO, "openvol sb again\n"));
  Ext2DebugSb(Private);
  
  //if (Private->Root == NULL) DEBUG((EFI_D_INFO,"openvol plm root \n"));

  // ext2fs_mountroot(Private);
  PrivateFile = Private->Root;
  DEBUG ((EFI_D_INFO, "openvol before\n"));
  error=ext2fs_vget(Private, 2, &PrivateFile);
  if (error != 0)
    return EFI_UNSUPPORTED;
  
  DEBUG ((EFI_D_INFO, "openvol after\n"));
  Ext2DebugSb (Private);
  Ext2DebugDinode (PrivateFile->File->i_din.e2fs_din);


  error=ext2fs_vget(Private, 34820, &pfile);
  Ext2DebugDinode (pfile->File->i_din.e2fs_din);

  struct vop_read_args v;
  struct uio uio;
  struct iovec uio_iov;
  
  uio_iov.iov_base = AllocateZeroPool (size);
  
  uio_iov.iov_len = size;
  uio.uio_iov = &uio_iov;
  uio.uio_iovcnt = 1;
  uio.uio_offset = 0;
  uio.uio_resid = size;
  uio.uio_rw = UIO_READ;
  
  v.a_vp = pfile;
  v.a_uio = &uio;
  v.a_ioflag = 0;
  error = ext2fs_read(&v);
  str = uio_iov.iov_base;
  DEBUG ((EFI_D_INFO, "\n Bytes left unread, filesize < than uio_resid : %d\n",uio.uio_resid));
  DEBUG ((EFI_D_INFO, "\n File content: \n"));
  Ext2DebugCharBuffer(uio_iov.iov_base, 100);

  FreePool (uio_iov.iov_base);

  struct vop_readdir_args ap;
  int a_eofflag;
  off_t *a_cookies;
  ap.a_vp = PrivateFile;
  void *c;

  uio_iov.iov_base = AllocateZeroPool (2048);
  c = uio_iov.iov_base;
  uio_iov.iov_len = 2048;
  uio.uio_iov = &uio_iov;
  uio.uio_offset = 0;
  uio.uio_resid = 2048;
  uio.uio_rw = UIO_READ;
  ap.a_uio = &uio;
  ap.a_eofflag = &a_eofflag;
  ap.a_cookies = &a_cookies;
  ap.a_cred = 0;

  error = ext2fs_readdir(&ap);
  str = uio_iov.iov_base;
  DEBUG ((EFI_D_INFO, "\n Bytes left unread, filesize < than uio_resid : %d\n",uio.uio_resid));
  DEBUG ((EFI_D_INFO, "\n File content: \n"));
  Ext2DebugCharBuffer(uio_iov.iov_base, 2048);
  Ext2DebugCharBuffer(c, 2048);
 
  return EFI_SUCCESS;
}

#define struct
int bread (struct vnode *devvp, INTN Sblock, INTN SbSize, INTN a, INTN b, OUT buf_t **Buffer)
#undef struct
{
    EXT2_DEV *Private = EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(devvp->Filesystem);
    EFI_DISK_IO_PROTOCOL *DiskIo = Private->DiskIo;
    EFI_BLOCK_IO_PROTOCOL *BlockIo = Private->BlockIo;
    UINT32 MediaId = BlockIo->Media->MediaId;
    VOID *data = NULL;

    EFI_STATUS Status;

    *Buffer = AllocateZeroPool (sizeof (struct buf));
    data = AllocateZeroPool (SbSize);
    
    DEBUG ((EFI_D_INFO, "bread cred = %d\n", a));
    
    if (a == 0) {
    	Status = DiskIo->ReadDisk (DiskIo, MediaId, Sblock*512, SbSize, data);
    } else {
	int error = 0;
        daddr_t a_bnp;
	
	struct vop_bmap_args v;
	
	v.a_vp = devvp;
	v.a_bn = Sblock;
	v.a_bnp = &a_bnp;
	v.a_runp = NULL;
	
	error = ext2fs_bmap(&v);
	
	DEBUG((EFI_D_INFO, "bread a_bnp %d\n", a_bnp));
	
	if (!error){
	    Status = DiskIo->ReadDisk (DiskIo, MediaId, a_bnp*512, SbSize, data);
	} else {
	    DEBUG ((EFI_D_INFO, "bread: shit bmap failed\n"));
	    return error;
	}
    }

    (*Buffer)->b_data = AllocateCopyPool (SbSize, data);
    FreePool (data);
    
    if (EFI_ERROR (Status)) {
     (*Buffer)->b_resid = 1; //whateva'
     DEBUG ((EFI_D_INFO, "bread: error reading ext2 blocks\n"));
     return Status;
    }

    (*Buffer)->b_resid = 0;
    return EFI_SUCCESS;
}

void brelse (IN buf_t *Buffer, INTN a)
{
  FreePool (Buffer->b_data);
  FreePool (Buffer);
}

int
#define struct
breadn(struct vnode *vp, daddr_t blkno, INTN size, daddr_t rablks[],
#undef struct
         int rasizes[], int nrablks, INTN a, INTN flags,
         buf_t **bpp)
{
    return bread(vp, blkno, size, a, 0, bpp);
}

void
#define struct
VOP_STRATEGY(struct vnode *vp,
#undef struct
	 struct buf *bp)
 {

    EXT2_DEV *Private = EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(vp->Filesystem);
    EFI_DISK_IO_PROTOCOL *DiskIo = Private->DiskIo;
    EFI_BLOCK_IO_PROTOCOL *BlockIo = Private->BlockIo;
    UINT32 MediaId = BlockIo->Media->MediaId;
    
    VOID *data = AllocateZeroPool(Private->fs->e2fs_bsize);
    
    EFI_STATUS Status;

    if (bp->b_flags & B_READ) {
	Status = DiskIo->ReadDisk (DiskIo, MediaId, bp->b_blkno*512, Private->fs->e2fs_bsize, data);
	if (EFI_ERROR (Status)) {
	    DEBUG ((EFI_D_INFO, "ReadDisk failed VOP_STRATEGY\n"));
	    bp->b_resid = -1;
	}
	
	bp->b_data = data;
    }
    bp->b_resid = 0;
    DEBUG ((EFI_D_INFO, "ReadDisk success VOP_STRATEGY\n"));
}

struct buf* getblk (
#define struct
		struct vnode *vp, daddr_t metalbn, int crap, int a, int b)
#undef struct
{
    struct buf *buf;

    buf = AllocateZeroPool(sizeof(struct buf));

    buf->b_oflags = 0;
    
    return buf;
}