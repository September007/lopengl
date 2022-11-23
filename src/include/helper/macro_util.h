#pragma once


#ifdef Debug

#define DebugArea(...) __VA_ARGS__

#else

#define DebugArea(...)
#endif

#define ASSERT(...)  assert(__VA_ARGS__)//_ASSERT(__VA_ARGS__)