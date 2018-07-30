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
#include "InfinitePlaneEdgeHelper.h"

inline void FillPoint(const real32& fFarAlong, const TVector3& pt1, const TVector3& pt2, TVector3& result)
{
	//do the 3D point
	result = pt1 + fFarAlong * (pt2 - pt1);
};

void InfinitePlaneEdgeHelper::doPlaneObjectIntersect(FacetMesh* amesh, FacetMesh* newmesh
		, uint32& lNewEdgeIndex, uint32& lNewVertexIndex
		, uint32& lNewVertexCount, uint32& lNewEdgeCount)
{
	real32 fCutOff = 0, fFarAlong;
	TVector3 pt02, pt01, pt12;

	uint32 instanceFacetCount = amesh->fFacets.GetElemCount();

	uint32 lOnLine, lUnderLine, lOverLine, lFacetCount = 0;

	for (uint32 instanceFacetIndex = 0; instanceFacetIndex < instanceFacetCount; instanceFacetIndex++)
	{
		const TVector3& instancePt1 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt1];
		const TVector3& instancePt2 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt2];
		const TVector3& instancePt3 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt3];

		lOnLine = 0; lUnderLine = 0; lOverLine = 0;
		if (instancePt1.z > fCutOff)
		{
			lUnderLine++;
		}
		else if (instancePt1.z < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (instancePt2.z > fCutOff)
		{
			lUnderLine++;
		}
		else if (instancePt2.z < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (instancePt3.z > fCutOff)
		{
			lUnderLine++;
		}
		else if (instancePt3.z < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}

		if (lOnLine == 1)
		{
			//we don't care if only one point is on the line
		}
		else if (lOnLine == 2)
		{
			//grab those two points and that's our edge
			lNewVertexCount+=2;
			lNewEdgeCount+=1;
		}
		else if (lOnLine == 3)
		{
			//add all edges to our list
			lNewVertexCount+=6;
			lNewEdgeCount+=3;
		}
		else if (lOverLine == 3)
		{
			//we don't care
		}
		else if (lUnderLine == 3)
		{
			//we don't care
		}
		else 
		{
			lNewVertexCount+=2;
			lNewEdgeCount+=1;
		}
	}

	newmesh->fVertices.SetElemCount(lNewVertexCount);
	newmesh->fEdgeList.SetElemCount(lNewEdgeCount);

	for (uint32 instanceFacetIndex = 0; instanceFacetIndex < instanceFacetCount; instanceFacetIndex++)
	{
		const TVector3& instancePt1 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt1];
		const TVector3& instancePt2 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt2];
		const TVector3& instancePt3 = amesh->fVertices[amesh->fFacets[instanceFacetIndex].pt3];

		lOnLine = 0; lUnderLine = 0; lOverLine = 0;
		if (instancePt1.z > fCutOff)
		{
			lUnderLine++;
		}
		else if (instancePt1.z < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (instancePt2.z > fCutOff)
		{
			lUnderLine++;
		}
		else if (instancePt2.z < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (instancePt3.z > fCutOff)
		{
			lUnderLine++;
		}
		else if (instancePt3.z < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}

		if (lOnLine == 1)
		{
			//we don't care if only one point is on the line
		}
		else if (lOnLine == 2)
		{
			//grab those two points and that's our edge
			if (instancePt1.z != 0)
			{
				newmesh->fVertices[lNewVertexIndex] = instancePt2;
				newmesh->fVertices[lNewVertexIndex + 1] = instancePt3;
			}
			else if (instancePt2.z != 0)
			{
				newmesh->fVertices[lNewVertexIndex] = instancePt1;
				newmesh->fVertices[lNewVertexIndex + 1] = instancePt3;
			}
			else if (instancePt3.z != 0)
			{
				newmesh->fVertices[lNewVertexIndex] = instancePt2;
				newmesh->fVertices[lNewVertexIndex + 1] = instancePt1;
			}
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].x = lNewVertexIndex;
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].y = lNewVertexIndex + 1;
			lNewVertexIndex+=2;
			lNewEdgeIndex+=1;

		}
		else if (lOnLine == 3)
		{
			//add all edges to our list
			newmesh->fVertices[lNewVertexIndex] = instancePt1;
			newmesh->fVertices[lNewVertexIndex + 1] = instancePt2;
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].x = lNewVertexIndex;
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].y = lNewVertexIndex + 1;
			newmesh->fVertices[lNewVertexIndex + 2] = instancePt2;
			newmesh->fVertices[lNewVertexIndex + 3] = instancePt3;
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex + 1].x = lNewVertexIndex + 2;
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex + 1].y = lNewVertexIndex + 3;
			newmesh->fVertices[lNewVertexIndex + 4] = instancePt3;
			newmesh->fVertices[lNewVertexIndex + 5] = instancePt1;
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex + 2].x = lNewVertexIndex + 4;
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex + 2].y = lNewVertexIndex + 5;
			lNewVertexIndex+=6;
			lNewEdgeIndex+=3;
		}
		else if (lOverLine == 3)
		{
			//we don't care
		}
		else if (lUnderLine == 3)
		{
			//we don't care
		}
		else if (lUnderLine == 2)
		{
			//we chop down the facet and do a partial
			if (instancePt1.z <= fCutOff)
			{
				fFarAlong = (fCutOff - instancePt1.z) / (instancePt2.z - instancePt1.z);
				FillPoint(fFarAlong, instancePt1, instancePt2, pt01);

				fFarAlong = (fCutOff - instancePt1.z) / (instancePt3.z - instancePt1.z);
				FillPoint(fFarAlong, instancePt1, instancePt3, pt02);
				newmesh->fVertices[lNewVertexIndex] = pt01;
				newmesh->fVertices[lNewVertexIndex + 1] = pt02;

			}
			else if (instancePt2.z <= fCutOff)
			{
				fFarAlong = (fCutOff - instancePt2.z) / (instancePt1.z - instancePt2.z);
				FillPoint(fFarAlong, instancePt2, instancePt1, pt01);

				fFarAlong = (fCutOff - instancePt2.z) / (instancePt3.z - instancePt2.z);
				FillPoint(fFarAlong, instancePt2, instancePt3, pt12);							
				newmesh->fVertices[lNewVertexIndex] = pt01;
				newmesh->fVertices[lNewVertexIndex + 1] = pt12;

			}
			else if (instancePt3.z <= fCutOff)
			{
				fFarAlong = (fCutOff - instancePt3.z) / (instancePt2.z - instancePt3.z);
				FillPoint(fFarAlong, instancePt3, instancePt2, pt12);

				fFarAlong = (fCutOff - instancePt3.z) / (instancePt1.z - instancePt3.z);
				FillPoint(fFarAlong, instancePt3, instancePt1, pt02);							
				newmesh->fVertices[lNewVertexIndex] = pt12;
				newmesh->fVertices[lNewVertexIndex + 1] = pt02;

			}
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].x = lNewVertexIndex;
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].y = lNewVertexIndex + 1;
			lNewVertexIndex+=2;
			lNewEdgeIndex+=1;
		}
		else if (lUnderLine == 1)
		{
			//we chop down the facet and do a partial
			if (instancePt1.z >= fCutOff)
			{
				fFarAlong = (fCutOff - instancePt1.z) / (instancePt2.z - instancePt1.z);
				FillPoint(fFarAlong, instancePt1, instancePt2, newmesh->fVertices[lNewVertexIndex]);

				fFarAlong = (fCutOff - instancePt1.z) / (instancePt3.z - instancePt1.z);
				FillPoint(fFarAlong, instancePt1, instancePt3, newmesh->fVertices[lNewVertexIndex + 1]);							

			}
			else if (instancePt2.z >= fCutOff)
			{
				fFarAlong = (fCutOff - instancePt2.z) / (instancePt1.z - instancePt2.z);
				FillPoint(fFarAlong, instancePt2, instancePt1, newmesh->fVertices[lNewVertexIndex]);

				fFarAlong = (fCutOff - instancePt2.z) / (instancePt3.z - instancePt2.z);
				FillPoint(fFarAlong, instancePt2, instancePt3, newmesh->fVertices[lNewVertexIndex + 1]);							

			}
			else if (instancePt3.z >= fCutOff)
			{
				fFarAlong = (fCutOff - instancePt3.z) / (instancePt2.z - instancePt3.z);
				FillPoint(fFarAlong, instancePt3, instancePt2, newmesh->fVertices[lNewVertexIndex]);

				fFarAlong = (fCutOff - instancePt3.z) / (instancePt1.z - instancePt3.z);
				FillPoint(fFarAlong, instancePt3, instancePt1, newmesh->fVertices[lNewVertexIndex + 1]);							

			}
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].x = lNewVertexIndex;
			newmesh->fEdgeList.fVertexIndices[lNewEdgeIndex].y = lNewVertexIndex + 1;
			lNewVertexIndex+=2;
			lNewEdgeIndex+=1;
			
		}
	}
}