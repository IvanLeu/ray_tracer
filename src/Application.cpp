#include "Application.h"

Application::Application()
	:
	wnd(1280, 720, "Ray Tracer App"),
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

}

void Application::OnRender()
{
	gfx.BeginFrame();

	for (int y = 100; y < 200; ++y) {
		for (int x = 100; x < 200; ++x) {
			gfx.PutPixel(x, y, { 0.0f, 0.0f, 1.0f, 1.0f });
		}
	}

	for (int y = 300; y < 400; ++y) {
		for (int x = 300; x < 400; ++x) {
			gfx.PutPixel(x, y, { 0.0f, 0.0f, 1.0f, 1.0f });
		}
	}

	gfx.EndFrame();
}
