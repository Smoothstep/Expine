#pragma once

#include <DXTK\Mouse.h>
#include <DXTK\Keyboard.h>

#include "D3D.h"

namespace D3D
{
	using namespace DirectX;

	typedef Mouse::ButtonStateTracker::ButtonState	MouseState;
	typedef Keyboard::Keys							KeyboardKey;

	class CInput
	{
	private:

		std::unique_ptr<Mouse>			m_pMouse;
		Mouse::State					m_MouseState;
		Mouse::ButtonStateTracker		m_MouseButtonStateTracker;

		std::unique_ptr<Keyboard>		m_pKeyboard;
		Keyboard::State					m_KeyboardState;
		Keyboard::KeyboardStateTracker	m_KeyboardStateTracker;

	public:

		CInput() :

			m_pMouse
			(
				std::make_unique<Mouse>()
			),
			m_pKeyboard
			(
				std::make_unique<Keyboard>()
			)
		{
			Reset();
		}

		CInput
		(
			HWND hWnd
		) :
			m_pMouse
			(
				std::make_unique<Mouse>()
			),
			m_pKeyboard
			(
				std::make_unique<Keyboard>()
			)
		{
			Reset();
			SetMouseWindow(hWnd);
		}

		inline void ResetMouse()
		{
			m_MouseButtonStateTracker.Reset();
		}

		inline void ResetKeyboard()
		{
			m_KeyboardStateTracker.Reset();
		}

		inline void Reset()
		{
			ResetMouse();
			ResetKeyboard();
		}

		inline void SetMouseWindow(HWND hWnd)
		{
			m_pMouse->SetWindow(hWnd);
		}

		inline void SetMouseMode(Mouse::Mode Mode)
		{
			m_pMouse->SetMode(Mode);
		}

		inline void AcquireKeyboardInput
		(
			UINT	iMessage,
			WPARAM	wParam,
			LPARAM	lParam
		)
		{
			m_pKeyboard->ProcessMessage
			(
				iMessage,
				wParam,
				lParam
			);

			m_KeyboardStateTracker.Update(m_KeyboardState = m_pKeyboard->GetState());
		}

		inline void AcquireMouseInput
		(
			UINT	iMessage,
			WPARAM	wParam,
			LPARAM	lParam
		)
		{
			m_pMouse->ResetScrollWheelValue();
			m_pMouse->ProcessMessage
			(
				iMessage,
				wParam,
				lParam
			);

			m_MouseButtonStateTracker.Update(m_MouseState = m_pMouse->GetState());
		}

		inline void AcquireInput
		(
			UINT	iMessage,
			WPARAM	wParam,
			LPARAM	lParam
		)
		{
			AcquireKeyboardInput
			(
				iMessage,
				wParam,
				lParam
			);

			AcquireMouseInput
			(
				iMessage,
				wParam,
				lParam
			);
		}

		inline int GetMouseX() const
		{
			return m_MouseState.x;
		}

		inline int GetMouseY() const
		{
			return m_MouseState.y;
		}

		inline int GetMouseWheel() const
		{
			return m_MouseState.scrollWheelValue;
		}

		inline void GetMouseXYZ
		(
			int & x,
			int & y,
			int & z
		)	const
		{
			x = GetMouseX();
			y = GetMouseY();
			z = GetMouseWheel();
		}

		inline MouseState LeftButtonState() const
		{
			return m_MouseButtonStateTracker.leftButton;
		}

		inline MouseState RightButtonState() const
		{
			return m_MouseButtonStateTracker.rightButton;
		}

		inline bool IsLeftButton(MouseState State) const
		{
			return m_MouseButtonStateTracker.leftButton == State;
		}

		inline bool IsRightButton(MouseState State) const
		{
			return m_MouseButtonStateTracker.rightButton == State;
		}

		inline bool IsKeyPressed(KeyboardKey Key) const
		{
			return m_KeyboardStateTracker.IsKeyPressed(Key);
		}

		inline bool IsKeyReleased(KeyboardKey Key) const
		{
			return m_KeyboardStateTracker.IsKeyReleased(Key);
		}

		inline bool IsKeyUp(KeyboardKey Key) const
		{
			return m_KeyboardState.IsKeyUp(Key);
		}

		inline bool IsKeyDown(KeyboardKey Key) const
		{
			return m_KeyboardState.IsKeyDown(Key);
		}
	};
}