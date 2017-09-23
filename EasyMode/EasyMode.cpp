#include <windows.h>
#include <vector>
#include <string>

const int KEY_SIZE = 256;
bool KEY_FILTER[KEY_SIZE] = { false, };
const std::string APP_NAME("World of Warcraft");

HWND g_hWnd = NULL;
HBRUSH g_pausedBrush = NULL;
HBRUSH g_resumedBrush = NULL;
HHOOK g_keyboardHook = NULL;
bool g_paused = true;

std::vector<HWND> g_hAppWindows;

BOOL CALLBACK EnumWindowsProc(
	__in  HWND hWnd,
	__in  LPARAM lParam
)
{
	int length = GetWindowTextLengthA(hWnd);
	if (APP_NAME.length() == length)
	{
		char* buffer;
		buffer = new char[length + 1];
		memset(buffer, 0, (length + 1) * sizeof(char));
		GetWindowTextA(hWnd, buffer, length + 1);
		if (strcmp(APP_NAME.c_str(), buffer) == 0)
		{
			g_hAppWindows.push_back(hWnd);
		}
	}
	return TRUE;
}

int PrepareWindows()
{
	EnumWindows(EnumWindowsProc, NULL);

	return (int)g_hAppWindows.size();
}

void BroadcastKey(WPARAM keyState, DWORD vkCode)
{
	HWND hActiveWindow = GetForegroundWindow();
	auto it = std::find(g_hAppWindows.begin(), g_hAppWindows.end(), hActiveWindow);
	if (it != g_hAppWindows.end())
	{
		for (it = g_hAppWindows.begin(); it != g_hAppWindows.end(); ++it)
		{
			// Do not post message to the active window since we will call CallNextHookEx
			if (*it == hActiveWindow)
			{
				continue;
			}
			PostMessage(*it, (UINT)keyState, vkCode, 0);
		}
	}
}

// wParam: vk http://www.kbdedit.com/manual/low_level_vk_list.html
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (nCode == HC_ACTION)
	{
		if (wParam == WM_KEYDOWN || wParam == WM_KEYUP)
		{
			PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;
			switch (p->vkCode)
			{
			case VK_PAUSE:
				if (wParam == WM_KEYDOWN)
				{
					g_paused = !g_paused;
					if (!g_paused)
					{
						if (PrepareWindows() < 2)
						{
							MessageBox(NULL, L"Need at least two instances!", L"Error", 0);
							g_paused = true;
						}
					}
					// Change windows color
					InvalidateRect(g_hWnd, NULL, TRUE);
				}
				break;
			default:
				if (!g_paused)
				{
					if (KEY_FILTER[p->vkCode])
					{
						BroadcastKey(wParam, p->vkCode);
					}
				}
				break;
			}
		}
	}

	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		RECT rect;
		GetClientRect(hWnd, &rect);
		FillRect(hdc, &rect, (g_paused ? g_pausedBrush : g_resumedBrush));
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
		break;
	}

	return 0;
}

int Initialize(HINSTANCE hInstance, int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPLICATION));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"EasyLifeWindow";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, L"Call to RegisterClassEx failed!", L"EasyLife", NULL);
		return 1;
	}

	g_hWnd = CreateWindow(
		wcex.lpszClassName,
		L"EasyLife",
		WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		0, 64,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!g_hWnd)
	{
		MessageBox(NULL, L"Call to CreateWindow failed!", L"EasyLife", NULL);
		return 1;
	}

	ShowWindow(g_hWnd, nCmdShow);
	UpdateWindow(g_hWnd);
	InvalidateRect(g_hWnd, NULL, TRUE);

	KEY_FILTER['W'] = true;
	KEY_FILTER['R'] = true;
	KEY_FILTER['Z'] = true;
	KEY_FILTER['5'] = true;
	KEY_FILTER[VK_F7] = true;
	KEY_FILTER[VK_F8] = true;
	KEY_FILTER[VK_SPACE] = true;
	KEY_FILTER[VK_DOWN] = true;
	KEY_FILTER[VK_DECIMAL] = true;

	g_pausedBrush = CreateSolidBrush(RGB(64, 64, 64));
	g_resumedBrush = CreateSolidBrush(RGB(0, 255, 0));

	g_keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
	return 0;
}

void Shutdown()
{
	UnhookWindowsHookEx(g_keyboardHook);
	NOTIFYICONDATA nid = {};
	Shell_NotifyIcon(NIM_DELETE, &nid);
}

int CALLBACK WinMain(
	_In_ HINSTANCE hInstance,
	_In_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	if (Initialize(hInstance, nCmdShow) > 0)
	{
		return 1;
	}
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	Shutdown();
	return (int)msg.wParam;
}
