#include "Camera.h"
#include "VectorUtils.h"
#include <algorithm>

using namespace DirectX;

Camera::Camera(int width, int height, float fovAngleYDegrees, float nearClip, float farClip)
	:
	m_Width(width),
	m_Height(height),
	fovAngleY(fovAngleYDegrees),
	nearClip(nearClip),
	farClip(farClip),
	m_Position(XMVectorSet(0.0f, 0.0f, -2.0f, 1.0f))
{
	aspectRatio = (float)m_Width / (float)m_Height;
	rayDirections.resize(m_Width * m_Height);

	m_forwardDir = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	m_rightDir = XMVector3Normalize(XMVector3Cross(m_forwardDir, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));

	RecalculateProjection();
	RecalculateView();
	RecalculateRayDirections();
}

const std::vector<DirectX::XMFLOAT3>& Camera::GetRayDirections() const noexcept
{
	return rayDirections;
}

void Camera::Move(float dt, DirectX::XMFLOAT3 v) noexcept
{
	if (Utils::IsZero(v)) {
		return;
	}

	moved = true;

	// v components should be just +-1 or 0
	if (v.x != 0) {
		m_Position += m_rightDir * v.x * moveSpeed * dt;
	}
	if (v.y != 0) {
		m_Position += XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f) * v.y * moveSpeed * dt;
	}
	if (v.z != 0) {
		m_Position += m_forwardDir * v.z * moveSpeed * dt;
	}
}

void Camera::Rotate(float dt, float deltaX, float deltaY) noexcept
{
	if (deltaX == 0 && deltaY == 0) {
		return;
	}

	moved = true;

	float pitchDelta = deltaY * rotationSpeed * dt;
	float yawDelta = deltaX * rotationSpeed * dt;

	auto q1 = XMQuaternionRotationAxis(m_rightDir, pitchDelta);
	auto q2 = XMQuaternionRotationAxis(XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f), yawDelta);

	m_forwardDir = XMVector3Rotate(m_forwardDir, XMQuaternionNormalize(XMQuaternionMultiply(q1, q2)));
	m_rightDir = XMVector3Normalize(XMVector3Cross(m_forwardDir, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
}

void Camera::RecalculateRayDirections() noexcept
{
	for (int y = 0; y < m_Height; ++y) {
		for (int x = 0; x < m_Width; ++x) {
			DirectX::XMFLOAT2 coord = { (float)x / (float)m_Width, (float)y / (float)m_Height };
			coord.x = coord.x * 2.0f - 1.0f;
			coord.y = coord.y * 2.0f - 1.0f;

			XMVECTOR target = XMVector4Transform(XMVectorSet(coord.x, coord.y, 1.0f, 1.0f), m_InverseProjection); // homogeneous clip space
			
			target = XMVectorScale(target, 1.0f / XMVectorGetW(target)); // perspective divide

			XMVECTOR rayVector = XMVector3Normalize(XMVector3TransformNormal(target, m_InverseView)); // world space

			XMFLOAT3 rayDirection;
			XMStoreFloat3(&rayDirection, rayVector);
			rayDirections[x + y * m_Width] = rayDirection;
		}
	}
}

void Camera::Update() noexcept
{
	if (moved) {
		RecalculateView();
		RecalculateRayDirections();
		moved = false;
	}
}

void Camera::SetMoveSpeed(float speed) noexcept
{
	moveSpeed = speed;
}

void Camera::SetRotationSpeed(float speed) noexcept
{
	rotationSpeed = speed;
}

void Camera::RecalculateProjection() noexcept
{
	m_Projection = XMMatrixPerspectiveFovLH(XMConvertToRadians(fovAngleY), aspectRatio, nearClip, farClip);
	auto det = XMMatrixDeterminant(m_Projection);
	m_InverseProjection = XMMatrixInverse(&det, m_Projection);
}

void Camera::RecalculateView() noexcept
{
	m_View = XMMatrixLookAtLH(m_Position, m_Position + m_forwardDir, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
	auto det = XMMatrixDeterminant(m_View);
	m_InverseView = XMMatrixInverse(&det, m_View);
}

const DirectX::XMMATRIX& Camera::GetInverseProjection() const noexcept
{
	return m_InverseProjection;
}

const DirectX::XMMATRIX& Camera::GetView() const noexcept
{
	return m_View;
}

const DirectX::XMMATRIX& Camera::GetInverseView() const noexcept
{
	return m_InverseView;
}

const DirectX::XMVECTOR& Camera::GetPosition() const noexcept
{
	return m_Position;
}

const DirectX::XMVECTOR& Camera::GetDirection() const noexcept
{
	return m_forwardDir;
}

const DirectX::XMMATRIX& Camera::GetProjection() const noexcept
{
	return m_Projection;
}