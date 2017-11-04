#include <windows.h>
#include <vector>
#include <string>

const wchar_t WINDOWS_NAME[] = L"EasyMode - 0.1.0";
const int KEY_SIZE = 256;
bool KEY_FILTER[KEY_SIZE] = { false, };
const std::string APP_NAME("World of Warcraft");
const int MAX_CLIENT_COUNT = 5;
const int WINDOWS_HEADER_SIZE = 22;

HWND g_hWnd = NULL;
HBRUSH g_pausedBrush = NULL;
HBRUSH g_resumedBrush = NULL;
HHOOK g_keyboardHook = NULL;
bool g_paused = true;

HWND g_hWindows[MAX_CLIENT_COUNT] = { 0, };
HDC g_hDC = NULL;
int g_clientCount = 0;

BOOL CALLBACK EnumWindowsProc(
	__in  HWND hWnd,
	__in  LPARAM lParam
)
{
	// Skip child windows
	if (GetTopWindow(hWnd))
		return TRUE;

	int length = GetWindowTextLengthA(hWnd);
	if (APP_NAME.length() == length)
	{
		char* buffer;
		buffer = new char[length + 1];
		memset(buffer, 0, (length + 1) * sizeof(char));
		GetWindowTextA(hWnd, buffer, length + 1);
		if (strcmp(APP_NAME.c_str(), buffer) == 0)
		{
			if (g_clientCount == MAX_CLIENT_COUNT)
			{
				MessageBox(NULL, L"Too many clients!", L"Error", 0);
				return TRUE;
			}

			g_hWindows[g_clientCount] = hWnd;
			g_clientCount++;
		}
	}
	return TRUE;
}

int PrepareWindows()
{
	EnumWindows(EnumWindowsProc, NULL);
	return g_clientCount;
}

char str[256] = { 0, };

DWORD GetKeyFromWindow(HWND hWnd, DWORD vkCode, WPARAM keyState)
{
	RECT rect = { 0, };
	GetWindowRect(hWnd, &rect);
	int x = (rect.right - rect.left) / 2 + rect.left;
	int y = (rect.bottom - rect.top + WINDOWS_HEADER_SIZE) / 2 + rect.top;
	COLORREF color = GetPixel(g_hDC, x, y);

	sprintf_s(str, "x%d y%d - R%#X G%#X B%#X key%#X state=%IX hWnd%IX\r\n", 
		x, y, GetRValue(color), GetGValue(color), GetBValue(color), 
		vkCode, keyState , hWnd);
	OutputDebugStringA(str);

	return vkCode;
}

void BroadcastKey(WPARAM keyState, DWORD vkCode)
{
	HWND hActiveWindow = GetForegroundWindow();
	HWND topWindow = GetTopWindow(hActiveWindow);
	if (topWindow)
	{
		hActiveWindow = topWindow;
		sprintf_s(str, "TopWindow%IX \r\n", hActiveWindow);
		OutputDebugStringA(str);
	}

	bool needToBroadcast = false;
	for (int i = 0; i < g_clientCount; i++)
	{
		if (g_hWindows[i] == hActiveWindow)
		{
			needToBroadcast = true;
			break;
		}
	}
	if (!needToBroadcast)
		return;
	for (int i = 0; i < g_clientCount; i++)
	{
		if (g_hWindows[i] == hActiveWindow)
		{
			// Do not post message to the active window since we will call CallNextHookEx
			continue;
		}

		DWORD key = vkCode;
		//if (keyState == WM_KEYDOWN)
		//{
		//	DWORD key = GetKeyFromWindow(g_hWindows[i], vkCode, keyState);
		//}
		PostMessage(g_hWindows[i], (UINT)keyState, key, 0);
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
							MessageBox(NULL, L"Need at least two clients!", L"Error", 0);
							g_paused = true;
							g_clientCount = 0;
						}
						g_hDC = GetDC(NULL);
						for (int i = 0; i < g_clientCount; i++)
						{
							RECT r;
							GetWindowRect(g_hWindows[i], &r);

							HWND topWindow = GetTopWindow(g_hWindows[i]);
							char topWindowName[256] = { 0, };
							if (topWindow)
								GetWindowTextA(topWindow, topWindowName, 256);
							sprintf_s(str, "Client[%d] hWnd=%IX l=%d r=%d t=%d b=%d top=%IX name=%s\r\n",
								i, g_hWindows[i],
								r.left, r.right, r.top, r.bottom,
								GetTopWindow(g_hWindows[i]), topWindowName);
							OutputDebugStringA(str);
						}
					}
					else
					{
						if (g_hDC)
						{
							ReleaseDC(NULL, g_hDC);
							g_hDC = NULL;
						}
						g_clientCount = 0;
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
		if (g_hDC)
		{
			ReleaseDC(NULL, g_hDC);
			g_hDC = NULL;
		}
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
	wcex.lpszClassName = L"EasyModeWindow";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_APPLICATION));

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, L"Call to RegisterClassEx failed!", L"EasyMode", NULL);
		return 1;
	}

	g_hWnd = CreateWindow(
		wcex.lpszClassName,
		WINDOWS_NAME,
		WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT,
		200, 64,
		NULL,
		NULL,
		hInstance,
		NULL
	);

	if (!g_hWnd)
	{
		MessageBox(NULL, L"Call to CreateWindow failed!", L"EasyMode", NULL);
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
