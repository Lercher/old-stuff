// Worm.cpp: implementation of the Worm class.
//
//////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <math.h>
#include "Worm.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Worm::Worm()
{
	long i, x, y;
	COLORREF cr;

	x = RndAB(rc.left, rc.right);
	y = RndAB(rc.top, rc.bottom);
	HeadX = x;
	HeadY = y;
	cr = RGB(RndAB(100, 255), RndAB(100, 255), RndAB(100, 255));
	DeltaColor = RGB(0, 0, RndAB(-5, 5));
	DeltaDeltaDirection = 0.0;
	DeltaDirection = 0.0;
	Direction = ranf() * 3.1415 * 2;
	DirectionBias = 0.1;
	DeltaSpeed = 0.1;
	Speed = 1.0;
	SpeedBias = 0.1;
	Size = RndAB(4, 10);
	for(i=0; i<SIZE; i++)
	{
		Segment[i].x = x;
		Segment[i].y = y;
		Color[i] = cr;
	}
}

Worm::~Worm()
{
	// nix zu tun
}

void Worm::Move()
{
	long i;

	// Neue Beschleunigung
	DeltaSpeed = (ranf() - 0.5 + SpeedBias);
	// Neue Speed
	Speed += DeltaSpeed;
	if (Speed < Size / 2)
	{
		SpeedBias = 0.05;
	}
	if (Speed < 1.0)
	{
		SpeedBias = 0.15;
		Speed = 1;
	}
	if (Speed > Size-1)
	{
		SpeedBias = -0.05;
		Speed = Size-1;
	}
	// Neue DrehBeschleunigung
	DeltaDeltaDirection = (ranf() - 0.5 + DirectionBias) * 0.08;
	// Neue DrehGeschwindigkeit
	DeltaDirection += DeltaDeltaDirection;
	if (DeltaDirection > MAX_TURN)
	{
		DirectionBias = -0.25;
		DeltaDirection = MAX_TURN;
	}
	if (DeltaDirection < -MAX_TURN)
	{
		DirectionBias = 0.25;
		DeltaDirection = -MAX_TURN;
	}
	Direction += DeltaDirection;
	// Segemente bewegen
	LastSegment.x = Segment[SIZE-1].x;
	LastSegment.y = Segment[SIZE-1].y;
	for(i=SIZE-1; i>0; i--)
	{
		Segment[i].x = Segment[i-1].x;
		Segment[i].y = Segment[i-1].y;
		Color[i] = Color[i-1];
	}
	HeadX += Speed * cos(Direction);
	HeadY += Speed * sin(Direction);
	if (HeadY < rc.top)
	{
		HeadY = rc.top;
		Direction = -Direction;
	}
	if (HeadY > rc.bottom)
	{
		HeadY = rc.bottom;
		Direction = -Direction;
	}
	if (HeadX < rc.left)
	{
		HeadX = rc.left;
		Direction = 3.1415 - Direction;
	}
	if (HeadX > rc.right)
	{
		HeadX = rc.right;
		Direction = 3.1415 - Direction;
	}
	Segment[0].x = (long) HeadX;
	Segment[0].y = (long) HeadY;
	Color[0] = (Color[0] + DeltaColor) & 0x00FFFFFFul;
}
// sic! a, nicht (a)
#define P1(a,b) a*(p->x - Size), b*(p->y - Size)
#define P2(a,b) a*(p->x + Size), b*(p->y + Size)

void Worm::Paint(HDC hDC)
{
	HRGN hR1, hR2;
	POINT *p;

	p = &LastSegment;
	hR1 = CreateEllipticRgn(P1(1,1), P2(1,1));
	p = &(Segment[SIZE-1]);
	hR2 = CreateEllipticRgn(P1(1,1), P2(1,1));
	CombineRgn(hR1, hR1, hR2, RGN_DIFF);
	FillRgn(hDC, hR1, GetStockObject(WHITE_BRUSH));
	DeleteObject(hR1);
	DeleteObject(hR2);

	p = &LastSegment;
	hR1 = CreateEllipticRgn(P1(rc.right-1,1), P2(rc.right-1,1));
	p = &(Segment[SIZE-1]);
	hR2 = CreateEllipticRgn(P1(rc.right-1,1), P2(rc.right-1,1));
	CombineRgn(hR1, hR1, hR2, RGN_DIFF);
	FillRgn(hDC, hR1, GetStockObject(WHITE_BRUSH));
	DeleteObject(hR1);
	DeleteObject(hR2);

	p = &LastSegment;
	hR1 = CreateEllipticRgn(P1(1,rc.bottom-1), P2(1,rc.bottom-1));
	p = &(Segment[SIZE-1]);
	hR2 = CreateEllipticRgn(P1(1,rc.bottom-1), P2(1,rc.bottom-1));
	CombineRgn(hR1, hR1, hR2, RGN_DIFF);
	FillRgn(hDC, hR1, GetStockObject(WHITE_BRUSH));
	DeleteObject(hR1);
	DeleteObject(hR2);

	p = &LastSegment;
	hR1 = CreateEllipticRgn(P1(rc.right-1,rc.bottom-1), P2(rc.right-1,rc.bottom-1));
	p = &(Segment[SIZE-1]);
	hR2 = CreateEllipticRgn(P1(rc.right-1,rc.bottom-1), P2(rc.right-1,rc.bottom-1));
	CombineRgn(hR1, hR1, hR2, RGN_DIFF);
	FillRgn(hDC, hR1, GetStockObject(WHITE_BRUSH));
	DeleteObject(hR1);
	DeleteObject(hR2);

	SelectObject(hDC, GetStockObject(BLACK_PEN));
	PaintSegement(hDC, &Segment[0], Color[0]);
}

void Worm::PaintSegement(HDC hDC, POINT *p, COLORREF cr)
{
	HBRUSH hBrush;

	hBrush = CreateSolidBrush(cr);
	SelectObject(hDC, hBrush);
	Ellipse(hDC, P1(1,1), P2(1,1));
	Ellipse(hDC, P1(rc.right-1,1), P2(rc.right-1,1));
	Ellipse(hDC, P1(1,rc.bottom-1), P2(1,rc.bottom-1));
	Ellipse(hDC, P1(rc.right-1,rc.bottom-1), P2(rc.right-1,rc.bottom-1));
	SelectObject(hDC, GetStockObject(WHITE_BRUSH));
	DeleteObject(hBrush);
}

void Worm::UnDraw(HDC hDC)
{
	long i;

	SelectObject(hDC, GetStockObject(WHITE_PEN));
	PaintSegement(hDC, &LastSegment, RGB(255,255,255));
	for(i=SIZE-1; i>=0; i--)
		PaintSegement(hDC, &Segment[i], RGB(255,255,255));
}
