#pragma once

#include "DXGIHelper.h"

#include "RawCommandQueue.h"
#include "RawCommandAllocator.h"
#include "RawRenderTarget.h"
#include "RawDepthStencilView.h"
#include "RawFence.h"

#include "Input.h"
#include "Scene.h"
#include "SceneController.h"

#include "KeySystem.h"

#define BUFFER_COUNT	2
#define SAMPLE_COUNT	1

namespace D3D
{
	class CKeyInputHandler
	{
		Uint64 KeySet[4] = { 0 };
		Uint32 PressedKeyCount = 0;

	protected:

		SharedPointer<KeyActionHandler> KeyInputActionHandler;
		KeyCombination Combination;

	public:

		virtual void Process() = 0;

		void KeyDown(Uint8 K)
		{
			if (PressedKeyCount > 3)
			{
				return;
			}

			++PressedKeyCount;

			const auto I = K / 64;
			const auto O = K % 64;
			{
				KeySet[I] |= (1 << O);
			}

			Combination.SetKey(&KeyMap::Array[K]);
			{
				Process();
			}
		}

		void KeyUp(Uint8 K)
		{
			if (PressedKeyCount < 1)
			{
				return;
			}

			--PressedKeyCount;

			const auto I = K / 64;
			const auto O = K % 64;
			{
				KeySet[I] &= ~(1 << O);
			}

			Combination.UnsetKey(&KeyMap::Array[K]);
		}

		bool IsKeyPressed(Uint8 K)
		{
			const auto I = K / 64;
			const auto O = K % 64;
			{
				return (KeySet[I] & (1 >> O)) == 0;
			}
		}
	};

	class CMouseInputHandler
	{
	public:

		virtual void Process() = 0;
	};

	class CScreenInputHandler : public CKeyInputHandler, public CMouseInputHandler
	{

	public:

		CScreenInputHandler(WeakPointer<KeyActionHandler> ParentHandler = nullptr)
		{
			KeyInputActionHandler = new KeyActionHandler(ParentHandler);
		}

	private:
		
		virtual void Process() override
		{
			const auto Code = Combination.CombinationCode();
			
			if (Code != 0)
			{
				KeyInputActionHandler->HandleKeyAction(Combination.CombinationCode());
			}
		}
	};

	class CScreen
	{
	public:

		struct InitializeParameter
		{
			ScreenWindow	DefaultWindow;
			ScreenViewport	DefaultViewport;

			inline InitializeParameter()
			{}

			inline InitializeParameter
			(
				const ScreenWindow		& Window,
				const ScreenViewport	& Viewport
			)
			{
				DefaultWindow	= Window;
				DefaultViewport = Viewport;
			}
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

		inline const RCommandQueue & GetDirectCommandQueue() const
		{
			return CommandQueueDirect.GetRef();
		}

		inline const RCommandQueue & GetComputeCommandQueue() const
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

		inline void WaitForGPU() const
		{
			CCommandQueueDirect::Instance().WaitForGPU(BackBufferIndex);
		}

		inline void NextFrame()
		{
			const UINT64 Value = CCommandQueueDirect::Instance().SignalFence(BackBufferIndex) + 1;
			{
				OnAfterRender();
			}

			CCommandQueueDirect::Instance().WaitForCompletion(BackBufferIndex);
			CCommandQueueDirect::Instance().GotoNextFrame(BackBufferIndex, Value);
		}

	private:

		SharedPointer<CScene>			Scene;
		SharedPointer<CSceneController> SceneController;

		SharedPointer<CInput> Input;

	public:

		inline CScene * GetActiveScene() const
		{
			return Scene.Get();
		}

	private:

		ErrorCode InitializeScene();
		ErrorCode InitializeInput();

	public:

		void Render()
		{
			SceneController->Update();
			Scene->RenderScene();
		}

		void OnActivation(UINT Message, WPARAM wParam, LPARAM lParam)
		{
			if (Input)
			{
				Input->AcquireInput(Message, wParam, lParam);
			}
		}
		
		void OnMouseInput(UINT Message, WPARAM wParam, LPARAM lParam)
		{
			Input->AcquireMouseInput(Message, wParam, lParam);

			int MouseX;
			int MouseY;
			int MouseZ;

			Input->GetMouseXYZ
			(
				MouseX, 
				MouseY, 
				MouseZ
			);

			switch (Input->LeftButtonState())
			{
				case MouseState::PRESSED:
				{
					Input->SetMouseMode(Mouse::MODE_RELATIVE);

					SceneController->BeginDrag
					(
						MouseX,
						MouseY
					);
				}

				break;

				case MouseState::HELD:
				{
					SceneController->Drag
					(
						MouseX,
						MouseY
					);
				}

				break;

				case MouseState::RELEASED:
				{
					Input->SetMouseMode(Mouse::MODE_ABSOLUTE);

					SceneController->EndDrag();
				}

				break;
			}
		}

		void OnKeyboardInput(UINT Message, WPARAM wParam, LPARAM lParam)
		{
			Input->AcquireKeyboardInput(Message, wParam, lParam);

			if (Input->IsKeyDown(KeyboardKey::W))
			{
				SceneController->MoveForward();
			}
			else if (Input->IsKeyReleased(KeyboardKey::W))
			{
				SceneController->StopMove();
			}

			if (Input->IsKeyDown(KeyboardKey::S))
			{
				SceneController->MoveBackward();
			}
			else if (Input->IsKeyReleased(KeyboardKey::S))
			{
				SceneController->StopMove();
			}

			if (Input->IsKeyDown(KeyboardKey::D))
			{
				SceneController->MoveRight();
			}
			else if (Input->IsKeyReleased(KeyboardKey::D))
			{
				SceneController->StopMove();
			}

			if (Input->IsKeyDown(KeyboardKey::A))
			{
				SceneController->MoveLeft();
			}
			else if (Input->IsKeyReleased(KeyboardKey::A))
			{
				SceneController->StopMove();
			}
		}

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