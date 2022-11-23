#pragma once
//#define _DEBUG

#ifdef _DEBUG

#define DebugArea(...) __VA_ARGS__

#else

#define DebugArea(...)
#endif

#define ASSERT(...) _ASSERT(__VA_ARGS__)