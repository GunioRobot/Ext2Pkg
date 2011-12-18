/*	$NetBSD: inode.h,v 1.57 2010/07/28 11:03:48 hannken Exp $	*/

/*
 * Copyright (c) 1982, 1989, 1993
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
 *	@(#)inode.h	8.9 (Berkeley) 5/14/95
 */

/** @file

Modified for edk2

Copyright (c) 2011, Alin-Florin Rus-Rebreanu <alin@softwareliber.ro>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef _UFS_UFS_INODE_H_
#define	_UFS_UFS_INODE_H_

#ifndef _EXT2_TIANOCORE_SOURCE
#include <sys/vnode.h>
#include <ufs/ufs/dinode.h>
#include <ufs/ufs/dir.h>
#include <ufs/ufs/quota.h>
#include <ufs/ext2fs/ext2fs_dinode.h>
#include <miscfs/genfs/genfs_node.h>
#else
/** @file

Modified for edk2

Copyright (c) 2011, Alin-Florin Rus-Rebreanu <alin@softwareliber.ro>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/
#include "CompatibilityLayer.h"
#include <Protocol/SimpleFileSystem.h>
#endif

/*
 * Per-filesystem inode extensions.
 */

struct ext2fs_inode_ext {
	daddr_t ext2fs_last_lblk;	/* last logical block allocated */
	daddr_t ext2fs_last_blk;	/* last block allocated on disk */
};

/*
 * The inode is used to describe each active (or recently active) file in the
 * UFS filesystem. It is composed of two types of information. The first part
 * is the information that is needed only while the file is active (such as
 * the identity of the file and linkage to speed its lookup). The second part
 * is the permanent meta-data associated with the file which is read in
 * from the permanent dinode from long term storage when the file becomes
 * active, and is put back when the file is no longer being used.
 */
struct inode {
//	LIST_ENTRY(inode) i_hash;/* Hash chain.
	EFI_FILE_PROTOCOL *vp;

	u_int32_t i_flag;	/* flags, see below */
	ino_t	  i_number;	/* The identity of the inode. */

	union {			/* Associated filesystem. */
		struct	m_ext2fs *e2fs;	/* EXT2FS */
	} inode_u;

#define	i_e2fs	inode_u.e2fs

	void	*i_unused1;	/* Unused. */

	/*
	 * Side effects; used during directory lookup.
	 */
	int32_t	  i_count;	/* Size of free slot in directory. */
	doff_t	  i_endoff;	/* End of useful stuff in directory. */
	doff_t	  i_diroff;	/* Offset in dir, where we found last entry. */
	doff_t	  i_offset;	/* Offset of free space in directory. */
	u_int32_t i_reclen;	/* Size of found directory entry. */
	/*
	 * Inode extensions
	 */
	union {
		/* Other extensions could go here... */
		struct	ext2fs_inode_ext e2fs;
	} inode_ext;

#define	i_e2fs_last_lblk	inode_ext.e2fs.ext2fs_last_lblk
#define	i_e2fs_last_blk		inode_ext.e2fs.ext2fs_last_blk
	/*
	 * Copies from the on-disk dinode itself.
	 *
	 * These fields are currently only used by FFS and LFS,
	 * do NOT use them with ext2fs.
	 */
	u_int16_t i_mode;	/* IFMT, permissions; see below. */
	int16_t   i_nlink;	/* File link count. */
	u_int64_t i_size;	/* File byte count. */
	u_int32_t i_flags;	/* Status flags (chflags). */
	int32_t   i_gen;	/* Generation number. */
	u_int32_t i_uid;	/* File owner. */
	u_int32_t i_gid;	/* File group. */
	u_int16_t i_omode;	/* Old mode, for ufs_reclaim. */

	/*
	 * The on-disk dinode itself.
	 */
	union {
		struct	ext2fs_dinode *e2fs_din; /* 128 bytes of the on-disk
						   dinode. */
	} i_din;
};

#define	i_e2fs_mode		i_din.e2fs_din->e2di_mode
#define	i_e2fs_uid		i_din.e2fs_din->e2di_uid
#define	i_e2fs_size		i_din.e2fs_din->e2di_size
#define	i_e2fs_atime		i_din.e2fs_din->e2di_atime
#define	i_e2fs_ctime		i_din.e2fs_din->e2di_ctime
#define	i_e2fs_mtime		i_din.e2fs_din->e2di_mtime
#define	i_e2fs_dtime		i_din.e2fs_din->e2di_dtime
#define	i_e2fs_gid		i_din.e2fs_din->e2di_gid
#define	i_e2fs_nlink		i_din.e2fs_din->e2di_nlink
#define	i_e2fs_nblock		i_din.e2fs_din->e2di_nblock
#define	i_e2fs_flags		i_din.e2fs_din->e2di_flags
#define	i_e2fs_blocks		i_din.e2fs_din->e2di_blocks
#define	i_e2fs_gen		i_din.e2fs_din->e2di_gen
#define	i_e2fs_facl		i_din.e2fs_din->e2di_facl
#define	i_e2fs_dacl		i_din.e2fs_din->e2di_dacl
#define	i_e2fs_faddr		i_din.e2fs_din->e2di_faddr
#define	i_e2fs_nfrag		i_din.e2fs_din->e2di_nfrag
#define	i_e2fs_fsize		i_din.e2fs_din->e2di_fsize
#define	i_e2fs_rdev		i_din.e2fs_din->e2di_rdev
#define	i_e2fs_uid_high		i_din.e2fs_din->e2di_uid_high
#define	i_e2fs_gid_high		i_din.e2fs_din->e2di_gid_high

/* These flags are kept in i_flag. */
#define	IN_ACCESS	0x0001		/* Access time update request. */
#define	IN_CHANGE	0x0002		/* Inode change time update request. */
#define	IN_UPDATE	0x0004		/* Inode was written to; update mtime. */
#define	IN_MODIFY	0x2000		/* Modification time update request. */
#define	IN_MODIFIED	0x0008		/* Inode has been modified. */
#define	IN_ACCESSED	0x0010		/* Inode has been accessed. */
#define	IN_RENAME	0x0020		/* Inode is being renamed. */
#define	IN_SHLOCK	0x0040		/* File has shared lock. */
#define	IN_EXLOCK	0x0080		/* File has exclusive lock. */
#define	IN_CLEANING	0x0100		/* LFS: file is being cleaned */
#define	IN_ADIROP	0x0200		/* LFS: dirop in progress */
#define	IN_SPACECOUNTED	0x0400		/* Blocks to be freed in free count. */
#define	IN_PAGING       0x1000		/* LFS: file is on paging queue */

#if defined(_KERNEL)

/*
 * Structure used to pass around logical block paths generated by
 * ufs_getlbns and used by truncate and bmap code.
 */
struct indir {
	daddr_t in_lbn;		/* Logical block number. */
	int	in_off;			/* Offset in buffer. */
	int	in_exists;		/* Flag if the block exists. */
};


/* This overlays the fid structure (see fstypes.h). */
struct ufid {
	u_int16_t ufid_len;	/* Length of structure. */
	u_int16_t ufid_pad;	/* Force 32-bit alignment. */
	u_int32_t ufid_ino;	/* File number (ino). */
	int32_t	  ufid_gen;	/* Generation number. */
};
#endif /* _KERNEL */

#endif /* !_UFS_UFS_INODE_H_ */
