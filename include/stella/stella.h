//
// Created by Homin Su on 2023/6/7.
//

#ifndef STELLA_INCLUDE_STELLA_STELLA_H_
#define STELLA_INCLUDE_STELLA_STELLA_H_

#ifndef STELLA_ASSERT
#include <cassert>
#define STELLA_ASSERT(x) assert(x)
#endif // STELLA_ASSERT

/**
 * @brief const array length
 */
#ifndef STELLA_LENGTH
#define STELLA_LENGTH(CONST_ARRAY) (sizeof(CONST_ARRAY) / sizeof(CONST_ARRAY[0]))
#endif // STELLA_LENGTH

/**
 * @brief const string length
 */
#ifndef STELLA_STR_LENGTH
#if defined(_MSC_VER)
#define STELLA_STR_LENGTH(CONST_STR) _countof(CONST_STR)
#else
#define STELLA_STR_LENGTH(CONST_STR) (sizeof(CONST_STR) / sizeof(CONST_STR[0]))
#endif
#endif // STELLA_STR_LENGTH

// stringification
#define STELLA_STRINGIFY(X) STELLA_DO_STRINGIFY(X)
#define STELLA_DO_STRINGIFY(X) #X

// concatenation
#define STELLA_JOIN(X, Y) STELLA_DO_JOIN(X, Y)
#define STELLA_DO_JOIN(X, Y) X##Y

/**
 * @brief adopted from Boost
 */
#define STELLA_VERSION_CODE(x,y,z) (((x)*100000) + ((y)*100) + (z))

/**
 * @brief gnuc version
 */
#if defined(__GNUC__)
#define STELLA_GNUC \
    STELLA_VERSION_CODE(__GNUC__,__GNUC_MINOR__,__GNUC_PATCHLEVEL__)
#endif

#if defined(__clang__) || (defined(STELLA_GNUC) && STELLA_GNUC >= STELLA_VERSION_CODE(4,2,0))

#define STELLA_PRAGMA(x) _Pragma(STELLA_STRINGIFY(x))
#if defined(__clang__)
#define STELLA_DIAG_PRAGMA(x) STELLA_PRAGMA(clang diagnostic x)
#else
#define STELLA_DIAG_PRAGMA(x) STELLA_PRAGMA(GCC diagnostic x)
#endif
#define STELLA_DIAG_OFF(x) STELLA_DIAG_PRAGMA(ignored STELLA_STRINGIFY(STELLA_JOIN(-W,x)))

// push/pop support in Clang and GCC>=4.6
#if defined(__clang__) || (defined(STELLA_GNUC) && STELLA_GNUC >= STELLA_VERSION_CODE(4,6,0))
#define STELLA_DIAG_PUSH STELLA_DIAG_PRAGMA(push)
#define STELLA_DIAG_POP  STELLA_DIAG_PRAGMA(pop)
#else // GCC >= 4.2, < 4.6
#define STELLA_DIAG_PUSH /* ignored */
#define STELLA_DIAG_POP /* ignored */
#endif

#elif defined(_MSC_VER)

// pragma (MSVC specific)
#define STELLA_PRAGMA(x) __pragma(x)
#define STELLA_DIAG_PRAGMA(x) STELLA_PRAGMA(warning(x))

#define STELLA_DIAG_OFF(x) STELLA_DIAG_PRAGMA(disable: x)
#define STELLA_DIAG_PUSH STELLA_DIAG_PRAGMA(push)
#define STELLA_DIAG_POP  STELLA_DIAG_PRAGMA(pop)

#else

#define STELLA_DIAG_OFF(x) /* ignored */
#define STELLA_DIAG_PUSH   /* ignored */
#define STELLA_DIAG_POP    /* ignored */

#endif

#endif //STELLA_INCLUDE_STELLA_STELLA_H_
