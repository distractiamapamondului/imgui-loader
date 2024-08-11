#ifndef stdafx_hpp
#define stdafx_hpp

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include<math.h>
#include <Windows.h>

#include <iostream>
#include <thread>
#include <cmath>
#include <limits>
#include <string>
#include <sstream>
#include <vector>

#include <d3d9.h>                         
#include <d3dx9.h>

#define IMGUI_DEFINE_MATH_OPERATORS

#include "Inject\injectSK.hpp"
//#include "Inject/injectGH.hpp"
#include "Cleaner\StringRemove.hpp"

#include "Cleaner\Cleaner.hpp"
#include "Cleaner\Delappdata.hpp"
#include "Cleaner\Deljson.hpp"
#include "Cleaner\Finaly.hpp"
#include "Cleaner\Type.hpp"

#include "Protection\xorstr.hpp"

#include "ext\imgui\imgui.h"
#include "ext\imgui\imgui_freetype.h"
#include "ext\imgui\imgui_impl_dx9.h"
#include "ext\imgui\imgui_impl_win32.h"
#include "ext\imgui\imgui_internal.h"
#include "ext\imgui\imgui_stdlib.h"
#include "ext\imgui\imgui_custom.h"
#include "ext\imgui\imspinner.h"

#endif