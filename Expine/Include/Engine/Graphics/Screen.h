#pragma once

#include "Utils/DXGIHelper.h"

#include "Raw/RawCommandQueue.h"
#include "Raw/RawCommandAllocator.h"
#include "Raw/RawRenderTarget.h"
#include "Raw/RawDepthStencilView.h"
#include "Raw/RawFence.h"

#include "Scene/Scene.h"
#include "Scene/SceneController.h"

#define BUFFER_COUNT	2
#define SAMPLE_COUNT	1

namespace D3D
{
	class _EX_ CScreen
	{
	public:

		struct InitializeParameter
		{
			ScreenWindow	DefaultWindow;
			ScreenViewport	DefaultViewport;

			InitializeParameter() 
				: DefaultWindow(D3D::GlobalDefault)
				, DefaultViewport()
			{}

			explicit constexpr InitializeParameter
			(
				const ScreenWindow		& Window,
				const ScreenViewport	& Viewport
			) 
				: DefaultWindow(Window)
				, DefaultViewport(Viewport)
			{}
		};

	private:

		// Screen properties

		ScreenWindow						Window;
		ScreenViewport						Viewport;

		// Default Queue & Allocator
		
		SharedPointer<CCommandQueueDirect>	CommandQueueDirect;
		SharedPointer<CCommandQueueCompute> CommandQueueCompute;
		SharedPointer<CCommandQueueCopy>	CommandQueueCopy;

		SharedPointer<RCommandAllocator>	CommandAllocator[FRAME_COUNT];

		// Default output target

		RenderTargetViewGroup<FRAME_COUNT>	BackBuffer;
		
		// Index of the backbuffer. (From 0 to FRAME_COUNT - 1)

		std::atomic<UINT> BackBufferIndex = 0;

		// Default Values

		constexpr static DXGI_USAGE			BufferUsageDefault	= DXGI_USAGE_RENDER_TARGET_OUTPUT;
		constexpr static DXGI_SWAP_EFFECT	SwapEffectDefault	= DXGI_SWAP_EFFECT_FLIP_DISCARD;
		constexpr static UINT				SampleCountDefault	= SAMPLE_COUNT;
		constexpr static DXGI_SCALING		ScalingDefault		= DXGI_SCALING_NONE;
		

	private:

		ComPointer<IDXGIOutput>				DXGIOutput;
		ComPointer<IDXGIFactory4>			DXGIFactory;
		ComPointer<IDXGIAdapter1>			DXGIAdapter;
		ComPointer<IDXGISwapChain1>			DXGISwapChain;
		ComPointer<IDXGISwapChain3>			DXGISwapChain3;

	private:

		ErrorCode InitializeCommandAllocatorAndQueue();
		ErrorCode InitializeBackBuffer();
		ErrorCode InitializeSwapChain();
		ErrorCode InitializeDevice();

	public:

		inline const ScreenWindow & GetWindow() const
		{
			return Window;
		}

		inline const ScreenViewport & GetViewport() const
		{
			return Viewport;
		}

		inline const RCommandAllocator & GetCommandAllocator() const
		{
			return CommandAllocator[BackBufferIndex].GetRef();
		}

		inline const CCommandQueueDirect & GetDirectCommandQueue() const
		{
			return CommandQueueDirect.GetRef();
		}

		inline const CCommandQueueCopy & GetCopyCommandQueue() const
		{
			return CommandQueueCopy.GetRef();
		}

		inline const CCommandQueueCompute & GetComputeCommandQueue() const
		{
			return CommandQueueCompute.GetRef();
		}

		inline RRenderTargetView * GetBackBuffer() const
		{
			return BackBuffer[BackBufferIndex].Get();
		}

		inline RRenderTargetView * GetBackBuffer
		(
			const UINT Index
		)	const
		{
			return BackBuffer[Index].Get();
		}

	public:

		 CScreen();
		~CScreen();

		ErrorCode Create
		(
			const InitializeParameter & Parameter
		);

	public:

		inline void PresentSync() const
		{
			ThrowOnError(DXGISwapChain->Present(1, 0));
		}

		inline void Present() const
		{
			ThrowOnError(DXGISwapChain->Present(0, 0));
		}

	public:

		inline UINT GetBackBufferIndex() const
		{
			return BackBufferIndex;
		}

		inline UINT UpdateBackBufferIndex()
		{
			return BackBufferIndex = DXGISwapChain3->GetCurrentBackBufferIndex();
		}

		void WaitForGPU() const;
		void NextFrame();

	private:

		SharedPointer<CScene>				Scene;
		SharedPointer<CSceneController>		SceneController;
		UniquePointer<CScreenInputHandler>	InputHandler;

	public:

		CScene * GetActiveScene() const;
		CSceneController * GetSceneController() const;

	private:

		ErrorCode InitializeScene();
		ErrorCode InitializeInput();

	public:

		void Render();
		void Message(UINT Message, WPARAM wParam, LPARAM lParam);

	private:

		inline void OnRender()
		{

		}

		inline void OnBeginRender()
		{

		}

		inline void OnAfterRender()
		{
			UpdateBackBufferIndex();
		}
	};
}