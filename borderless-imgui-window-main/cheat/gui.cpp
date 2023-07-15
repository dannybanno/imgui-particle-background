#include "gui.h"
#include <cmath>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include <string>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); // set click points
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	ImGui::StyleColorsLight();

	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\CASCADIACODE.TTF", 21.25f);

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}


void gui::Render() noexcept
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin(
		"h e l l o !",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);

	//retrieve the ImGui draw list
	// 
	//ImDrawList* drawList = ImGui::GetBackgroundDrawList();	//draw behind imgui stuff, looks dark and eh
	
	//ImDrawList* drawList = ImGui::GetForegroundDrawList();	//draw ontop of all the other imgui stuff

	ImDrawList* drawList = ImGui::GetWindowDrawList();			//draws like foreground does but behind the imgui stuff. Ideal

	//particle properties
	static const int numParticles = 115;
	static ImVec2 particlePositions[numParticles];
	static ImVec2 particleDistance;
	static ImVec2 particleVelocities[numParticles];
	static ImU32 particleColors[numParticles];

	static bool initialized = false;
	if (!initialized)
	{
		for (int i = 0; i < numParticles; ++i)
		{
			particlePositions[i] = ImVec2(
				ImGui::GetWindowPos().x + ImGui::GetWindowSize().x * static_cast<float>(rand()) / RAND_MAX,
				ImGui::GetWindowPos().y + ImGui::GetWindowSize().y * static_cast<float>(rand()) / RAND_MAX
			);

			particleVelocities[i] = ImVec2(
				static_cast<float>((rand() % 11) - 5),
				static_cast<float>((rand() % 11) - 5)
			);

			particleColors[i] = IM_COL32_WHITE;
		}

		initialized = true;
	}

	ImVec2 cursorPos = ImGui::GetIO().MousePos;
	for (int i = 0; i < numParticles; ++i)
	{
		//draw lines to particles
		for (int j = i + 1; j < numParticles; ++j)
		{
			float distance = std::hypotf(particlePositions[j].x - particlePositions[i].x, particlePositions[j].y - particlePositions[i].y);
			float opacity = 1.0f - (distance / 55.0f);  // opacity cahnge

			if (opacity > 0.0f)
			{
				ImU32 lineColor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, opacity));
				drawList->AddLine(particlePositions[i], particlePositions[j], lineColor);
			}
		}

		//draw lines to cursor
		float distanceToCursor = std::hypotf(cursorPos.x - particlePositions[i].x, cursorPos.y - particlePositions[i].y);
		float opacityToCursor = 1.0f - (distanceToCursor / 52.0f);  // Adjust the divisor to control the opacity change

		if (opacityToCursor > 0.0f)
		{
			ImU32 lineColorToCursor = ImGui::GetColorU32(ImVec4(1.0f, 1.0f, 1.0f, opacityToCursor));
			drawList->AddLine(cursorPos, particlePositions[i], lineColorToCursor);
		}
	}

	//update and render particles
	float deltaTime = ImGui::GetIO().DeltaTime;
	for (int i = 0; i < numParticles; ++i)
	{
		particlePositions[i].x += particleVelocities[i].x * deltaTime;
		particlePositions[i].y += particleVelocities[i].y * deltaTime;

		// Stay in window
		if (particlePositions[i].x < ImGui::GetWindowPos().x)
			particlePositions[i].x = ImGui::GetWindowPos().x + ImGui::GetWindowSize().x;
		else if (particlePositions[i].x > ImGui::GetWindowPos().x + ImGui::GetWindowSize().x)
			particlePositions[i].x = ImGui::GetWindowPos().x;

		if (particlePositions[i].y < ImGui::GetWindowPos().y)
			particlePositions[i].y = ImGui::GetWindowPos().y + ImGui::GetWindowSize().y;
		else if (particlePositions[i].y > ImGui::GetWindowPos().y + ImGui::GetWindowSize().y)
			particlePositions[i].y = ImGui::GetWindowPos().y;

		// Render particles below UI components
		drawList->AddCircleFilled(particlePositions[i], 1.5f, particleColors[i]);
	}

	
	//ImGui::Text("Num of particles: %d", numParticles);

	//login field for example of use
	static bool loggedIn = false;

	static char username[256];
	static char password[256];

	if (!loggedIn)
	{
		// Center the input text
		//username
		ImGui::SetCursorPos(ImVec2(200, 150));
		ImGui::SetNextItemWidth(400.0f);
		ImGui::InputText("Username", username, 100.0f);

		//password
		ImGui::SetCursorPos(ImVec2(200, 190));
		ImGui::SetNextItemWidth(400.0f);
		ImGui::InputText("Password", password, 100.0f, ImGuiInputTextFlags_Password);

		//button
		ImGui::SetCursorPos(ImVec2(390, 230));
		if (ImGui::Button("Log In"))
		{
			//logs in 
			if (std::string(username) == "admin" && std::string(password) == "admin")
			{
				loggedIn = true;
			}
		}
	}
	else
	{
		//logged in stuff
		ImGui::Text("Welcome, %s!", username);
		ImGui::Text("You are logged in.");

		if (ImGui::Button("Log Out"))
		{
			loggedIn = false;
			std::memset(username, 0, sizeof(username));
			std::memset(password, 0, sizeof(password));
		}
	}




	ImGui::End();
}

