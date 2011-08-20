/** @file
Ext2 Driver Reference EDKII Module

Used to keep a compatibility layer between NetBSD and edk data types.

Copyright (c) 2011, Alin-Florin Rus-Rebreanu <alin@softwareliber.ro>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef _COMPATIBILITYLAYER_H_
#define _COMPATIBILITYLAYER_H_

#include <errno.h>

#define _KERNEL
#define memcpy CopyMem
#define memset(a,b,c) SetMem(a,c,b)
#define malloc(a,b,c) AllocateZeroPool(a)
#define free(a,b) FreePool(a)
#define strcpy(a,b,c,d) AsciiStrCpy(a,d)
#define strncpy(a,b,c) AsciiStrnCpy(a,b,c)
#define copystr(a,b,c,d) AsciiStrCpy((CHAR8 *)a,(CHAR8 *)b)
#define LIST_ENTRY(a) LIST_ENTRY
#define printf(a,...) DEBUG((EFI_D_INFO,(a),##__VA_ARGS__))
#define log(a,...) DEBUG((EFI_D_INFO,(a),##_VA_ARGS__))
#define cred 0
#define kauth_cred_t INTN
#define panic(a) DEBUG((EFI_D_ERROR,(a)))
#define VT_EXT2FS 0
#define ext2fs_vnodeop_p (ino==2)?2:0
#define vput(a) FreePool(a)
#define mount EXT2_DEV
#define vnode EXT2_EFI_FILE_PRIVATE
#define VTOI(a) a->File
#define ITOV(a) EXT2_EFI_FILE_PRIVATE_DATA_FROM_THIS(a->vp)
#define vrele(a) FreePool(a)
#define root_device 1
#define v_mount Filesystem
#define device_class(a) getnewvnode (0, mp, 1, NULL, &rootvp)
#define DV_DISK 0
#define M_TEMP 0
#define M_WAITOK 0
#define M_ZERO 0

#define VFS_VGET(a,b,c) ext2fs_vget(EXT2_SIMPLE_FILE_SYSTEM_PRIVATE_DATA_FROM_THIS(a), b, c)

#define ROOTINO EXT2_ROOTINO

#define ENONDEV -1
#define MOUNT_EXT2FS 1

#define false 1
#define vfs_unbusy(a,b,c) FreePool(rootvp)
#define vfs_destroy(a)

#define pool_get(a,b) malloc (sizeof (struct inode), 0, 0)
#define roundup(x, y) ((((x)+((y)-1))/(y))*(y))
#define	roundup2(x, m)	(((x) + (m) - 1) & ~((m) - 1))
#define	howmany(x, y)	(((x)+((y)-1))/(y))

#define MNT_RDONLY 1

#ifndef _BMAP_BREAD_
#define NOCRED 0
#else
#define NOCRED 1
#endif

#define B_MODIY 0

#undef bswap16
#undef bswap32
#undef bswap64

#define IFMT EXT2_IFMT
#define IFREG EXT2_IFREG
#define E2FS_REV0 0
#define EXT2F_ROCOMPAT_LARGEFILE 0x0002

#define IO_ADV_DECODE(a) 0
#define UBC_READ 0
#define UBC_PARTIALOK 1
#define UBC_UNMAP_FLAG(a) 2
#define MIN(a,b) (((a) < (b)) ? (a) : (b))

#define blkptrtodb(ump, b) ((b) << (ump)->fs->e2fs_fsbtodb)
#define um_seqinc 1
#define ump mp

#define ubc_uiomove(a,b,c,d,e) ext2_ubc_uiomove(vp, b, c, d, e)

#define MAXBSIZE 65536

#define INT32_MAX 0x7fffffff

#define mutex_enter(a)
#define mutex_exit(a)
#define incore(a,b) NULL
#define trace(a,b,c)

#define BO_DONE 1
#define BO_DELWRI 1
#define B_READ 1
#define B_WRITE 2

#define MNINDIR(a) NINDIR(a->fs)

#define biowait(bp) bp->b_resid

#define UIO_SETUP_SYSSPACE(a)
#define UIO_SYSSPACE 0
#define IO_NODELOCKED 0

#define VEXEC 0
#define VOP_ACCESS(a,b,c) 0
#define cache_lookup(a,b,c) 0

/*
 * namei operations
 */
#define LOOKUP          0       /* perform name lookup only */
#define CREATE          1       /* setup for file creation */
#define DELETE          2       /* setup for file deletion */
#define RENAME          3       /* setup for file renaming */
#define OPMASK          3       /* mask for operation */

#define NOCROSSMOUNT    0x0000100       /* do not cross mount points */
#define RDONLY          0x0000200       /* lookup with read-only semantics */
#define ISDOTDOT        0x0002000       /* current component name is .. */
#define MAKEENTRY       0x0004000       /* entry is to be added to name cache */
#define ISLASTCN        0x0008000       /* this is last component of pathname */
#define ISWHITEOUT      0x0020000       /* found whiteout */
#define DOWHITEOUT      0x0040000       /* do whiteouts */
#define REQUIREDIR      0x0080000       /* must be a directory */
#define CREATEDIR       0x0200000       /* trailing slashes are ok */
#define INRENAME        0x0400000       /* operation is a part of ``rename'' */
#define INRELOOKUP      0x0800000       /* set while inside relookup() */
#define PARAMASK        0x0eee300       /* mask of parameter descriptors */

typedef _EFI_SIZE_T_ size_t;
typedef UINT8 u_char;
typedef UINT64 vsize_t;
typedef INT8 int8_t;
typedef UINT8 uint8_t;
typedef INT16 int16_t;
typedef UINT16 uint16_t;
typedef UINT16 u_int16_t;
typedef INT32 int32_t;
typedef UINT32 uint32_t;
typedef UINT32 u_int32_t;
typedef INT64 int64_t;
typedef UINT64 uint64_t;
typedef UINT64 u_int64_t;
typedef INT64 quad_t;
typedef UINT64 u_quad_t;
typedef ULONGN u_long;

typedef INT64 off_t;
typedef INT32 doff_t; //weird
typedef INT64 daddr_t;
typedef UINT64 ino_t;

#define memcmp(buf1,buf2,count)           (int)(CompareMem(buf1,buf2,(UINTN)(count)))

struct buf {
  VOID *b_data;
  int b_resid;
  daddr_t b_blkno;
  int b_flags;
  int b_oflags;
};

typedef struct buf buf_t;

/**
Switches the endianness of a 16-bit integer.

This function swaps the bytes in a 16-bit unsigned value to switch the value
from little endian to big endian or vice versa. The byte swapped value is
returned.

@param  Value     A 16-bit unsigned value.

@return The byte swapped Value.

**/
uint16_t bswap16(uint16_t Value);

/**
Switches the endianness of a 32-bit integer.

This function swaps the bytes in a 32-bit unsigned value to switch the value
from little endian to big endian or vice versa. The byte swapped value is
returned.

@param  Value A 32-bit unsigned value.

@return The byte swapped Value.

**/
uint32_t bswap32(uint32_t Value);

/**
Switches the endianness of a 64-bit integer.

This function swaps the bytes in a 64-bit unsigned value to switch the value
from little endian to big endian or vice versa. The byte swapped value is
returned.

@param  Value A 64-bit unsigned value.

@return The byte swapped Value.

**/
uint64_t bswap64(uint64_t Value);

#endif
