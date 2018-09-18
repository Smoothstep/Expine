
#include "Defines.h"
#include "Engine/Graphics/Scene/View/Camera.h"
#include "Engine/Graphics/Scene/SceneView.h"
#include "Engine/Graphics/Scene/Scene.h"
#include "Engine/Graphics/Screen.h"

#include <thread>

HINSTANCE g_hMainInstance;

static const wchar_t gc_wszVibeClass[] = L"Vibe Test Class";
static const wchar_t gc_wszVibeTitle[] = L"Vibe Test";

using namespace D3D;

void OnPaint(CScreen * Screen)
{
	if (Screen)
	{
		Screen->Render();
	}

	CShaderManager::Instance().Update();
}

void OnActivate(CScreen * Screen, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (Screen)
	{
		//Screen->OnActivation(Message, wParam, lParam);
	}
}

void OnMouseInput(CScreen * Screen, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (Screen)
	{
		//Screen->OnMouseInput(Message, wParam, lParam);
	}
}

void OnKeyboardInput(CScreen * Screen, UINT Message, WPARAM wParam, LPARAM lParam)
{
	if (Screen)
	{
		//Screen->OnKeyboardInput(Message, wParam, lParam);
	}
}

void RenderLoop(CScreen * Screen)
{
	while (true)
	{
		Screen->Render();
	}
}

LRESULT CALLBACK WndProc
(
	HWND	hWnd,
	UINT	Message,
	WPARAM	wParam,
	LPARAM	lParam
)
{
	switch (Message)
	{
		case WM_CREATE:
		{
			LPCREATESTRUCT CreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);

			if(CreateStruct)
			{
				SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(CreateStruct->lpCreateParams));
			}
		}

		break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
		}

		break;

		case WM_PAINT:
		{
			if (GetForegroundWindow() == hWnd)
			{
				OnPaint(reinterpret_cast<D3D::CScreen*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)));
			}
		}

		break;

		case WM_ACTIVATEAPP:
		{
			OnActivate(reinterpret_cast<D3D::CScreen*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)), Message, wParam, lParam);
		}

		case WM_INPUT:
		case WM_MOUSEMOVE:
		case WM_LBUTTONDOWN:
		case WM_LBUTTONUP:
		case WM_RBUTTONDOWN:
		case WM_RBUTTONUP:
		case WM_MBUTTONDOWN:
		case WM_MBUTTONUP:
		case WM_MOUSEWHEEL:
		case WM_XBUTTONDOWN:
		case WM_XBUTTONUP:
		case WM_MOUSEHOVER:
		{
			OnMouseInput(reinterpret_cast<D3D::CScreen*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)), Message, wParam, lParam);
		}

		break;

		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			OnKeyboardInput(reinterpret_cast<D3D::CScreen*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)), Message, wParam, lParam);
		}

		break;

		default:
		{
			return DefWindowProc(hWnd, Message, wParam, lParam);
		}
	}

	return 0;
}

ATOM RegisterVibeClass
(
	HINSTANCE hInstance
)
{
    WNDCLASSEXW wcex;
	{
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style          = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc    = WndProc;
		wcex.cbClsExtra     = 0;
		wcex.cbWndExtra     = 0;
		wcex.hInstance      = hInstance;
		wcex.hIcon          = NULL;
		wcex.hCursor        = NULL;
		wcex.hbrBackground  = NULL;
		wcex.lpszMenuName   = gc_wszVibeTitle;
		wcex.lpszClassName  = gc_wszVibeClass;
		wcex.hIconSm        = NULL;
	}

    return RegisterClassExW(&wcex);
}

void D3DTest(HWND hWnd)
{

}

void InitializeEngine(D3D::CScreen * Screen)
{
	static D3D::CScene MainScene(Screen);
	MainScene.Initialize();
	MainScene.LoadArea(0);
}

D3D::ErrorCode InitializeScreen(D3D::CScreen * Screen, UINT Width, UINT Height, HWND hWnd)
{
	D3D::ScreenWindow Window;
	D3D::ScreenViewport Viewport;

	Viewport.Height = 1080.0;
	Viewport.Width = 1920.0;
	Viewport.MaxDepth = 1.0;
	Viewport.MinDepth = 0.0;
	Viewport.TopLeftX = 0.0;
	Viewport.TopLeftY = 0.0;
	Viewport.ScissorRect.bottom = 1080;
	Viewport.ScissorRect.left = 0;
	Viewport.ScissorRect.right = 1920;
	Viewport.ScissorRect.top = 0;

	Window.WindowHandle = hWnd;
	Window.WindowHeight = 1080;
	Window.WindowWidth = 1920;
	Window.WindowIsFullscreen = FALSE;
	Window.Output.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	D3D::CScreen::InitializeParameter Params;

	Params.DefaultViewport = Viewport;
	Params.DefaultWindow = Window;

	return Screen->Create(Params);
}

bool InitializeInstance
(
	HINSTANCE	hInstance,
	int			nCmdShow
)
{
	D3D::CScreen * Screen = new D3D::CScreen();

	g_hMainInstance = hInstance;

	HWND hWnd = CreateWindow
	(
		gc_wszVibeClass,
		gc_wszVibeTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, 1920, 1080,
		NULL,
		NULL,
		hInstance,
		Screen
	);

	if (!hWnd)
	{
		return false;
	}

	if (InitializeScreen(Screen, 1920, 1080, hWnd))
	{
		return false;
	}

	CScene * Scene = Screen->GetActiveScene();

	//Scene->GetCamera()->SetPosition(Vector3f(1000, 2000, 400));
	//Scene->GetCamera()->SetRotation(Rotation(-25, 180 + 90, 0));
	//Scene->GetCamera()->SetPosition(Vector3f(1000, 2000, 150));
	//Scene->GetCamera()->SetRotation(Rotation(5, 180 + 90, 0));
	//Scene->SetCamera(Scene->GetCamera());
	//Scene->AddView(1, new CSceneView());
	//Scene->SetInputPerspective(1);
	//Scene->GetCamera()->SetPosition(Vector3f(1200, 1500, 200));
	//Scene->GetCamera()->SetRotation(Rotation(0, -180 - 90, 0));
	//Screen->GetActiveScene()->SwitchToGlobalLightPerspective();
	Scene->LoadArea(0);
	
	//Screen->GetActiveScene()->SwitchToGlobalLightPerspective();

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	//RenderThread = new boost::thread(&RenderLoop, Screen);

	return true;
}

void DebugConsole()
{
	AllocConsole();
	FILE* pFile;
	freopen_s(&pFile, "conin$", "r", stdin);
	freopen_s(&pFile, "conout$", "w", stdout);
	freopen_s(&pFile, "conout$", "w", stderr);
}
/*
void DownsampleShadowMap(Uint8 * Data, Uint8 * Downsampled, int32_t Width, int32_t Height, int32_t NewWidth, int32_t NewHeight)
{
	const float RatioX = Width / NewWidth;
	const float RatioY = Height / NewHeight;

	const int32_t SX = Math::Ceil(RatioX / 2.0f);
	const int32_t SY = Math::Ceil(RatioY / 2.0f);

	const int32_t MX = Math::Floor(RatioX);
	const int32_t MY = Math::Floor(RatioY);

	const float RX = (RatioX - MX) / 2.0f;
	const float RY = (RatioY - MY) / 2.0f;

	const float MaxDistance = Vector2f::Distance(Vector2f(0, 0), Vector2f(SX, SY));

	for (int32_t Y = 0; Y < NewHeight; ++Y)
	{
		for (int32_t X = 0; X < NewWidth; ++X)
		{
			int32_t TranslatedX = RatioX * X;
			int32_t TranslatedY = RatioY * Y;

			Uint8 Sample;
			Uint8 Lerp = Data[TranslatedX + TranslatedY * Width];

			int32_t MLX = TranslatedX - SX;
			int32_t MRX = TranslatedX + SX;
			int32_t MLY = TranslatedY - SY;
			int32_t MRY = TranslatedY + SY;

			for (int32_t 
				LY = Math::Max(MLY, 0);
				LY < Math::Min(MRY, Height); ++LY)
			{
				for (int32_t 
					LX = Math::Max(MLX, 0);
					LX < Math::Min(MRX, Width); ++LX)
				{
					Sample = Data[LX + LY * Height];
					
					float LerpFactor = 0.5f - 0.5f * (Vector2f::Distance(Vector2f(TranslatedX, TranslatedY), Vector2f(LX, LY)) / MaxDistance);

					if (LX == MLX || LX == MRX)
					{
						LerpFactor -= RX;
					}
					
					if (LY == MLY || LY == MRY)
					{
						LerpFactor -= RY;
					}

					Lerp = Lerp + (Sample - Lerp) * LerpFactor;
				}
			}

			Downsampled[X + Y * NewWidth] = Lerp;
		}
	}
}

void GenerateShadowMap()
{
	constexpr int TerrainW = 2000;
	constexpr int TerrainH = 2000;

	constexpr int32_t DownsampleFactor = 4;

	File::CFile HeightMapFile(L"heightmap.raw");
	File::CFile ShadowMapFile(L"Shadowmap.raw");

	HeightMapFile.ReadFileContent();

	ShadowMapFile.GetContentRef().ResizeUninitialized
	(
		TerrainW / DownsampleFactor * 
		TerrainH / DownsampleFactor
	);

	Uint8  * ShadowMap = new Uint8[HeightMapFile.GetContentRef().size() / sizeof(Uint16)];
	Uint16 * HeightMap = (Uint16*)HeightMapFile.GetContentRef().data();

	std::fill(ShadowMap, ShadowMap + TerrainW * TerrainH, 255);

	Vector3f S(0.0f, 0.0f, 1000.0f);
	Vector3f P;
	Vector3f D;

	Vector2f Size(TerrainW, TerrainH);
	Vector2f Steps;
	Vector3f SunTarget(1000.0f, 1000.0f, 0);
	Vector3f L = (SunTarget - S).GetSafeNormal();

	Float End;
	Float Dist;
	constexpr Float HeightScale = 0.01;

	for (int Y = 0; Y < TerrainH; ++Y)
	{
		for (int X = 0; X < TerrainW; ++X)
		{
			P.X = X;
			P.Y = Y;

			if (S.X == P.X && S.Y == P.Y)
			{
				continue;
			}

			P.Z = HeightMap[X + Y * TerrainW] * HeightScale;

			D = (P - S);
			Dist = D.Size2D();;
			Float SoP = S | P;
			Float Magnitude = D.Size();

			//Vector2f Delta(D.X, D.Y);
			//Delta.Normalize();
			D.Normalize();
			float dotDL = Math::Clamp(L | D, SMALL_NUMBER, 1.0f);

			Float R = Math::Abs(D.X) > Math::Abs(D.Y) ? 1.0 / D.X : 1.0 / D.Y;

			Vector3f Step = dotDL * Vector3f(D.X * R, D.Y * R, D.Z);

			float Len = P.Z / (Math::Max(1.0f, -Step.Z));

			// Calculate bounds.

			End = P.X + Step.X * Len;

			if (End >= Size.X)
			{
				Len = (Size.X - P.X) * Step.X;
			}
			else if (End < 0)
			{
				Len = P.X * -Step.X;
			}

			End = P.Y + Step.Y * Len;
			
			if (End >= Size.Y)
			{
				Len = (Size.Y - P.Y) * Step.Y;
			}
			else if (End < 0)
			{
				Len = P.Y * -Step.Y;
			}

			Len -= 1.0f;
			float LenEstimated = Len;
			float Z = P.Z;

			while(Len > 0)
			{
				P += Step;

				int IX = P.X;
				int IY = P.Y;

				int O = IX + IY * TerrainW;

				float Sample = HeightMap[O] * HeightScale + 0.1f;

				if (P.Z >= Sample)
				{
					int L = O + 1;
					int R = O - 1;
					int T = O - TerrainW;
					int B = O + TerrainW;
					int LT = O - TerrainW + 1;
					int RT = O - TerrainW - 1;
					int LB = O + TerrainW + 1;
					int LR = O + TerrainW - 1;

					Uint8 Shadow = 256 / 256 * (LenEstimated - Len);
					Uint8 ShadowAvg = Shadow;

					if (L > -1)
					{
						ShadowAvg = Math::Max(256, ShadowAvg + ShadowMap[L] / 8);
						ShadowMap[L] = Math::Max(256, ShadowMap[L] + Shadow / 8);
					}
					if (T > -1)
					{
						ShadowAvg = Math::Max(256, ShadowAvg + ShadowMap[T] / 8);
						ShadowMap[T] = Math::Max(256, ShadowMap[T] + Shadow / 8);
					}
					if (LT > -1)
					{
						ShadowAvg = Math::Max(256, ShadowAvg + ShadowMap[LT] / 8);
						ShadowMap[LT] = Math::Max(256, ShadowMap[LT] + Shadow / 8);
					}

					ShadowMap[O] = ShadowAvg;
				}

				Len -= 1.0f;
			}
		}
	}

	DownsampleShadowMap(
		ShadowMap, 
		ShadowMapFile.GetContentRef().data(), 
		TerrainW, 
		TerrainH, 
		TerrainW / DownsampleFactor, 
		TerrainH / DownsampleFactor);

	ShadowMapFile.WriteFileContent();
}*/

int APIENTRY wWinMain
(
	HINSTANCE	hInstance,
    HINSTANCE	hPrevInstance,
    LPWSTR		lpCmdLine,
    int			nCmdShow
)
{
	DebugConsole();

	ATOM Class = RegisterVibeClass(hInstance);

	if (!Class)
	{
		return false;
	}

    if (!InitializeInstance(hInstance, nCmdShow))
    {
        return false;
    }

    MSG Msg;

    while (GetMessage(&Msg, NULL, 0, 0))
    {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
    }

    return (int)Msg.wParam;
}