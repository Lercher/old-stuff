// Worms.cpp: implementation of the Worms class.
//
//////////////////////////////////////////////////////////////////////

#include <Windows.h>
#include "Worms.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Worms::Worms()
{
	long i;

	theWorms[0] = new Worm;
	for(i=1; i<WORMS; i++)
		theWorms[i] = NULL;
}

Worms::~Worms()
{
	long i;

	for(i=0; i<WORMS; i++)
		if (theWorms[i] != NULL)
		{
			delete theWorms[i];
			theWorms[i] = NULL;
		}
}

void Worms::Paint(HDC hDC)
{
	long i;

	for(i=0; i<WORMS; i++)
		if (theWorms[i] != NULL)
			theWorms[i]->Paint(hDC);
}

void Worms::Move()
{
	long i;

	for(i=0; i<WORMS; i++)
		if (theWorms[i] != NULL)
			theWorms[i]->Move();
}

void Worms::AddNew()
{
	long i;

	for(i=0; i<WORMS; i++)
		if (theWorms[i] == NULL)
		{
			theWorms[i] = new Worm;
			break;
		}
}
