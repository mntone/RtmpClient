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

#include <wrl/client.h>
#include <wrl/implements.h>
#include <wrl/wrappers/corewrappers.h>
#include <wrl/event.h>

using namespace Concurrency;

// This Project Header Files:
#include "Utilities.h"
