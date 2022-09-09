﻿#pragma once

#include <atlstr.h>
#include <clocale>
#include <cstdint>
#include <cstdio>
#include <io.h>
#include <string>
#include <Tlhelp32.h>
#include <vector>
#include <windows.h>
#include "shlobj.h"

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8 s8;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef uint8 u8;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;
typedef u8 byte;

extern bool isRun;
extern HANDLE DBGVIEW;
