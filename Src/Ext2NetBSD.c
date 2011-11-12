#include "Ext2.h"
#include "Ext2File.h"
#include "ext2fs_dinode.h"

EXT2_EFI_FILE_PRIVATE mExt2File_t = {
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

#define struct
int getnewvnode (int a, struct mount *mp, int c, void *p, struct vnode **vpp)
{

  struct vnode *vp;

  vp = AllocateCopyPool (
           sizeof (struct vnode),
           (VOID *) &mExt2File_t);

  if (vp == NULL) {
    DEBUG ((EFI_D_INFO, "getnewvnode: Alloc error \n"));
    return 1;
  }
  
  vp->Filesystem = &mp->Filesystem;

  if (c == 2) {
    DEBUG ((EFI_D_INFO, "getnewnode: c==2\n"));
    mp->Root = vp;
    vp->v_type = VDIR;
    *vpp = vp;
    return 0;
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

int ext2_ubc_uiomove(void *uobj, struct uio *uio, vsize_t todo,
		int advice, int flags)
{
    EXT2_EFI_FILE_PRIVATE *vp = uobj;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *sfs = vp->Filesystem;
    EXT2_DEV *ext_fs = EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(sfs);

    int error;
    struct buf *bp;
    daddr_t lbn, nextlbn;
    long size, xfersize, blkoffset;
    struct m_ext2fs *fs = ext_fs->fs;
    struct inode *ip = vp->File;
    char *p = uio->uio_iov->iov_base;

    size = fs->e2fs_bsize;

    for (error = 0, bp = NULL; todo > 0; bp = NULL) {

	lbn = lblkno(fs, uio->uio_offset);
	nextlbn = lbn + 1;
	blkoffset = blkoff(fs, uio->uio_offset);
	xfersize = fs->e2fs_bsize - blkoffset;

	if (todo < xfersize)
	    xfersize = todo;

	if (lblktosize(fs, nextlbn) >= ext2fs_size(ip))
	    error = bread(vp, lbn, size, 1, 0, &bp);
	else {
    	    int nextsize = fs->e2fs_bsize;
	    error = breadn(vp, lbn, size, &nextlbn, &nextsize, 1, 1, 0, &bp);
	}
	if (error)
	    return error;

	memcpy(p, (char *)bp->b_data + blkoffset, xfersize);
	p += xfersize;
	todo -= xfersize;
	uio->uio_resid -= xfersize;
	uio->uio_offset += xfersize;
	brelse(bp, 0);
    }

    return 0;
}

int ffs(int i)
{
    int count = 1;
    if (i == 0) return 0;

    while ((i & 1) != 1) {
	i>>=1;
	count++;
    }
    return count;
}

/*
 * Create an array of logical block number/offset pairs which represent the
 * path of indirect blocks required to access a data block.  The first "pair"
 * contains the logical block number of the appropriate single, double or
 * triple indirect block and the offset into the inode indirect block array.
 * Note, the logical block number of the inode single/double/triple indirect
 * block appears twice in the array, once with the offset into the i_ffs1_ib and
 * once with the offset into the page itself.
 */
#define struct
int
ufs_getlbns(struct vnode *vp,
#undef struct
	 daddr_t bn, struct indir *ap, int *nump)
{
        daddr_t metalbn, realbn;
        int64_t blockcnt;
        int lbc;
        int i, numlevels, off;
        EXT2_DEV *mp = EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(vp->Filesystem);
	struct m_ext2fs *m_fs = mp->fs;
        if (nump)
                *nump = 0;
        numlevels = 0;
        realbn = bn;
        if (bn < 0)
                bn = -bn;

        /*
         * Determine the number of levels of indirection.  After this loop
         * is done, blockcnt indicates the number of data blocks possible
         * at the given level of indirection, and NIADDR - i is the number
         * of levels of indirection needed to locate the requested block.
         */

        bn -= NDADDR;
        for (lbc = 0, i = NIADDR;; i--, bn -= blockcnt) {
                if (i == 0)
                        return (EFBIG);

                lbc += ffs(NINDIR(m_fs)) - 1;
                blockcnt = (int64_t)1 << lbc;

                if (bn < blockcnt)
                        break;
        }

        /* Calculate the address of the first meta-block. */
        metalbn = -((realbn >= 0 ? realbn : -realbn) - bn + NIADDR - i);

        /*
         * At each iteration, off is the offset into the bap array which is
         * an array of disk addresses at the current level of indirection.
         * The logical block number and the offset in that block are stored
         * into the argument array.
         */
        ap->in_lbn = metalbn;
        ap->in_off = off = NIADDR - i;
        ap->in_exists = 0;
        ap++;
        for (++numlevels; i <= NIADDR; i++) {
                /* If searching for a meta-data block, quit when found. */
                if (metalbn == realbn)
                        break;

                lbc -= ffs(NINDIR(m_fs)) - 1;
                off = (bn >> lbc) & (MNINDIR(ump) - 1);

                ++numlevels;
                ap->in_lbn = metalbn;
                ap->in_off = off;
                ap->in_exists = 0;
                ++ap;

                metalbn -= -1 + ((int64_t)off << lbc);
        }
        if (nump)
                *nump = numlevels;
        return (0);
}

int
#define struct
VOP_READ (struct vnode *vp,
#undef struct
	struct uio *uio, int ioflag, int b)
{
    int error;
    struct vop_read_args a;
    a.a_vp = vp;
    a.a_uio = uio;
    a.a_ioflag = ioflag;

    error = ext2fs_read(&a);

    return error;

}

int
#define struct
vn_rdwr(enum uio_rw rw, struct vnode *vp, void *base, int len, off_t offset,
    int segflg, int ioflg, kauth_cred_t vcred, size_t *aresid,
    void *l)
#undef struct
{
        struct uio auio;
        struct iovec aiov;
        int error = 0;

        auio.uio_iov = &aiov;
        auio.uio_iovcnt = 1;
        aiov.iov_base = base;
        aiov.iov_len = len;
        auio.uio_resid = len;
        auio.uio_offset = offset;
        auio.uio_rw = rw;

        if (rw == UIO_READ) {
                error = VOP_READ(vp, &auio, ioflg, cred);
//        } else {
//                error = VOP_WRITE(vp, &auio, ioflg, cred);
        }

        if (aresid)
                *aresid = auio.uio_resid;
        else
                if (auio.uio_resid && error == 0)
                        error = EIO;

        return (error);
}

