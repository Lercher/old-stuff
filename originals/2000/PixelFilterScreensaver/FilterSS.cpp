// FilterSS.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

static LPVOID pBitsFrom = 0;
static LPVOID pBitsTo = 0;
static int resx = 0;
static int resy = 0;

// Create a 24-bit-per-pixel surface.
static HBITMAP Create24BPPDIBSection(HDC hDC, int iWidth, int iHeight, LPVOID *pBits)
{
	BITMAPINFO bmi;
	HBITMAP hbm;

	// Initialize to 0s.
	ZeroMemory(&bmi, sizeof(bmi));

	// Initialize the header.
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = iWidth;
	bmi.bmiHeader.biHeight = iHeight;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	// Create the surface.
	hbm = CreateDIBSection(hDC, &bmi, DIB_RGB_COLORS, pBits, NULL, 0);

	return(hbm);
}

static COLORREF MyGetPixel(int x, int y)
{
	return ((LPDWORD)pBitsFrom)[x + resx*y];
}

static void MySetPixel(int x, int y, COLORREF cr)
{
	((LPDWORD)pBitsTo)[x + resx*y] = cr;
}

int FilterElementMatrix(int ox, int oy, int x, int y)
{
	int cr;

	cr = MyGetPixel(x + ox - 1, y - oy + 1);
	return cr & 0xffl;
}

void FilterElement(int x, int y)
{
	int m = 0;
	int mm, im;
	long cr, crest;

	crest = MyGetPixel(x, y);
	cr = crest & 0xffl;
	crest &= 0xffff00l;

	m += FilterElementMatrix(0, 0, x, y);
	m += FilterElementMatrix(1, 0, x, y);
	m += FilterElementMatrix(2, 0, x, y);
	m += FilterElementMatrix(0, 1, x, y);
	m += FilterElementMatrix(1, 1, x, y);
	m += FilterElementMatrix(2, 1, x, y);
	m += FilterElementMatrix(0, 2, x, y);
	m += FilterElementMatrix(1, 2, x, y);
	m += FilterElementMatrix(2, 2, x, y);

	mm = m - 0x480;
	im = 0x900 - m;

	if       (m <= 0x100)
		cr = 0;
	else if (m <= 0x300)
		cr += m/0x10 + (rand() & 0x7);
	else if (m <= 0x480)
		cr += m/0x20 + (rand() & 0x7);
	else if (m <= 0x600)
		cr += mm/0x10 - (rand() & 0x7);
	else if (m <= 0x800)
		cr += -im/0x10 - (rand() & 0x7);
	else
		cr /= 2;

	if ((rand() & 0xff) == 1)
		cr = 0xff;
	if ((rand() & 0xff) == 1)
		cr = 0x00;

	if (cr < 0) 
		cr = 0;
	else if (cr > 0xffl) 
		cr = 0xffl;

	MySetPixel(x, y, cr + crest);
}

void FilterScreen(void)
{	
	HDC hDC;
	HDC hMemFrom;
	HDC hMemTo;
	HBITMAP hBmpFrom;
	HBITMAP hBmpTo;
	int x, y, i;

	hDC = GetDC(NULL); // get a handle to the screen
	if (hDC)
	{
		resx = GetDeviceCaps(hDC, HORZRES);
		resy = GetDeviceCaps(hDC, VERTRES);

		hMemFrom = CreateCompatibleDC(hDC);
		hMemTo = CreateCompatibleDC(hDC);
		hBmpFrom = Create24BPPDIBSection(hDC, resx, resy, &pBitsFrom);
		hBmpTo = Create24BPPDIBSection(hDC, resx, resy, &pBitsTo);
		SelectObject(hMemFrom, hBmpFrom);
		SelectObject(hMemTo, hBmpTo);
		GdiFlush();

		BitBlt(hMemTo, 0, 0, resx, resy, hDC, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hDC); hDC = 0;

		Beep(1000, 100);
		for(i = 0; i < 400; i++)
		{
			BitBlt(hMemFrom, 0, 0, resx, resy, hMemTo, 0, 0, SRCCOPY);
#if 1
			for(y=1; y < resy-1; y++)
				for(x=1; x < resx-1; x++)
#else
			for(y=400; y < 600; y++)
				for(x=40; x < 140; x++)
#endif
					FilterElement(x, y);
			
			hDC = GetDC(NULL);
			BitBlt(hDC, 0, 0, resx, resy, hMemTo, 0, 0, SRCCOPY);
			ReleaseDC(NULL, hDC); hDC = 0;
			Beep(2000, 10);
		}

		DeleteDC(hMemFrom);
		DeleteObject(hBmpFrom);
		DeleteDC(hMemTo);
		DeleteObject(hBmpTo);
	}
}

int main(int argc, char* argv[])
{
	FilterScreen();
	return 0;
}


