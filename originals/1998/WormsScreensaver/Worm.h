// Worm.h: interface for the Worm class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORM_H__13CDBC86_C3DE_11D1_A84C_204C4F4F5020__INCLUDED_)
#define AFX_WORM_H__13CDBC86_C3DE_11D1_A84C_204C4F4F5020__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define SIZE 25
#define MAX_TURN (0.1734) // 30 Grad = ca. 0.5

class Worm  
{
public:
	void UnDraw(HDC hDC);
	void Paint(HDC hDC);
	void Move(void);
	Worm();
	virtual ~Worm();
protected:
	COLORREF DeltaColor;
	double DirectionBias;
	double SpeedBias;
	double DeltaDeltaDirection;
	double DeltaSpeed;
	double HeadX;
	double HeadY;
	void PaintSegement(HDC hDC, POINT *p, COLORREF cr);
	POINT LastSegment;
	double Speed;
	double DeltaDirection;
	double Direction;
	COLORREF Color[SIZE];
	long Size;
	POINT Segment[SIZE];
};

#endif // !defined(AFX_WORM_H__13CDBC86_C3DE_11D1_A84C_204C4F4F5020__INCLUDED_)
