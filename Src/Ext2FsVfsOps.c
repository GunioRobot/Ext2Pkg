/*	$NetBSD: ext2fs_vfsops.c,v 1.160 2011/06/12 03:36:00 rmind Exp $	*/

/*
 * Copyright (c) 1989, 1991, 1993, 1994
 *	The Regents of the University of California.  All rights reserved.
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
 *	@(#)ffs_vfsops.c	8.14 (Berkeley) 11/28/94
 * Modified for ext2fs by Manuel Bouyer.
 */

/*
 * Copyright (c) 1997 Manuel Bouyer.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *	@(#)ffs_vfsops.c	8.14 (Berkeley) 11/28/94
 * Modified for ext2fs by Manuel Bouyer.
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

#include "Ext2.h"
#include "ext2fs_dir.h"

int
ext2fs_mountfs(EXT2_DEV *mp);

static int
ext2fs_checksb(struct ext2fs *fs, int ronly);

int vfs_rootmountalloc (EXT2_DEV *mp) {

    mp->f_mntonname[0] = '/';
    mp->f_mntonname[1] = '\0';

    return 0;

}

int
ext2fs_mountroot(EXT2_DEV *mp)
{
        struct m_ext2fs *fs;
        int error = -1;

        if (EFI_ERROR (vfs_rootmountalloc(mp))) {
                return (error);
        }

        if ((error = ext2fs_mountfs(mp)) != 0) {
                return (error);
        }

        fs = mp->fs;
        memset(fs->e2fs_fsmnt, 0, sizeof(fs->e2fs_fsmnt));
        (void) copystr(mp->f_mntonname, fs->e2fs_fsmnt,
            sizeof(fs->e2fs_fsmnt) - 1, 0);
        if (fs->e2fs.e2fs_rev > E2FS_REV0) {
                memset(fs->e2fs.e2fs_fsmnt, 0, sizeof(fs->e2fs.e2fs_fsmnt));
                (void) copystr(mp->f_mntonname, fs->e2fs.e2fs_fsmnt,
                    sizeof(fs->e2fs.e2fs_fsmnt) - 1, 0);
        }
        return (0);
}

/*
 * Common code for mount and mountroot
 */
int
ext2fs_mountfs(EXT2_DEV *mp)
{
	struct buf *bp;
	struct ext2fs *fs;
	struct m_ext2fs *m_fs;
	int error, i, ronly;

	ronly = (mp->mnt_flag & MNT_RDONLY) != 0;

	bp = NULL;

	error = bread(mp, SBLOCK, SBSIZE, cred, 0, &bp);
	if (error)
		goto out;
	fs = (struct ext2fs *)bp->b_data;
	error = ext2fs_checksb(fs, ronly);
	if (error)
		goto out;
	mp->fs = malloc(sizeof(struct m_ext2fs), M_UFSMNT, M_WAITOK);
	memset(mp->fs, 0, sizeof(struct m_ext2fs));
	e2fs_sbload((struct ext2fs *)bp->b_data, &mp->fs->e2fs);
	brelse(bp, 0);
	bp = NULL;
	m_fs = mp->fs;
	m_fs->e2fs_ronly = ronly;

#ifdef DEBUG_EXT2
	printf("ext2 ino size %zu\n", EXT2_DINODE_SIZE(m_fs));
#endif
	if (ronly == 0) {
		if (m_fs->e2fs.e2fs_state == E2FS_ISCLEAN)
			m_fs->e2fs.e2fs_state = 0;
		else
			m_fs->e2fs.e2fs_state = E2FS_ERRORS;
		m_fs->e2fs_fmod = 1;
	}

	/* compute dynamic sb infos */
	m_fs->e2fs_ncg =
	    howmany(m_fs->e2fs.e2fs_bcount - m_fs->e2fs.e2fs_first_dblock,
	    m_fs->e2fs.e2fs_bpg);
	m_fs->e2fs_fsbtodb = m_fs->e2fs.e2fs_log_bsize + LOG_MINBSIZE - DEV_BSHIFT;
	m_fs->e2fs_bsize = MINBSIZE << m_fs->e2fs.e2fs_log_bsize;
	m_fs->e2fs_bshift = LOG_MINBSIZE + m_fs->e2fs.e2fs_log_bsize;
	m_fs->e2fs_qbmask = m_fs->e2fs_bsize - 1;
	m_fs->e2fs_bmask = ~m_fs->e2fs_qbmask;
	m_fs->e2fs_ngdb =
	    howmany(m_fs->e2fs_ncg, m_fs->e2fs_bsize / sizeof(struct ext2_gd));
	m_fs->e2fs_ipb = m_fs->e2fs_bsize / EXT2_DINODE_SIZE(m_fs);
	m_fs->e2fs_itpg = m_fs->e2fs.e2fs_ipg / m_fs->e2fs_ipb;

	m_fs->e2fs_gd = malloc(m_fs->e2fs_ngdb * m_fs->e2fs_bsize,
	    M_UFSMNT, M_WAITOK);
	for (i = 0; i < m_fs->e2fs_ngdb; i++) {
		error = bread(mp ,
		    fsbtodb(m_fs, m_fs->e2fs.e2fs_first_dblock +
		    1 /* superblock */ + i),
		    m_fs->e2fs_bsize, NOCRED, 0, &bp);
		if (error) {
			free(m_fs->e2fs_gd, M_UFSMNT);
			goto out;
		}
		e2fs_cgload((struct ext2_gd *)bp->b_data,
		    &m_fs->e2fs_gd[
			i * m_fs->e2fs_bsize / sizeof(struct ext2_gd)],
		    m_fs->e2fs_bsize);
		brelse(bp, 0);
		bp = NULL;
	}

	return (0);

out:
	brelse(bp, 0);

	return (error);
}


static int
ext2fs_checksb(struct ext2fs *fs, int ronly)
{

        if (fs2h16(fs->e2fs_magic) != E2FS_MAGIC) {
                return (EINVAL);                /* XXX needs translation */
        }
        if (fs2h32(fs->e2fs_rev) > E2FS_REV1) {
#ifdef DIAGNOSTIC
                printf("Ext2 fs: unsupported revision number: %x\n",
                    fs2h32(fs->e2fs_rev));
#endif
                return (EINVAL);                /* XXX needs translation */
        }
        if (fs2h32(fs->e2fs_log_bsize) > 2) { /* block size = 1024|2048|4096 */
#ifdef DIAGNOSTIC
                printf("Ext2 fs: bad block size: %d "
                    "(expected <= 2 for ext2 fs)\n",
                    fs2h32(fs->e2fs_log_bsize));
#endif
                return (EINVAL);           /* XXX needs translation */
        }
        if (fs2h32(fs->e2fs_rev) > E2FS_REV0) {
                if (fs2h32(fs->e2fs_first_ino) != EXT2_FIRSTINO) {
                        printf("Ext2 fs: unsupported first inode position\n");
                        return (EINVAL);      /* XXX needs translation */
                }
                if (fs2h32(fs->e2fs_features_incompat) &
                    ~EXT2F_INCOMPAT_SUPP) {
                        printf("Ext2 fs: unsupported optional feature\n");
                        return (EINVAL);      /* XXX needs translation */
                }
                if (!ronly && fs2h32(fs->e2fs_features_rocompat) &
                    ~EXT2F_ROCOMPAT_SUPP) {
                        return (EROFS);      /* XXX needs translation */
                }
        }
        return (0);
}

