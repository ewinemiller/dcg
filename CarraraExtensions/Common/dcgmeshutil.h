/*  Carrara plug-in utilities
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
#ifndef __DCGMESHUTIL__
#define __DCGMESHUTIL__
  
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCCountedPtr.h"
#include "MCClassArray.h"
#include "I3DExVertexPrimitive.h"
#include "I3dShFacetMesh.h"
#include "I3DShObject.h"

#if VERSIONNUMBER >= 0x060000
extern boolean getIsFigurePrimitive(I3DShObject* object);
extern boolean hasGlobalDeformers(I3DShObject* object);
extern void fixBadFigureClone(I3DExVertexPrimitive* vertexPrimitive, 
	I3DExVertexPrimitive* originalVertexPrimitive);

#endif

extern void removeBonesAndMorphs(I3DExVertexPrimitive* vertexPrimitive);
extern void	buildEdgeList(FacetMesh *mesh);
extern MCCOMErr getVertexPrimitiveFromObject(I3DShObject* object, I3DExVertexPrimitive** vertexPrimitive);




#endif
