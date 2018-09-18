#pragma once

#include <Types.h>
#include <Smart.h>

#include <Hyper/Point.h>
#include <Windowsx.h>

#include "Engine/IO/Input.h"
#include "Engine/IO/KeySystem.h"

#include <Utils/File/Config.h>

namespace D3D
{
	using namespace Hyper;

	class CKeyInputHandler
	{
		Uint64 KeySet[4] = { 0 };
		Uint32 PressedKeyCount = 0;

	protected:

		SharedPointer<KeyActionHandler> KeyInputActionHandler;

		KeyCombination Combination;
		KeyCombination CombinationLast;

	public:
		CKeyInputHandler(WeakPointer<KeyActionHandler> ParentHandler);

		virtual void Process() = 0;
		
		KeyAction * AddKeyAction(KeyAction * Action)
		{
			return KeyInputActionHandler->AddAction(Action);
		}

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

	class IMouseSink;
	class CMouseInputHandler
	{
		friend class IMouseSink;

	protected:

		TDeque<SharedPointer<IMouseSink> > Sinks;

	public:

		virtual void Process() = 0;

		void DispatchMouseWheel(Int16 Amount);
		void DispatchMouseClick(const IntPoint& Position, Int Button, bool Up);
		void DispatchMouseMove(const IntPoint& Position);
	};

	class IMouseSink : public SharedFromThis<IMouseSink>
	{
		friend class CMouseInputHandler;

	private:

		CMouseInputHandler * Handler = nullptr;

	public:

		virtual ~IMouseSink();

	protected:

		void Unregister();

		virtual void MouseClick(const IntPoint& Position, Int Button, bool Up) = 0;
		virtual void MouseMove(const IntPoint& Position) = 0;
		virtual void MouseWheel(Uint16 Amount) = 0;
	};

	class CScreenInputHandler : public CKeyInputHandler, public CMouseInputHandler
	{
	private:

		CScreen * Screen;
		Uint16 Keystate = 0;

		CSceneController * SceneController;

	public:

		CScreenInputHandler(CScreen * Screen, 
							WeakPointer<KeyActionHandler> ParentHandler = nullptr);

		void SetupKeyMapping(const File::CConfig& Config);

		void HandleInput(UINT Message, 
						 WPARAM wParam, 
						 LPARAM lParam);

	private:
		
		virtual void Process() override;
	};
}