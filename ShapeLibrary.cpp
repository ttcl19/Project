#include "stdafx.h"
#include "ShapeLibrary.h"

//WARNING: updating any shape requires updating a bunch of stuff in this class. Careful!

namespace Shapes {
	
	extern int I[][24] = {
		{0, 1, 0, 0, 2, 0,
		 0, 1, 0, 0, 2, 0, 
		 0, 1, 0, 0, 2, 0, 
		 0, 1, 0, 0, 2, 0},

	  /*{0, 0, 1, 2, 0, 0,
		 0, 0, 1, 2, 0, 0, 
		 0, 0, 1, 2, 0, 0, 
		 0, 0, 1, 2, 0, 0}*/
	};
	
	extern int J[][24] = {
		{0, 0, 0, 0, 0, 0,
		 0, 1, 0, 0, 2, 0, 
		 0, 1, 0, 0, 2, 0, 
		 1, 1, 0, 2, 2, 0},

		{0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 1, 1, 1, 2, 2, 2, 
		 0, 0, 1, 0, 0, 2},

		{0, 0, 0, 0, 0, 0,
		 0, 1, 1, 0, 2, 2, 
		 0, 1, 0, 0, 2, 0, 
		 0, 1, 0, 0, 2, 0},

		{0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 1, 0, 0, 2, 0, 0, 
		 1, 1, 1, 2, 2, 2}
	};

	extern int L[][24] = {
		{0, 0, 0, 0, 0, 0,
		 0, 1, 0, 0, 2, 0, 
		 0, 1, 0, 0, 2, 0, 
		 0, 1, 1, 0, 2, 2},

		 {0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 0, 0, 1, 0, 0, 2, 
		 1, 1, 1, 2, 2, 2},

		{0, 0, 0, 0, 0, 0,
		 1, 1, 0, 2, 2, 0, 
		 0, 1, 0, 0, 2, 0, 
		 0, 1, 0, 0, 2, 0},

		{0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 1, 1, 1, 2, 2, 2, 
		 1, 0, 0, 2, 0, 0},

	};
	
	extern int O[][24] = {
		{0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 1, 1, 0, 2, 2, 0, 
		 1, 1, 0, 2, 2, 0},

		{0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 0, 1, 1, 0, 2, 2, 
		 0, 1, 1, 0, 2, 2}
	};

	extern int Z[][24] = {
		{0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 1, 1, 0, 2, 2, 0, 
		 0, 1, 1, 0, 2, 2},

		{0, 0, 0, 0, 0, 0,
		 0, 0, 1, 0, 0, 2, 
		 0, 1, 1, 0, 2, 2, 
		 0, 1, 0, 0, 2, 0},
		
		/*{0, 0, 0, 0, 0, 0,
		 0, 1, 0, 0, 2, 0, 
		 1, 1, 0, 2, 2, 0, 
		 1, 0, 0, 2, 0, 0}*/
	};

	extern int T[][24] = {

		{0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 1, 1, 1, 2, 2, 2, 
		 0, 1, 0, 0, 2, 0},

		{0, 0, 0, 0, 0, 0,
		 1, 0, 0, 2, 0, 0, 
		 1, 1, 0, 2, 2, 0,
		 1, 0, 0, 2, 0, 0},

		{0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 0, 1, 0, 0, 2, 0, 
		 1, 1, 1, 2, 2, 2},

		{0, 0, 0, 0, 0, 0,
		 0, 0, 1, 0, 0, 2, 
		 0, 1, 1, 0, 2, 2, 
		 0, 0, 1, 0, 0, 2},

	};

	extern int S[][24] = {
		{0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 
		 0, 1, 1, 0, 2, 2, 
		 1, 1, 0, 2, 2, 0},

		{0, 0, 0, 0, 0, 0,
		 0, 1, 0, 0, 2, 0, 
		 0, 1, 1, 0, 2, 2, 
		 0, 0, 1, 0, 0, 2},
		/*
		{0, 0, 0, 0, 0, 0,
		 1, 0, 0, 2, 0, 0, 
		 1, 1, 0, 2, 2, 0, 
		 0, 1, 0, 0, 2, 0}*/
	};

	extern int bounds[7][4][2][4] = 
	{//shape

		//I
		{//orientation
			{//player
				{//bound: X1 - 0, Y1 - 1, X2 - 2, Y2 - 3
					1,0,2,4
				},
				{4,0,5,4}
			}
		},

		//J
		{//orientation
			{//player
				{0,1,2,4}, {3,1,5,4}
			},
			{ 
				{0,2,3,4}, {3,2,6,4}
			},
			{
				{1,1,3,4}, {4,1,6,4}
			},
			{
				{0,2,3,4}, {3,2,6,4}
			}
		},

		//L
		{//orientation
			{
				{1,1,3,4},{4,1,6,4}
			},
			{
				{0,2,3,4},{3,2,6,4}
			},
			{
				{0,1,2,4},{3,1,5,4}
			},
			{
				{0,2,3,4},{3,2,6,4}
			}
		},
		
		//O
		{ //orientation
			{
				{0,2,2,4},{3,2,5,4}
			},
			{
				{1,2,3,4},{4,2,6,4}
			}
		},
		
		//Z
		{ //orientation
			{
				{0,2,3,4},{3,2,6,4}
			},
			{
				{1,1,3,4},{4,1,6,4}
			}
		
		},

		//T
		{
			{
				{0,2,3,4},{3,2,6,4}
			},
			{
				{0,1,2,4},{3,1,5,4}
			},
			{
				{0,2,3,4},{3,2,6,4}
			},
			{
				{1,1,3,4},{4,1,6,4}
			}
		},

		//S
		{
			{
				{0,2,3,4},{3,2,6,4}
			},
			{
				{1,1,3,4},{4,1,6,4}
			}
		}

	};

	extern int ori[] = {
		sizeof(I) / sizeof(I[0]),
		sizeof(J) / sizeof(J[0]),
		sizeof(L) / sizeof(L[0]),
		sizeof(O) / sizeof(O[0]),
		sizeof(Z) / sizeof(Z[0]),
		sizeof(T) / sizeof(T[0]),
		sizeof(S) / sizeof(S[0])
	};

	
};