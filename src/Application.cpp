#include "Application.h"
#include <iostream>

Application::Application()
	:
	wnd(1280, 1024, "Ray Tracer App"),
	pInputState(std::make_shared<InputState>())
{
	wnd.BindInputState(pInputState);
}

int Application::Run()
{
	while (running) {
		if (const auto e = wnd.ProcessMessages()) {
			running = false;
			return *e;
		}
		OnUpdate();
		OnRender();
	}
	return 0;
}

Application::~Application()
{
}

void Application::OnUpdate()
{
	if (pInputState->kbd.KeyIsPressed(VK_SPACE)) {
		std::cout << "Space" << std::endl;
	}
	if (!pInputState->mouse.IsInWindow()) {
		std::cout << "Mouse ran away!" << std::endl;;
	}
}

void Application::OnRender()
{
}
