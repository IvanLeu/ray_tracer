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

	Material& orangeSphere = scene.materials.emplace_back();
	orangeSphere.Albedo = { 1.0f, 0.55f, 0.0f, 1.0f };
	orangeSphere.Roughness = 0.0f;

	Material& blueSphere = scene.materials.emplace_back();
	blueSphere.Albedo = { 0.2f, 0.3f, 1.0f, 1.0f };
	blueSphere.Roughness = 0.1f;

	{
		Sphere sphere;
		sphere.position = { 0.0f, 0.0f, 0.0f };
		sphere.radius = 1.0f;
		sphere.materialIndex = 0;
		scene.spheres.push_back(sphere);
	}

	{
		Sphere sphere;
		sphere.position = { 0.0f, 101.0f, 0.0f };
		sphere.radius = 100.0f;
		sphere.materialIndex = 1;
		scene.spheres.push_back(sphere);
	}
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

	if (camera.Update()) {
		renderer.ResetFrameIndex();
	}
}

void Application::OnRender()
{
	gfx.BeginFrame();

	OnRenderUI();
	renderer.Render(gfx, scene, camera);

	gfx.EndFrame();
}

void Application::OnRenderUI()
{
	ImGui::Begin("Scene");
	for (size_t i = 0; i < scene.spheres.size(); ++i) {
		ImGui::PushID((int)i);

		ImGui::DragFloat3("Position", &scene.spheres[i].position.x, 0.1f);
		ImGui::DragFloat("Radius", &scene.spheres[i].radius, 0.1f);
		ImGui::DragInt("Material ID", &scene.spheres[i].materialIndex, 1.0f, 0, (int)scene.materials.size() - 1);

		ImGui::Separator();

		ImGui::PopID();
	}

	ImGui::Text("Materials");
	for (size_t i = 0; i < scene.materials.size(); ++i) {
		ImGui::PushID((int)i);

		ImGui::ColorEdit4("Albedo", &scene.materials[i].Albedo.x);
		ImGui::DragFloat("Roughness", &scene.materials[i].Roughness, 0.005f, 0.0f, 1.0f);
		ImGui::DragFloat("Mettalic", &scene.materials[i].Metallic, 0.005f, 0.0f, 1.0f);

		ImGui::Separator();

		ImGui::PopID();
	}

	ImGui::End();

	renderer.RenderUI();
}
