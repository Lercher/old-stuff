#include <windows.h>
#include <scrnsave.h>
#include <time.h>
#include <stdlib.h>

#include "resource.h"

UINT t1, t2;   // Timers
long d, d2, nx, ny;

static double ranf(void) 
{
	return (double)rand() / RAND_MAX;
}

static long RndAB(long A, long B)
{
	return A + (long)(((double)(B-A)) * ranf());
}

#define XY(a,b)  d*x+a,   d*y+b
#define XY1(a,b) d*x+a+1, d*y+b+1

void DoPaint(HWND hWnd)
{
	HDC hDC;
//	HBRUSH hBrush, oldBrush;
	HPEN hPen, oldPen;
	RECT rc;
	long x, y;

    hDC = GetDC(hWnd); 
    GetClientRect(hWnd, &rc); 
	hPen = CreatePen(PS_SOLID, 0, RGB(0,0,0));
//	hBrush = CreateSolidBrush(RGB(176,212,250));
	oldPen = SelectObject(hDC, hPen);
//	oldBrush = SelectObject(hDC, hBrush);
    FillRect(hDC, &rc, GetStockObject(WHITE_BRUSH));
	//Rectangle(hDC, rc.left, rc.top, rc.right, rc.bottom);
	d = rc.right / RndAB(25, 45);
	d2 = d / 2;
	d = d2 * 2 + 1;
	nx = (rc.right / d) + 1;
	ny = (rc.bottom / d) + 1;
	for(y=-1; y < ny; y++)
	{
		for(x=-1; x < nx; x++)
		{
			if (RndAB(0,1000) < 500)
			{
				// O -
				// - O
				Arc(hDC, XY(0,0), XY1(d,d),     XY(d2-1,d+1), XY(d,d2-1));
				Arc(hDC, XY(d,d), XY1(d+d,d+d), XY(d+d2,d),   XY(d+1,d+d2));
			}
			else
			{
				// - O
				// O -
				Arc(hDC, XY(d,0), XY1(d+d,d), XY(d,d2-1),   XY(d+d2+1,d+1));
				Arc(hDC, XY(0,d), XY1(d,d+d), XY(d-1,d+d2), XY(d2,d));
			}
		}
	}
	SelectObject(hDC, oldPen);
//	SelectObject(hDC, oldBrush);
	DeleteObject(hPen);
//	DeleteObject(hBrush);
    ReleaseDC(hWnd,hDC);
}

void FillIt(HWND hWnd)
{
	HDC hDC;
	HBRUSH hBrush, oldBrush;
	//HPEN hPen, oldPen;
	long x, y;
	COLORREF cr;

    hDC = GetDC(hWnd); 
	hBrush = CreateSolidBrush(RGB(RndAB(100, 255),RndAB(100,255),RndAB(100,255)));
	x = d2 * RndAB(-1, 2*nx);
	y = d2 * RndAB(-1, 2*ny);
	//hPen = CreatePen(PS_SOLID, 0, RGB(0,0,0));
	//oldPen = SelectObject(hDC, hPen);
	oldBrush = SelectObject(hDC, hBrush);
	cr = GetPixel(hDC, x+d2/2, y);
	if (cr == RGB(255,255,255))  // nur weisse stellen fuellen
		ExtFloodFill(hDC, x+d2/2, y, cr, FLOODFILLSURFACE);
	//Rectangle(hDC, x+d2/2-2, y-2, x+d2/2+2, y+2);
	//SelectObject(hDC, oldPen);
	SelectObject(hDC, oldBrush);
	//DeleteObject(hPen);
	DeleteObject(hBrush);
    ReleaseDC(hWnd,hDC);
}

LRESULT CALLBACK ScreenSaverProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{	
	switch(message)
	{
	case WM_ERASEBKGND: 
		DoPaint(hWnd);
        return TRUE; 	
	case WM_CREATE:
		srand(time(NULL));
		t1 = SetTimer(hWnd, 1, 40003, NULL);
		t2 = SetTimer(hWnd, 2,   305, NULL);
		break;
	case WM_DESTROY:
		KillTimer(hWnd, t1);
		KillTimer(hWnd, t2);
		break;
	case WM_TIMER:
		if (wParam == 1)
			DoPaint(hWnd);
		else
			FillIt(hWnd);
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
