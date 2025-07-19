#pragma once

#if defined(_MSC_VER)
#define EXPORT extern "C" __declspec(dllexport)
#elif defined(__GNUC__)
#define EXPORT extern "C"
#endif

#include <vcmp.h>
#include <squirrel.h>

#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <cstdarg>
#include <cstdio> // For vsnprintf
#include <string> // For std::string

#define PLUGIN_NAME "SqRequest"

// A definition needed for Squirrel's print function
#ifdef SQUNICODE
#define scvprintf vwprintf
#else
#define scvprintf vprintf
#endif

namespace SqRequest
{
	void OutputDebug(const char *format, ...);
	void OutputMsg(const char *format, ...);
	void OutputErr(const char *format, ...);
	void OutputWarn(const char *format, ...);
} // Namespace - SqRequest