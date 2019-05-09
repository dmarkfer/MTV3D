#pragma once

#include "targetver.h"

#include "Resource.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <CommCtrl.h>
#pragma comment(lib, "comctl32.lib")
#include <commdlg.h>


#include <tchar.h>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <utility>
#include <thread>
#include <mutex>


#define BLACK		RGB(0, 0, 0)
#define DARK_GRAY	RGB(35, 35, 35)
#define THEME_BLUE	RGB(0, 162, 232)
#define WHITE		RGB(255, 255, 255)

#define SPLASH_DIM 384 // 768/2
#define SPLASH_TTL 5000 // ms

#define WCHAR_ARR_MAX 256
#define FILEPATH_MAX_LENGTH 1001

#define WM_THREAD_DONE		WM_USER + 1
