#include "Precompiled.h"
#include "Command/CommandQueue.h"
#include "Scene/Scene.h"
#include "Scene/SceneView.h"
#include "Scene/SceneRenderer.h"
#include "ScreenIO.h"
#include "Screen.h"

namespace D3D
{
	void CScreen::WaitForGPU() const
	{
		CCommandQueueDirect::Instance().WaitForGPU(BackBufferIndex);
	}

	void CScreen::NextFrame()
	{
		const UINT64 Value = CCommandQueueDirect::Instance().SignalFence(BackBufferIndex) + 1;
		{
			OnAfterRender();
		}

		CCommandQueueDirect::Instance().WaitForCompletion(BackBufferIndex);
		CCommandQueueDirect::Instance().GotoNextFrame(BackBufferIndex, Value);
	}

	ErrorCode CScreen::InitializeCommandAllocatorAndQueue()
	{
		ErrorCode Error;

		CommandQueueCompute = CCommandQueueCompute::Instance_Pointer();
		CommandQueueDirect = CCommandQueueDirect::Instance_Pointer();
		CommandQueueCopy = CCommandQueueCopy::Instance_Pointer();

		for (UINT N = 0; N < FRAME_COUNT; ++N)
		{
			CommandAllocator[N] = new RCommandAllocator();

			if ((Error = CommandAllocator[N]->Create(D3D12_COMMAND_LIST_TYPE_DIRECT)))
			{
				return Error;
			}
		}

		return S_OK;
	}

	ErrorCode CScreen::InitializeBackBuffer()
	{
		ErrorCode Error;

		if (BackBuffer.DescriptorHeap == NULL)
		{
			BackBuffer.DescriptorHeap = new RDescriptorHeap();

			if ((Error = BackBuffer.DescriptorHeap->Create_RTV(BUFFER_COUNT)))
			{
				return Error;
			}
		}

		for (UINT N = 0; N < BUFFER_COUNT; ++N)
		{
			UniquePointer<RResource> BBuffer = new RResource();

			if ((Error = BBuffer->FromSwapChain(DXGISwapChain.Get(), N)))
			{
				return Error;
			}

			BackBuffer.RenderTargetView[N] = new RRenderTargetView();

			if ((Error = BackBuffer[N]->CreateFromResource(BBuffer.Detach(), DescriptorHeapEntry(BackBuffer.DescriptorHeap.Get(), N))))
			{
				return Error;
			}
		}

		return S_OK;
	}

	ErrorCode CScreen::InitializeSwapChain()
	{
		DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
		{
			SwapChainDesc.BufferCount		= BUFFER_COUNT;
			SwapChainDesc.Height			= Window.WindowHeight;
			SwapChainDesc.Width				= Window.WindowWidth;
			SwapChainDesc.Format			= Window.Output.Format;
			SwapChainDesc.Scaling			= DXGI_SCALING_NONE;
			SwapChainDesc.BufferUsage		= DXGI_USAGE_RENDER_TARGET_OUTPUT;
			SwapChainDesc.SwapEffect		= DXGI_SWAP_EFFECT_FLIP_DISCARD;
			SwapChainDesc.SampleDesc.Count	= Window.NumSamples;
		}

		ErrorCode Error;

		DXGI_SWAP_CHAIN_FULLSCREEN_DESC FullScreenDesc = {};
		{
			FullScreenDesc.Windowed			= !Window.WindowIsFullscreen;
			FullScreenDesc.RefreshRate		= Window.Output.RefreshRate;
			FullScreenDesc.Scaling			= Window.Output.ScalingMode;
			FullScreenDesc.ScanlineOrdering = Window.Output.ScanlineOrderMode;
		}

		if ((Error = DXGIFactory->CreateSwapChainForHwnd
		(
			CommandQueueDirect->Get(),
			Window.WindowHandle,
			&SwapChainDesc,
			&FullScreenDesc,
			NULL,
			&DXGISwapChain
		)))
		{
			return Error;
		}

		if ((Error = DXGISwapChain->QueryInterface(&DXGISwapChain3)))
		{
			return Error;
		}

		return S_OK;
	}

	ErrorCode CScreen::InitializeDevice()
	{
		ErrorCode Error;

#ifdef _DEBUG
		const UINT Flags = DXGI_CREATE_FACTORY_DEBUG;
#else
		const UINT Flags = 0;
#endif

		if ((Error = CreateDXGIFactory2(Flags, IID_PPV_ARGS(&DXGIFactory))))
		{
			CErrorLog::Log() << "Unable to create factory: " << Error;
			return Error;
		}

		if ((DXGIAdapter = GetHardwareAdapter(DXGIFactory.Get())) == NULL)
		{
			CErrorLog::Log() << "Unable to find compatible adapter: " << E_FAIL;
			return E_FAIL;
		}

		if ((DXGIOutput = GetBestOutput(DXGIAdapter.Get(), Window)) == NULL)
		{
			CErrorLog::Log() << "Unable to find compatible output: " << E_FAIL;
			return E_FAIL;
		}

		if ((Error = DEVICE.CreateDevice(DXGIAdapter.Get())))
		{
			CErrorLog::Log() << "Unable to create device: " << Error;
			return Error;
		}

		return S_OK;
	}

	CScreen::CScreen()
	{

	}

	CScreen::~CScreen()
	{}

	ErrorCode CScreen::Create(const CScreen::InitializeParameter & Parameter)
	{
		Window = Parameter.DefaultWindow;
		Viewport = Parameter.DefaultViewport;

		ErrorCode Error;

		if ((Error = InitializeDevice()))
		{
			return Error;
		}

		if ((Error = InitializeCommandAllocatorAndQueue()))
		{
			CErrorLog::Log() << "Unable to create command allocator and list: " << Error;
			return Error;
		}

		if ((Error = InitializeInput()))
		{
			CErrorLog::Log() << "Unable to initialize input: " << Error;
			return Error;
		}

		if ((Error = InitializeSwapChain()))
		{
			CErrorLog::Log() << "Unable to create swap chain: " << Error;
			return Error;
		}

		if ((Error = InitializeBackBuffer()))
		{
			CErrorLog::Log() << "Unable to get Backbuffer: " << Error;
			return Error;
		}

		if ((Error = InitializeScene()))
		{
			CErrorLog::Log() << "Unable to create scene: " << Error;
			return Error;
		}

		return S_OK;
	}

	CScene * CScreen::GetActiveScene() const
	{
		return Scene.Get();
	}

	CSceneController * CScreen::GetSceneController() const
	{
		return SceneController.Get();
	}

	ErrorCode CScreen::InitializeScene()
	{
		ErrorCode Error;

		Scene.Construct(this);

		if ((Error = Scene->Initialize()))
		{
			return Error;
		}

		SceneController.Construct(Scene.Get());

		return S_OK;
	}

	ErrorCode CScreen::InitializeInput()
	{
		InputHandler = new CScreenInputHandler(Window.WindowHandle);
		{
			return S_OK;
		}
	}

	void CScreen::Render()
	{
		SceneController->Update();
		Scene->RenderScene();
	}

	void CScreen::Message(UINT Message, WPARAM wParam, LPARAM lParam)
	{
		InputHandler->HandleInput(Message, wParam, lParam);
	}
}
