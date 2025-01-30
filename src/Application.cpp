#include "Application.h"
#include "imgui.h"
#include "VectorUtils.h"

#include <chrono>

Application::Application()
	:
	wnd(1280, 720, "Ray Tracer App"),
	pInputState(std::make_shared<InputState>()),
	camera(gfx.GetWidth(), gfx.GetHeight(), 45.0f, 0.1f, 100.0f)
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
		const float dt = timer.Mark();
		OnUpdate(dt);
		OnRender();
	}
	return 0;
}

Application::~Application()
{
}

void Application::OnUpdate(float dt)
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

	DirectX::XMFLOAT3 moveVector = { 0.0f, 0.0f, 0.0f };
	int deltaX = 0;
	int deltaY = 0;

	if (!wnd.CursorEnabled()) {
		if (pInputState->kbd.KeyIsPressed('W')) {
			moveVector.z = 1.0f;
		}
		if (pInputState->kbd.KeyIsPressed('A')) {
			moveVector.x = 1.0f;
		}
		if (pInputState->kbd.KeyIsPressed('S')) {
			moveVector.z = -1.0f;
		}
		if (pInputState->kbd.KeyIsPressed('D')) {
			moveVector.x = -1.0f;
		}
		if (pInputState->kbd.KeyIsPressed('Q')) {
			moveVector.y = 1.0f;
		}
		if (pInputState->kbd.KeyIsPressed('E')) {
			moveVector.y = -1.0f;
		}


		while (const auto raw = pInputState->mouse.ReadRawDelta()) {
			deltaX = raw->x;
			deltaY = raw->y;
		}

		camera.Move(dt, moveVector);
		camera.Rotate(dt, (float)deltaX, (float)deltaY);
	}

	camera.Update();
}

void Application::OnRender()
{
	gfx.BeginFrame();

	OnRenderUI();
	renderer.Render(gfx, camera);

	gfx.EndFrame();
}

void Application::OnRenderUI()
{
	renderer.RenderUI();
}
