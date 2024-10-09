#include "Window.h"
#include <windowsx.h>

Window::Window(int width_in, int height_in, const char* name)
	:
	width(width_in),
	height(height_in)
{
	// Windows class registration
	WNDCLASSEX wc = {0};
	wc.cbSize = sizeof(wc);
	wc.style = CS_OWNDC;
	wc.lpfnWndProc = HandleMsgSetup;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInst;
	wc.hIcon = nullptr;
	wc.hCursor = nullptr;
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = windowClassName;
	wc.hIconSm = nullptr;

	RegisterClassEx(&wc);

	// Creating the window
	// Initialising window rect
	RECT rect;
	rect.left = 100;
	rect.right = rect.left + width;
	rect.top = 100;
	rect.bottom = rect.top + height;

	DWORD wndStyle = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

	if (AdjustWindowRect(&rect, wndStyle, FALSE) == 0) {
		// TODO: throw
		exit(1);
	}

	hWnd = CreateWindow(
		windowClassName,
		"RayTracer",
		wndStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width,
		height,
		NULL,
		NULL,
		hInst,
		this);

	if (hWnd == NULL) {
		// TODO: throw
		exit(1);
	}

	ShowWindow(hWnd, SW_SHOWDEFAULT);
}

Window::~Window() noexcept
{
	DestroyWindow(hWnd);
	UnregisterClass(windowClassName, hInst);
}

std::optional<int> Window::ProcessMessages() noexcept
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
		if (msg.wParam == WM_QUIT) {
			return (int)msg.wParam;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return {};
}

std::pair<int, int> Window::GetSize() const noexcept
{
	return std::pair<int, int>(width, height);
}

LRESULT CALLBACK Window::HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
{
	// use create parameter passed in CreateWindow() to store window class pointer at WINAPI
	if (msg == WM_NCCREATE) {
		// extract ptr to window class from creation data
		const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lparam);
		Window* const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
		// set WINAPI-managed user data to store ptr to window class
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
		// set message proc to normal (non-setup) handler now that setup is finished
		SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));
		// forward message to window class handler
		return pWnd->HandleMsg(hwnd, msg, wparam, lparam);
	}
	// if we get a message before WM_NCCREATE, handle with default handler
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

LRESULT CALLBACK Window::HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
{
	// retrieve ptr to window class
	Window* const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
	// forward message to window class handler
	return pWnd->HandleMsg(hwnd, msg, wparam, lparam);
}

LRESULT Window::HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept
{
	switch (msg)
	{
	case WM_ERASEBKGND:
		// notify the OS that erasing will be handled by the application to prevert flicker.
		return 1;
	case WM_CLOSE:
		// TODO: Fire an event to application to quit
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	case WM_SIZE: {
		// Get the updated size
		//RECT rect;
		//GetWindowRect(hwnd, &rect);
		//width = rect.right - rect.left;
		//height = rect.bottom - rect.top;
		
		// TODO: event to for window resize
	} break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP: {
		// Key pressed/released
		//bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
		// TODO: input processing
	} break;
	case WM_MOUSEMOVE: {
		//int x_position = GET_X_LPARAM(lparam);
		//int y_position = GET_Y_LPARAM(lparam);
		// TODO: input processing
	} break;
	case WM_MOUSEWHEEL: {
		//int delta = GET_WHEEL_DELTA_WPARAM(wparam);
		//if (delta != 0) {
		//	delta = (delta < 0) ? -1 : 1;
		//}
		// TODO: input processing
	} break;
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_MBUTTONUP:
	case WM_RBUTTONUP: {
		//bool pressed = (msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg == WM_RBUTTONDOWN);
		// TODO: input proccesing
	} break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}
