#include "Application.h"
#include "imgui.h"
#include "VectorUtils.h"

#include <chrono>

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
	while (const auto e = pInputState->kbd.ReadKey()) {
		if (!e->IsPress()) {
			continue;
		}
		switch (e->GetCode()) {
		case VK_ESCAPE:
			if (!wnd.CursorEnabled()) {
				wnd.EnableCursor();
				pInputState->mouse.DisableRaw();
			}
			else {
				wnd.DisableCursor();
				pInputState->mouse.EnableRaw();
			}
			break;
		}
	}

}

void Application::OnRender()
{
	gfx.BeginFrame();

	OnRenderUI();
	renderer.Render(gfx);

	gfx.EndFrame();
}

void Application::OnRenderUI()
{
	renderer.RenderUI();
}
