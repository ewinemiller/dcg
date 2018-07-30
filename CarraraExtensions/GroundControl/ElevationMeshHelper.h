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
template <class c>
 real32 sqr(c pfIn)
{
	return pfIn * pfIn;
}

const real32 sqr(const TVector3 &first, const TVector3 &second) 
{
	return sqr(first.x - second.x) + sqr(first.y - second.y) + sqr(first.z - second.z);
}

const real32 sqr(const TVector2 &first, const TVector2 &second) 
{
	return sqr(first.x - second.x) + sqr(first.y - second.y);
}

template <class c>
 real64 sqrd(c pfIn)
{
	return pfIn * pfIn;
}

const real64 sqr(const TVector2d &first, const TVector2d &second) 
{
	return sqrd(first.x - second.x) + sqrd(first.y - second.y);
}

real32 GetArea(const TVector3& pt1, const TVector3& pt2, const TVector3& pt3)
{
	real32 fEdgeLength[3];

	real32 cos;
	real32 theta;

	fEdgeLength[0] = sqrt(sqr(pt1, pt2));
	fEdgeLength[1] = sqrt(sqr(pt2, pt3));
	fEdgeLength[2] = sqrt(sqr(pt3, pt1));
	
	if ((fEdgeLength[1] == 0)||(fEdgeLength[2] == 0)||(fEdgeLength[2] == 0))
	{
		return 0;
	}
	else
	{
		cos = (sqr(fEdgeLength[1]) + sqr(fEdgeLength[2]) - sqr(fEdgeLength[0]))
			/ (2 * fEdgeLength[1] * fEdgeLength[2]);

		if (cos < -1.0) { cos = -1.0;}
		else if (cos > 1.0) {cos = 1.0;}
		theta = acos(cos);

		return 0.5 * fEdgeLength[1] * fEdgeLength[2] * sin(theta);
	}


}
//
//TVector2d LineLineIntersection(TVector2d originD, TVector2d directionD, TVector2d originE, TVector2d directionE) {
//  directionD.Normalize(directionD);
//  directionE.Normalize(directionE);
//
//  real64 z = directionD.x * directionE.y - directionD.y * directionE.x;
//  TVector2d SR = originD - originE;
//  real64 t = (SR.x*directionE.y - SR.y*directionE.x) / z;
//
//  return originD - t*directionD;
//}
//
//
//TVector2d getCircumcircleCenter(const AdaptivePoint& pt1, const AdaptivePoint& pt2, const AdaptivePoint& pt3)
//{
//	const TVector2d A(pt1.x, pt1.y), B(pt2.x, pt2.y), C(pt3.x, pt3.y);
//
//	// lines from a to b and a to c
//	TVector2d AB = B - A;
//	TVector2d AC = C - A;
//
//	// find the points halfway on AB and AC
//	TVector2d halfAB = A + AB*0.5f;
//	TVector2d halfAC = A + AC*0.5f;
//
//	// build vectors perpendicular to ab and ac
//	TVector2d perpAB(AB.y, -AB.x);
//	TVector2d perpAC(AC.y, -AC.x);
//
//	return LineLineIntersection(halfAB, perpAB, halfAC, perpAC);
//}
//
//void replaceFaceIndex(const uint32 oldFacet,const uint32 newFacet, TIndex2 &edge)
//{
//	try {
//		if ((edge.x == oldFacet && edge.y != newFacet))
//		{
//			edge.x = newFacet;
//		}
//		else if	(edge.y == oldFacet && edge.x != newFacet)
//		{
//			edge.y = newFacet;
//		}
//#ifdef GC_EXTENDED_ERROR_CHECKING
//		else
//		{
//			alertAndThrow(&TMCString255("replaceFaceIndex did not find expected facet."));
//		}
//		if (edge.y == edge.x)
//		{
//			alertAndThrow(&TMCString255("replaceFaceIndex just made a bad edge."));
//		}
//#endif
//
//	}
//	catch (TMCException e)
//	{
//		Alert("replaceFacetEdge");
//		throw TMCException(e);
//	}
//};
//
//void replaceVertex(Triangle& facet, const int32 originalVertex, const int32 newVertex)
//{
//	try {
//		Triangle oldFacet = facet;
//		if (facet.pt1 == originalVertex)
//		{
//			facet.pt1 = newVertex;
//		}
//		else if(facet.pt2 == originalVertex)
//		{
//			facet.pt2 = newVertex;
//		}
//		else if(facet.pt3 == originalVertex)
//		{
//			facet.pt3 = newVertex;
//		}
//#ifdef GC_EXTENDED_ERROR_CHECKING
//		else
//		{
//			alertAndThrow(&TMCString255("replaceVertex did not find expected vertex."));
//		}
//		if (facet.pt1 == facet.pt2 || facet.pt2 == facet.pt3 || facet.pt3 == facet.pt1)
//		{
//			TMCString255 temp;
//			TMCString1023 accu;
//			temp.FromInt32(facet.pt1);
//			accu += TMCString255(" pt1 ") + temp;
//			temp.FromInt32(facet.pt2);
//			accu += TMCString255(" pt2 ") + temp;
//			temp.FromInt32(facet.pt3);
//			accu += TMCString255(" pt3 ") + temp;
//
//			temp.FromInt32(originalVertex);
//			accu += TMCString255(" originalVertex ") + temp;
//			temp.FromInt32(newVertex);
//			accu += TMCString255(" newVertex ") + temp;
//			Alert(accu);
//			alertAndThrow(&TMCString255("replaceVertex just made a bad facet."));
//		}
//#endif
//	}
//	catch (TMCException e)
//	{
//		Alert("replaceVertex");
//		throw TMCException(e);
//	}
//};
//
//void replaceVertexAfter(Triangle& facet, const int32 previousVertex, const int32 newVertex)
//{
//	try {
//#ifdef GC_EXTENDED_ERROR_CHECKING
//		if (previousVertex == newVertex)
//		{
//			alertAndThrow(&TMCString255("replaceVertexAfter previousVertex == newVertex."));
//		}
//#endif
//		Triangle oldFacet = facet;
//		if (facet.pt1 == previousVertex)
//		{
//			facet.pt2 = newVertex;
//		}
//		else if(facet.pt2 == previousVertex)
//		{
//			facet.pt3 = newVertex;
//		}
//		else if(facet.pt3 == previousVertex)
//		{
//			facet.pt1 = newVertex;
//		}
//#ifdef GC_EXTENDED_ERROR_CHECKING
//		else
//		{
//			alertAndThrow(&TMCString255("replaceVertexAfter did not find expected vertex."));
//		}
//		if (facet.pt1 == facet.pt2 || facet.pt2 == facet.pt3 || facet.pt3 == facet.pt1)
//		{
//			TMCString255 temp;
//			TMCString1023 accu;
//			temp.FromInt32(oldFacet.pt1);
//			accu += TMCString255(" pt1 ") + temp;
//			temp.FromInt32(oldFacet.pt2);
//			accu += TMCString255(" pt2 ") + temp;
//			temp.FromInt32(oldFacet.pt3);
//			accu += TMCString255(" pt3 ") + temp;
//
//			temp.FromInt32(previousVertex);
//			accu += TMCString255(" previousVertex ") + temp;
//			temp.FromInt32(newVertex);
//			accu += TMCString255(" newVertex ") + temp;
//			Alert(accu);
//			alertAndThrow(&TMCString255("replaceVertexAfter just made a bad facet."));
//		}
//#endif
//	}
//	catch (TMCException e)
//	{
//		Alert("replaceVertexAfter");
//		throw TMCException(e);
//	}
//};
//
//#ifdef GC_EXTENDED_ERROR_CHECKING
//void validateFacetEdges(const Triangle& facet, const TIndex3& facetEdges, const FacetMesh* theMesh)
//{
//	const TIndex2& edgeVertices1 = theMesh->fEdgeList.fVertexIndices[facetEdges.x];
//	if ((edgeVertices1.x != facet.pt1 || edgeVertices1.y != facet.pt2)
//		&& (edgeVertices1.y != facet.pt1 || edgeVertices1.x != facet.pt2))
//	{
//		alertAndThrow(&TMCString255("validateFacetEdges just found a bad facet."));
//	}
//	const TIndex2& edgeVertices2 = theMesh->fEdgeList.fVertexIndices[facetEdges.y];
//	if ((edgeVertices2.x != facet.pt2 || edgeVertices2.y != facet.pt3)
//		&& (edgeVertices2.y != facet.pt2 || edgeVertices2.x != facet.pt3))
//	{
//		alertAndThrow(&TMCString255("validateFacetEdges just found a bad facet."));
//	}
//	const TIndex2& edgeVertices3 = theMesh->fEdgeList.fVertexIndices[facetEdges.z];
//	if ((edgeVertices3.x != facet.pt3 || edgeVertices3.y != facet.pt1)
//		&& (edgeVertices3.y != facet.pt3 || edgeVertices3.x != facet.pt1))
//	{
//		alertAndThrow(&TMCString255("validateFacetEdges just found a bad facet."));
//	}
//
//}
//#endif
//
//int32 getFacetPointNotOnEdge(const Triangle& facet, const TIndex2& vertices)
//{
//	try {
//		if (facet.pt1 != vertices[0] &&  facet.pt1 != vertices[1])
//		{
//			return facet.pt1;
//		}
//		else if (facet.pt2 != vertices[0] &&  facet.pt2 != vertices[1])
//		{
//			return facet.pt2;
//		}
//		else if (facet.pt3 != vertices[0] &&  facet.pt3 != vertices[1])
//		{
//			return facet.pt3;
//		}
//#ifdef GC_EXTENDED_ERROR_CHECKING
//		else
//		{
//			//should never get here
//			alertAndThrow(&TMCString255("getFacetPointNotOnEdge did not find expected vertex."));
//			return -1;
//		}
//#endif
//	}
//	catch (TMCException e)
//	{
//		Alert("getFacetPointNotOnEdge");
//		throw TMCException(e);
//	}
//};
//
//
//int32 getEdgeWithoutPoint(const TIndex3& facetEdges, const FacetMesh* theMesh, const uint32 pointIndex)
//{
//	try {
//		uint32 found = 0;
//		int32 retval = 0;
//		const TIndex2& edge0 = theMesh->fEdgeList.fVertexIndices[facetEdges.x];
//		if (edge0.x != pointIndex && edge0.y != pointIndex)
//		{
//			retval = facetEdges.x;
//			found++;
//		}
//		const TIndex2& edge1 = theMesh->fEdgeList.fVertexIndices[facetEdges.y];
//
//		if (edge1.x != pointIndex && edge1.y != pointIndex)
//		{
//			retval = facetEdges.y;
//			found++;
//		}
//		const TIndex2& edge2 = theMesh->fEdgeList.fVertexIndices[facetEdges.z];
//
//		if (edge2.x != pointIndex && edge2.y != pointIndex)
//		{
//			retval = facetEdges.z;
//			found++;
//		}
//#ifdef GC_EXTENDED_ERROR_CHECKING
//		if (found == 0)
//		{
//			alertAndThrow(&TMCString255("getEdgeWithoutPoint did not find expected edge."));
//		}
//		if (found > 1)
//		{
//			alertAndThrow(&TMCString255("getEdgeWithoutPoint found a bad facet."));
//		}
//#endif
//		return retval;
//	}
//	catch (TMCException e)
//	{
//		Alert("getEdgeWithoutPoint");
//		throw TMCException(e);
//	}
//}
//
//void fixFacetEdges(const FacetMesh* theMesh, const uint32 facetIndex
//							  , TIndex3& facetEdges
//							  , const uint32 edgesToSearch[], const uint32 edgeCount)
//{
//	try {
//		const Triangle& facet = theMesh->fFacets[facetIndex];
//		boolean corrected[3] = {false, false, false};
//
//		for (uint32 edgeIndex = 0; edgeIndex<edgeCount; edgeIndex++)
//		{
//			const TIndex2& currentEdgeVertices = theMesh->fEdgeList.fVertexIndices[edgesToSearch[edgeIndex]];
//			if (corrected[0] == false && 
//				((facet.pt1 == currentEdgeVertices.x && facet.pt2 == currentEdgeVertices.y)
//				||
//				(facet.pt2 == currentEdgeVertices.x && facet.pt1 == currentEdgeVertices.y)))
//			{
//				facetEdges.x = edgesToSearch[edgeIndex];
//				corrected[0] = true;
//			}
//			else if (corrected[1] == false && 
//				((facet.pt2 == currentEdgeVertices.x && facet.pt3 == currentEdgeVertices.y)
//				||
//				(facet.pt3 == currentEdgeVertices.x && facet.pt2 == currentEdgeVertices.y)))
//			{
//				facetEdges.y = edgesToSearch[edgeIndex];
//				corrected[1] = true;
//			}
//			else if (corrected[2] == false && 
//				((facet.pt3 == currentEdgeVertices.x && facet.pt1 == currentEdgeVertices.y)
//				||
//				(facet.pt1 == currentEdgeVertices.x && facet.pt3 == currentEdgeVertices.y)))
//			{
//				facetEdges.z = edgesToSearch[edgeIndex];
//				corrected[2] = true;
//			}
//		}
//#ifdef GC_EXTENDED_ERROR_CHECKING
//		if (corrected[0] == false || corrected[1] == false || corrected[2] == false)
//		{
//			alertAndThrow(&TMCString255("fixFacetEdges did not find all expected edges."));
//		}
//#endif
//	}
//	catch (TMCException e)
//	{
//		Alert("fixFacetEdges");
//		throw TMCException(e);
//	}
//}
//
//void replaceFacetIndices(FacetMesh* theMesh, const TIndex3& facetEdges, const uint32 oldFacetIndex, const uint32 newFacetIndex)
//{
//	try {
//		for (uint32 edgeIndex = 0; edgeIndex < 3; edgeIndex++)
//		{
//			TIndex2& faceIndices = theMesh->fEdgeList.fFaceIndices[facetEdges[edgeIndex]];
//			if (faceIndices.x == oldFacetIndex && faceIndices.y != newFacetIndex)
//			{
//				faceIndices.x = newFacetIndex;
//			}
//			else if (faceIndices.y == oldFacetIndex && faceIndices.x != newFacetIndex)
//			{
//				faceIndices.y = newFacetIndex;
//			}
//		}
//	}
//	catch (TMCException e)
//	{
//		Alert("replaceFacetIndices");
//		throw TMCException(e);
//	}
//}
//
//int32 getOppositeFacet(const uint32 facetIndex,const TIndex2& faceIndices)
//{
//	try {
//#ifdef GC_EXTENDED_ERROR_CHECKING
//		if (faceIndices.x == faceIndices.y)
//		{
//			alertAndThrow(&TMCString255("getOppositeFacet found a bad edge."));
//		}
//#endif
//		if (faceIndices.x == facetIndex)
//		{
//			return faceIndices.y;
//		}
//		else if (faceIndices.y == facetIndex)
//		{
//			return faceIndices.x;
//		}
//#ifdef GC_EXTENDED_ERROR_CHECKING
//		else
//		{
//			alertAndThrow(&TMCString255("getOppositeFacet could not find expected facet."));
//		}
//#endif
//	}
//	catch (TMCException e)
//	{
//		Alert("getOppositeFacet");
//		throw TMCException(e);
//	}
//
//}
//
//
//void buildSmoothNormals(FacetMesh* theMesh)
//{
//
//	uint32 facetCount = theMesh->fFacets.GetElemCount();
//	uint32 pointCount = theMesh->fVertices.GetElemCount();
//
//
//	for (uint32 facetIndex = 0; facetIndex < facetCount; facetIndex++)
//	{
//		TVector3 normal;
//		Triangle& facet = theMesh->fFacets[facetIndex];
//		TVector3& pt1 = theMesh->fVertices[facet.pt1];
//		TVector3& pt2 = theMesh->fVertices[facet.pt2];
//		TVector3& pt3 = theMesh->fVertices[facet.pt3];
//
//		normal = (pt2 - pt1) ^ (pt3 - pt2);
//		normal.Normalize(normal);
//
//		real32 area = GetArea(pt1, pt2, pt3);
//
//		//weigh the normal so that a bigger attached facet has more influence.
//		theMesh->fNormals[facet.pt1] += normal * area;
//		theMesh->fNormals[facet.pt2] += normal * area;
//		theMesh->fNormals[facet.pt3] += normal * area;
//	}
//
//	for (uint32 normalIndex = 0; normalIndex < pointCount; normalIndex++)
//	{
//		theMesh->fNormals[normalIndex].Normalize(theMesh->fNormals[normalIndex]);
//	}
//	
//
//};
//
///*void dump(FacetMesh* theMesh, TMCArray<FacetEdgeIndices>& facetEdges, uint32 pointCount, uint32 facetCount, int32 split
//		  , TMCArray<AdaptiveEdge>& edges, uint32 edgeCount)
//{
//#ifdef _DEBUG
//	char temp[120];
//	sprintf_s(temp, 120, "\n***********Split %d\n\0"
//		, split
//		);
//	OutputDebugStringA(temp);
//	for (uint32 facetIndex = 0; facetIndex < facetCount; facetIndex++)
//	{
//		const Triangle& facet = theMesh->fFacets[facetIndex];
//		sprintf_s(temp, 120, "F%2u P1  %2u  P2  %2u  P3  %2u \n\0"
//			, facetIndex
//			, facet.pt1
//			, facet.pt2
//			, facet.pt3);
//		OutputDebugStringA(temp);
//		const FacetEdgeIndices& facetEdge = facetEdges[facetIndex];
//		sprintf_s(temp, 120, "    E1  %2u  E2  %2u  E3  %2u \n\0"
//			, facetEdge.edges[0]
//			, facetEdge.edges[1]
//			, facetEdge.edges[2]);
//		OutputDebugStringA(temp);
//	}
//	for (uint32 edgeIndex = 0; edgeIndex < edgeCount; edgeIndex++)
//	{
//		const AdaptiveEdge& edge = edges[edgeIndex];
//		sprintf_s(temp, 120, "E%2u Error  %.8f V  %4d, %4d  F  %4d, %4d \n\0"
//			, edgeIndex
//					, edge.error
//					, edge.vertices[0]
//					, edge.vertices[1]
//					, edge.facets[0]
//					, edge.facets[1]
//					
//					);
//		OutputDebugStringA(temp);
//	}
//	for (uint32 pointIndex = 0; pointIndex < pointCount; pointIndex++)
//	{
//		sprintf_s(temp, 120, "P%2d %.8f , %.8f, %.8f\n\0"
//			, pointIndex
//			, theMesh->fVertices[pointIndex].x
//			, theMesh->fVertices[pointIndex].y
//			, theMesh->fVertices[pointIndex].z);
//		OutputDebugStringA(temp);
//	}
//
////						char temp[80];
////						sprintf_s(temp, 80, "releasing  0x%x used by 0x%x on thread 0x%x\n\0", &element.element, usedBy, GetCurrentThreadId());
////						OutputDebugStringA(temp);
//#endif
//}
//*/