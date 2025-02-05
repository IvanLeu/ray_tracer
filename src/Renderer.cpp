#include "Renderer.h"
#include "VectorUtils.h"
#include "imgui.h"

#include <chrono>

Renderer::Renderer(Graphics& gfx)
	:
	m_Width(gfx.GetWidth()),
	m_Height(gfx.GetHeight())
{
	gfx.SetTextureClearColor(clearColor);
}

void Renderer::Render(Graphics& gfx, const Scene& scene, const Camera& camera)
{
	m_ActiveScene = &scene;
	m_ActiveCamera = &camera;

	auto start = std::chrono::high_resolution_clock::now();

	for (int y = 0; y < m_Height; ++y) {
		for (int x = 0; x < m_Width; ++x) {

			auto color = Utils::Clamp(PerPixel(x, y), 0.0f, 1.0f);
			color.w = 1.0f;
			gfx.PutPixel(x, y, color);
		}
	}

	auto end = std::chrono::high_resolution_clock::now();

	lastRenderTime = std::chrono::duration<float, std::milli>(end - start).count();
}

void Renderer::RenderUI()
{
	ImGui::Begin("Settings");

	ImGui::Text("Last render: %.3fms", lastRenderTime);
	ImGui::SliderFloat3("Light direction", &lightDir.x, -1.0f, 1.0f);

	ImGui::End();
}

DirectX::XMFLOAT4 Renderer::PerPixel(int x, int y) const
{
	Ray ray;
	DirectX::XMStoreFloat3(&ray.origin, m_ActiveCamera->GetPosition());
	ray.direction = m_ActiveCamera->GetRayDirections()[x + y * m_Width];

	DirectX::XMFLOAT3 color = { 0.0f, 0.0f, 0.0f };
	float multiplier = 1.0f;

	int nBounces = 2;
	for (int i = 0; i < nBounces; ++i) {
		HitPayload payload = TraceRay(ray);

		if (payload.hitDistance < 0.0f) {
			color = Utils::Add(color, Utils::Scale(Utils::ToFloat3(clearColor), multiplier));
			break;
		}

		const float f = std::max(Utils::Dot(payload.WorldNormal, Utils::Normalize(Utils::Negate(lightDir))), 0.0f);
		
		const Sphere& sphere = m_ActiveScene->spheres[payload.objectIndex];
		auto sphereColor = Utils::Scale(Utils::ToFloat3(sphere.albedo), f);
		color = Utils::Add(color, Utils::Scale(sphereColor, multiplier));

		multiplier *= 0.7f;

		ray.origin = Utils::Add(payload.WorldPosition, Utils::Scale(payload.WorldNormal, 0.0001f));
		ray.direction = Utils::Reflect(ray.direction, payload.WorldNormal);
	}

	return Utils::ToFloat4(color, 1.0f);
}

Renderer::HitPayload Renderer::TraceRay(const Ray& ray) const
{
	int closestSphere = -1;
	float hitDistance = std::numeric_limits<float>::max();

	for (size_t i = 0; i < m_ActiveScene->spheres.size(); ++i) {
		const Sphere& sphere = m_ActiveScene->spheres[i];

		DirectX::XMFLOAT3 origin = Utils::Subtract(ray.origin, sphere.position);

		float a = Utils::Dot(ray.direction, ray.direction);
		float b = 2.0f * Utils::Dot(origin, ray.direction);
		float c = Utils::Dot(origin, origin) - sphere.radius * sphere.radius;

		float D = b * b - 4.0f * a * c;

		if (D < 0.0f) {
			continue;
		}

		const float closestHit = (-b - sqrt(D)) / (2.0f * a);
		if (closestHit >= 0.0f && closestHit < hitDistance) {
			hitDistance = closestHit;
			closestSphere = (int)i;
		}
	}

	if (closestSphere == -1) {
		return Miss();
	}
	
	return ClosestHit(ray, hitDistance, closestSphere);
}

Renderer::HitPayload Renderer::ClosestHit(const Ray& ray, float hitDistance, int objectIndex) const
{
	HitPayload payload;
	payload.hitDistance = hitDistance;
	payload.objectIndex = objectIndex;

	const Sphere& sphere = m_ActiveScene->spheres[objectIndex];
	DirectX::XMFLOAT3 origin = Utils::Subtract(ray.origin, sphere.position);

	payload.WorldPosition = Utils::Add(origin, Utils::Scale(ray.direction, hitDistance));
	payload.WorldNormal = Utils::Normalize(payload.WorldPosition);

	payload.WorldPosition = Utils::Add(payload.WorldPosition, sphere.position);

	return payload;
}

Renderer::HitPayload Renderer::Miss() const
{
	HitPayload payload;
	payload.hitDistance = -1.0f;
	return payload;
}
