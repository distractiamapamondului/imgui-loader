#pragma once

using namespace ImGui;

namespace custom
{
    bool Checkbox( const char *label, bool *v )
    {
        ImGuiWindow *window = GetCurrentWindow( );
        if ( window->SkipItems )
            return false;

        ImGuiContext &g = *GImGui;
        const ImGuiStyle &style = g.Style;
        const ImGuiID id = window->GetID( label );
        const ImVec2 label_size = CalcTextSize( label, NULL, true );

        const float square_sz = 21.5f;
        ImVec2 pos = window->DC.CursorPos;

        const ImRect total_bb( pos, pos + ImVec2( square_sz + ( label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f ), label_size.y + style.FramePadding.y * 2.0f ) );
        ItemSize( total_bb, style.FramePadding.y );
        if ( !ItemAdd( total_bb, id ) )
            return false;

        bool hovered, held;
        bool pressed = ButtonBehavior( total_bb, id, &hovered, &held );
        if ( pressed )
        {
            *v = !( *v );
            MarkItemEdited( id );
        }

        const ImRect check_bb( pos, ImVec2( pos.x, pos.y ) + ImVec2( square_sz * 1.5, square_sz * 0.8 ) );
        RenderNavHighlight( total_bb, id );

        auto col = ImColor( 138, 43, 226 );

        ImVec2 UpperLeft( check_bb.GetTL( ).x, check_bb.GetTL( ).y + 2.f );
        ImVec2 LowerRight( check_bb.GetBR( ).x, check_bb.GetBR( ).y + 2.f );
        ImRect newRect( UpperLeft, LowerRight );
        window->DrawList->AddRectFilled( newRect.Min, newRect.Max, ImColor( 52, 61, 70 ), 10 );

        ImVec2 sUpperLeft( check_bb.GetTL( ).x, check_bb.GetTL( ).y + 2.f );
        ImVec2 sLowerRight( check_bb.GetCenter( ).x, check_bb.GetBR( ).y + 2.f );
        ImRect sRect( sUpperLeft, sLowerRight );
        window->DrawList->AddRectFilled( sRect.Min, sRect.Max, ImColor( 255, 255, 255 ), 10 );

        ImU32 check_col = ImColor( 138, 43, 226 );
        if ( *v )
        {
            window->DrawList->AddRectFilled( ImVec2( newRect.Min.x - 1.f, newRect.Min.y - 1.f ), ImVec2( newRect.Max.x + .5f, newRect.Max.y + 1.f ), ImColor( 80, 80, 80 ), 10 );

            ImVec2 tUpperLeft( check_bb.GetCenter( ).x, check_bb.GetTL( ).y + 2.f );
            ImVec2 tLowerRight( check_bb.GetBR( ).x, check_bb.GetBR( ).y + 2.f );
            ImRect tRect( tUpperLeft, tLowerRight );
            window->DrawList->AddRectFilled( tRect.Min, tRect.Max, ImColor( 255, 255, 255 ), 10 );
        }

        if ( label_size.x > 0.0f )
            RenderText( ImVec2( check_bb.Max.x + style.ItemInnerSpacing.x, check_bb.Min.y + style.FramePadding.y ), label );

        IMGUI_TEST_ENGINE_ITEM_INFO( id, label, window->DC.ItemFlags | ImGuiItemStatusFlags_Checkable | ( *v ? ImGuiItemStatusFlags_Checked : 0 ) );
        return pressed;
    }
}