#include "Precompiled.h"
#include "Screen.h"
#include "ScreenIO.h"

namespace D3D
{

	void CMouseInputHandler::DispatchMouseWheel(Int16 Amount)
	{
		for (auto& Sink : Sinks)
		{
			Sink->MouseWheel(Amount);
		}
	}

	void CMouseInputHandler::DispatchMouseClick(const IntPoint& Position, Int Button, bool Up)
	{
		for (auto& Sink : Sinks)
		{
			Sink->MouseClick(Position, Button, Up);
		}
	}

	void CMouseInputHandler::DispatchMouseMove(const IntPoint& Position)
	{
		for (auto& Sink : Sinks)
		{
			Sink->MouseMove(Position);
		}
	}

	void IMouseSink::Unregister()
	{
		if (Handler)
		{
			auto Iter = std::find(Handler->Sinks.begin(), Handler->Sinks.end(), SharedFromThis<IMouseSink>::GetSharedPointer());

			if (Iter != Handler->Sinks.end())
			{
				Handler->Sinks.erase(Iter);
			}
		}
	}

	IMouseSink::~IMouseSink()
	{
		Unregister();
	}

	CKeyInputHandler::CKeyInputHandler(WeakPointer<KeyActionHandler> ParentHandler)
		: KeyInputActionHandler(ParentHandler)
	{}

	CScreenInputHandler::CScreenInputHandler(CScreen * Screen, WeakPointer<KeyActionHandler> ParentHandler)
		: Screen(Screen)
		, CKeyInputHandler(ParentHandler)
	{}

	void CScreenInputHandler::SetupKeyMapping(const File::CConfig& Config)
	{
		CSceneController * Controller = Screen->GetSceneController();

		StringValue<StringView> Value;

		if (Config.GetValue("Move Camera Forward", Value))
			AddKeyAction(new KeyActionFunction(Value, [=](bool Pressed) {
				if (Pressed)
					Controller->MoveForward();
				else
					Controller->MoveBackward();
			}));

		if (Config.GetValue("Move Camera Backward", Value))
			AddKeyAction(new KeyActionFunction(Value, [=](bool Pressed) {
				if (Pressed)
					Controller->MoveBackward();
				else
					Controller->MoveForward();
			}));

		if (Config.GetValue("Move Camera Left", Value))
			AddKeyAction(new KeyActionFunction(Value, [=](bool Pressed) {
				if (Pressed)
					Controller->MoveLeft();
				else
					Controller->MoveRight();
			}));

		if (Config.GetValue("Move Camera Right", Value))
			AddKeyAction(new KeyActionFunction(Value, [=](bool Pressed) {
				if (Pressed)
					Controller->MoveRight();
				else
					Controller->MoveLeft();
			}));

		if (Config.GetValue("Move Camera Up", Value))
			AddKeyAction(new KeyActionFunction(Value, [=](bool Pressed) {
				if (Pressed)
					Controller->MoveUp();
				else
					Controller->MoveDown();
		}));

		if (Config.GetValue("Move Camera Down", Value))
			AddKeyAction(new KeyActionFunction(Value, [=](bool Pressed) {
				if (Pressed)
					Controller->MoveDown();
				else
					Controller->MoveUp();
		}));

		Config.GetValue("Move Camera Down", Value);
		Config.GetValue("Rotate Camera Left", Value);
		Config.GetValue("Rotate Camera Right", Value);
		Config.GetValue("Zoom Camera In", Value);
		Config.GetValue("Zoom Camera Out", Value);
	}

	void CScreenInputHandler::Process()
	{
		if (CombinationLast.CombinationCode() != 0)
		{
			KeyInputActionHandler->HandleKeyAction(CombinationLast.CombinationCode(), false);
		}

		CombinationLast = Combination;

		KeyInputActionHandler->HandleKeyAction(Combination.CombinationCode(), true);
	}

	void CScreenInputHandler::HandleInput(UINT Message, WPARAM wParam, LPARAM lParam)
	{
		switch (Message)
		{
			case WM_KEYDOWN:
			{
				KeyDown(wParam);
				break;
			}
			case WM_KEYUP:
			{
				KeyUp(wParam);
				break;
			}
			case WM_MOUSEHWHEEL:
			{
				DispatchMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
				break;
			}
			case WM_MOUSEMOVE:
			{
				DispatchMouseMove(IntPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)));
				break;
			}
			case WM_LBUTTONDOWN:
			{
				DispatchMouseClick(IntPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), VK_LBUTTON, false);
				break;
			}
			case WM_LBUTTONUP:
			{
				DispatchMouseClick(IntPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), VK_LBUTTON, true);
				break;
			}
			case WM_RBUTTONUP:
			{
				DispatchMouseClick(IntPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), VK_RBUTTON, false);
				break;
			}
			case WM_RBUTTONDOWN:
			{
				DispatchMouseClick(IntPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)), VK_RBUTTON, true);
				break;
			}
		}
		Keystate = GET_KEYSTATE_WPARAM(wParam);
	}
}