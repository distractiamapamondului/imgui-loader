#include "stdafx.hpp"

#include "imgui_custom.h"

bool ImGui::Header(std::string title)
{
	const ImVec2 Position = ImGui::GetWindowPos();
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	ImDrawList* draw = ImGui::GetWindowDrawList();

	ImVec2 LocationRectFilled = ImVec2(ImGui::GetWindowSize().x, 50);
	draw->AddRectFilled(Position, LocationRectFilled, ImGui::GetColorU32(ImGuiCol_HeaderBg), ImGui::GetStyle().WindowRounding, ImDrawFlags_RoundCornersTop);

	ImVec2 LocationText = ImVec2(Position.x + 20, Position.y + 15);

	std::transform(title.begin(), title.end(), title.begin(), ::toupper);
	draw->AddText(ImGui::GetIO().Fonts->Fonts[2], 20.0f, LocationText, ImGui::GetColorU32(ImGuiCol_Color), title.c_str());

	draw->AddLine(Position + ImVec2(0, LocationRectFilled.y), LocationRectFilled, ImGui::GetColorU32(ImGuiCol_Border), 2.f);

	if (ImGui::MinimizeButton(ImVec2(ImGui::GetWindowSize().x - 83, 19))) {
		ShowWindow(GetActiveWindow(), SW_MINIMIZE);
	}
	bool closeButton = ImGui::CloseButton(ImVec2(ImGui::GetWindowSize().x - 50, 15));

	return closeButton;
}


void ImGui::MenuChild(const char* name, ImVec2 size) {
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = ImGui::GetCurrentWindow();

	ImVec2 Position = window->DC.CursorPos;

	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_MenuChildHeaderBg));
	ImGui::BeginChild(std::string(name).append("main").c_str(), ImVec2(size.x, size.y), ImGuiChildFlags_Border, ImGuiWindowFlags_NoScrollbar);
	ImGui::GetWindowDrawList()->AddText(ImGui::GetIO().Fonts->Fonts[2], ImGui::GetIO().Fonts->Fonts[2]->FontSize, { Position.x + 15, Position.y + 8 }, ImGui::GetColorU32(ImGuiCol_Color), name);
	ImGui::SetCursorPosY(35);
	ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_MenuChildBg));
	ImGui::BeginChild(name, ImVec2({ size.x, size.y == 0 ? size.y : size.y - 35 }), ImGuiChildFlags_Border);
	ImGui::SetCursorPosX(15);

	ImGui::BeginGroup();

}

void ImGui::EndMenuChild() {
	ImGui::EndGroup();
	ImGui::EndChild();
	ImGui::PopStyleColor();
	ImGui::EndChild();
	ImGui::PopStyleColor();
}

struct cheat_state {
	ImVec4 background;
	ImVec4 text;
};

bool ImGui::InsertCheat(const char* IconAwesome, const char* label, const char* value, bool selected) {
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(std::string(label).append("insertCheat").c_str());
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = CalcItemSize(ImVec2(ImGui::GetWindowWidth(), 50), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, 0);

	static std::map<ImGuiID, cheat_state> anim;
	auto it_anim = anim.find(id);

	if (it_anim == anim.end())
	{
		anim.insert({ id, cheat_state() });
		it_anim = anim.find(id);
	}

	it_anim->second.background = ImLerp(it_anim->second.background, IsItemActive() || hovered ? ImColor(87, 83, 78, 45) : ImColor(6, 4, 6), g.IO.DeltaTime * 6.f);
	it_anim->second.text = ImLerp(it_anim->second.text, IsItemActive() || selected ? ImColor(79, 63, 147) : ImColor(126, 125, 126), g.IO.DeltaTime * 6.f);

	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(it_anim->second.background), true);

	ImGui::GetWindowDrawList()->AddText(io.Fonts->Fonts[8], 17.0f, ImVec2(bb.Min.x + 20, bb.Min.y + (bb.GetHeight() / 2) - (ImGui::CalcTextSize(IconAwesome).y / 3.f)), ImGui::ColorConvertFloat4ToU32(it_anim->second.text), IconAwesome);
	ImGui::GetWindowDrawList()->AddText(io.Fonts->Fonts[2], 18.0f, ImVec2(bb.Min.x + 45, bb.Min.y + (bb.GetHeight() / 2) - (ImGui::CalcTextSize(label).y / 2)), ImGui::ColorConvertFloat4ToU32(it_anim->second.text), label);
	ImGui::GetWindowDrawList()->AddText(io.Fonts->Fonts[2], 18.0f, ImVec2(bb.GetWidth() - ImGui::CalcTextSize(value).x, bb.Min.y + (bb.GetHeight() / 2) - (ImGui::CalcTextSize(label).y / 2)), ImGui::ColorConvertFloat4ToU32(it_anim->second.text), value);

	return pressed;
}

struct information_state {
	ImVec4 background;
};

bool ImGui::CheatInformation(const char* IconAwesome, const char* label, const char* value, c_InformationType InformationType) {
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(std::string(label).append("CheatInformation").c_str());
	const ImVec2 label_size = CalcTextSize(label, NULL, true);
	ImGuiIO& io = ImGui::GetIO();

	ImVec2 pos = window->DC.CursorPos;
	ImVec2 size = CalcItemSize(ImVec2(ImGui::GetWindowWidth() - 40, 50), label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect bb(pos, pos + size);
	ItemSize(size, style.FramePadding.y);
	if (!ItemAdd(bb, id))
		return false;

	bool hovered, held;
	bool pressed = ButtonBehavior(bb, id, &hovered, &held, 0);

	static std::map<ImGuiID, information_state> anim;
	auto it_anim = anim.find(id);

	if (it_anim == anim.end())
	{
		anim.insert({ id, information_state() });
		it_anim = anim.find(id);
	}

	it_anim->second.background = ImLerp(it_anim->second.background, IsItemActive() || hovered ? ImColor(10, 9, 10) : ImColor(9, 7, 9), g.IO.DeltaTime * 6.f);

	RenderNavHighlight(bb, id);
	RenderFrame(bb.Min, bb.Max, ImGui::ColorConvertFloat4ToU32(it_anim->second.background), true, ImGui::GetStyle().FrameRounding);

	ImGui::GetWindowDrawList()->AddText(io.Fonts->Fonts[8], 17.0f, ImVec2(bb.Min.x + 10, bb.Min.y + (bb.GetHeight() / 2) - (ImGui::CalcTextSize(IconAwesome).y / 3.f)), ImColor(79, 63, 147), IconAwesome);
	ImGui::GetWindowDrawList()->AddText(io.Fonts->Fonts[2], 18.0f, ImVec2(bb.Min.x + 40, bb.Min.y + (bb.GetHeight() / 2) - (ImGui::CalcTextSize(label).y / 2)), ImColor(79, 63, 147), label);

	ImColor FillColor = ImColor(126, 125, 126);

	if (InformationType == Subscription) {
		FillColor = ImColor(17, 48, 42);
	}
	else if (InformationType == Status_Undetectable) {
		FillColor = ImColor(17, 48, 42);
	}
	else if (InformationType == Status_Unstable) {
		FillColor = ImColor(53, 15, 18);
	}

	ImVec2 Position(bb.Max.x - ImGui::CalcTextSize(value).x - 30, bb.Min.y + 10);
	ImVec2 textSize = ImGui::CalcTextSize(value);

	ImGui::GetWindowDrawList()->AddRectFilled(Position, ImVec2(Position.x + textSize.x + 20, bb.Max.y - 10), FillColor, ImGui::GetStyle().FrameRounding / 2);
	ImGui::GetWindowDrawList()->AddText(io.Fonts->Fonts[2], 18.0f, ImVec2(bb.Max.x - ImGui::CalcTextSize(value).x - 25, bb.Min.y + (bb.GetHeight() / 2) - (ImGui::CalcTextSize(value).y / 2)), ImColor(126, 125, 126), value);

	return pressed;
}


struct minimize_button {
	float close_opacity;
};

bool ImGui::MinimizeButton(const ImVec2& pos)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	const ImGuiID id = window->GetID(std::string("minimizeButton").append("unclass").c_str());

	const ImRect bb(pos, pos + ImVec2(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
	ImRect bb_interact = bb;
	const float area_to_visible_ratio = window->OuterRectClipped.GetArea() / bb.GetArea();
	if (area_to_visible_ratio < 1.5f)
		bb_interact.Expand(ImFloor(bb_interact.GetSize() * -0.25f));


	bool is_clipped = !ItemAdd(bb_interact, id);

	bool hovered, held;
	bool pressed = ButtonBehavior(bb_interact, id, &hovered, &held);
	if (is_clipped)
		return pressed;

	ImVec2 center = bb.GetCenter();

	if (hovered)
		ImGui::SetMouseCursor(7);

	static std::map <ImGuiID, minimize_button> anim;
	auto it_anim = anim.find(id);
	if (it_anim == anim.end()) {
		anim.insert({ id, { 0.0f } });
		it_anim = anim.find(id);
	}

	it_anim->second.close_opacity = ImLerp(it_anim->second.close_opacity, (hovered ? 1.f : 0.5f), 0.07f * (1.0f - ImGui::GetIO().DeltaTime));

	float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;
	center -= ImVec2(0.5f, 0.5f);

	window->DrawList->AddLine(center + ImVec2(cross_extent + 10, cross_extent - 5), center + ImVec2(cross_extent, cross_extent - 5), ImColor(1.0f, 1.0f, 1.0f, it_anim->second.close_opacity), 1.0f);

	return pressed;
}

struct close_button {
	float close_opacity;
};


bool ImGui::CloseButton(const ImVec2& pos)
{
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	const ImGuiID id = window->GetID(std::string("closeButton").append("unclass").c_str());

	const ImRect bb(pos, pos + ImVec2(g.FontSize, g.FontSize) + g.Style.FramePadding * 2.0f);
	ImRect bb_interact = bb;
	const float area_to_visible_ratio = window->OuterRectClipped.GetArea() / bb.GetArea();
	if (area_to_visible_ratio < 1.5f)
		bb_interact.Expand(ImFloor(bb_interact.GetSize() * -0.25f));


	bool is_clipped = !ItemAdd(bb_interact, id);

	bool hovered, held;
	bool pressed = ButtonBehavior(bb_interact, id, &hovered, &held);
	if (is_clipped)
		return pressed;

	ImVec2 center = bb.GetCenter();

	if (hovered)
		ImGui::SetMouseCursor(7);

	static std::map <ImGuiID, close_button> anim;
	auto it_anim = anim.find(id);
	if (it_anim == anim.end()) {
		anim.insert({ id, { 0.0f } });
		it_anim = anim.find(id);
	}

	it_anim->second.close_opacity = ImLerp(it_anim->second.close_opacity, (hovered ? 1.f : 0.5f), 0.07f * (1.0f - ImGui::GetIO().DeltaTime));

	float cross_extent = g.FontSize * 0.5f * 0.7071f - 1.0f;
	center -= ImVec2(0.5f, 0.5f);

	window->DrawList->AddLine(center + ImVec2(+cross_extent, +cross_extent), center + ImVec2(-cross_extent, -cross_extent), ImColor(1.0f, 1.0f, 1.0f, it_anim->second.close_opacity), 1.0f);
	window->DrawList->AddLine(center + ImVec2(+cross_extent, -cross_extent), center + ImVec2(-cross_extent, +cross_extent), ImColor(1.0f, 1.0f, 1.0f, it_anim->second.close_opacity), 1.0f);

	return pressed;
}