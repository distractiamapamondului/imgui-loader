#pragma once

#include "imgui.h"
#include "imgui_internal.h"

#include <unordered_map>
#include <map>
#include <string>
#include <algorithm>
#include <cctype>

enum c_InformationType {
	Status_Unstable = 0,
	Status_Undetectable = 1,
	Subscription = 2,
};

namespace ImGui {
	IMGUI_API bool Header(std::string title);

	IMGUI_API void MenuChild(const char* name, ImVec2 size);
	IMGUI_API void EndMenuChild();

	IMGUI_API bool InsertCheat(const char* IconAwesome, const char* label, const char* value, bool selected);
	IMGUI_API bool CheatInformation(const char* IconAwesome, const char* label, const char* value, c_InformationType InformationType);

	IMGUI_API bool MinimizeButton(const ImVec2& pos);
	IMGUI_API bool CloseButton(const ImVec2& pos);
};
