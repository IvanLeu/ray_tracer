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

void Renderer::Render(Graphics& gfx, const Scene& scene, const Camera& camera)
{
	Ray ray;
	DirectX::XMStoreFloat3(&ray.origin, camera.GetPosition());

	timer.Mark();
	for (int y = 0; y < m_Height; ++y) {
		for (int x = 0; x < m_Width; ++x) {
			ray.direction = camera.GetRayDirections()[x + y * m_Width];
			gfx.PutPixel(x, y, TraceRay(scene, ray));
		}
	}
	lastRenderTime = timer.Mark();
}

void Renderer::RenderUI()
{
	ImGui::Begin("Settings");

	ImGui::Text("Last render: %.3fms", lastRenderTime);
	ImGui::SliderFloat3("Light direction", &lightDir.x, -1.0f, 1.0f);

	ImGui::End();
}

DirectX::XMFLOAT4 Renderer::TraceRay(const Scene& scene, const Ray& ray) const
{
	const Sphere* closestSphere = nullptr;
	float hitDistance = std::numeric_limits<float>::max();

	for (const Sphere& sphere : scene.spheres) {
		DirectX::XMFLOAT3 origin = Utils::Subtract(ray.origin, sphere.position);

		float a = Utils::Dot(ray.direction, ray.direction);
		float b = 2.0f * Utils::Dot(origin, ray.direction);
		float c = Utils::Dot(origin, origin) - sphere.radius * sphere.radius;

		float D = b * b - 4.0f * a * c;

		if (D < 0.0f) {
			continue;
		}

		const float closestHit = (-b - sqrt(D)) / (2.0f * a);
		if (closestHit < hitDistance) {
			hitDistance = closestHit;
			closestSphere = &sphere;
		}
	}

	if (!closestSphere) {
		return clearColor;
	}

	DirectX::XMFLOAT3 origin = Utils::Subtract(ray.origin, closestSphere->position);

	const DirectX::XMFLOAT3 hitPos = Utils::Add(origin, Utils::Scale(ray.direction, hitDistance));
	const DirectX::XMFLOAT3 normal = Utils::Normalize(hitPos);

	const float f = std::max(Utils::Dot(normal, Utils::Normalize(Utils::Negate(lightDir))), 0.0f);
	const DirectX::XMFLOAT4 finalColor = { closestSphere->albedo.x * f, closestSphere->albedo.y * f, closestSphere->albedo.z * f, 1.0f };

	return finalColor;
}
