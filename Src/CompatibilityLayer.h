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

#include <Library/BaseLib.h>
#include <endian.h>
#include <sys/cdefs.h>

#define memcpy CopyMem
#define memset SetMem
#define malloc(a,b,c) AllocateZeroPool(a)
#define free(a,b) FreePool(a)
#define strcpy(a,b,c,d) AsciiStrCpy(a,d)

#define	roundup2(x, m)	(((x) + (m) - 1) & ~((m) - 1))
#define	howmany(x, y)	(((x)+((y)-1))/(y))

#undef bswap16
#undef bswap32
#undef bswap64

#define INT32_MAX 0x7fffffff

typedef VOID void;
typedef UINT8 u_char;

typedef INT32 int;
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
typedef UINT65 u_quad_t;

typedef INT64 off_t;
typedef INT64 daddr_t;
typedef UINT64 ino_t;

struct buf {
  VOID *b_data; 
};

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
