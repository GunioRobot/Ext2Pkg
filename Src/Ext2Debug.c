#include "Ext2.h"
#include <sys/dirent.h>

VOID Ext2DebugSb (IN EXT2_DEV *Private) {

  struct ext2fs *sb;
  struct m_ext2fs *fs;
  struct ext2_gd *gd;
  fs = Private->fs;
  sb = &fs->e2fs;

  DEBUG ((EFI_D_INFO, "\n===============Superblock===================\n"));
  DEBUG ((EFI_D_INFO, "\next2fs sb size    : %d\n", sizeof (struct ext2fs)));
  DEBUG ((EFI_D_INFO, "e2fs_icount	   : %d\n", sb->e2fs_icount));
  DEBUG ((EFI_D_INFO, "e2fs_bcount         : %d\n", sb->e2fs_bcount));
  DEBUG ((EFI_D_INFO, "e2fs_rbcount        : %d\n", sb->e2fs_rbcount));
  DEBUG ((EFI_D_INFO, "e2fs_fbcount        : %d\n", sb->e2fs_fbcount));
  DEBUG ((EFI_D_INFO, "e2fs_first_dblock   : %d\n", sb->e2fs_first_dblock));
  DEBUG ((EFI_D_INFO, "e2fs_log_bsize      : %d\n", sb->e2fs_log_bsize));
  DEBUG ((EFI_D_INFO, "e2fs_fsize          : %d\n", sb->e2fs_fsize));
  DEBUG ((EFI_D_INFO, "e2fs_bpg            : %d\n", sb->e2fs_bpg));
  DEBUG ((EFI_D_INFO, "e2fs_fpg            : %d\n", sb->e2fs_fpg));
  DEBUG ((EFI_D_INFO, "e2fs_ipg            : %d\n", sb->e2fs_ipg));
  DEBUG ((EFI_D_INFO, "e2fs_mtime          : %d\n", sb->e2fs_mtime));
  DEBUG ((EFI_D_INFO, "e2fs_wtime          : %d\n", sb->e2fs_wtime));
  DEBUG ((EFI_D_INFO, "e2fs_magic          : 0x%x\n", sb->e2fs_magic));
  DEBUG ((EFI_D_INFO, "e2fs_state          : %d\n", sb->e2fs_state));
  DEBUG ((EFI_D_INFO, "e2fs_first_ino      : %d\n", sb->e2fs_first_ino));
  DEBUG ((EFI_D_INFO, "e2fs_inode_size     : %d\n", sb->e2fs_inode_size));
  DEBUG ((EFI_D_INFO, "e2fs_block_group_nr : %d\n", sb->e2fs_block_group_nr));

  DEBUG ((EFI_D_INFO, "\n===============In memory===================\n"));
  DEBUG ((EFI_D_INFO, "e2fs_fsmnt          : %s\n", fs->e2fs_fsmnt));
  DEBUG ((EFI_D_INFO, "e2fs_ronly          : %d\n", fs->e2fs_ronly));
  DEBUG ((EFI_D_INFO, "e2fs_fmod           : %d\n", fs->e2fs_fmod));
  DEBUG ((EFI_D_INFO, "e2fs_bsize          : %d\n", fs->e2fs_bsize));
  DEBUG ((EFI_D_INFO, "e2fs_bshift         : %d\n", fs->e2fs_bshift));
  DEBUG ((EFI_D_INFO, "e2fs_bmask          : %d\n", fs->e2fs_bmask));
  DEBUG ((EFI_D_INFO, "e2fs_qbmask         : %d\n", fs->e2fs_qbmask));
  DEBUG ((EFI_D_INFO, "e2fs_fsbtodb        : %d\n", fs->e2fs_fsbtodb));
  DEBUG ((EFI_D_INFO, "e2fs_ncg            : %d\n", fs->e2fs_ncg));
  DEBUG ((EFI_D_INFO, "e2fs_ngdb           : %d\n", fs->e2fs_ngdb));
  DEBUG ((EFI_D_INFO, "e2fs_ipb            : %d\n", fs->e2fs_ipb));
  DEBUG ((EFI_D_INFO, "e2fs_itpg           : %d\n", fs->e2fs_itpg));

  gd = fs->e2fs_gd;

  DEBUG ((EFI_D_INFO, "\n============Block group descriptor=========\n"));
  DEBUG ((EFI_D_INFO, "ext2bgd_b_bitmap    : %d\n", gd->ext2bgd_b_bitmap));
  DEBUG ((EFI_D_INFO, "ext2bgd_i_bitmap    : %d\n", gd->ext2bgd_i_bitmap));
  DEBUG ((EFI_D_INFO, "ext2bgd_i_tables    : %d\n", gd->ext2bgd_i_tables));
  DEBUG ((EFI_D_INFO, "ext2bgd_nbfree      : %d\n", gd->ext2bgd_nbfree));
  DEBUG ((EFI_D_INFO, "ext2bgd_nifree      : %d\n", gd->ext2bgd_nifree));
  DEBUG ((EFI_D_INFO, "ext2bgd_ndirs       : %d\n", gd->ext2bgd_ndirs));
  DEBUG ((EFI_D_INFO, "\n==========================================\n"));

}

VOID Ext2DebugDinode (struct ext2fs_dinode *d) {
  DEBUG ((EFI_D_INFO, "\n============Ext2 dinode===================\n"));
  DEBUG ((EFI_D_INFO, "e2di_mode           : %d\n", d->e2di_mode));
  DEBUG ((EFI_D_INFO, "e2di_uid           : %d\n", d->e2di_uid));
  DEBUG ((EFI_D_INFO, "e2di_size           : %d\n", d->e2di_size));
  DEBUG ((EFI_D_INFO, "e2di_atime           : %d\n", d->e2di_atime));
  DEBUG ((EFI_D_INFO, "e2di_ctime           : %d\n", d->e2di_ctime));
  DEBUG ((EFI_D_INFO, "e2di_mtime           : %d\n", d->e2di_mtime));
  DEBUG ((EFI_D_INFO, "e2di_dtime           : %d\n", d->e2di_dtime));
  DEBUG ((EFI_D_INFO, "e2di_gid           : %d\n", d->e2di_gid));
  DEBUG ((EFI_D_INFO, "e2di_nlink           : %d\n", d->e2di_nlink));
  DEBUG ((EFI_D_INFO, "e2di_nblock           : %d\n", d->e2di_nblock));
  DEBUG ((EFI_D_INFO, "e2di_flags           : %d\n", d->e2di_flags));
  DEBUG ((EFI_D_INFO, "e2di_gen           : %d\n", d->e2di_gen));
  DEBUG ((EFI_D_INFO, "e2di_faddr           : %d\n", d->e2di_faddr));
  DEBUG ((EFI_D_INFO, "e2di_nfrag           : %d\n", d->e2di_nfrag));
  DEBUG ((EFI_D_INFO, "e2di_fsize           : %d\n", d->e2di_fsize));
  DEBUG ((EFI_D_INFO, "e2di_blocks[0],[1]   : %d, %d\n", d->e2di_blocks[0], d->e2di_blocks[1]));

}

VOID Ext2DebugDirect (struct ext2fs_direct *dir) {

  CHAR16 Name[dir->e2d_namlen+1];

  AsciiStrToUnicodeStr(dir->e2d_name, Name);

  DEBUG ((EFI_D_INFO, "\n============Ext2 direct===================\n"));
  DEBUG ((EFI_D_INFO, "e2d_ino              : %d\n", dir->e2d_ino));
  DEBUG ((EFI_D_INFO, "e2d_reclen           : %d\n", dir->e2d_reclen));
  DEBUG ((EFI_D_INFO, "e2d_namelen          : %d\n", dir->e2d_namlen));
  DEBUG ((EFI_D_INFO, "e2d_type             : %d\n", dir->e2d_type));
  DEBUG ((EFI_D_INFO, "e2d_name             : %s\n", Name));

}
VOID Ext2DebugDirent (struct dirent *dir) {

  CHAR16 Name[dir->d_namlen];

  AsciiStrToUnicodeStr(dir->d_name, Name);

  DEBUG ((EFI_D_INFO, "\n============Ext2 direct===================\n"));
  DEBUG ((EFI_D_INFO, "d_fileno             : %d\n", dir->d_fileno));
  DEBUG ((EFI_D_INFO, "d_reclen             : %d\n", dir->d_reclen));
  DEBUG ((EFI_D_INFO, "d_namelen            : %d\n", dir->d_namlen));
  DEBUG ((EFI_D_INFO, "d_name               : %s\n", Name));

}

VOID Ext2DebugCharBuffer (VOID *buf, INTN size) {

  char *str = NULL;
  int i;

  str = buf;
  for (i = 0; i < size; i++) {
    DEBUG ((EFI_D_INFO, "%c", str[i]));
  }
}

VOID Ext2DebugPrintContent (EXT2_EFI_FILE_PRIVATE *PrivateFile) {

  struct vop_read_args v;
  struct uio uio;
  struct iovec uio_iov;
  int error = 0;

  uio_iov.iov_base = AllocateZeroPool (1024); //bsize
  uio_iov.iov_len = 1024;

  uio.uio_iov = &uio_iov;
  uio.uio_iovcnt = 1;
  uio.uio_offset = 0;
  uio.uio_resid = 1024;
  uio.uio_rw = UIO_READ;

  v.a_vp = PrivateFile;
  v.a_uio = &uio;
  v.a_ioflag = 0;

  error = ext2fs_read(&v);
  DEBUG ((EFI_D_INFO, "\n *** Content of regular file %s *** \n", PrivateFile->Filename));
  Ext2DebugCharBuffer(uio_iov.iov_base, 100); //100 chars should be enough, otherwise
						// we're poluting the logs i guess
  FreePool (uio_iov.iov_base);
}

VOID Ext2DebugListTree (IN EXT2_DEV *Private, EXT2_EFI_FILE_PRIVATE *PrivateFile) {

    EXT2_EFI_FILE_PRIVATE *pFile;
    struct vop_readdir_args ap;
    int a_eofflag;
    off_t *a_cookies;
    ap.a_vp = PrivateFile;
    struct dirent *dir;
    void *c;
    struct uio uio;
    struct iovec uio_iov;
    int error;
    int ino = PrivateFile->File->i_number;
    //CHAR16 Name[255];

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
    dir = (struct dirent *) c;

    DEBUG ((EFI_D_INFO, "\n *** In folder %s ***\n", PrivateFile->Filename));
    while (dir->d_fileno != 0) {

	Ext2DebugDirent(dir);
	//AsciiStrToUnicodeStr(dir->d_name, Name);
	if ((dir->d_fileno != ino) && (AsciiStrCmp(dir->d_name,".") != 0)
	    && (AsciiStrCmp(dir->d_name,"..") != 0)) {
	  error = ext2fs_vget(Private, dir->d_fileno, &pFile);
	//  pFile->Filename = AllocateZeroPool ((1+AsciiStrLen(dir->d_name))*sizeof(CHAR16));
	//  StrCpy(pFile->Filename, Name);
	  if (pFile->v_type == VREG) {
	      Ext2DebugPrintContent(pFile);
	  } else {
	      Ext2DebugListTree(Private,pFile);
	  }
	//  FreePool(pFile->Filename);
	  FreePool(pFile);
	}
	dir = (struct dirent *) ((char *)dir + dir->d_reclen);
    }
}
