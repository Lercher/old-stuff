// Screensaver mit dem Hop-Along Algorithmus

#include <windows.h>
#include "resource.h"
#include "HipHop.h"

#define APPNAME "SymChaos"

#ifdef SS
# define STANDALONE 0
#else
# define STANDALONE 1
#endif

#if !STANDALONE
# include <scrnsave.h>
#endif

HINSTANCE hInst;		// current instance
HWND hWnd;				// current window Handle
DWORD ThreadID;			// Thread Identifier that does the Calc
HANDLE hThread = NULL;	// ThreadHandle
UINT t1, t2, t3;		// 3 Timer

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static void DoThread(BOOL bStart);
static void Activate(BOOL bActivate);
static void DoTimer(WPARAM tID);

#if STANDALONE
int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
	MSG msg;
	WNDCLASSEX wcex;
	HACCEL hAccelTable;

	hInst = hInstance;

	// Window Klasse registrieren
	wcex.style         = CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS | CS_DBLCLKS;
	wcex.lpfnWndProc   = (WNDPROC)WndProc;
	wcex.cbClsExtra    = 0;
	wcex.cbWndExtra    = 0;
	wcex.hInstance     = hInstance;
	wcex.hIcon         = LoadIcon (hInstance, MAKEINTRESOURCE(ID_APP));
	wcex.hCursor       = NULL;
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName  = NULL;
	wcex.lpszClassName = APPNAME;
	wcex.cbSize        = sizeof(WNDCLASSEX);
	wcex.hIconSm       = NULL;
	if (!RegisterClassEx(&wcex))
		return FALSE;

	// Fenster oeffnen
	hWnd = CreateWindow(APPNAME, APPNAME, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0,
		NULL, NULL, hInstance, NULL);

	if (!hWnd)
		return (FALSE);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	hAccelTable = LoadAccelerators (hInstance, APPNAME);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (!TranslateAccelerator (msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;

	lpCmdLine; // This will prevent 'unused formal parameter' warnings
}
#endif

#if STANDALONE
LRESULT CALLBACK WndProc(HWND hWnd1, UINT message, WPARAM wParam, LPARAM lParam)
#else
LRESULT CALLBACK ScreenSaverProc(HWND hWnd1, UINT message, WPARAM wParam, LPARAM lParam)
#endif
{	
	hWnd = hWnd1;
	switch(message)
	{
	case WM_KEYDOWN:
		if (wParam == VK_CONTROL)
		{
			KillTimer(hWnd, t2);
			t2 = SetTimer(hWnd, 22, 123341, NULL);
			bEraseBackground = 1;
			return TRUE;
		}
		break;
	case WM_ERASEBKGND: 
		bEraseBackground = 1;
        return TRUE; 	
	case WM_SHOWWINDOW:
		DoThread((BOOL) wParam);
		break;
	case WM_DESTROY:
		DoThread(FALSE);
		break;
	case WM_TIMER:
		DoTimer(wParam);
		break;
	}
#if STANDALONE
	return DefWindowProc(hWnd, message, wParam, lParam);
#else
	return DefScreenSaverProc(hWnd, message, wParam, lParam);
#endif
}

static void DoThread(BOOL bStart)
{
	if (bStart)
	{
		t1 = SetTimer(hWnd, 11,  1000, NULL);
		t2 = SetTimer(hWnd, 22, 634341, NULL);
		t3 = SetTimer(hWnd, 33,  9461, NULL);
		OutputDebugString("Create thread\n");
		hThread = CreateThread(NULL, 0, ThreadProc, (LPVOID) hWnd, 0, &ThreadID);
		if (hThread == NULL)
			PostQuitMessage(1);
	}
	else
	{	
		KillTimer(hWnd, t1);
		KillTimer(hWnd, t2);
		KillTimer(hWnd, t3);
		OutputDebugString("Stop thread\n");
		if (hThread)
		{
			bThreadContinue = FALSE;
			PostQuitMessage(0);
		}
	}
}

static void DoTimer(WPARAM tID)
{
	switch(tID)
	{
	case 11:
		//ChangeBrush();
		break;
	case 22:
		bEraseBackground = 1;
		break;
	case 33:
		//ChangeWidth();
		break;
	}
}

#if !STANDALONE
BOOL WINAPI ScreenSaverConfigureDialog (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst) 
{ 
    return TRUE; 
} 
#endif