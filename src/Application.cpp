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
	timer.Mark();
	gfx.BeginFrame(clearColor);

	ImGui::Begin("Settings");

	ImGui::Text("Last render: %.3fms", lastRenderTime);
	ImGui::SliderFloat("Radius", &radius, 0.5f, 1.5f);

	ImGui::ColorEdit3("Sphere color", &sphereColor.x);
	ImGui::SliderFloat3("Light direction", &lightDir.x, -1.0f, 1.0f);

	ImGui::End();

	for (int y = 0; y < wnd.GetHeight(); ++y) {
		for (int x = 0; x < wnd.GetWidth(); ++x) {
			DirectX::XMFLOAT2 coord = { (float)x / (float)wnd.GetWidth(), (float)y / (float)wnd.GetHeight() };
			coord.x = coord.x * 2.0f - 1.0f;
			coord.y = coord.y * 2.0f - 1.0f;
			gfx.PutPixel(x, y, PerPixel(coord));
		}
	}

	gfx.EndFrame();

	lastRenderTime = timer.Mark();
}

DirectX::XMFLOAT4 Application::PerPixel(DirectX::XMFLOAT2 coord) const
{
	coord.x *= (float)wnd.GetWidth() / (float)wnd.GetHeight();

	DirectX::XMFLOAT3 dir = { coord.x, coord.y, 1.0f };

	float a = Utils::Dot(dir, dir);
	float b = 2.0f * Utils::Dot(cameraPos, dir);
	float c = Utils::Dot(cameraPos, cameraPos) - radius * radius;

	float D = b * b - 4.0f * a * c;

	if (D < 0.0f) {
		return clearColor;
	}

	const float closestHit = (-b - sqrt(D)) / (2.0f * a);
	const DirectX::XMFLOAT3 hitPos = Utils::Add(cameraPos, Utils::Scale(dir, closestHit));
	const DirectX::XMFLOAT3 normal = Utils::Normalize(hitPos);

	const float f = max(Utils::Dot(normal, Utils::Normalize(Utils::Negate(lightDir))), 0.0f);
	const DirectX::XMFLOAT4 finalColor = { sphereColor.x * f, sphereColor.y * f, sphereColor.z * f, 1.0f };

	return finalColor;
}
