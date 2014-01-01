#pragma once

#define WIN32_LEAN_AND_MEAN

// STL Headers:
#include <memory>
#include <queue>
#include <vector>
#include <sstream>
#include <string>
#include <unordered_map>
#include <limits>
#include <queue>
#include <mutex>
#include <condition_variable>

// Windows Headers:
#define WIN32_LEAN_AND_MEAN
#define NOSERVICE
#define NOMCX
#define NOIME
#define NOMINMAX
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN
#undef NOSERVICE
#undef NOMCX
#undef NOIME
#undef NOMINMAX
#include <collection.h>
#include <ppltasks.h>

// This Project Headers:
#include "utility.h"
