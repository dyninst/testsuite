#pragma once

#if defined __cplusplus
#  include <atomic>
#  define testsuite_atomic(T, name, value) static std::atomic<T> name{value};
#else
#  include <stdatomic.h>
#  define testsuite_atomic(T, name, value) _Atomic T name = value;
#endif
