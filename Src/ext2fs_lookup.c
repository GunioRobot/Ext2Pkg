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

extern	int dirchk;

static void	ext2fs_dirconv2ffs(struct ext2fs_direct *e2dir,
					  struct dirent *ffsdir);
/*
#define struct
static int	ext2fs_dirbadentry(struct vnode *dp,
#undef struct
					  struct ext2fs_direct *de,
					  int entryoffsetinblock);
*/
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
