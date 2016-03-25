// Worms.h: interface for the Worms class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_WORMS_H__13CDBC87_C3DE_11D1_A84C_204C4F4F5020__INCLUDED_)
#define AFX_WORMS_H__13CDBC87_C3DE_11D1_A84C_204C4F4F5020__INCLUDED_

#include "Worm.h"	// Added by ClassView
#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define WORMS 100

class Worms  
{
public:
	void AddNew(void);
	void Move(void);
	void Paint(HDC hDC);
	Worms();
	virtual ~Worms();

protected:
	Worm *theWorms[WORMS];
};

#endif // !defined(AFX_WORMS_H__13CDBC87_C3DE_11D1_A84C_204C4F4F5020__INCLUDED_)
