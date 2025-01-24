#include "Graphics.h"

#include <d3dcompiler.h>
#include <stdexcept>
#include <ranges>
#include <vector>
#include <cassert>

using namespace Microsoft::WRL;
using namespace DirectX;

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
	height(wnd.GetHeight()),
	pixels(new XMFLOAT4[width * height])
{
	viewPort = CD3DX12_VIEWPORT(0.0f, 0.0f, (FLOAT)width, (FLOAT)height);
	rect = CD3DX12_RECT(0, 0, LONG_MAX, LONG_MAX);
	proj = XMMatrixTranspose(XMMatrixScaling(1.0f, float(width / height), 1.0f));
	StartUp(wnd);
}

void Graphics::StartUp(Window& wnd) {
	UINT factoryFlags = 0;

#ifdef _DEBUG
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

	// Vertex Buffer 
	{
		struct Vertex {
			XMFLOAT3 pos;
			XMFLOAT2 tc;
		};

		Vertex vertices[] = {
			{{ -1.0f, -1.0f, 0.0f }, {0.0f, 0.0f}}, // 0
			{{ -1.0f, 1.0f, 0.0f }, {0.0f, 1.0f}},  // 1
			{{ 1.0f, 1.0f, 0.0f }, {1.0f, 1.0f}},   // 2
			{{ 1.0f, -1.0f, 0.0f }, {1.0f, 0.0f}},  // 3
		};

		{
			auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				NULL,
				IID_PPV_ARGS(&pVertexBuffer)));
		}

		ComPtr<ID3D12Resource> pVertexUploadBuffer;
		{
			auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices));
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				NULL,
				IID_PPV_ARGS(&pVertexUploadBuffer)));
		}

		Vertex* uploadData = nullptr;
		ThrowIfFailed(pVertexUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&uploadData)));
		std::ranges::copy(vertices, uploadData);
		pVertexUploadBuffer->Unmap(0, nullptr);

		ThrowIfFailed(pCommandAllocator->Reset());
		ThrowIfFailed(pCommandList->Reset(pCommandAllocator.Get(), nullptr));

		pCommandList->CopyResource(pVertexBuffer.Get(), pVertexUploadBuffer.Get());

		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pVertexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
			pCommandList->ResourceBarrier(1, &barrier);
		}

		ThrowIfFailed(pCommandList->Close());

		ID3D12CommandList* lists[] = { pCommandList.Get() };
		pCommandQueue->ExecuteCommandLists(1, lists);

		ThrowIfFailed(pCommandQueue->Signal(pFence.Get(), ++fenceValue));
		ThrowIfFailed(pFence->SetEventOnCompletion(fenceValue, nullptr));

		vertexBufferView = {
			.BufferLocation = pVertexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = sizeof(vertices),
			.StrideInBytes = sizeof(Vertex)
		};
	}

	// Index Buffer 
	{
		WORD indices[] = {
			0,1,2,0,2,3
		};

		nIndices = (UINT)std::size(indices);

		{
			auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_COPY_DEST,
				NULL,
				IID_PPV_ARGS(&pIndexBuffer)));
		}

		ComPtr<ID3D12Resource> pIndexUploadBuffer;
		{
			auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
			auto resourceDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices));
			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				NULL,
				IID_PPV_ARGS(&pIndexUploadBuffer)));
		}

		WORD* uploadData = nullptr;
		ThrowIfFailed(pIndexUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&uploadData)));
		std::ranges::copy(indices, uploadData);
		pIndexUploadBuffer->Unmap(0, nullptr);

		ThrowIfFailed(pCommandAllocator->Reset());
		ThrowIfFailed(pCommandList->Reset(pCommandAllocator.Get(), nullptr));

		pCommandList->CopyResource(pIndexBuffer.Get(), pIndexUploadBuffer.Get());

		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pIndexBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_COMMON);
			pCommandList->ResourceBarrier(1, &barrier);
		}

		ThrowIfFailed(pCommandList->Close());

		ID3D12CommandList* lists[] = { pCommandList.Get() };
		pCommandQueue->ExecuteCommandLists(1, lists);

		ThrowIfFailed(pCommandQueue->Signal(pFence.Get(), ++fenceValue));
		ThrowIfFailed(pFence->SetEventOnCompletion(fenceValue, nullptr));

		indexBufferView = {
			.BufferLocation = pIndexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = sizeof(indices),
			.Format = DXGI_FORMAT_R16_UINT
		};
	}

	// Root Signature
	{
		const D3D12_ROOT_SIGNATURE_FLAGS rootFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
			D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS;

		CD3DX12_ROOT_SIGNATURE_DESC rootDesc;

		CD3DX12_DESCRIPTOR_RANGE descRange;
		descRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		CD3DX12_ROOT_PARAMETER rootParams[2];
		rootParams[0].InitAsConstants(sizeof(XMMATRIX) / 4, 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
		rootParams[1].InitAsDescriptorTable(1, &descRange, D3D12_SHADER_VISIBILITY_PIXEL);

		CD3DX12_STATIC_SAMPLER_DESC desc(0, D3D12_FILTER_MIN_MAG_MIP_LINEAR);

		rootDesc.Init((UINT)std::size(rootParams), &rootParams[0], 1, &desc, rootFlags);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootDesc,
			D3D_ROOT_SIGNATURE_VERSION_1_0,
			&signature,
			&error));

		ThrowIfFailed(pDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&pRootSignature)));
	}

	// Pipeline State Object
	{
		struct PipelineStateStream {
			CD3DX12_PIPELINE_STATE_STREAM_ROOT_SIGNATURE rootSignature;
			CD3DX12_PIPELINE_STATE_STREAM_INPUT_LAYOUT inputLayout;
			CD3DX12_PIPELINE_STATE_STREAM_PRIMITIVE_TOPOLOGY topology;
			CD3DX12_PIPELINE_STATE_STREAM_VS VS;
			CD3DX12_PIPELINE_STATE_STREAM_PS PS;
			CD3DX12_PIPELINE_STATE_STREAM_RENDER_TARGET_FORMATS rtvFormats;
		} pipelineStateStream;

		const D3D12_INPUT_ELEMENT_DESC inputDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
		};

		ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", &pVertexShader));
		ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", &pPixelShader));

		pipelineStateStream.rootSignature = pRootSignature.Get();
		pipelineStateStream.inputLayout = { .pInputElementDescs = inputDesc, .NumElements = std::size(inputDesc) };
		pipelineStateStream.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipelineStateStream.VS = CD3DX12_SHADER_BYTECODE{pVertexShader.Get()};
		pipelineStateStream.PS = CD3DX12_SHADER_BYTECODE{pPixelShader.Get()};
		pipelineStateStream.rtvFormats = {
			.RTFormats = {DXGI_FORMAT_B8G8R8A8_UNORM},
			.NumRenderTargets = 1
		};

		D3D12_PIPELINE_STATE_STREAM_DESC desc = {
			.SizeInBytes = sizeof(pipelineStateStream),
			.pPipelineStateSubobjectStream = &pipelineStateStream
		};

		ThrowIfFailed(pDevice->CreatePipelineState(&desc, IID_PPV_ARGS(&PSO)));
	}

	// Main texture
	{
		// Default
		{
			auto heapProps = CD3DX12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_DEFAULT };
			auto resDesc = CD3DX12_RESOURCE_DESC::Tex2D(
				DXGI_FORMAT_R32G32B32A32_FLOAT,
				width,
				height,
				1u,
				1u);

			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProps, 
				D3D12_HEAP_FLAG_NONE, 
				&resDesc, 
				D3D12_RESOURCE_STATE_COMMON, 
				NULL, 
				IID_PPV_ARGS(&pTexture)));
		}

		uploadTextureBufferSize = GetRequiredIntermediateSize(pTexture.Get(), 0, 1);

		// Upload
		{
			auto heapProps = CD3DX12_HEAP_PROPERTIES{ D3D12_HEAP_TYPE_UPLOAD };
			auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(uploadTextureBufferSize);

			ThrowIfFailed(pDevice->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				NULL,
				IID_PPV_ARGS(&pUploadTexture)));
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC desc;
		desc.Format = pTexture->GetDesc().Format;
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Texture2D = { .MipLevels = 1 };

		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {
			.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
			.NumDescriptors = 1,
			.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			.NodeMask = 0
		};

		ThrowIfFailed(pDevice->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&srvHeap)));

		pDevice->CreateShaderResourceView(pTexture.Get(), &desc, srvHeap->GetCPUDescriptorHandleForHeapStart());

		textureData.pData = pixels.get();
		textureData.RowPitch = width * sizeof(XMFLOAT4);
		textureData.SlicePitch = textureData.RowPitch * height;
	}
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
	std::ranges::fill_n(pixels.get(), width * height, clearTextureColor);
}

void Graphics::EndFrame()
{
	ThrowIfFailed(pCommandAllocator->Reset());
	ThrowIfFailed(pCommandList->Reset(pCommandAllocator.Get(), nullptr));

	// Update the texture
	{
		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pTexture.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
			pCommandList->ResourceBarrier(1, &barrier);
		}

		UpdateSubresources(pCommandList.Get(), pTexture.Get(), pUploadTexture.Get(), 0, 0, 1, &textureData);

		{
			auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			pCommandList->ResourceBarrier(1, &barrier);
		}
	}

	UINT frameIndex = pSwapChain->GetCurrentBackBufferIndex();
	auto rtvHandle = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtvHeap->GetCPUDescriptorHandleForHeapStart(), frameIndex, rtvIncrementSize);

	pCommandList->RSSetViewports(1, &viewPort);
	pCommandList->RSSetScissorRects(1, &rect);

	pCommandList->SetGraphicsRootSignature(pRootSignature.Get());
	pCommandList->SetPipelineState(PSO.Get());

	pCommandList->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	pCommandList->IASetIndexBuffer(&indexBufferView);

	pCommandList->OMSetRenderTargets(1, &rtvHandle, TRUE, nullptr);

	pCommandList->SetDescriptorHeaps(1, srvHeap.GetAddressOf());
	pCommandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / 4, &proj, 0);
	pCommandList->SetGraphicsRootDescriptorTable(1, srvHeap->GetGPUDescriptorHandleForHeapStart());

	{
		auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(pRTV[frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
		pCommandList->ResourceBarrier(1, &barrier);
	}

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };

	pCommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	pCommandList->DrawIndexedInstanced(nIndices, 1, 0, 0, 0);

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

