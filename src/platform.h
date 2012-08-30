#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef __clang__
#       define __PURE__(decl) decl
#       define __CONST__(decl) decl
#       define __DEPRECATED__ __attribute__ ((deprecated))
#       define __SENTINEL_(decl) decl_
#       define __HOT__(decl) decl 
#       define __COLD__(decl) decl 
#       define __NONNULL__(decl,...) decl, 
#       define __NORETURN__(decl) decl 
#       define __THREADLOCAL__(decl) decl __thread
#       define __UNUSED__(decl) decl 
#       define __WARN_UNUSED__(decl) decl 
#       define __ALIGNED__(decl, size) decl		 
#       define __CONSTEXPR__ constexpr
#elif defined(__GNUC__)
#       define __PURE__(decl) decl __attribute__((pure))
#       define __CONST__(decl) decl __attribute__((const))
#       define __DEPRECATED__(decl) decl __attribute__ ((deprecated))
#       define __SENTINEL__(decl) decl __attribute__ ((sentinel))
#       define __HOT__(decl) decl __attribute__ ((hot))
#       define __COLD__(decl) decl __attribute__ ((cold))
#       define __NONNULL__(decl,...) decl __attribute__((nonnull (__VA_ARGS__)))
#       define __NORETURN__(decl) decl __attribute__((noreturn))
#       define __THREADLOCAL__(decl) decl __thread
#       define __UNUSED__(decl) decl __attribute__ ((unused))
#       define __WARN_UNUSED__(decl) decl __attribute__((warn_unused_result))
#       define __ALIGNED__(decl, size) decl __attribute__((aligned(16)))
#       define __CONSTEXPR__ constexpr
#elif defined(WIN32)
#       define __PURE__(decl) decl
#       define __CONST__(decl) decl
#       define __DEPRECATED__(decl) __declspec(deprecated) decl
#       define __SENTINEL__(decl) decl
#       define __HOT__(decl) decl
#       define __COLD__(decl) decl
#       define __NONNULL__(decl,...) decl
#       define __NORETURN__(decl) decl
#       define __THREADLOCAL__(decl) __declspec(thread) decl
#       define __UNUSED__(decl) decl
#       define __WARN_UNUSED__(decl) decl
#       define __ALIGNED__(decl, size) __declspec(align(16)) decl
#       define __CONSTEXPR__ 
#else
#       error Unknown compiler/platform
#endif

#ifdef WIN32

#	define VC_EXTRALEAN
#	define WIN32_LEAN_AND_MEAN
#	define NOMINMAX
#	define NOCOMM
#	define NOSOUND

#	include <windows.h>

/* Fuck you microsoft, sincerly - me */

#	undef near
#	undef far


#	define M_E 2.71828182845904523536
#	define M_LOG2E 1.44269504088896340736
#	define M_LOG10E 0.434294481903251827651
#	define M_LN2 0.693147180559945309417
#	define M_LN10 2.30258509299404568402
#	define M_PI 3.14159265358979323846
#	define M_PI_2 1.57079632679489661923
#	define M_PI_4 0.785398163397448309616
#	define M_1_PI 0.318309886183790671538
#	define M_2_PI 0.636619772367581343076
#	define M_1_SQRTPI 0.564189583547756286948
#	define M_2_SQRTPI 1.12837916709551257390
#	define M_SQRT2 1.41421356237309504880
#	define M_SQRT_2 0.707106781186547524401

#	include <cmath>

	inline double round(double val) { return floor((val) + 0.5); };
	
	inline bool isblank(char c) { return (c == ' ' || c == '\t'); };

	typedef size_t ssize_t;

#endif

#endif /* PLATFORM_H */
