#include "Window.h"
#include "Input.h"

int main() {
	//test
	Window wnd(800, 600, "Window");
	auto pInputState = std::make_shared<InputState>();
	wnd.BindInputState(pInputState);

	while (true) {
		if (const auto ecode = Window::ProcessMessages()) {
			return *ecode;
		}

		if (pInputState->kbd.KeyIsPressed(VK_MENU)) {
			MessageBox(NULL, "Alt was pressed", "Spaaaaaaace!", MB_OK);
		}
	}

	return 0;
}