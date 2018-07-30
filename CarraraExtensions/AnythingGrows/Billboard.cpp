/*  Anything Grows - plug-in for Carrara
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
#include "copyright.h"
#include "Billboard.h"
#include "publicutilities.h"  

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Billboard(R_CLSID_Billboard);
#else
const MCGUID CLSID_Billboard={R_CLSID_Billboard};
#endif

// Constructor of the C++ Object :
Billboard::Billboard() 
{ 
	fData.fHeight = 10;
	fData.fWidth = 10;
	fData.lPanels = 1;
	fData.fZStart = 0;
}
  
// I3DExGeometricPrimitive methods
// -- Geometry calls

MCCOMErr Billboard::GetNbrLOD(int16& nbrLod)
{ 
	nbrLod = 1;
	return MC_S_OK;
}

  
MCCOMErr Billboard::GetFacetMesh (uint32 lodIndex, FacetMesh** outMesh)
{
	FacetMeshAccumulator acc;

	TFacet3D	newfacet;
	for (int32 lPanel = 0; lPanel < fData.lPanels; lPanel++)
	{
		real32 fTheta, fSin, fCos;
		
		fTheta = - static_cast<real32>(lPanel) / static_cast<real32>(fData.lPanels) * PI;

		fSin = sin(fTheta);
		fCos = cos(fTheta);

		newfacet.fUVSpace=0;
		newfacet.fVertices[0].fVertex[0]=-fData.fWidth * 0.5f * fCos;
		newfacet.fVertices[0].fVertex[1]=fData.fWidth * 0.5f * fSin;
		newfacet.fVertices[0].fVertex[2]=fData.fZStart + fData.fHeight;
		newfacet.fVertices[0].fUV[0]=1.0f;
		newfacet.fVertices[0].fUV[1]=1.0f;

		newfacet.fVertices[1].fVertex[0]=fData.fWidth * 0.5f * fCos;
		newfacet.fVertices[1].fVertex[1]=-fData.fWidth * 0.5f * fSin;
		newfacet.fVertices[1].fVertex[2]=fData.fZStart + fData.fHeight;
		newfacet.fVertices[1].fUV[0]=0.0f;
		newfacet.fVertices[1].fUV[1]=1.0f;

		newfacet.fVertices[2].fVertex[0]=-fData.fWidth * 0.5f * fCos;
		newfacet.fVertices[2].fVertex[1]=fData.fWidth * 0.5f * fSin;
		newfacet.fVertices[2].fVertex[2]=fData.fZStart;
		newfacet.fVertices[2].fUV[0]=1.0f;
		newfacet.fVertices[2].fUV[1]=0.0f;

		newfacet.fVertices[2].fNormal = 
			(newfacet.fVertices[1].fVertex - newfacet.fVertices[0].fVertex)
			^ (newfacet.fVertices[2].fVertex - newfacet.fVertices[1].fVertex);
		newfacet.fVertices[2].fNormal.Normalize(newfacet.fVertices[2].fNormal);
		newfacet.fVertices[0].fNormal = newfacet.fVertices[1].fNormal = newfacet.fVertices[2].fNormal;

		acc.AccumulateFacet(&newfacet);

		newfacet.fVertices[0].fVertex[0]=-fData.fWidth * 0.5f * fCos;
		newfacet.fVertices[0].fVertex[1]=fData.fWidth * 0.5f * fSin;
		newfacet.fVertices[0].fVertex[2]=fData.fZStart;
		newfacet.fVertices[0].fUV[0]=1.0f;
		newfacet.fVertices[0].fUV[1]=0.0f;

		newfacet.fVertices[1].fVertex[0]=fData.fWidth * 0.5f * fCos;
		newfacet.fVertices[1].fVertex[1]=-fData.fWidth * 0.5f * fSin;
		newfacet.fVertices[1].fVertex[2]=fData.fZStart + fData.fHeight;
		newfacet.fVertices[1].fUV[0]=0.0f;
		newfacet.fVertices[1].fUV[1]=1.0f;

		newfacet.fVertices[2].fVertex[0]=fData.fWidth * 0.5f * fCos;
		newfacet.fVertices[2].fVertex[1]=-fData.fWidth * 0.5f * fSin;
		newfacet.fVertices[2].fVertex[2]=fData.fZStart;
		newfacet.fVertices[2].fUV[0]=0.0f;
		newfacet.fVertices[2].fUV[1]=0.0f;

		newfacet.fVertices[2].fNormal = 
			(newfacet.fVertices[1].fVertex - newfacet.fVertices[0].fVertex)
			^ (newfacet.fVertices[2].fVertex - newfacet.fVertices[1].fVertex);
		newfacet.fVertices[2].fNormal.Normalize(newfacet.fVertices[2].fNormal);
		newfacet.fVertices[0].fNormal = newfacet.fVertices[1].fNormal = newfacet.fVertices[2].fNormal;
		acc.AccumulateFacet(&newfacet);
	}
	acc.MakeFacetMesh(outMesh);
	return MC_S_OK;
}

// Give the boundary Box
void Billboard::GetBoundingBox(TBBox3D& bbox)
{
	if (fData.lPanels == 1)
	{
		bbox.fMin[0]=-fData.fWidth * 0.5f;
		bbox.fMax[0]=fData.fWidth * 0.5f;
		bbox.fMin[1]=0.0f;
		bbox.fMax[1]=0.0f;
		bbox.fMin[2]=fData.fZStart;
		bbox.fMax[2]=fData.fHeight + fData.fZStart;
	} 
	else
	{
		bbox.fMin[0]=-fData.fWidth * 0.5f;
		bbox.fMax[0]=fData.fWidth * 0.5f;
		bbox.fMin[1]=-fData.fWidth * 0.5f;
		bbox.fMax[1]=fData.fWidth * 0.5f;
		bbox.fMin[2]=fData.fZStart;
		bbox.fMax[2]=fData.fHeight + fData.fZStart;
	}
}
  
// -- Shading calls
uint32 Billboard::GetUVSpaceCount()
{
	return 1; // the Billboard is describe with only 1 UV-Space
}

MCCOMErr Billboard::GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo)
// Evolve3D GetUVSpace. It's already normalized.
{
	if (uvSpaceID == 0)
	{ 
		uvSpaceInfo->fWraparound[0] = false;  // No Wrap around
		uvSpaceInfo->fWraparound[1] = false;
	}
	return MC_S_OK;
}