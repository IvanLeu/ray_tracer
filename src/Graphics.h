#pragma once

#include "Window.h"
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12\d3dx12.h"

class Graphics {
public:
	Graphics(Window& wnd);
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;
	~Graphics();
	void BeginFrame();
	void EndFrame();
private:
	void StartUp(Window& wnd);
	void ShutDown();
private:
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug2> pDebug;
#endif
	static constexpr UINT nBuffers = 2;
	int width;
	int height;
	UINT rtvIncrementSize = 0;
	UINT fenceValue = 0;
	D3D12_VIEWPORT viewPort;
	D3D12_RECT rect;
	Microsoft::WRL::ComPtr<ID3D12Device2> pDevice;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> pSwapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> pCommandQueue;
	Microsoft::WRL::ComPtr<ID3D12Resource> pRTV[nBuffers];
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> pCommandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> pCommandList;
	Microsoft::WRL::ComPtr<ID3D12Fence> pFence;

};