#pragma once

#include <DirectX/D3D.h>
#include <type_traits>
#include <optional>

class Key
{
	const char * 
		ASCII_Representation;
	unsigned char 
		ASCII_Code;

public:
	constexpr Key() 
		: ASCII_Code(0)
		, ASCII_Representation(nullptr)
	{}

	constexpr Key(const char * Rep, const char Code) 
		: ASCII_Code(Code)
		, ASCII_Representation(Rep)
	{}

	constexpr Key & operator=(const Key& Other)
	{
		ASCII_Code = Other.ASCII_Code;
		ASCII_Representation = Other.ASCII_Representation;
		return *this;
	}

	constexpr unsigned char GetCode() const
	{
		return ASCII_Code;
	}

	constexpr const char * GetRepresentation() const
	{
		return ASCII_Representation;
	}
};

class KeyCombination
{ 
	static constexpr Uint NumKeys = 4;

private:
	TArray<const Key*, NumKeys> Keys;
	Uint32 Code;

public:
	constexpr KeyCombination() 
		: Code(0)
		, Keys({0, 0, 0, 0})
	{}

	constexpr Uint32 CombinationCode() const
	{
		return Code;
	}

	inline void SetKey(const Key * Ky, const Uint32 Index = -1)
	{
		Keys[Index] = Ky;

		if (Index == -1)
		{
			for (Uint32 N = 0; N < 4; ++N)
			{
				const Key * K = Keys[N];

				if (K == NULL)
				{
					Code |= (static_cast<Uint32>(Ky->GetCode()) << (N * 8));
					break;
				}
			}
		}
		else
		{
			Code |= (static_cast<Uint32>(Ky->GetCode()) << (Index * 8));
		}
	}

	inline void UnsetKey(const Key * K)
	{
		for (Uint32 N = 0; Keys[N] != NULL; ++N)
		{
			const Key * Ky = Keys[N];

			if (Ky->GetCode() == K->GetCode())
			{
				Keys[N] = NULL;
				Code &= ~(static_cast<Uint32>(Ky->GetCode()) << (N * 8));
				break;
			}
		}
	}

	inline const Key * GetKey(const Uint32 Index) const
	{
		return Keys[Index];
	}
};

namespace KeyMap
{
	static constexpr Uint NumKeys = 256;
	static THashMap<StringView, Key*> Map;
	static const TArray<const char*, NumKeys> KeyReps = {
		"NULL",
		"LBUTTON",
		"RBUTTON",
		"CANCEL",
		"MBUTTON",
		"XBUTTON1",
		"XBUTTON2",
		"NULL",
		"BACK",
		"TAB",
		"NULL",
		"NULL",
		"CLEAR",
		"RETURN",
		"NULL",
		"NULL",
		"SHIFT",
		"CONTROL",
		"MENU",
		"PAUSE",
		"CAPITAL",
		"IMEMODE",
		"NULL",
		"JUNJA",
		"FINAL",
		"HANJA",
		"NULL",
		"ESCAPE",
		"CONVERT",
		"NONCONVERT",
		"ACCEPT",
		"MODECHANGE",
		"SPACE",
		"PRIOR",
		"NEXT",
		"END",
		"HOME",
		"LEFT",
		"UP",
		"RIGHT",
		"DOWN",
		"SELECT",
		"PRINT",
		"EXECUTE",
		"SNAPSHOT",
		"INSERT",
		"DELETE",
		"HELP",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"LWIN",
		"RWIN",
		"APPS",
		"NULL",
		"SLEEP",
		"NUMPAD0",
		"NUMPAD1",
		"NUMPAD2",
		"NUMPAD3",
		"NUMPAD4",
		"NUMPAD5",
		"NUMPAD6",
		"NUMPAD7",
		"NUMPAD8",
		"NUMPAD9",
		"MULTIPLY",
		"ADD",
		"SEPERATOR",
		"SUBTRACT",
		"DECIMAL",
		"DIVIDE",
		"F1",
		"F2",
		"F3",
		"F4",
		"F5",
		"F6",
		"F7",
		"F8",
		"F9",
		"F10",
		"F11",
		"F12",
		"F13",
		"F14",
		"F15",
		"F16",
		"F17",
		"F18",
		"F19",
		"F20",
		"F21",
		"F22",
		"F23",
		"F24",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NUMLOCK",
		"SCROLL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"LSHIFT",
		"RSHIFT",
		"LCONTROL",
		"RCONTROL",
		"LMENU",
		"RMENU",
		"BROWSER_BACK",
		"BROWSER_FORWARD",
		"BROWSER_REFRESH",
		"BROWSER_STOP",
		"BROWSER_SEARCH",
		"BROWSER_FAVORITES",
		"BROWSER_HOME",
		"VOLUME_MUTE",
		"VOLUME_DOWN",
		"VOLUME_UP",
		"MEDIA_NEXT_TRACK",
		"MEDIA_PREV_TRACK",
		"MEDIA_STOP",
		"MEDIA_PLAY_PAUSE",
		"LAUNCH_MAIL",
		"LAUNCH_MEDIA_SELECT",
		"LAUNCH_APP1",
		"LAUNCH_APP2",
		"NULL",
		"NULL",
		"OEM_1",
		"OEM_PLUS",
		"OEM_COMMA",
		"OEM_MIUS",
		"OEM_PERIOD",
		"OEM_2",
		"OEM_3",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"OEM_4",
		"OEM_5",
		"OEM_6",
		"OEM_7",
		"OEM_8",
		"NULL",
		"NULL",
		"OEM_102",
		"NULL",
		"NULL",
		"PROCESSKEY",
		"NULL",
		"PACKET",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"ATTN",
		"CRSEL",
		"EXSEL",
		"EREOF",
		"PLAY",
		"ZOOM",
		"NONAME",
		"PA1",
		"OEM_CLEAR",
		"NULL",
		"NULL",
		"NULL",
		"NULL",
		"NULL"
	};

	static const TArray<Key, KeyReps.size()> Array = [&]() -> const TArray<Key, KeyReps.size()>
	{
		static TArray<Key, KeyReps.size()> Keys;

		for (Uint32 Code = 0; Code < KeyReps.size(); ++Code)
		{
			if (std::strcmp(KeyReps[Code], "NULL") != 0)
			{
				Map.emplace(KeyReps[Code], &(Keys[Code] = Key(KeyReps[Code], Code)));
			}
			else
			{
				Keys[Code] = Key("NULL", Code);
			}
		}

		return Keys;
	}();
};

using KeyInputPrerequisites = std::function<bool()>;

class KeyAction
{
	constexpr static size_t MaxKeys = 4;

	KeyCombination Combination;

public:

	constexpr KeyAction(KeyCombination Combination) 
		: Combination(Combination)
	{}

	KeyAction(const StringView& KeyRep)
	{
		Key ** K = KeyMap::Map.Find(KeyRep);
		
		if (K)
		{
			Combination.SetKey(*K);
		}
	}

	KeyAction(const std::initializer_list<StringView> KeyReps)
	{
		for (size_t N = 0; N < std::min(KeyReps.size(), MaxKeys); ++N)
		{
			Key ** K = KeyMap::Map.Find(*(KeyReps.begin() + N));

			if (K)
			{
				Combination.SetKey(*K);
			}
		}
	}

	KeyAction(const StringViewList& KeyReps)
	{
		for (size_t N = 0; N < std::min(KeyReps.size(), MaxKeys); ++N)
		{
			Key ** K = KeyMap::Map.Find(KeyReps[N]);

			if (K)
			{
				Combination.SetKey(*K);
			}
		}
	}

	virtual bool Execute(bool Pressed)
	{
		return false;
	}

	const KeyCombination& GetCombination() const
	{
		return Combination;
	}
};

template<class Cl>
class KeyActionFunction : public KeyAction
{
	Cl Function;

public:
	template<class... Init>
	KeyActionFunction(Cl Function, Init&&... Args) 
		: KeyAction(std::forward<Init>(Args)...)
		, Function(Function)
	{}

	virtual bool Execute(bool Pressed) final
	{
		if constexpr (std::is_same<std::invoke_result_t<Cl>, bool>::value)
		{
			return Function(Pressed);
		}
		else
		{
			Function(Pressed);
			return true;
		}
	}
};

class KeyActionHandler
{
private:

	WeakPointer<KeyActionHandler> ParentHandler;

private:

	std::optional<KeyInputPrerequisites> Prerequisites;

protected:

	THashMap<Uint32, KeyAction*> ActionMap;

	inline bool ExecuteAction(const Uint32 Code, bool Pressed)
	{
		KeyAction ** Action = ActionMap.Find(Code);

		if (!Action)
		{
			return false;
		}

		return Action[0]->Execute(Pressed);
	}

public:
	
	inline KeyActionHandler() :
		ParentHandler(nullptr)
	{}

	inline void SetPrerequisites(const KeyInputPrerequisites& NewPrerequisites)
	{
		Prerequisites = NewPrerequisites;
	}

	inline KeyActionHandler(WeakPointer<KeyActionHandler>& Handler) :
		ParentHandler(Handler)
	{}

	inline bool HandleKeyAction(const Uint32 Code, bool Pressed)
	{
		if (ParentHandler &&
			ParentHandler->HandleKeyAction(Code, Pressed))
		{
			return false;
		}

		if (Prerequisites && (*Prerequisites)())
		{
			return false;
		}

		return ExecuteAction(Code, Pressed);
	}

	inline KeyAction * AddAction(KeyAction * Action)
	{
		const Uint32 Code = Action->GetCombination().CombinationCode();

		KeyAction ** AlreadyMappedAction = ActionMap.Find(Code);

		if (AlreadyMappedAction)
		{
			return *AlreadyMappedAction;
		}

		ActionMap.emplace(Code, Action);

		return nullptr;
	}

	inline void RemoveAction(const Uint32 Code)
	{
		ActionMap.erase(Code);
	}
};