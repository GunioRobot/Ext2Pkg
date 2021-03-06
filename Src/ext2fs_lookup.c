/*	$NetBSD: ext2fs_lookup.c,v 1.63 2010/11/30 10:43:06 dholland Exp $	*/

/*
 * Modified for NetBSD 1.2E
 * May 1997, Manuel Bouyer
 * Laboratoire d'informatique de Paris VI
 */
/*
 *  modified for Lites 1.1
 *
 *  Aug 1995, Godmar Back (gback@cs.utah.edu)
 *  University of Utah, Department of Computer Science
 */
/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)ufs_lookup.c	8.6 (Berkeley) 4/1/94
 */

#ifndef _EXT2_TIANOCORE_SOURCE
#include <sys/cdefs.h>
__KERNEL_RCSID(0, "$NetBSD: ext2fs_lookup.c,v 1.63 2010/11/30 10:43:06 dholland Exp $");

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/namei.h>
#include <sys/buf.h>
#include <sys/file.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/malloc.h>
#include <sys/dirent.h>
#include <sys/kauth.h>
#include <sys/proc.h>

#include <ufs/ufs/inode.h>
#include <ufs/ufs/ufsmount.h>
#include <ufs/ufs/ufs_extern.h>

#include <ufs/ext2fs/ext2fs_extern.h>
#include <ufs/ext2fs/ext2fs_dir.h>
#include <ufs/ext2fs/ext2fs.h>
#else
#include "Ext2.h"
#include "Ext2File.h"
#include "CompatibilityLayer.h"
#include "inode.h"
#include "ext2fs_dir.h"
#include <sys/dirent.h>
#include "ext2fs_dinode.h"
#endif

int dirchk = 1;

static void	ext2fs_dirconv2ffs(struct ext2fs_direct *e2dir,
					  struct dirent *ffsdir);

#define struct
static int	ext2fs_dirbadentry(struct vnode *dp,
#undef struct
					  struct ext2fs_direct *de,
					  int entryoffsetinblock);

/*
 * the problem that is tackled below is the fact that FFS
 * includes the terminating zero on disk while EXT2FS doesn't
 * this implies that we need to introduce some padding.
 * For instance, a filename "sbin" has normally a reclen 12
 * in EXT2, but 16 in FFS.
 * This reminds me of that Pepsi commercial: 'Kid saved a lousy nine cents...'
 * If it wasn't for that, the complete ufs code for directories would
 * have worked w/o changes (except for the difference in DIRBLKSIZ)
 */
static void
ext2fs_dirconv2ffs(struct ext2fs_direct *e2dir, struct dirent *ffsdir)
{
	memset(ffsdir, 0, sizeof(struct dirent));
	ffsdir->d_fileno = fs2h32(e2dir->e2d_ino);
	ffsdir->d_namlen = e2dir->e2d_namlen;

	ffsdir->d_type = DT_UNKNOWN;		/* don't know more here */
#ifdef DIAGNOSTIC
#if MAXNAMLEN < E2FS_MAXNAMLEN
	/*
	 * we should handle this more gracefully !
	 */
	if (e2dir->e2d_namlen > MAXNAMLEN)
		panic("ext2fs: e2dir->e2d_namlen");
#endif
#endif
	strncpy(ffsdir->d_name, e2dir->e2d_name, ffsdir->d_namlen);

	/* Godmar thinks: since e2dir->e2d_reclen can be big and means
	   nothing anyway, we compute our own reclen according to what
	   we think is right
	 */
	ffsdir->d_reclen = _DIRENT_SIZE(ffsdir);
}

/*
 * Vnode op for reading directories.
 *
 * Convert the on-disk entries to <sys/dirent.h> entries.
 * the problem is that the conversion will blow up some entries by four bytes,
 * so it can't be done in place. This is too bad. Right now the conversion is
 * done entry by entry, the converted entry is sent via uiomove.
 *
 * XXX allocate a buffer, convert as many entries as possible, then send
 * the whole buffer to uiomove
 */
int
ext2fs_readdir(void *v)
{
	struct vop_readdir_args /* {
		struct vnode *a_vp;
		struct uio *a_uio;
		kauth_cred_t a_cred;
		int **a_eofflag;
		off_t **a_cookies;
		int ncookies;
	} */ *ap = v;
	struct uio *uio = ap->a_uio;
	int error;
	size_t e2fs_count, readcnt;
#define struct
	struct vnode *vp = ap->a_vp;
#undef struct
	struct m_ext2fs *fs = VTOI(vp)->i_e2fs;

	struct ext2fs_direct *dp;
	struct dirent *dstd;
	struct uio auio;
	struct iovec aiov;
	void *dirbuf;
	off_t off = uio->uio_offset;
	off_t *cookies = NULL;
	int nc = 0, ncookies = 0;
	int e2d_reclen;

	if (vp->v_type != VDIR)
		return (ENOTDIR);

	e2fs_count = uio->uio_resid;
	/* Make sure we don't return partial entries. */
	e2fs_count -= (uio->uio_offset + e2fs_count) & (fs->e2fs_bsize -1);
	if (e2fs_count <= 0)
		return (EINVAL);
	
	auio = *uio;
	auio.uio_iov = &aiov;
	auio.uio_iovcnt = 1;
	aiov.iov_len = e2fs_count;
	auio.uio_resid = e2fs_count;
	UIO_SETUP_SYSSPACE(&auio);
	dirbuf = malloc(e2fs_count, M_TEMP, M_WAITOK);
	dstd = malloc(sizeof(struct dirent), M_TEMP, M_WAITOK | M_ZERO);
	if (ap->a_ncookies) {
		nc = e2fs_count / _DIRENT_MINSIZE((struct dirent *)0);
		ncookies = nc;
		cookies = malloc(sizeof (off_t) * ncookies, M_TEMP, M_WAITOK);
		*ap->a_cookies = cookies;
	}
	memset(dirbuf, 0, e2fs_count);
	aiov.iov_base = dirbuf;

	error = VOP_READ(ap->a_vp, &auio, 0, ap->a_cred);
	
	if (error == 0) {
		readcnt = e2fs_count - auio.uio_resid;
		
		for (dp = (struct ext2fs_direct *)dirbuf;
			(char *)dp < (char *)dirbuf + readcnt; ) {
			e2d_reclen = fs2h16(dp->e2d_reclen);
			if (e2d_reclen == 0) {
				error = EIO;
				break;
			}
			ext2fs_dirconv2ffs(dp, dstd);
			if(dstd->d_reclen > uio->uio_resid) {
				break;
			}

			error = uiomove(dstd, dstd->d_reclen, uio);
			if (error != 0) {
				break;
			}
			off = off + e2d_reclen;
			
			if (cookies != NULL) {
				*cookies++ = off;
				if (--ncookies <= 0){
					break;  /* out of cookies */
				}
			}
			/* advance dp */
			dp = (struct ext2fs_direct *) ((char *)dp + e2d_reclen);
		}
		/* we need to correct uio_offset */
		uio->uio_offset = off;
	}
	free(dirbuf, M_TEMP);
	free(dstd, M_TEMP);
	*ap->a_eofflag = ext2fs_size(VTOI(ap->a_vp)) <= uio->uio_offset;
	if (ap->a_ncookies) {
		if (error) {
			free(*ap->a_cookies, M_TEMP);
			*ap->a_ncookies = 0;
			*ap->a_cookies = NULL;
		} else
			*ap->a_ncookies = nc - ncookies;
	}
	
	return (error);
}

/*
 * Convert a component of a pathname into a pointer to a locked inode.
 * This is a very central and rather complicated routine.
 * If the file system is not maintained in a strict tree hierarchy,
 * this can result in a deadlock situation (see comments in code below).
 *
 * The cnp->cn_nameiop argument is LOOKUP, CREATE, RENAME, or DELETE depending
 * on whether the name is to be looked up, created, renamed, or deleted.
 * When CREATE, RENAME, or DELETE is specified, information usable in
 * creating, renaming, or deleting a directory entry may be calculated.
 * If flag has LOCKPARENT or'ed into it and the target of the pathname
 * exists, lookup returns both the target and its parent directory locked.
 * When creating or renaming and LOCKPARENT is specified, the target may
 * not be ".".  When deleting and LOCKPARENT is specified, the target may
 * be "."., but the caller must check to ensure it does an vrele and vput
 * instead of two vputs.
 *
 * Overall outline of ext2fs_lookup:
 *
 *      check accessibility of directory
 *      look for name in cache, if found, then if at end of path
 *        and deleting or creating, drop it, else return name
 *      search for name in directory, to found or notfound
 * notfound:
 *      if creating, return locked directory, leaving info on available slots
 *      else return error
 * found:
 *      if at end of path and deleting, return information to allow delete
 *      if at end of path and rewriting (RENAME and LOCKPARENT), lock target
 *        inode and return info to allow rewrite
 *      if not at end, add name to cache; if at end and neither creating
 *        nor deleting, add name to cache
 */
int
ext2fs_lookup(void *v)
{
        struct vop_lookup_args /* {
                struct vnode *a_dvp;
                struct vnode **a_vpp;
                struct componentname *a_cnp;
        } */ *ap = v;
#define struct
        struct vnode *vdp = ap->a_dvp;  /* vnode for directory being searched */
#undef struct
        struct inode *dp = VTOI(vdp);   /* inode for directory being searched */
        struct buf *bp;                 /* a buffer of directory entries */
        struct ext2fs_direct *ep;       /* the current directory entry */
        int entryoffsetinblock;         /* offset of ep in bp's buffer */
        enum {NONE, COMPACT, FOUND} slotstatus;
        doff_t slotoffset;              /* offset of area with free space */
        int slotsize;                   /* size of area at slotoffset */
        int slotfreespace;              /* amount of space free in slot */
        int slotneeded;                 /* size of the entry we're seeking */
        int numdirpasses;               /* strategy for directory search */
        doff_t endsearch;               /* offset to end directory search */
        doff_t prevoff;                 /* prev entry dp->i_offset */
#define struct
        struct vnode *pdp;              /* saved dp during symlink work */
        struct vnode *tdp;              /* returned by VFS_VGET */
#undef struct
        doff_t enduseful;               /* pointer past last used dir slot */
        u_long bmask;                   /* block offset mask */
        int namlen, error;
#define struct
        struct vnode **vpp = ap->a_vpp;
#undef struct
        struct componentname *cnp = ap->a_cnp;
//        kauth_cred_t cred = cnp->cn_cred;
        int flags;
        int nameiop = cnp->cn_nameiop;
//        struct ufsmount *ump = dp->i_ump;
//        int dirblksiz = ump->um_dirblksiz;
	int dirblksiz = (EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(vdp->Filesystem))->fs->e2fs_bsize;
        ino_t foundino;

        flags = cnp->cn_flags;

        bp = NULL;
        slotoffset = -1;
        *vpp = NULL;

        /*
         * Check accessiblity of directory.
         */
        if ((error = VOP_ACCESS(vdp, VEXEC, cred)) != 0)
                return (error);

//        if ((flags & ISLASTCN) && (vdp->v_mount->mnt_flag & MNT_RDONLY) &&
//            (cnp->cn_nameiop == DELETE || cnp->cn_nameiop == RENAME))
//                return (EROFS);

        /*
         * We now have a segment name to search for, and a directory to search.
         *
         * Before tediously performing a linear scan of the directory,
         * check the name cache to see if the directory/name pair
         * we are looking for is known already.
         */
        if ((error = cache_lookup(vdp, vpp, cnp)) >= 0)
                return (error);

        /*
         * Suppress search for slots unless creating
         * file and at end of pathname, in which case
         * we watch for a place to put the new file in
         * case it doesn't already exist.
         */
        slotstatus = FOUND;
        slotfreespace = slotsize = slotneeded = 0;
/*        if ((nameiop == CREATE || nameiop == RENAME) &&
            (flags & ISLASTCN)) {
                slotstatus = NONE;
                slotneeded = EXT2FS_DIRSIZ(cnp->cn_namelen);
        }
*/
        /*
         * If there is cached information on a previous search of
         * this directory, pick up where we last left off.
         * We cache only lookups as these are the most common
         * and have the greatest payoff. Caching CREATE has little
         * benefit as it usually must search the entire directory
         * to determine that the entry does not exist. Caching the
         * location of the last DELETE or RENAME has not reduced
         * profiling time and hence has been removed in the interest
         * of simplicity.
         */
        // !!!!!!!!!!!!!!!!!!!!!!! needs fix !!!!!!!!!!!!!!!!!
        bmask = 1023;
        if (nameiop != LOOKUP || dp->i_diroff == 0 ||
            dp->i_diroff >= ext2fs_size(dp)) {
                entryoffsetinblock = 0;
                dp->i_offset = 0;
                numdirpasses = 1;
        } else {
                dp->i_offset = dp->i_diroff;
                if ((entryoffsetinblock = dp->i_offset & bmask) &&
                    (error = ext2fs_blkatoff(vdp, (off_t)dp->i_offset, NULL, &bp)))
                        return (error);
                numdirpasses = 2;
//                nchstats.ncs_2passes++;
        }
        prevoff = dp->i_offset;
        endsearch = roundup(ext2fs_size(dp), dirblksiz);
        enduseful = 0;

searchloop:
        while (dp->i_offset < endsearch) {
//                if (curcpu()->ci_schedstate.spc_flags & SPCF_SHOULDYIELD)
//                        preempt();
                /*
                 * If necessary, get the next directory block.
                 */
                if ((dp->i_offset & bmask) == 0) {
                        if (bp != NULL)
                                brelse(bp, 0);
                        error = ext2fs_blkatoff(vdp, (off_t)dp->i_offset, NULL,
                            &bp);
                        if (error != 0)
                                return (error);
                        entryoffsetinblock = 0;
                }
                /*
                 * If still looking for a slot, and at a dirblksize
                 * boundary, have to start looking for free space again.
                 */
                if (slotstatus == NONE &&
                    (entryoffsetinblock & (dirblksiz - 1)) == 0) {
                        slotoffset = -1;
                        slotfreespace = 0;
                }
                /*
                 * Get pointer to next entry.
                 * Full validation checks are slow, so we only check
                 * enough to insure forward progress through the
                 * directory. Complete checks can be run by patching
                 * "dirchk" to be true.
                 */
//                KASSERT(bp != NULL);
                ep = (struct ext2fs_direct *)
                        ((char *)bp->b_data + entryoffsetinblock);
                if (ep->e2d_reclen == 0 ||
                    (dirchk &&
                     ext2fs_dirbadentry(vdp, ep, entryoffsetinblock))) {
                        int i;
                        ufs_dirbad(dp, dp->i_offset, "mangled entry");
                        i = dirblksiz - (entryoffsetinblock & (dirblksiz - 1));
                        dp->i_offset += i;
                        entryoffsetinblock += i;
                        continue;
                }

                /*
                 * If an appropriate sized slot has not yet been found,
                 * check to see if one is available. Also accumulate space
                 * in the current block so that we can determine if
                 * compaction is viable.
                 */
                if (slotstatus != FOUND) {
                        int size = fs2h16(ep->e2d_reclen);
                        if (ep->e2d_ino != 0)
                                size -= EXT2FS_DIRSIZ(ep->e2d_namlen);
                        if (size > 0) {
                                if (size >= slotneeded) {
                                      slotstatus = FOUND;
                                        slotoffset = dp->i_offset;
                                        slotsize = fs2h16(ep->e2d_reclen);
                                } else if (slotstatus == NONE) {
                                        slotfreespace += size;
                                        if (slotoffset == -1)
                                                slotoffset = dp->i_offset;
                                        if (slotfreespace >= slotneeded) {
                                                slotstatus = COMPACT;
                                                slotsize = dp->i_offset +
                                                    fs2h16(ep->e2d_reclen) -
                                                    slotoffset;
                                        }
                                }
                        }
                }

                /*
                 * Check for a name match.
                 */
                if (ep->e2d_ino) {
                        namlen = ep->e2d_namlen;
                        if (namlen == cnp->cn_namelen &&
                            !memcmp(cnp->cn_nameptr, ep->e2d_name,
                            (unsigned)namlen)) {
                                /*
                                 * Save directory entry's inode number and
                                 * reclen in ndp->ni_ufs area, and release
                                 * directory buffer.
                                 */
                                foundino = fs2h32(ep->e2d_ino);
                                dp->i_reclen = fs2h16(ep->e2d_reclen);
                                goto found;
                        }
                }
                prevoff = dp->i_offset;
                dp->i_offset += fs2h16(ep->e2d_reclen);
                entryoffsetinblock += fs2h16(ep->e2d_reclen);
                if (ep->e2d_ino)
                        enduseful = dp->i_offset;
        }
/* notfound: */
        /*
         * If we started in the middle of the directory and failed
         * to find our target, we must check the beginning as well.
         */
        if (numdirpasses == 2) {
                numdirpasses--;
                dp->i_offset = 0;
                endsearch = dp->i_diroff;
                goto searchloop;
        }
        if (bp != NULL)
                brelse(bp, 0);
        /*
         * If creating, and at end of pathname and current
         * directory has not been removed, then can consider
         * allowing file to be created.
       */
/*        if ((nameiop == CREATE || nameiop == RENAME) &&
            (flags & ISLASTCN) && dp->i_e2fs_nlink != 0) {
*/                /*
                 * Access for write is interpreted as allowing
                 * creation of files in the directory.
                 */
/*                error = VOP_ACCESS(vdp, VWRITE, cred);
                if (error)
                        return (error);
*/                /*
                 * Return an indication of where the new directory
                 * entry should be put.  If we didn't find a slot,
                 * then set dp->i_count to 0 indicating
                 * that the new slot belongs at the end of the
                 * directory. If we found a slot, then the new entry
                 * can be put in the range from dp->i_offset to
                 * dp->i_offset + dp->i_count.
                 */
/*                if (slotstatus == NONE) {
                        dp->i_offset = roundup(ext2fs_size(dp), dirblksiz);
                        dp->i_count = 0;
                        enduseful = dp->i_offset;
                } else {
                        dp->i_offset = slotoffset;
                        dp->i_count = slotsize;
                        if (enduseful < slotoffset + slotsize)
                                enduseful = slotoffset + slotsize;
                }
                dp->i_endoff = roundup(enduseful, dirblksiz);
#if 0
                dp->i_flag |= IN_CHANGE | IN_UPDATE;
#endif
*/                /*
                 * We return with the directory locked, so that
                 * the parameters we set up above will still be
                 * valid if we actually decide to do a direnter().
                 * We return ni_vp == NULL to indicate that the entry
                 * does not currently exist; we leave a pointer to
                 * the (locked) directory inode in ndp->ni_dvp.
                 *
                 * NB - if the directory is unlocked, then this
                 * information cannot be used.
                 */
//                return (EJUSTRETURN);
//        }
        /*
         * Insert name into cache (as non-existent) if appropriate.
         */
//        if ((cnp->cn_flags & MAKEENTRY) && nameiop != CREATE)
//                cache_enter(vdp, *vpp, cnp);
        return (ENOENT);

found:
//        if (numdirpasses == 2)
//                nchstats.ncs_pass2++;
        /*
         * Check that directory length properly reflects presence
         * of this entry.
         */
        if (dp->i_offset + EXT2FS_DIRSIZ(ep->e2d_namlen) > ext2fs_size(dp)) {
                ufs_dirbad(dp, dp->i_offset, "i_size too small");
                error = ext2fs_setsize(dp,
                                dp->i_offset + EXT2FS_DIRSIZ(ep->e2d_namlen));
                if (error) {
                        brelse(bp, 0);
                        return (error);
                }
                dp->i_flag |= IN_CHANGE | IN_UPDATE;
                uvm_vnp_setsize(vdp, ext2fs_size(dp));
        }
        brelse(bp, 0);

        /*
         * Found component in pathname.
         * If the final component of path name, save information
         * in the cache as to where the entry was found.
         */
        if ((flags & ISLASTCN) && nameiop == LOOKUP)
                dp->i_diroff = dp->i_offset &~ (dirblksiz - 1);

        /*
         * If deleting, and at end of pathname, return
         * parameters which can be used to remove file.
         * Lock the inode, being careful with ".".
         */
//        if (nameiop == DELETE && (flags & ISLASTCN)) {
                /*
                 * Write access to directory required to delete files.
                 */
//                if ((error = VOP_ACCESS(vdp, VWRITE, cred)) != 0)
//                        return (error);
                /*
                 * Return pointer to current entry in dp->i_offset,
                 * and distance past previous entry (if there
                 * is a previous entry in this block) in dp->i_count.
                 * Save directory inode pointer in ndp->ni_dvp for dirremove().
                 */
/*                if ((dp->i_offset & (dirblksiz - 1)) == 0)
                        dp->i_count = 0;
                else
                        dp->i_count = dp->i_offset - prevoff;
                if (dp->i_number == foundino) {
                        vref(vdp);
                        *vpp = vdp;
                        return (0);
                }
                if (flags & ISDOTDOT)
                        VOP_UNLOCK(vdp);*/ /* race to get the inode */
/*                error = VFS_VGET(vdp->v_mount, foundino, &tdp);
                if (flags & ISDOTDOT)
                        vn_lock(vdp, LK_EXCLUSIVE | LK_RETRY);
                if (error)
                        return (error);
*/                /*
                 * If directory is "sticky", then user must own
                 * the directory, or the file in it, else she
                 * may not delete it (unless she's root). This
                 * implements append-only directories.
                 */
/*                if ((dp->i_e2fs_mode & ISVTX) &&
                    kauth_authorize_generic(cred, KAUTH_GENERIC_ISSUSER, NULL) &&
                    kauth_cred_geteuid(cred) != dp->i_uid &&
                    VTOI(tdp)->i_uid != kauth_cred_geteuid(cred)) {
                       vput(tdp);
                        return (EPERM);
                }
                *vpp = tdp;
                return (0);
        }
*/
        /*
         * If rewriting (RENAME), return the inode and the
         * information required to rewrite the present directory
         * Must get inode of directory entry to verify it's a
         * regular file, or empty directory.
         */
/*        if (nameiop == RENAME && (flags & ISLASTCN)) {
                error = VOP_ACCESS(vdp, VWRITE, cred);
                if (error)
                        return (error);
*/                /*
                 * Careful about locking second inode.
                 * This can only occur if the target is ".".
                 */
/*                if (dp->i_number == foundino)
                        return (EISDIR);
                if (flags & ISDOTDOT)
                        VOP_UNLOCK(vdp); *//* race to get the inode */
/*                error = VFS_VGET(vdp->v_mount, foundino, &tdp);
                if (flags & ISDOTDOT)
                        vn_lock(vdp, LK_EXCLUSIVE | LK_RETRY);
                if (error)
                        return (error);
                *vpp = tdp;
                return (0);
        }
*/
        /*
         * Step through the translation in the name.  We do not `vput' the
         * directory because we may need it again if a symbolic link
         * is relative to the current directory.  Instead we save it
         * unlocked as "pdp".  We must get the target inode before unlocking
         * the directory to insure that the inode will not be removed
         * before we get it.  We prevent deadlock by always fetching
         * inodes from the root, moving down the directory tree. Thus
         * when following backward pointers ".." we must unlock the
         * parent directory before getting the requested directory.
         * There is a potential race condition here if both the current
         * and parent directories are removed before the VFS_VGET for the
         * inode associated with ".." returns.  We hope that this occurs
         * infrequently since we cannot avoid this race condition without
         * implementing a sophisticated deadlock detection algorithm.
         * Note also that this simple deadlock detection scheme will not
         * work if the file system has any hard links other than ".."
         * that point backwards in the directory structure.
         */
        pdp = vdp;
        if (flags & ISDOTDOT) {
//                VOP_UNLOCK(pdp);        /* race to get the inode */
                error = VFS_VGET(vdp->v_mount, foundino, &tdp);
//                vn_lock(pdp, LK_EXCLUSIVE | LK_RETRY);
                if (error) {
                        return (error);
                }
                *vpp = tdp;
        } else if (dp->i_number == foundino) {
        //        vref(vdp);      /* we want ourself, ie "." */
                *vpp = vdp;
        } else {
                error = VFS_VGET(vdp->v_mount, foundino, &tdp);
                if (error)
                        return (error);
                *vpp = tdp;
        }

        /*
         * Insert name into cache if appropriate.
         */
//        if (cnp->cn_flags & MAKEENTRY)
//                cache_enter(vdp, *vpp, cnp);
        return (0);
}

/*
 * Do consistency checking on a directory entry:
 *      record length must be multiple of 4
 *      entry must fit in rest of its dirblksize block
 *      record must be large enough to contain entry
 *      name is not longer than EXT2FS_MAXNAMLEN
 *      name must be as long as advertised, and null terminated
 */
/*
 *      changed so that it confirms to ext2fs_check_dir_entry
 */
static int
#define struct
ext2fs_dirbadentry(struct vnode *dp,
#undef struct 
		struct ext2fs_direct *de,
                int entryoffsetinblock)
{
//        struct ufsmount *ump = VFSTOUFS(dp->v_mount);
//        int dirblksiz = ump->um_dirblksiz;
	int dirblksiz = (EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(dp->Filesystem))->fs->e2fs_bsize; 

        const char *error_msg = NULL;
        int reclen = fs2h16(de->e2d_reclen);
        int namlen = de->e2d_namlen;

        if (reclen < EXT2FS_DIRSIZ(1)) /* e2d_namlen = 1 */
		error_msg = "rec_len is smaller than minimal";
        else if (reclen % 4 != 0)
       		error_msg = "rec_len % 4 != 0";
        else if (namlen > EXT2FS_MAXNAMLEN)
                error_msg = "namlen > EXT2FS_MAXNAMLEN";
        else if (reclen < EXT2FS_DIRSIZ(namlen))
                error_msg = "reclen is too small for name_len";
        else if (entryoffsetinblock + reclen > dirblksiz)
                error_msg = "directory entry across blocks";
        else if (fs2h32(de->e2d_ino) >
                VTOI(dp)->i_e2fs->e2fs.e2fs_icount)
	        error_msg = "inode out of bounds";

        if (error_msg != NULL) {
                printf( "bad directory entry: %s\n"
        	       "offset=%d, inode=%lu, rec_len=%d, name_len=%d \n",
                        error_msg, entryoffsetinblock,
                        (unsigned long) fs2h32(de->e2d_ino),
                        reclen, namlen);
                panic("ext2fs_dirbadentry");
        }

        return error_msg == NULL ? 0 : 1;
}

/*
 * Check if a directory is empty or not.
 * Inode supplied must be locked.
 *
 * Using a struct dirtemplate here is not precisely
 * what we want, but better than using a struct ext2fs_direct.
 *
 * NB: does not handle corrupted directories.
 */
int
ext2fs_dirempty(struct inode *ip, ino_t parentino, kauth_cred_t vcred)
{
        off_t off;
        struct ext2fs_dirtemplate dbuf;
        struct ext2fs_direct *dp = (struct ext2fs_direct *)&dbuf;
        int error, namlen;
        size_t count;

#define MINDIRSIZ (sizeof (struct ext2fs_dirtemplate) / 2)

        for (off = 0; off < ext2fs_size(ip); off += fs2h16(dp->e2d_reclen)) {
                error = vn_rdwr(UIO_READ, ITOV(ip), (void *)dp, MINDIRSIZ, off,
                   UIO_SYSSPACE, IO_NODELOCKED, vcred, &count, NULL);
                /*
                 * Since we read MINDIRSIZ, residual must
                 * be 0 unless we're at end of file.
                 */
                if (error || count != 0)
                        return (0);
                /* avoid infinite loops */
                if (dp->e2d_reclen == 0)
                        return (0);
                /* skip empty entries */
                if (dp->e2d_ino == 0)
                        continue;
                /* accept only "." and ".." */
                namlen = dp->e2d_namlen;
                if (namlen > 2)
                        return (0);
                if (dp->e2d_name[0] != '.')
                        return (0);
                /*
                 * At this point namlen must be 1 or 2.
                 * 1 implies ".", 2 implies ".." if second
                 * char is also "."
                 */
                if (namlen == 1)
                        continue;
                if (dp->e2d_name[1] == '.' && fs2h32(dp->e2d_ino) == parentino)
                        continue;
                return (0);
        }
        return (1);
}


/*
 * Check if source directory is in the path of the target directory.
 * Target is supplied locked, source is unlocked.
 * The target is always vput before returning.
 */
#undef cred
int
ext2fs_checkpath(struct inode *source, struct inode *target,
	kauth_cred_t cred)
{
#define struct
	struct vnode *vp;
#undef struct
	int error, rootino, namlen;
	struct ext2fs_dirtemplate dirbuf;
	uint32_t ino;

	vp = ITOV(target);
	if (target->i_number == source->i_number) {
		error = EEXIST;
		goto out;
	}
	rootino = ROOTINO;
	error = 0;
	if (target->i_number == rootino)
		goto out;

	for (;;) {
		if (vp->v_type != VDIR) {
			error = ENOTDIR;
			break;
		}
		error = vn_rdwr(UIO_READ, vp, (void *)&dirbuf,
			sizeof (struct ext2fs_dirtemplate), (off_t)0,
			UIO_SYSSPACE, IO_NODELOCKED, cred, (size_t *)0,
			NULL);
		if (error != 0)
			break;
		namlen = dirbuf.dotdot_namlen;
		if (namlen != 2 ||
			dirbuf.dotdot_name[0] != '.' ||
			dirbuf.dotdot_name[1] != '.') {
			error = ENOTDIR;
			break;
		}
		ino = fs2h32(dirbuf.dotdot_ino);
		if (ino == source->i_number) {
			error = EINVAL;
			break;
		}
		if (ino == rootino)
			break;
		vput(vp);
		error = VFS_VGET(vp->v_mount, ino, &vp);
		if (error != 0) {
			vp = NULL;
			break;
		}
	}

out:
	if (error == ENOTDIR) {
		printf("checkpath: .. not a directory\n");
		panic("checkpath");
	}
	if (vp != NULL)
		vput(vp);
	return (error);
}
