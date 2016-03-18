﻿/*
 * Copyright (c) 2016
 * Geco Gaming Company
 *
 * Permission to use, copy, modify, distribute and sell this software
 * and its documentation for GECO purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation. Geco Gaming makes no
 * representations about the suitability of this software for GECO
 * purpose.  It is provided "as is" without express or implied warranty.
 *
 */

#ifndef __INCLUDE_GECO_EXPORT_H
#define __INCLUDE_GECO_EXPORT_H

//#include"DefaultNetDefines.h"

#if defined(_WIN32) && !(defined(__GNUC__)  || defined(__GCCXML__)) \
      && !defined(GECO_HAS_STATIC_LIB) && defined(GECO_HAS_DYNAMIC_LIB)
#define GECO_EXPORT __declspec(dllexport)
#else
#define GECO_EXPORT
#endif

#define GECO_STATIC_FACTORY_DECL(x) \
static x* GetInstance(void); \
static void DestroyInstance( x *i);

#define GECO_STATIC_FACTORY_DEFI(x,y) \
x* x::GetInstance(void) {return JACKIE_INET::OP_NEW<y>( TRACE_FILE_AND_LINE_ );} \
void x::DestroyInstance( x *i) {JACKIE_INET::OP_DELETE(( y* ) i, TRACE_FILE_AND_LINE_);}
#endif
