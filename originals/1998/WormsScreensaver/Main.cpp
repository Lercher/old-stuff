#include <windows.h>
#include <scrnsave.h>
#include <time.h>
#include <stdlib.h>
#include "main.h"
#include "resource.h"
#include "Worms.h"

UINT t1;   // Timers
RECT rc;
Worms *pWuermer;

double ranf(void) 
{
	return (double)rand() / RAND_MAX;
}

long RndAB(long A, long B)
{
	return A + (long)(((double)(B-A)) * ranf());
}

void DoPaint(HWND hWnd)
{
	HDC hDC;
	HBRUSH oldBrush;
	HPEN oldPen;

    hDC = GetDC(hWnd); 
    GetClientRect(hWnd, &rc); 
	oldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
	oldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));

	pWuermer->Paint(hDC);
	pWuermer->Move();
	if (RndAB(0, 1000) < 100)
		pWuermer->AddNew();

	SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
    ReleaseDC(hWnd,hDC);
}

void CreateWorms(HWND hWnd)
{
	HDC hDC;

    hDC = GetDC(hWnd); 
    GetClientRect(hWnd, &rc); 
    ReleaseDC(hWnd,hDC);
	pWuermer = new Worms;
}

LRESULT CALLBACK ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	switch(message)
	{
	case WM_ERASEBKGND: 
		// nothing to do
        return TRUE; 	
	case WM_CREATE:
		srand(time(NULL));
		t1 = SetTimer(hWnd, 1, 203, NULL);
		CreateWorms(hWnd);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, t1);
		delete pWuermer;
		break;
	case WM_TIMER:
		if (wParam == 1)
			DoPaint(hWnd);
		break;
	}
	return DefScreenSaverProc(hWnd, message, wParam, lParam);
}

BOOL WINAPI ScreenSaverConfigureDialog (HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE hInst) 
{ 
    return TRUE; 
} 
