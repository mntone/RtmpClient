#pragma once

#include <SDKDDKVer.h>
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#endif

// STL Headers
#include <memory>
#include <queue>
#include <vector>
#include <sstream>
#include <string>
#include <map>

// Windows Header Files:
#include <windows.h>
#include <collection.h>
#include <ppltasks.h>

using namespace concurrency;

// This Project Header Files:
#include "Utilities.h"
