/*  Terrain Tools - plug-in for Carrara
    Copyright (C) 2005 Eric Winemiller

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
#ifndef __ShoreLineCommon__
#define __ShoreLineCommon__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "I3dShScene.h"

struct TerrainToolsCache
{
	IEdgeTree* edgetree;
	TMCArray<real32> oneoverlinemagnitude;
	TMCArray<TVector3> p2minusp1;
	TMCCountedPtr<FacetMesh> mesh;

	TerrainToolsCache()
	{
		edgetree = NULL;
	};
	~TerrainToolsCache()
	{
		cleanup();
	};
	void cleanup()
	{
		if (edgetree != NULL)
		{
			delete edgetree;
		}
		edgetree = NULL;
		mesh = NULL;
		oneoverlinemagnitude.ArrayFree();
		p2minusp1.ArrayFree();
	};

};

enum FalloffStyle {foAbsolute = 1, foLinear = 2, foRadial = 3, foInvRadial = 4, foCustom = 5};
enum ObjectEffect {seExclude = 0, seInclude = 1};


#endif