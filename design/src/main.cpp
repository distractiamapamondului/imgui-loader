#include "stdafx.hpp"
#include "menu\menu.hpp"

int __stdcall wWinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow )
{
    const auto window_ { std::make_unique<menu::c_window>( ) };
    const auto render_ { std::make_unique<menu::c_render>( ) };

    menu::context_t ctx {};

    if ( window_->create( " ", { 500, 350 }, ctx, render_->menu_style ) )
    {
        window_->render( ctx, { 500, 350 }, render_->menu_panel );
    }

    return EXIT_SUCCESS;
}
