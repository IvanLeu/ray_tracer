#include "Window.h"
#include <windowsx.h>
#include <assert.h>

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
		name,
		wndStyle,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rect.right - rect.left,
		rect.bottom - rect.top,
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
		if (msg.message == WM_QUIT) {
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

void Window::BindInputState(std::shared_ptr<InputState> input) noexcept {
	assert(input);
	pInputState = input;
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
	case WM_KILLFOCUS: {
		if (pInputState != nullptr) {
			pInputState->kbd.ClearState();
		}
	} break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN: {
		if (pInputState != nullptr) {
			if (!(lparam & 0x40000000) || pInputState->kbd.AutorepeatEnabled()) {
				pInputState->kbd.OnKeyPressed(static_cast<unsigned char>(wparam));
			}
		}
	} break;
	case WM_KEYUP:
	case WM_SYSKEYUP: {
		if (pInputState != nullptr) {
			pInputState->kbd.OnKeyReleased(static_cast<unsigned char>(wparam));
		}
	} break;
	case WM_CHAR: {
		if (pInputState != nullptr) {
			pInputState->kbd.OnChar(static_cast<unsigned char>(wparam));
		}
	} break;
	case WM_MOUSEMOVE: {
		if (pInputState != nullptr) {
			const POINTS pt = MAKEPOINTS(lparam);

			if (pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height) {
				pInputState->mouse.OnMouseMove(pt.x, pt.y);
				if (!pInputState->mouse.IsInWindow()) {
					SetCapture(hWnd);
					pInputState->mouse.OnMouseEnter();
				}
			}
			else {
				if (wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) {
					pInputState->mouse.OnMouseMove(pt.x, pt.y);
				}
				else {
					ReleaseCapture();
					pInputState->mouse.OnMouseLeave();
				}
			}
		}
	} break;
	case WM_MOUSEWHEEL: {
		if (pInputState != nullptr) {
			const int zdelta = GET_WHEEL_DELTA_WPARAM(wparam);
			const POINTS pt = MAKEPOINTS(lparam);
			pInputState->mouse.OnWheelDelta(pt.x, pt.y, zdelta);
		}
	} break;
	case WM_LBUTTONDOWN: {
		if (pInputState != nullptr) {
			const POINTS pt = MAKEPOINTS(lparam);
			pInputState->mouse.OnLeftPressed(pt.x, pt.y);
		}
	} break;
	case WM_MBUTTONDOWN: {
		if (pInputState != nullptr) {
			const POINTS pt = MAKEPOINTS(lparam);
			pInputState->mouse.OnMiddlePressed(pt.x, pt.y);
		}
	} break;
	case WM_RBUTTONDOWN: {
		if (pInputState != nullptr) {
			const POINTS pt = MAKEPOINTS(lparam);
			pInputState->mouse.OnRightPressed(pt.x, pt.y);
		}
	} break;
	case WM_LBUTTONUP: {
		if (pInputState != nullptr) {
			const POINTS pt = MAKEPOINTS(lparam);
			pInputState->mouse.OnLeftReleased(pt.x, pt.y);
		}
	} break;
	case WM_MBUTTONUP: {
		if (pInputState != nullptr) {
			const POINTS pt = MAKEPOINTS(lparam);
			pInputState->mouse.OnMiddleReleased(pt.x, pt.y);
		}
	} break;
	case WM_RBUTTONUP: {
		if (pInputState != nullptr) {
			const POINTS pt = MAKEPOINTS(lparam);
			pInputState->mouse.OnRightReleased(pt.x, pt.y);
		}
	} break;
	}

	return DefWindowProc(hwnd, msg, wparam, lparam);
}
