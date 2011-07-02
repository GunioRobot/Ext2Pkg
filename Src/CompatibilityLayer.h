/** @file
Ext2 Driver Reference EDKII Module

Used to keep a compatibility layer between NetBSD and edk data types.

Copyright (c) 2011, Alin-Florin Rus-Rebreanu <alin.codejunkie@gmail.com>

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution. The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
**/

#ifndef _COMPATIBILITYLAYER_H_
#define _COMPATIBILITYLAYER_H_

typedef UINT8 u_char;

typedef INT8 int8_t;
typedef UINT8 uint8_t;
typedef INT16 int16_t;
typedef UINT16 uint16_t;
typedef INT32 int32_t;
typedef UINT32 uint32_t;
typedef INT64 int64_t;
typedef UINT64 uint64_t;

typedef INT64 off_t;
typedef INT64 daddr_t;

#endif
