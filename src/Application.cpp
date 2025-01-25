#include "Application.h"
#include "imgui.h"

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

}

void Application::OnRender()
{
	timer.Mark();
	gfx.BeginFrame(clearColor);

	ImGui::Begin("Settings");

	ImGui::Text("Last render: %.3fms", lastRenderTime);
	ImGui::SliderFloat("Radius", &radius, 1.0f, 10.0f);

	ImGui::End();

	for (int y = 0; y < wnd.GetHeight(); ++y) {
		for (int x = 0; x < wnd.GetWidth(); ++x) {
			DirectX::XMFLOAT3 coord = { (float)x / (float)wnd.GetWidth(), (float)y / (float)wnd.GetHeight(), 0.0f };
			coord.x = coord.x * 2.0f - 1.0f;
			coord.y = coord.y * 2.0f - 1.0f;
			coord.z = coord.z * 2.0f - 1.0f;
			gfx.PutPixel(x, y, PerPixel(coord));
		}
	}

	gfx.EndFrame();

	lastRenderTime = timer.Mark();
}

DirectX::XMFLOAT4 Application::PerPixel(DirectX::XMFLOAT3 coord) const
{
	DirectX::XMFLOAT3 dir = { coord.x, coord.y, 1.0f };

	float a = dir.x * dir.x + dir.y * dir.y + dir.z + dir.z;
	float b = 2.0f * (camera.x * dir.x + camera.y * dir.y + camera.z * dir.z);
	float c = camera.x * camera.x + camera.y * camera.y + camera.z * camera.z - radius * radius;

	float D = b * b - 4 * a * c;

	if (D < 0.0f) {
		return clearColor;
	}
	else {
		return { 1.0f, 0.0f, 1.0f, 1.0f };
	}

}
