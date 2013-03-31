#using <System.dll>
#include "StringToANSI.hpp"

using namespace System;
using namespace System::Runtime::InteropServices;

namespace NSass
{
	char* MarshalString(String^ s)
	{
		return (char*) ((Marshal::StringToHGlobalAnsi (s)).ToPointer());
	}

	void FreeString(char* p)
	{
		if (p) Marshal::FreeHGlobal (IntPtr ((void *) p));
	}

	const char* MarshalConstString(String^ s)
	{
		return (const char*) ((Marshal::StringToHGlobalAnsi (s)).ToPointer());
	}

	void FreeConstString(const char* p)
	{
		if (p) Marshal::FreeHGlobal (IntPtr ((void *) p));
	}
}