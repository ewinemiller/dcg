/*  Ground Control - plug-in for Carrara
    Copyright (C) 2003 Eric Winemiller

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    
    http://digitalcarversguild.com/
    email: ewinemiller @ digitalcarversguild.com (remove space)
*/
#ifndef __ELEVATIONMESH__
#define __ELEVATIONMESH__

#include "MCArray.h"
#include "Vector3.h"
#include "I3DShFacetMesh.h"
#include "DCGBinaryTree.h"
#include "dcgdebug.h"
#include "copyright.h"
#include <iostream>
#include <fstream>
#include "IShTokenStream.h"
#include "GroundControlLib.h"

//#define GC_EXTENDED_ERROR_CHECKING
using namespace std;

class CountedRealArray : public TMCSMPCountedObject 
{

public:
	static void Create(CountedRealArray ** a)
	{
			 TMCCountedCreateHelper<CountedRealArray > result(a);
			 result = new CountedRealArray ();
	};

	void setData(const uint32 index, const real32 value)
	{
		data[index] = value;
	};
	
	real32 getData(const uint32 index)
	{
		return data[index];
	};

	uint32 getDataElemCount(void) const	
	{
		return data.GetElemCount();
	};

#if (VERSIONNUMBER >= 0x080000)
	real32*	getDataPointer(MCSize i)			
	{
		return data.Pointer(i);
	}		

	void setDataElemCount(MCSize i)
	{
		data.SetElemCount(i);
	}
#else
	real32*	getDataPointer(uint32 i)			
	{
		return data.Pointer(i);
	}		

	void setDataElemCount(uint32 i)
	{
		data.SetElemCount(i);
	}
#endif

protected:
	CountedRealArray(){};
	~CountedRealArray(){};
	TMCArray<real32> data;
};


enum Filtering {fiPoint = 1, fiBilinear = 2};

class ElevationMesh
{
public:
	gcMapInfo fullMapInfo;
	TMCCountedPtrArray<CountedRealArray> fElevation;
	TMCArray<TVector2> elevationRange;
	TVector2 completeElevationRange;

	TMCString255 sDescription;
	TMCString255 sResolution;
	real32 getValue(const TVector2& uv, const Filtering& fiFiltering);

	real32 getElevationTop()
	{
		if (farAlong > 0)
			return elevationRange[currentFrame].y * (1.0f - farAlong)
				 + elevationRange[currentFrame + 1].y * farAlong;
		else
			return elevationRange[currentFrame].y;
	}

	real32 getElevationBottom()
	{
		if (farAlong > 0)
			return elevationRange[currentFrame].x * (1.0f - farAlong)
				 + elevationRange[currentFrame + 1].x * farAlong;
		else
			return elevationRange[currentFrame].x;
	}

	real32 getShaderValue(const TVector2& uv, const Filtering& fiFiltering);

	void setCompleteElevationRange();
	ofstream* log;
	// IExStreamIO calls
	virtual MCCOMErr			MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
	virtual MCCOMErr			MCCOMAPI Write(IShTokenStream* stream);


protected:
	ElevationMesh();
	~ElevationMesh();

	void BuildMesh(FacetMesh* theMesh
								, const real32 fSizeX,const real32 fSizeY,const real32 fSizeZ, const real32 baseElevation
								, const int meshType
								, unsigned long targetPolyCount
								, const real32 maxError);

	real32 setCurrentFrame(real32 newFrame, boolean interpolateFrames)
	{
		if (newFrame > fElevation.GetElemCount())
		{
			alertAndThrow(&TMCString255("Frame out of range."));
		}
		currentFrame = floor(newFrame);
		if (interpolateFrames)
		{
			farAlong = newFrame - currentFrame;
			return newFrame;
		}
		else
		{
			farAlong = 0;
			return currentFrame;
		}
	};

	uint32 getCurrentFrame()
	{
		return currentFrame;
	};

	real32 getCurrentInterpolatedFrame()
	{
		return farAlong + currentFrame;
	};

private:
	uint32 currentFrame;
	real32 farAlong;
};

#endif

