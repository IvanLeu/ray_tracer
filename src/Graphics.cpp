#include "Graphics.h"

#include <stdexcept>

using namespace Microsoft::WRL;

static inline std::string HrToString(HRESULT hr)
{
	char s_str[64] = {};
	sprintf_s(s_str, "HRESULT of 0x%08X", static_cast<UINT>(hr));
	return std::string(s_str);
}

static inline void ThrowIfFailed(HRESULT hr)
{
	if (FAILED(hr))
	{
		throw std::runtime_error(HrToString(hr));
	}
}

Graphics::Graphics(Window& wnd)
	:
	width(wnd.GetWidth()),
	height(wnd.GetHeight())
{
	viewPort = CD3DX12_VIEWPORT(0.0f, 0.0f, (FLOAT)width, (FLOAT)height);
	rect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
	StartUp(wnd);
}

void Graphics::StartUp(Window& wnd) {
	UINT factoryFlags = 0;

#ifdef _DEBUG
	ComPtr<ID3D12Debug1> pDebug;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&pDebug)));
	pDebug->SetEnableGPUBasedValidation(TRUE);
	pDebug->EnableDebugLayer();

	factoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	ComPtr<IDXGIFactory4> pFactory;
	ThrowIfFailed(CreateDXGIFactory2(factoryFlags, IID_PPV_ARGS(&pFactory)));

	// Should check for feature level
	ThrowIfFailed(D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&pDevice)));

	// Create CommandQueue
	{
		D3D12_COMMAND_QUEUE_DESC desc;
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&pCommandQueue));
	}

	// SwapChain
	{
		DXGI_SWAP_CHAIN_DESC1 desc = {
			.Width = (UINT)width,
			.Height = (UINT)height,
			.Format = DXGI_FORMAT_B8G8R8A8_UNORM,
			.Stereo = FALSE,
			.SampleDesc = {.Count = 1, .Quality = 0},
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = nBuffers,
			.Scaling = DXGI_SCALING_STRETCH,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
			.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED,
			.Flags = 0
		};

		ComPtr<IDXGISwapChain1> pSwapChainTemp;
		ThrowIfFailed(pFactory->CreateSwapChainForHwnd(pCommandQueue.Get(), wnd.GetHandle(), &desc, nullptr, nullptr, &pSwapChainTemp));

		pSwapChainTemp.As(&pSwapChain);
	}	

	ThrowIfFailed(pFactory->MakeWindowAssociation(wnd.GetHandle(), DXGI_MWA_NO_ALT_ENTER));

	// RTV
	{
		// RTV Heap
		{
			D3D12_DESCRIPTOR_HEAP_DESC desc = {
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
				.NumDescriptors = nBuffers,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
				.NodeMask = 0
			};

			ThrowIfFailed(pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&rtvHeap)));
		}

		auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart());
		rtvIncrementSize = pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		for (int i = 0; i < nBuffers; ++i) {
			ThrowIfFailed(pSwapChain->GetBuffer(i, IID_PPV_ARGS(&pRTV[i])));
			pDevice->CreateRenderTargetView(pRTV[i].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(rtvIncrementSize);
		}
	}

	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&pCommandAllocator)));

	ThrowIfFailed(pDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, pCommandAllocator.Get(), nullptr, IID_PPV_ARGS(&pCommandList)));
	ThrowIfFailed(pCommandList->Close());

	// The fence
	ThrowIfFailed(pDevice->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&pFence)));
}

void Graphics::ShutDown()
{
	ThrowIfFailed(pCommandQueue->Signal(pFence.Get(), ++fenceValue));
	ThrowIfFailed(pFence->SetEventOnCompletion(fenceValue, nullptr));
}

Graphics::~Graphics()
{
	ShutDown();
}

void Graphics::BeginFrame()
{
	// Clear the texture
}

void Graphics::EndFrame()
{
	ThrowIfFailed(pCommandAllocator->Reset());
	ThrowIfFailed(pCommandList->Reset(pCommandAllocator.Get(), nullptr));

	UINT frameIndex = pSwapChain->GetCurrentBackBufferIndex();
	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvIncrementSize);

	pCommandList->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);

	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pRTV[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		pCommandList->ResourceBarrier(1, &barrier);
	}

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

	pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pRTV[frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		pCommandList->ResourceBarrier(1, &barrier);
	}

	ThrowIfFailed(pCommandList->Close());

	ID3D12CommandList* lists[] = {pCommandList.Get()};
	pCommandQueue->ExecuteCommandLists(1, lists);

	pSwapChain->Present(1, 0);

	ThrowIfFailed(pCommandQueue->Signal(pFence.Get(), ++fenceValue));
	ThrowIfFailed(pFence->SetEventOnCompletion(fenceValue, nullptr));
}

