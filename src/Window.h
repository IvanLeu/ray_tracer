#pragma once

#include <Windows.h>
#include <memory>
#include <optional>
#include "Input.h"

class Window {
public:
	Window(int width, int height, const char* name);
	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	~Window() noexcept;
	static std::optional<int> ProcessMessages() noexcept;
	std::pair<int, int> GetSize() const noexcept;
	void BindInputState(std::shared_ptr<InputState> input) noexcept;
private:
	static LRESULT CALLBACK HandleMsgSetup(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	static LRESULT CALLBACK HandleMsgThunk(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
	LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept;
private:
	int width;
	int height;
	HWND hWnd;
	HINSTANCE hInst;
	static constexpr const char* windowClassName = "Simple Ray Tracer";
	std::shared_ptr<InputState> pInputState = nullptr;
};