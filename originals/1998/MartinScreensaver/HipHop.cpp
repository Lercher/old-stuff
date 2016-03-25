// HipHop.cpp - Thread mit dem eigentlichen Hopalong Algorithmus

#include <windows.h>
#include <math.h>
#include <time.h>

#include "HipHop.h"

BOOL bThreadContinue;
static HANDLE hProcess;
static HDC hDC;
static HWND hWnd;

// Hopalong Vars
static double x, y, y11, x11;
static double A, B, C, ZOOM;
int CR = 0, CG = 0, CB = 0, Width = 3;

// HopAlong Functions
static void f_Martin1(void)
{
  x11 = y  - ( (x<0) ? sqrt(fabs(B*x-C)) : -sqrt(fabs(B*x-C)) );
  y11 = A - x;
}

static void f_Martin2(void)
{
  x11 = y  - sin(x);
  y11 = A - x;
}

#define Rmod 536870911l
#define Rmul 3.99
#define Ranfset(l) (ranfseed=((long)((labs(l)%Rmod)*Rmul))%Rmod)
long    ranfseed=268435456l;

static double ranf(void) 
{
   ranfseed = (long) (ranfseed*Rmul)%Rmod;
   return(ranfseed/(double)Rmod);
}

static double RndAB(double A, double B)
{
	return A + (B-A) * ranf();
}

static void RandomizeABC(void)
{
    A = RndAB(40.0, 1540.0);
    B = RndAB(3.0, 20.0);
    C = RndAB(100.0, 3100.0);
	ZOOM = RndAB(0.15, 0.5);
	x = 0;
	y = 0;
}

#define N 1000
#define Hop()	f_Martin1()

// Main Threads does the Drawing
static void TMain(void)
{
	long n, xx, yy;
	HPEN hPen;
	HBRUSH hBrush;

	if(hDC = GetDC(hWnd))
	{
		time(&xx);
		xx = xx & 1023;
		for(n=0; n < xx; n++)
			(void) ranf();
		RandomizeABC();

		hBrush = CreateSolidBrush(RGB(CR, CG, CB));
		hPen = CreatePen(PS_NULL, 0, 0);
		SelectObject(hDC, hBrush);
		SelectObject(hDC, hPen);
		while(bThreadContinue)
		{
			WaitForInputIdle(hProcess, 1000);	// Höchstens 1s warten
			for(n = 0; n < N; n++)
			{
				Hop();
				x = x11;
				y = y11;
				xx = (long)(x * ZOOM);
				yy = (long)(y * ZOOM);
				Rectangle(hDC, xx, yy, xx+Width, yy+Width);
			}
		}
		ReleaseDC(hWnd, hDC);
	}
}

void ChangeBrush(void)
{
	HBRUSH hBrush;

    CR = ((int)RndAB(0.0, 255.0)) & 255;
	CG = ((int)RndAB(0.0, 255.0)) & 255;
	CB = ((int)RndAB(CB + 16, CB + 48)) & 255;
	hBrush = CreateSolidBrush(RGB(CR, CG, CB));
	DeleteObject(SelectObject(hDC, hBrush));
}

void ChangeWidth(void)
{
	Width = (int) RndAB(2, 4);
}

void ChangeParams(void)
{
	RECT rect;

	ChangeBrush();
	GetClientRect(hWnd, &rect);
	Rectangle(hDC, rect.left-1, rect.top-1, rect.right+5, rect.bottom+5);
	ChangeBrush();
	RandomizeABC();
}

DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	hProcess = GetCurrentProcess();		// need not be closed
	bThreadContinue = TRUE;
	hWnd = (HWND) lpParameter; 
	TMain();
	return 11;
}

