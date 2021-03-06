/*
 * Copyright (C) Tildeslash Ltd. All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 *
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.
 */


#ifndef ZDB_INCLUDED
#define ZDB_INCLUDED
#ifdef __cplusplus
extern "C" {
#endif

/**
 * Include this interface in your C code to import the libzdb API.
 * This meta interface also wraps libzdb API interface files in an
 * 'extern "C"' block to allow for including libzdb in a C++ or in
 * a Objective-C++ project.
 *
 * @file
 */

/* Mask out __attribute__ extension for non- GCC/llvm-clang compilers. */
#if (! (defined(__GNUC__) || defined(__clang__)))
#define __attribute__(x)
#endif

#if defined(_MSC_VER) && !defined(inline)
#define inline __inline
#endif

#if defined(_MSC_VER) && !defined(__func__)
#define __func__ __FUNCTION__
#endif

/* libzdb API interfaces */
#include "exceptions/SQLException.h"
#include "net/URL.h"
#include "db/ResultSet.h"
#include "db/PreparedStatement.h"
#include "db/Connection.h"
#include "db/ConnectionPool.h"

#ifdef __cplusplus
}
#endif
#endif
