// HipHop.cpp - Thread mit dem eigentlichen Hopalong Algorithmus

#include <windows.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


#include "HipHop.h"

BOOL bThreadContinue, bEraseBackground = 0;
static HANDLE hProcess;
static HDC hDC;
static HWND hWnd;
static COLORREF crIntitial;

// Hopalong Vars
static double x, y, y11, x11;
static double A, B, C, D, ZOOM, SYM;
static long w, h;

#define Hop1()	f_Martin3()
#define Hop2()	f_Martin5()

// HopAlong Functions

static void f_Martin5(void)
{
	double xxyy, x510x3y25xy4;

	xxyy = x*x + y*y;
	x510x3y25xy4 = x*x*x*x*x - 10*x*x*x*y*y + 5*x*y*y*y*y;

	x11 = A*x + B*x*xxyy + C*x*x510x3y25xy4 + D*(x*x*x*x - 6*x*x*y*y + y*y*y*y);
	y11 = A*y + B*y*xxyy + C*y*x510x3y25xy4 + D*(4*x*y*y*y - 4*x*x*x*y);
}

static void f_Martin3(void)
{
	double xxyy, xxx3xyy;

	xxyy = x*x + y*y;
	xxx3xyy = x*x*x - 3*x*y*y;
	x11 = A*x + B*x*(xxyy) + C*x*(xxx3xyy) + D * (x*x - y*y);
	y11 = A*y + B*y*(xxyy) + C*y*(xxx3xyy) - 2*D*x*y;
}

/*
#define Rmod 536870911l
#define Rmul 3.99
#define Ranfset(l) (ranfseed=((long)((labs(l)%Rmod)*Rmul))%Rmod)
long    ranfseed=268435456l;

static double ranf(void) 
{
   ranfseed = (long) (ranfseed*Rmul)%Rmod;
   return(ranfseed/(double)Rmod);
}
*/

static double RndAB(double A, double B)
{
	return A + (B-A) * ((double) rand() / (double) RAND_MAX);
}

static void RandomizeABC(void)
{
	char buf[120];
	HFONT hFont, hOldFont;
	RECT r;
	int CR = 0, CG = 0, CB = 0;

	GetWindowRect(hWnd, &r);

    A = RndAB(-1.999, -1.69);
    B = RndAB(1.2, 2.5);
    C = RndAB(-0.98, 0.0);
	D = RndAB(1.3, 1.4);
	ZOOM = RndAB(h/2.5, h/1.8);
	SYM = RndAB(0, 1);
	x = 0.01;
	y = 0.22;

    CR = ((int)RndAB(0.0, 128.0)) & 255;
	CG = ((int)RndAB(0.0, 128.0)) & 255;
	CB = ((int)RndAB(CB + 16, CB + 48)) & 255;
	crIntitial = RGB(CR, CG, CB);

	sprintf(buf, "A:%1.3lf B:%1.3lf C:%1.3lf D:%1.3lf", A, B, C, D);
	SetTextColor(hDC, RGB(0, 0, 0));
	hFont = CreateFont(-9, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
	hOldFont = (HFONT) SelectObject(hDC, hFont);
	SetTextAlign(hDC, TA_RIGHT);
	TextOut(hDC, w-2, 1, buf, strlen(buf));
	SelectObject(hDC, hOldFont);
	DeleteObject(hFont);
}

#define N 5000

void ChangeParams(void)
{
	HBRUSH hBrush, hOldBrush;
	RECT rect;

	hBrush = CreateSolidBrush(RGB(255, 255, 255));
	hOldBrush = (HBRUSH) SelectObject(hDC, hBrush);
	GetClientRect(hWnd, &rect);
	Rectangle(hDC, rect.left-1, rect.top-1, rect.right+5, rect.bottom+5);
	SelectObject(hDC, hOldBrush);
	DeleteObject(hBrush);
	w = rect.right - rect.left; 
	h = rect.bottom - rect.left;
	RandomizeABC();
	bEraseBackground = 0;
}

// Main Threads does the Drawing
void TMain(void)
{
	int nOut = 0;
	long n, xx, yy, nNearNull;
	COLORREF c;

	if(hDC = GetDC(hWnd))
	{
		srand((unsigned)time( NULL ));
		while(bThreadContinue)
		{
			WaitForInputIdle(hProcess,1000);	// Höchstens 1s warten
			if (bEraseBackground)
			{
				ChangeParams();
				nOut = 0;
				nNearNull = 0;
			}
			for(n = 0; n < N; n++)
			{
				if (SYM > 0.5)
					Hop1();
				else
					Hop2();
				x = x11;
				y = y11;
				xx = (long)((x + 1.1) * ZOOM);
				yy = (long)((y + 1) * ZOOM);
				
				c = GetPixel(hDC, xx, yy);
				if (c == RGB(255,255,255))
					c = crIntitial;
				else
					c=((c+0x010000l)&0xff0000l) + ((c+0x0100l)&0x00ff00l) + ((c+0x01l)&0x0000ffl);
				SetPixel(hDC, xx, yy,  c);

				if (xx > 2*w) nOut++;
				if (yy > 2*h) nOut++;
				if (xx < -w) nOut++;
				if (yy < -h) nOut++;

				if (fabs(x) + fabs(y) < 0.1)
					nNearNull++;
				if (nOut > N/2)
					bEraseBackground = 1;
				if (xx == 0 || yy == 0)
					bEraseBackground = 1;
			}
			if (nNearNull == 0)
				bEraseBackground = 1;
		}
		ReleaseDC(hWnd, hDC);
	}
}


DWORD WINAPI ThreadProc(LPVOID lpParameter)
{
	hProcess = GetCurrentProcess();		// need not be closed
	bThreadContinue = TRUE;
	hWnd = (HWND) lpParameter; 
	TMain();
	return 11;
}

