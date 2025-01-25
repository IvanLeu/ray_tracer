#pragma once

#include "Window.h"
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <DirectXMath.h>
#include "d3dx12\d3dx12.h"

#include <memory>

class Graphics {
public:
	Graphics(Window& wnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();
	void BeginFrame();
	void EndFrame();
	inline void PutPixel(int x, int y, DirectX::XMFLOAT4 color)
	{
		assert(x >= 0 && x <= width);
		assert(y >= 0 && y <= height);

		pixels[y * width + x] = color;
	}
	inline DirectX::XMFLOAT4 GetPixel(int x, int y) const
	{
		assert(x >= 0 && x <= width);
		assert(y >= 0 && y <= height);

		return  pixels[y * width + x];
	}
private:
	void StartUp(Window& wnd);
	void ShutDown();
	void InitImGuiGfx();
private:
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> pDebug;
#endif
	DirectX::XMMATRIX proj;
	static constexpr UINT nBuffers = 2;
	static constexpr DirectX::XMFLOAT4 clearTextureColor{ 1.0f, 0.0f, 0.0f, 1.0f };
	int width;
	int height;
	UINT rtvIncrementSize = 0;
	UINT fenceValue = 0;
	UINT nIndices = 0;
	UINT64 uploadTextureBufferSize = 0;
	D3D12_VIEWPORT viewPort;
	D3D12_RECT rect;
	Microsoft::WRL::ComPtr<ID3D12Device2> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Resource> pRenderTargets[nBuffers];
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList;
	Microsoft::WRL::ComPtr<ID3D12Fence> pFence;
	Microsoft::WRL::ComPtr<ID3D12Resource> pVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D12Resource> pIndexBuffer;
	Microsoft::WRL::ComPtr<ID3DBlob> pVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pPixelShader;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> pRootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> PSO;
	Microsoft::WRL::ComPtr<ID3D12Resource> pTexture;
	Microsoft::WRL::ComPtr<ID3D12Resource> pUploadTexture;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> srvHeap;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> imguiHeap;
	D3D12_SUBRESOURCE_DATA textureData;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	std::unique_ptr<DirectX::XMFLOAT4[]> pixels = nullptr;
};