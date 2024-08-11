#include "stdafx.hpp"
#include "menu.hpp"

#include "custom.hpp"
#include "ext\resources\font\poppins.hpp"
#include "ext\resources\image\image.hpp"
#include "ext\particle\AnimVector.h"

#include "Inject/injectSk.hpp"
#include "./xor.hpp"

namespace menu
{
	auto c_window::wnd_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) -> LRESULT WINAPI
	{
		if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wparam, lparam))
			return true;

		switch (message)
		{
		case WM_QUIT:
			PostQuitMessage(0);
			break;

		case WM_NCHITTEST:
			RECT rect{};
			GetWindowRect(hwnd, &rect);

			POINT current_pos{};
			GetCursorPos(&current_pos);

			auto hit = DefWindowProcA(hwnd, message, wparam, lparam);
			if (hit == HTCLIENT && (current_pos.y < (rect.top + 15)))
				return HTCAPTION;

			break;
		}

		return DefWindowProcA(hwnd, message, wparam, lparam);
	}

	


	auto c_window::create(const std::string w_name, const ImVec2 w_size, context_t& w_context, const std::function<void(context_t&)> w_styles) -> bool
	{
		try
		{
			const auto init_centered = [](HWND hwnd)
				{
					RECT rc;
					GetWindowRect(hwnd, &rc);

					auto size = std::make_pair(GetSystemMetrics(SM_CXSCREEN) - rc.right, GetSystemMetrics(SM_CYSCREEN) - rc.bottom);

					SetWindowPos(hwnd, nullptr, size.first / 2, size.second / 2, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				};

			const auto create_device = [&]()
				{
					w_context.d3d = Direct3DCreate9(D3D_SDK_VERSION);
					if (w_context.d3d == nullptr)
						return false;

					std::memset(&w_context.params, 0, sizeof(w_context.params));

					w_context.params.Windowed = true;
					w_context.params.SwapEffect = D3DSWAPEFFECT_DISCARD;
					w_context.params.BackBufferFormat = D3DFMT_UNKNOWN;
					w_context.params.EnableAutoDepthStencil = true;
					w_context.params.AutoDepthStencilFormat = D3DFMT_D16;
					w_context.params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

					if (w_context.d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, w_context.hwnd, 0x00000040L, &w_context.params, &w_context.device) < 0)
						return false;

					return true;
				};

			w_context.wc =
			{
				sizeof(WNDCLASSEXA),
				CS_CLASSDC,
				(WNDPROC)wnd_proc,
				0L,
				0L,
				GetModuleHandleA(nullptr),
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				w_name.data(),
				nullptr
			};

			RegisterClassExA(&w_context.wc);

			w_context.hwnd = CreateWindowExA(0, w_context.wc.lpszClassName, w_context.wc.lpszClassName,
				WS_POPUP, 100, 100, w_size.x, w_size.y,
				nullptr, nullptr, w_context.wc.hInstance, nullptr);

			if (w_context.hwnd == nullptr)
				throw std::exception("error while creating the window");

			if (!create_device())
				throw std::exception("cannot create device!");

			init_centered(w_context.hwnd);;

			ShowWindow(w_context.hwnd, SW_SHOWDEFAULT);
			UpdateWindow(w_context.hwnd);

			ImGui::CreateContext();

			w_styles(w_context);

			ImGui_ImplWin32_Init(w_context.hwnd);
			ImGui_ImplDX9_Init(w_context.device);

			return true;
		}
		catch (std::exception& e)
		{
			cleanup_device(w_context);

			MessageBoxA(nullptr, e.what(), nullptr, MB_OK | MB_ICONERROR);
			return false;
		}

		return false;
	}

	auto c_window::render(context_t& w_context, const ImVec2 w_size, const std::function<void(context_t&, const ImVec2)> menu) -> void
	{
		MSG msg;
		std::memset(&msg, 0, sizeof(msg));

		while (msg.message != WM_QUIT)
		{
			if (PeekMessageA(&msg, nullptr, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
				continue;
			}

			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			menu(w_context, w_size);

			ImGui::EndFrame();

			w_context.device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, 0, 1.0f, 0);

			if (!w_context.context_state)
			{
				dot_destroy();
				msg.message = WM_QUIT;
			}

			if (w_context.device->BeginScene() >= 0)
			{
				ImGui::Render();
				ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
				w_context.device->EndScene();
			}

			if (w_context.device->Present(nullptr, nullptr, nullptr, nullptr) == D3DERR_DEVICELOST && w_context.device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			{
				ImGui_ImplDX9_InvalidateDeviceObjects();
				w_context.device->Reset(&w_context.params);
				ImGui_ImplDX9_CreateDeviceObjects();
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}

		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		cleanup_device(w_context);

		DestroyWindow(w_context.hwnd);
		UnregisterClassA(w_context.wc.lpszClassName, w_context.wc.hInstance);

		TerminateProcess(GetCurrentProcess(), 0);
	}

	auto c_render::menu_style(context_t& w_context) -> void
	{
		ImGui::StyleColorsDark();

		static auto& io = ImGui::GetIO();
		static auto& style = ImGui::GetStyle();

		poppins = io.Fonts->AddFontFromMemoryCompressedBase85TTF(fonts::poppins_regular.data(), 18);
		poppins_bd = io.Fonts->AddFontFromMemoryCompressedBase85TTF(fonts::poppins_bold.data(), 22);

		io.LogFilename = nullptr;
		io.IniFilename = nullptr;

		style.Colors[ImGuiCol_WindowBg] = ImColor(10, 10, 10);
		style.Colors[ImGuiCol_FrameBg] = ImColor(15, 15, 15);

		style.Colors[ImGuiCol_Button] = ImColor(25, 25, 25);
		style.Colors[ImGuiCol_ButtonHovered] = ImColor(40, 40, 40);
		style.Colors[ImGuiCol_ButtonActive] = ImColor(40, 40, 40);

		style.WindowPadding = { 0, 0 };
		style.WindowRounding = 13;
		style.WindowBorderSize = 0.0f;
		style.ChildRounding = 5;
		style.FrameRounding = 5;
	}

	auto c_render::menu_panel(context_t& w_context, const ImVec2 w_size) -> void
	{
		if (pic_world == nullptr)
			D3DXCreateTextureFromFileInMemory(w_context.device, world, sizeof(world), &pic_world);

		if (pic_clock == nullptr)
			D3DXCreateTextureFromFileInMemory(w_context.device, relogio, sizeof(relogio), &pic_clock);

		if (pic_version == nullptr)
			D3DXCreateTextureFromFileInMemory(w_context.device, version, sizeof(version), &pic_version);

		if (pic_calendary == nullptr)
			D3DXCreateTextureFromFileInMemory(w_context.device, calendary, sizeof(calendary), &pic_calendary);

		if (pic_arrow == nullptr)
			D3DXCreateTextureFromFileInMemory(w_context.device, arrow, sizeof(arrow), &pic_arrow);

		if (pic_button == nullptr)
			D3DXCreateTextureFromFileInMemory(w_context.device, p_button, sizeof(p_button), &pic_button);

		if (pic_user == nullptr)
			D3DXCreateTextureFromFileInMemory(w_context.device, p_user, sizeof(p_user), &pic_user);

		if (pic_lock == nullptr)
			D3DXCreateTextureFromFileInMemory(w_context.device, padlock, sizeof(padlock), &pic_lock);

		static bool stream_mode = false;
		SetWindowDisplayAffinity(w_context.hwnd, (stream_mode ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE));

		SetWindowLongA(w_context.hwnd, GWL_EXSTYLE, WS_EX_LAYERED);
		SetLayeredWindowAttributes(w_context.hwnd, RGB(0, 0, 0), 250, LWA_COLORKEY | LWA_ALPHA);

		if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			RECT rect;
			GetWindowRect(GetActiveWindow(), &rect);
			SetWindowPos(GetActiveWindow(), nullptr, rect.left + ImGui::GetMouseDragDelta().x, rect.top + ImGui::GetMouseDragDelta().y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		}

		static uint8_t selected_cheat = 0; // 0 = skript, 1 = gosth
		static auto flags{ ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDecoration };

		ImGui::SetNextWindowPos({ 0, 0 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(w_size, ImGuiCond_::ImGuiCond_Always);

		ImGui::Begin("###main_painel", nullptr, flags);
		{
			static auto* draw = ImGui::GetWindowDrawList();
			dot_draw();

			ImGui::PushFont(poppins);
			{
				ImGui::SetCursorPos({ w_size.x - 20, 10 });
				ImGui::Text("X");
				if (ImGui::IsItemClicked(0))
					w_context.context_state = false;
			}
			ImGui::PopFont();

			ImGui::SetCursorPos({ w_size.x - 70, 10 });
			custom::Checkbox(" ", &stream_mode);

			switch (panel)
			{
			case login_panel:
			{
				static float tab_alpha = 0.f;
				tab_alpha = ImClamp(tab_alpha + (0.5f * ImGui::GetIO().DeltaTime * 1.f), 0.f, 1.f);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * ImGui::GetStyle().Alpha);

				draw->AddImage( pic_world, { 180, 20 }, { 300, 130 } );

				draw->AddRectFilled({ 80, 130 }, { 420, 320 }, ImColor(10, 10, 10), 5);
				draw->AddRect({ 80, 130 }, { 420, 320 }, ImColor(70, 70, 70), 5);
				draw->AddLine({ 80,160 }, { 420, 160 }, ImColor(40, 40, 40));
				draw->AddText(poppins, 20, { 215, 135 }, ImColor(110, 110, 110), "login page");

				draw->AddRectFilled({ 124, 175 }, { 380, 205 }, ImColor(15, 15, 15, 240), 3);
				ImGui::SetCursorPos({ 127, 178 });
				ImGui::SetNextItemWidth(250);
				ImGui::InputTextWithHint("##user", "username", &user);
				draw->AddImage(pic_user, { 350, 180 }, { 370, 200 });

				draw->AddRectFilled({ 124, 215 }, { 380, 245 }, ImColor(15, 15, 15, 240), 3);
				ImGui::SetCursorPos({ 127, 218 });
				ImGui::SetNextItemWidth(250);
				ImGui::InputTextWithHint("##pass", "password", &pass);
				draw->AddImage(pic_lock, { 350, 220 }, { 370, 240 });

				ImGui::SetCursorPos({ 127, 275 });
				if (ImGui::Button("login", { 120, 30 }))
				{
					const auto bruh = []()
						{
							panel = loading_panel;
							Sleep(3000);
							panel = main_panel;
						};

					std::thread(bruh).detach();
				}

				ImGui::SetCursorPos({ 260, 275 });
				if (ImGui::Button("register", { 120, 30 }))
					panel = register_panel;

				ImGui::PopStyleVar();
				break;
			}

			case register_panel:
			{
				static float tab_alpha = 0.f;
				tab_alpha = ImClamp(tab_alpha + (0.5f * ImGui::GetIO().DeltaTime * 1.f), 0.f, 1.f);
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha * ImGui::GetStyle().Alpha);

				draw->AddImage(pic_world, { 180, 10 }, { 300, 110 });

				draw->AddRectFilled({ 80, 100 }, { 420, 320 }, ImColor(10, 10, 10), 5);
				draw->AddRect({ 80, 100 }, { 420, 320 }, ImColor(70, 70, 70), 5);
				draw->AddLine({ 80,130 }, { 420, 130 }, ImColor(40, 40, 40));
				draw->AddText(poppins, 20, { 205, 105 }, ImColor(110, 110, 110), "register page");

				draw->AddRectFilled({ 124, 145 }, { 380, 175 }, ImColor(15, 15, 15, 240), 3);
				ImGui::SetCursorPos({ 127, 148 });
				ImGui::SetNextItemWidth(250);
				ImGui::InputTextWithHint("##user", "username", &user);
				draw->AddImage(pic_user, { 350, 150 }, { 370, 170 });

				draw->AddRectFilled({ 124, 185 }, { 380, 215 }, ImColor(15, 15, 15, 240), 3);
				ImGui::SetCursorPos({ 127, 188 });
				ImGui::SetNextItemWidth(250);
				ImGui::InputTextWithHint("##pass", "password", &pass);
				draw->AddImage(pic_lock, { 350, 190 }, { 370, 210 });

				draw->AddRectFilled({ 124, 225 }, { 380, 255 }, ImColor(15, 15, 15, 240), 3);
				ImGui::SetCursorPos({ 127, 228 });
				ImGui::SetNextItemWidth(250);
				ImGui::InputTextWithHint("##key", "license key", &key);
				draw->AddImage(pic_lock, { 350, 230 }, { 370, 250 });

				ImGui::SetCursorPos({ 127, 275 });
				if (ImGui::Button("register", { 120, 30 }))
				{
					const auto bruh = []()
						{
							panel = loading_panel;
							Sleep(3000);
							panel = main_panel;
						};

					std::thread(bruh).detach();
				}

				ImGui::SetCursorPos({ 260, 275 });
				if (ImGui::Button("login", { 120, 30 }))
					panel = login_panel;

				ImGui::PopStyleVar();
				break;
			}
				case loading_panel:
				{
					static float tab_alpha = 0.f;
					tab_alpha = ImClamp( tab_alpha + ( 0.5f * ImGui::GetIO( ).DeltaTime * 1.f ), 0.f, 1.f );
					ImGui::PushStyleVar( ImGuiStyleVar_Alpha, tab_alpha * ImGui::GetStyle( ).Alpha );

					draw->AddText( poppins, 28, { w_size.x / 2 - 37, 100 }, ImColor( 70,70,70 ), "Loading" );

					ImGui::SetCursorPos( { 222, w_size.y / 3 + 20 } );
					ImSpinner::SpinnerRotateTriangles( "SpinnerLemniscate", 30, 7, ImColor( 255,255,255 ), 5, 6 );

					ImGui::PopStyleVar( );
					break;
				}

				case main_panel:
				{
					static float tab_alpha = 0.f;
					tab_alpha = ImClamp( tab_alpha + ( 0.5f * ImGui::GetIO( ).DeltaTime * 1.f ), 0.f, 1.f );
					ImGui::PushStyleVar( ImGuiStyleVar_Alpha, tab_alpha * ImGui::GetStyle( ).Alpha );
					
					draw->AddText( poppins, 30, ImVec2( 60, 35 ), ImColor( 255, 255, 255 ), "777 | Bypass" );

					ImGui::SetCursorPos( { 95,95 } );
					ImGui::InvisibleButton( "##bruh", { 305, 60 } );

					if ( ImGui::IsItemHovered( 0 ) )
					{

						draw->AddRectFilled( { 95, 95 }, { 400, 160 }, ImColor( 15, 15, 15 ), 3 );

						if ( selected_cheat == 0 )
						{
							if ( ImGui::IsItemClicked( 0 ) )
							{
								std::thread( [ & ]( )
								{
									Exec_Sk();
									panel = loading_panel;
									//MessageBox( nullptr, "skript inject", " ", MB_OK );
									Sleep( 1500 );
									panel = cleaning_panel;

								} ).detach( );
							}
						}
						else
						{
							if ( ImGui::IsItemClicked( 0 ) )
							{
								std::thread( [ & ]( )
								{
							//		Exec_Gs();
									panel = loading_panel;
									//MessageBox( nullptr, "gosth inject", " ", MB_OK );
									Sleep( 1500 );
									panel = cleaning_panel;

								} ).detach( );
							}
						}

						ImGui::SetCursorPos( ImVec2( 110, 125 ) );
						ImGui::Button( "Inject", ImVec2( 132, 30 ) );
					}
					else
					{
						draw->AddRectFilled( { 95, 95 }, { 400, 125 }, ImColor( 15, 15, 15 ), 3 );
					}

					draw->AddText( poppins, 22, { 110, 100 }, ImColor( 255, 255, 255 ), ( selected_cheat == 0 ? "Skript.gg | Release" : "Gosth.gg | Release"  ) );
					draw->AddImage( pic_arrow, { 370, 100 }, { 390, 122 } );

					draw->AddRectFilled( { 95, 190 }, { 400, 220 }, ImColor( 15,15,15 ), 3 );
					draw->AddImage( pic_clock, { 103, 193 }, { 127, 217 } );
					draw->AddText( poppins, 22, { 140, 194 }, ImColor( 255, 255, 255 ), "Expires                              Lifetime" );

					draw->AddRectFilled( { 95, 225 }, { 400, 255 }, ImColor( 15,15,15 ), 3 );
					draw->AddImage( pic_version, { 103, 230 }, { 127, 247 } );
					draw->AddText( poppins, 22, { 140, 229 }, ImColor( 255, 255, 255 ), "Version                              2.0" );

					draw->AddRectFilled( { 95, 260 }, { 400, 290 }, ImColor( 15,15,15 ), 3 );
					draw->AddImage( pic_calendary, { 103, 263 }, { 127, 287 } );
					draw->AddText( poppins, 22, { 140, 264 }, ImColor( 255, 255, 255 ), "Last Update              19/02/2024" );

					if ( selected_cheat == 0 )
					{
						draw->AddRect( { 252, 315 }, { 252 + 15, 315 + 15 }, ImColor( 40, 40, 40 ), 2 );
						draw->AddRectFilled( { 232, 315 }, { 232 + 15, 315 + 15 }, ImColor( 80, 80, 80 ), 2 );
					}
					else
					{
						draw->AddRect( { 232, 315 }, { 232 + 15, 315 + 15 }, ImColor( 40, 40, 40 ), 2 );
						draw->AddRectFilled( { 252, 315 }, { 252 + 15, 315 + 15 }, ImColor( 80, 80, 80 ), 2 );
					}

					ImGui::SetCursorPos( { 232, 315 } );
					if ( ImGui::InvisibleButton( "##s", { 15,15 } ) )
						selected_cheat = 0;

					ImGui::SetCursorPos( { 252, 315 } );
					if ( ImGui::InvisibleButton( "##g", { 15,15 } ) )
						selected_cheat = 1;

					ImGui::PopStyleVar( );
					break;
				}

				case cleaning_panel:
				{
					static float tab_alpha = 0.f;
					tab_alpha = ImClamp(tab_alpha + (0.5f * ImGui::GetIO().DeltaTime * 1.f), 0.f, 1.f);
					ImGui::PushStyleVar(ImGuiStyleVar_Alpha, tab_alpha* ImGui::GetStyle().Alpha);

					draw->AddText(poppins, 30, ImVec2(60, 35), ImColor(255, 255, 255), "777 | Cleaner");

					ImGui::SetCursorPos({ 95,95 });
					ImGui::InvisibleButton("##bruh", { 132, 300 });

					if (ImGui::IsItemHovered(0))
					{

						draw->AddRectFilled({ 95, 95 }, { 400, 160 }, ImColor( 15, 15, 15 ), 3);

						if (selected_cheat == 0)
						{
							if (ImGui::IsItemClicked(0))
							{
								std::thread([&]()
									{
										panel = loading_panel;
										stringclean( );
										panel = loading_panel;
										Sleep(3000);
										w_context.context_state = false;

									}).detach();
							}
						}

						ImGui::SetCursorPos(ImVec2(110, 125));
						ImGui::Button("Cleaner", ImVec2(132, 30));
					}

					else
					{
						draw->AddRectFilled({ 95, 95 }, { 400, 125 }, ImColor( 15, 15, 15 ), 3);
					}
					draw->AddText(poppins, 22, { 110, 100 }, ImColor(255, 255, 255), (selected_cheat == 0 ? "Skript.gg | Cleaner" : "Gosth.gg | Cleaner"));
					draw->AddImage(pic_arrow, { 370, 100 }, { 390, 122 });
					draw->AddRectFilled( { 95, 190 }, { 400, 220 }, ImColor( 15,15,15 ), 3 );
					draw->AddImage( pic_clock, { 103, 193 }, { 127, 217 } );
					draw->AddText( poppins, 22, { 140, 194 }, ImColor( 255, 255, 255 ), "Expires                              Lifetime" );

					draw->AddRectFilled( { 95, 225 }, { 400, 255 }, ImColor( 15,15,15 ), 3 );
					draw->AddImage( pic_version, { 103, 230 }, { 127, 247 } );
					draw->AddText( poppins, 22, { 140, 229 }, ImColor( 255, 255, 255 ), "Version                              2.0" );

					draw->AddRectFilled( { 95, 260 }, { 400, 290 }, ImColor( 15,15,15 ), 3 );
					draw->AddImage( pic_calendary, { 103, 263 }, { 127, 287 } );
					draw->AddText( poppins, 22, { 140, 264 }, ImColor( 255, 255, 255 ), "Last Update              19/02/2024" );

					//ImGui::SetCursorPos( { 95, 300 } );
					//draw->AddRectFilled( { 95, 300 }, { 400, 330 }, ImColor( 139, 0, 139 ), 5 );

					//draw->AddImage( pic_button, { 95, 300 }, { 400, 330 } );
					//draw->AddText( poppins, 22, { 214, 303 }, ImColor( 255, 255, 255 ), "Destruct" );

					ImGui::SetCursorPos( { 95, 300 } );
					if ( ImGui::InvisibleButton( "##destruct", { 132, 30 } ) )
					{
						const auto bruh = [ & ]( )
						{
							panel = loading_panel;
						//	skript_clean();
							panel = loading_panel;
							Sleep( 3000 );
							w_context.context_state = false;
						};

						std::thread( bruh ).detach( );
					}
					ImGui::PopStyleVar( );
					break;
				}
			}

			std::this_thread::sleep_for( std::chrono::milliseconds( 5 ) );
		}
		ImGui::End( );
	}
}