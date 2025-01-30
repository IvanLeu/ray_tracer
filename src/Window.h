#pragma once

#include <Windows.h>
#include <memory>
#include <optional>
#include "Input.h"

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

class Window {
public:
	Window(int width, int height, const char* name);
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	~Window() noexcept;
	static std::optional<int> ProcessMessages() noexcept;
	std::pair<int, int> GetSize() const noexcept;
	int GetWidth() const noexcept;
	int GetHeight() const noexcept;
	void BindInputState(std::shared_ptr<InputState> input) noexcept;
	HWND GetHandle() const;
	void EnableCursor() noexcept;
	void DisableCursor() noexcept;
	bool CursorEnabled() const noexcept;
private:
	void ConfineCursor() const;
	void FreeCursor() const;
	void ShowCursor() const;
	void HideCursor() const;
	void EnableImGuiMouse() const;
	void DisableImGuiMouse() const;
private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
private:
	int width;
	int height;
	bool cursorEnabled = true;
	HWND hWnd;
	HINSTANCE hInst;
	static constexpr const char* windowClassName = "Simple Ray Tracer";
	std::shared_ptr<InputState> pInputState = nullptr;
	std::vector<char> rawBuffer;
};