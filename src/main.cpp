#include "Window.h"

int main() {
	//test
	Window wnd(1920, 1080, "Brooooo");

	while (true) {
		if (const auto ecode = Window::ProcessMessages()) {
			return *ecode;
		}
	}

	return 0;
}