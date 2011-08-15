#include "Ext2.h"
#include "Ext2File.h"

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
};

#define struct
int getnewvnode (int a, struct mount *mp, int c, void *p, struct vnode **vpp)
{

  struct vnode *vp;

  vp = AllocateCopyPool (
           sizeof (struct vnode),
           (VOID *) &mExt2File);

  if (vp == NULL) {
    DEBUG ((EFI_D_INFO, "getnewvnode: Alloc error \n"));
    return 1;
  }
  
  vp->Filesystem = &mp->Filesystem;

  if (c == 2) {
    DEBUG ((EFI_D_INFO, "getnewnode: c==2\n"));
    mp->Root = vp;
    vp->v_type = VDIR;
  }
  
  vp->v_type = VREG;
  *vpp = vp;
  return 0;
}
#undef struct

int copyout_vmspace (void *vm, char *cp, void *iov_base, size_t cnt) {

    memcpy(iov_base, cp, cnt);
    return 0;
}


int copyin_vmspace (void *vm, void *iov_base, char *cp, size_t cnt) {

    memcpy(cp, iov_base, cnt);
    return 0;
}

int uiomove(void *buf, size_t n, struct uio *uio)
{

    void *vm = uio->uio_vmspace;
    struct iovec *iov;
    size_t cnt;
    int error = 0;
    char *cp = buf;
    
    while (n > 0 && uio->uio_resid) {
	iov = uio->uio_iov;
	cnt = iov->iov_len;
	if (cnt == 0) {
	    uio->uio_iov++;
	    uio->uio_iovcnt--;
	    continue;
	}
	if (cnt > 0)
	    cnt = n;
	    
	if (uio->uio_rw == UIO_READ) {
	    error = copyout_vmspace (vm, cp, iov->iov_base, cnt);
	} else {
	    error = copyin_vmspace (vm, iov->iov_base, cp, cnt);
	}
	if (error) {
	    break;
	}
	iov->iov_base = (char *)iov->iov_base + cnt;
	iov->iov_len -= cnt;
	uio->uio_resid -= cnt;
	uio->uio_offset += cnt;
	cp += cnt;
	n -= cnt;
    }
    return (error);
}

int ubc_uiomove(void *uobj, struct uio *uio, vsize_t todo,
		int advice, int flags)
{

 return 0;

}