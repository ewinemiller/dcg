/*  Anything Grooves - plug-in for Carrara
    Copyright (C) 2000 Eric Winemiller

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
#ifndef __IAGrPrim__
#define __IAGrPrim__  

#if CP_PRAGMA_ONCE
#pragma once
#endif


#include "I3DShFacetMesh.h"
#include "PublicUtilities.h"
#include "I3DShShader.h"
#include "DCGFacetMeshAccumulator.h"

struct TVertex3DLite
{
	TVector3		fVertex;		// x, y, z vertex coordinates
	TVector2		fUV;			// Texture u,v values at that vertex
};

class IAGrPrim {
	public:
		IAGrPrim();
		virtual ~IAGrPrim();
		virtual void DoBasicMesh(FacetMesh** outMesh) = 0;
		virtual void DoMesh(DCGFacetMeshAccumulator* pAccu, TMCCountedPtr<I3DShShader> pShader) = 0;
		virtual void GetBoundingBox(TBBox3D& bbox) = 0;
		virtual uint32 GetUVSpaceCount() = 0;
		virtual MCCOMErr GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo) = 0;
	
};


#endif