#include "main.hpp"

#if defined(WIN32) || defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#include "SqImports.h"
#include "includes.hpp"
#include "request.hpp"

// Definitions
HSQAPI sq;
HSQUIRRELVM v;

// Global variables (meh)
PluginFuncs *gFuncs;

using namespace Sqrat;
using namespace SqRequest;

static uint8_t OnServerInitialise()
{
	printf("\n");
	OutputMsg("Loaded SqRequest module for VC:MP 0.4 by Umar ([RT]UmaR^).");
	return 1;
}

static uint8_t BindPlugin()
{
	size_t size;
	int sqId = gFuncs->FindPlugin("SQHost2");

	if (sqId < 0)
	{
		OutputErr("Unable to find Squirrel module. " PLUGIN_NAME " definitions will be unavailable.");
		return 0;
	}
	const void **sqExports = gFuncs->GetPluginExports(sqId, &size);

	if (sqExports != NULL && size > 0)
	{

		SquirrelImports **sqDerefFuncs = (SquirrelImports **)sqExports;

		// Now let's change that to a SquirrelImports pointer
		SquirrelImports *sqFuncs = (SquirrelImports *)(*sqDerefFuncs);

		// Now we get the virtual machine
		if (sqFuncs)
		{
			// Get a pointer to the VM and API
			sq = *(sqFuncs->GetSquirrelAPI());
			v = *(sqFuncs->GetSquirrelVM());

			ErrorHandling::Enable(true);
			DefaultVM::Set(v);
			Table reqcn(v);
			SqRequest::Request::Register_SqRequest(reqcn);

			RootTable(v).Bind(_SC("SqRequest"), reqcn);
			return 1;
		}
	}
	else
	{
		OutputErr("The Squirrel module provided an invalid SquirrelImports structure. " PLUGIN_NAME " definitions will be unavailable.");
	}
	return 0;
}

static uint8_t OnPluginCommand(uint32_t commandIdentifier, const char *message)
{
	switch (commandIdentifier)
	{
	case 0x7D6E22D8: // Squirrel scripts load
		BindPlugin();
	default:
		return 1;
	}
}

void OnServerFrame(float)
{
	SqRequest::Request::Process();
}

EXPORT unsigned int VcmpPluginInit(PluginFuncs *functions, PluginCallbacks *callbacks, PluginInfo *info)
{
	// Set our plugin information
	info->pluginVersion = 0x100; // 1.0.0
	info->apiMajorVersion = PLUGIN_API_MAJOR;
	info->apiMinorVersion = PLUGIN_API_MINOR;
	sprintf(info->name, "%s", "Request Module for VC:MP");

	// Store functions for later use
	gFuncs = functions;

	// Store callback
	callbacks->OnServerInitialise = OnServerInitialise;
	callbacks->OnPluginCommand = OnPluginCommand;
	callbacks->OnServerFrame = OnServerFrame;

	// Done!
	return 1;
}

namespace SqRequest
{
	// Helper: FormatString using va_list (called from OutputMsg, OutputDebug, etc.)
	inline std::string FormatStringV(const char *format, va_list args)
	{
		va_list args_copy;
		va_copy(args_copy, args);
		int size = vsnprintf(nullptr, 0, format, args_copy) + 1;
		va_end(args_copy);

		std::vector<char> buf(size);
		vsnprintf(buf.data(), size, format, args);
		return std::string(buf.data());
	}

	// Helper: FormatString with variadic args
	inline std::string FormatString(const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		std::string result = FormatStringV(format, args);
		va_end(args);
		return result;
	}

	void OutputDebug(const char *format, ...)
	{
#ifdef _DEBUG
		va_list args;
		va_start(args, format);
		std::string msg = FormatStringV(format, args);
		va_end(args);
		OutputMsg("%s", msg.c_str());
#endif
	}

	void OutputMsg(const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		std::string msg = FormatStringV(format, args);
		va_end(args);

#if defined(WIN32) || defined(_WIN32)
		HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbBefore;
		GetConsoleScreenBufferInfo(hstdout, &csbBefore);

		SetConsoleTextAttribute(hstdout, FOREGROUND_GREEN);
		printf("[SqRequests]  ");

		SetConsoleTextAttribute(hstdout, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		printf("%s\n", msg.c_str());

		SetConsoleTextAttribute(hstdout, csbBefore.wAttributes);
#else
		printf("\033[0;32m[SqRequests]\033[0;37m %s\n", msg.c_str());
#endif
	}

	void OutputErr(const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		std::string msg = FormatStringV(format, args);
		va_end(args);

#if defined(WIN32) || defined(_WIN32)
		HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbBefore;
		GetConsoleScreenBufferInfo(hstdout, &csbBefore);

		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_INTENSITY);
		printf("[SqRequests]   ");

		SetConsoleTextAttribute(hstdout, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		printf("%s\n", msg.c_str());

		SetConsoleTextAttribute(hstdout, csbBefore.wAttributes);
#else
		printf("\033[0;31m[ERROR]\033[0;37m %s\n", msg.c_str());
#endif
	}

	void OutputWarn(const char *format, ...)
	{
		va_list args;
		va_start(args, format);
		std::string msg = FormatStringV(format, args);
		va_end(args);

#if defined(WIN32) || defined(_WIN32)
		HANDLE hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
		CONSOLE_SCREEN_BUFFER_INFO csbBefore;
		GetConsoleScreenBufferInfo(hstdout, &csbBefore);

		SetConsoleTextAttribute(hstdout, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
		printf("[SqRequests] ");

		SetConsoleTextAttribute(hstdout, FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		printf("%s\n", msg.c_str());

		SetConsoleTextAttribute(hstdout, csbBefore.wAttributes);
#else
		printf("\033[1;33m[WARNING]\033[0;37m %s\n", msg.c_str());
#endif
	}
} // namespace SqRequest
