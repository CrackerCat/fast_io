#pragma once
/*
https://en.cppreference.com/w/cpp/freestanding
There are two kinds of implementations defined by the C++ standard:
hosted and freestanding implementations.
For hosted implementations the set of standard library headers required by the C++ standard is much larger than for freestanding ones.
*/
//fast_io_hosted defines what we could use in a hosted environment.

#if !defined(__cplusplus)
#error "You are not using C++ compiler"
#endif

#if defined(__GNUC__) && __GNUC__>=11 && __cplusplus<202002L
#error "fast_io requires at least C++20 standard compiler."
#else
#include"fast_io_freestanding.h"
#if __STDC_HOSTED__==1 && (!defined(_GLIBCXX_HOSTED) || _GLIBCXX_HOSTED==1)
#include<cerrno>

#include"fast_io_hosted/posix_error_scatter.h"
#include"fast_io_hosted/posix_error.h"
#ifdef __MSDOS__
#undef __STRICT_ANSI__
#endif
#include<ctime>
#include"fast_io_hosted/timespec.h"

#include"fast_io_hosted/io_io_file.h"
#include"fast_io_hosted/cstring_view.h"

#include"fast_io_hosted/mmap.h"
#include"fast_io_hosted/posix_status.h"
#include"fast_io_hosted/platforms/native.h"
#include"fast_io_hosted/file_loaders/impl.h"

#include"fast_io_hosted/time.h"

#include"fast_io_hosted/threads/mutex/impl.h"
#include"fast_io_hosted/iomutex.h"
#include"fast_io_hosted/wrapper.h"
#include"fast_io_hosted/filesystem/native.h"
#include"fast_io_hosted/dll/dll.h"
#include"fast_io_hosted/process_revamp/native.h"

#include"fast_io_hosted/white_hole/white_hole.h"
//#include<memory_resource>
//#include"fast_io_hosted/pmr_resource.h"


#if defined(_WIN32) || defined(__CYGWIN__)
#include"fast_io_hosted/box.h"
#endif


#if defined(_GLIBCXX_STRING) || defined(_LIBCPP_STRING) || defined(_STRING_)
#include"fast_io_unit/string.h"
#endif

#if defined(_GLIBCXX_CHRONO) || defined(_LIBCPP_CHRONO) || defined(_CHRONO_)
#include"fast_io_unit/chrono.h"
#endif

#if defined(_GLIBCXX_COMPLEX) || defined(_LIBCPP_COMPLEX) || defined(_COMPLEX_)
#include"fast_io_unit/complex.h"
#endif

#endif

#endif