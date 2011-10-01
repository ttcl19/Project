#pragma once

#ifndef SHAPES
#define SHAPES

namespace Shapes {

	extern int I[][24];

	extern int J[][24];

	extern int L[][24];

	extern int O[][24];

	extern int Z[][24];

	extern int T[][24];

	extern int S[][24];

	//An array containing the number of elements in each of the above respective arrays 
	extern int ori[];

	//holds all the integer-valued bounds on objects
	extern int bounds[7][4][2][4];

	inline int boundsIndex(int ShapeIndex, int ori, int player, int bound)
	{
		//used by the X1, Y1, X2, Y2 functions to return the integer-valued bound.
		//bound: X1 - 0, Y1 - 1, X2 - 2, Y2 - 3

		return bounds[ShapeIndex - 1][ori][player - 1][bound];

	}

	//Functions that return the suitable integer of X1,Y1,X2,Y2 for taking pictures.
	inline int X1(int ShapeIndex, int ori, int player)
	{
		return boundsIndex(ShapeIndex,ori,player,0);
	}
	inline int Y1(int ShapeIndex, int ori, int player)
	{
		return boundsIndex(ShapeIndex,ori,player,1);
	}
	inline int X2(int ShapeIndex, int ori, int player)
	{
		return boundsIndex(ShapeIndex,ori,player,2);
	}
	inline int Y2(int ShapeIndex, int ori, int player)
	{
		return boundsIndex(ShapeIndex,ori,player,3);
	}
}

#endif
