#include "Renderer.h"
#include "VectorUtils.h"
#include "imgui.h"

Renderer::Renderer(Graphics& gfx)
	:
	m_Width(gfx.GetWidth()),
	m_Height(gfx.GetHeight())
{
	gfx.SetTextureClearColor(clearColor);
}

void Renderer::Render(Graphics& gfx, const Camera& camera)
{
	Ray ray;
	DirectX::XMStoreFloat3(&ray.origin, camera.GetPosition());

	timer.Mark();
	for (int y = 0; y < m_Height; ++y) {
		for (int x = 0; x < m_Width; ++x) {
			ray.direction = camera.GetRayDirections()[x + y * m_Width];
			gfx.PutPixel(x, y, TraceRay(ray));
		}
	}
	lastRenderTime = timer.Mark();
}

void Renderer::RenderUI()
{
	ImGui::Begin("Settings");

	ImGui::Text("Last render: %.3fms", lastRenderTime);
	ImGui::SliderFloat("Radius", &radius, 0.5f, 1.5f);

	ImGui::ColorEdit3("Sphere color", &sphereColor.x);
	ImGui::SliderFloat3("Light direction", &lightDir.x, -1.0f, 1.0f);

	ImGui::End();
}

DirectX::XMFLOAT4 Renderer::TraceRay(const Ray& ray) const
{
	float a = Utils::Dot(ray.direction, ray.direction);
	float b = 2.0f * Utils::Dot(ray.origin, ray.direction);
	float c = Utils::Dot(ray.origin, ray.origin) - radius * radius;

	float D = b * b - 4.0f * a * c;

	if (D < 0.0f) {
		return clearColor;
	}

	const float closestHit = (-b - sqrt(D)) / (2.0f * a);
	const DirectX::XMFLOAT3 hitPos = Utils::Add(ray.origin, Utils::Scale(ray.direction, closestHit));
	const DirectX::XMFLOAT3 normal = Utils::Normalize(hitPos);

	const float f = max(Utils::Dot(normal, Utils::Normalize(Utils::Negate(lightDir))), 0.0f);
	const DirectX::XMFLOAT4 finalColor = { sphereColor.x * f, sphereColor.y * f, sphereColor.z * f, 1.0f };

	return finalColor;
}
