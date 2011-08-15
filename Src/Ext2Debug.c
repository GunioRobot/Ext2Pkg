#include "Ext2.h"

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

  char plm[10] = "alin";

  DEBUG ((EFI_D_INFO, "plm test %s ha",plm));
  DEBUG ((EFI_D_INFO, "\n============Ext2 direct===================\n"));
  DEBUG ((EFI_D_INFO, "e2d_ino              : %d\n", dir->e2d_ino));
  DEBUG ((EFI_D_INFO, "e2d_reclen           : %d\n", dir->e2d_reclen));
  DEBUG ((EFI_D_INFO, "e2d_namelen          : %d\n", dir->e2d_namlen));
  DEBUG ((EFI_D_INFO, "e2d_type             : %d\n", dir->e2d_type));
  DEBUG ((EFI_D_INFO, "e2d_name             : %s\n", dir->e2d_name));

}
