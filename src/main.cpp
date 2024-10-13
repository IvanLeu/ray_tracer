#include "Window.h"
#include "Input.h"
#include <sstream>
#include <iostream>

int main() {
	//test
	Window wnd(800, 600, "Window");
	auto pInputState = std::make_shared<InputState>();
	wnd.BindInputState(pInputState);

	while (true) {
		if (const auto ecode = Window::ProcessMessages()) {
			return *ecode;
		}

	}

	return 0;
}