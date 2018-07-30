/*  Ground Control Library - Elevation map tools
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
#define ERROR_THRESHOLD 0.00001
#define CLOSE_TO_EDGE_THRESHOLD 0.0000001

#define GC_SOUTH_EAST 0
#define GC_NORTH_EAST 1
#define GC_SOUTH_WEST 2
#define GC_NORTH_WEST 3


#ifdef GC_EXTENDED_ERROR_CHECKING
void validateFacet(gcMapInfo *mapInfo, unsigned long facetIndex) {
	boolean badFacet = FALSE;

	if (mapInfo->mesh->facets[facetIndex].pointIndex[0] == mapInfo->mesh->facets[facetIndex].pointIndex[1] 
		|| mapInfo->mesh->facets[facetIndex].pointIndex[2] == mapInfo->mesh->facets[facetIndex].pointIndex[1]
		|| mapInfo->mesh->facets[facetIndex].pointIndex[0] == mapInfo->mesh->facets[facetIndex].pointIndex[2]) {
		//duplicate points
		badFacet = TRUE;
	}
	
	if (mapInfo->mesh->points[mapInfo->mesh->facets[facetIndex].pointIndex[0]].east ==  mapInfo->mesh->points[mapInfo->mesh->facets[facetIndex].pointIndex[1]].east
		&& mapInfo->mesh->points[mapInfo->mesh->facets[facetIndex].pointIndex[2]].east ==  mapInfo->mesh->points[mapInfo->mesh->facets[facetIndex].pointIndex[1]].east) {
		//flat east
		badFacet = TRUE;
	}

	if (mapInfo->mesh->points[mapInfo->mesh->facets[facetIndex].pointIndex[0]].north ==  mapInfo->mesh->points[mapInfo->mesh->facets[facetIndex].pointIndex[1]].north
		&& mapInfo->mesh->points[mapInfo->mesh->facets[facetIndex].pointIndex[2]].north ==  mapInfo->mesh->points[mapInfo->mesh->facets[facetIndex].pointIndex[1]].north) {
		//flat north
		badFacet = TRUE;
	}

}

void validateFacetEdge(gcMapInfo *mapInfo, unsigned long facetIndex) {
	boolean badFacet = FALSE;

	if (
		(mapInfo->mesh->facets[facetIndex].pointIndex[0] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[0]].pointIndex[0]
		|| mapInfo->mesh->facets[facetIndex].pointIndex[1] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[0]].pointIndex[1])
		&& (mapInfo->mesh->facets[facetIndex].pointIndex[1] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[0]].pointIndex[0]
		|| mapInfo->mesh->facets[facetIndex].pointIndex[0] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[0]].pointIndex[1])
		){
		//points don't align on first edge
		badFacet = TRUE;
	}
		
	if (
		(mapInfo->mesh->facets[facetIndex].pointIndex[1] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[1]].pointIndex[0]
		|| mapInfo->mesh->facets[facetIndex].pointIndex[2] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[1]].pointIndex[1])
		&& (mapInfo->mesh->facets[facetIndex].pointIndex[2] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[1]].pointIndex[0]
		|| mapInfo->mesh->facets[facetIndex].pointIndex[1] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[1]].pointIndex[1])
		){
		//points don't align on second edge
		badFacet = TRUE;
	}
		
	if (
		(mapInfo->mesh->facets[facetIndex].pointIndex[0] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[2]].pointIndex[0]
		|| mapInfo->mesh->facets[facetIndex].pointIndex[2] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[2]].pointIndex[1])
		&& (mapInfo->mesh->facets[facetIndex].pointIndex[2] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[2]].pointIndex[0]
		|| mapInfo->mesh->facets[facetIndex].pointIndex[0] != mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facetIndex].edgeIndex[2]].pointIndex[1])
		){
		//points don't align on third edge
		badFacet = TRUE;
	}
	
}
#endif

//stuff for building the mesh
int compareAdaptiveFacetError(const void *elem1, const void *elem2)
{
	if (((gcAdaptiveFacet*)elem1)->error < ((gcAdaptiveFacet*)elem2)->error)
		return -1;
	else if (((gcAdaptiveFacet*)elem1)->error > ((gcAdaptiveFacet*)elem2)->error)
		return 1;
	else
	{
		return 0;
	}
}

void replaceVertex(gcFacet *facet, const long originalVertex, const long newVertex)
{
	if (facet->pointIndex[0] == originalVertex)
	{
		facet->pointIndex[0] = newVertex;
	}
	else if(facet->pointIndex[1] == originalVertex)
	{
		facet->pointIndex[1] = newVertex;
	}
	else if(facet->pointIndex[2] == originalVertex)
	{
		facet->pointIndex[2] = newVertex;
	}
}

void replaceVertexAfter(gcFacet *facet, const long previousVertex, const long newVertex)
{
	if (facet->pointIndex[0] == previousVertex)
	{
		facet->pointIndex[1] = newVertex;
	}
	else if(facet->pointIndex[1] == previousVertex)
	{
		facet->pointIndex[2] = newVertex;
	}
	else if(facet->pointIndex[2] == previousVertex)
	{
		facet->pointIndex[0] = newVertex;
	}
}

long getFacetPointNotOnEdge(const gcFacet *facet, unsigned long *pointIndex)
{
	if (facet->pointIndex[0] != pointIndex[0] &&  facet->pointIndex[0] != pointIndex[1])
	{
		return facet->pointIndex[0];
	}
	else if (facet->pointIndex[1] != pointIndex[0] &&  facet->pointIndex[1] != pointIndex[1])
	{
		return facet->pointIndex[1];
	}
	else if (facet->pointIndex[2] != pointIndex[0] &&  facet->pointIndex[2] != pointIndex[1])
	{
		return facet->pointIndex[2];
	}
	return -1;
}


void fixFacetEdges(const gcMesh *mesh, const unsigned long facetIndex
							  , unsigned long *facetEdges
							  , const unsigned long *edgesToSearch, const unsigned long edgeCount)
{
	unsigned long edgeIndex; 

	const gcFacet facet = mesh->facets[facetIndex];
	boolean corrected[3] = {FALSE, FALSE, FALSE};

	for (edgeIndex = 0; edgeIndex < edgeCount; edgeIndex++)
	{
		const long currentEdgeVertices[2] = {mesh->edges[edgesToSearch[edgeIndex]].pointIndex[0], mesh->edges[edgesToSearch[edgeIndex]].pointIndex[1]};
		if (corrected[0] == FALSE && 
			((facet.pointIndex[0] == currentEdgeVertices[0] && facet.pointIndex[1] == currentEdgeVertices[1])
			||
			(facet.pointIndex[1] == currentEdgeVertices[0] && facet.pointIndex[0] == currentEdgeVertices[1])))
		{
			facetEdges[0] = edgesToSearch[edgeIndex];
			corrected[0] = TRUE;
		}
		else if (corrected[1] == FALSE && 
			((facet.pointIndex[1] == currentEdgeVertices[0] && facet.pointIndex[2] == currentEdgeVertices[1])
			||
			(facet.pointIndex[2] == currentEdgeVertices[0] && facet.pointIndex[1] == currentEdgeVertices[1])))
		{
			facetEdges[1] = edgesToSearch[edgeIndex];
			corrected[1] = TRUE;
		}
		else if (corrected[2] == FALSE && 
			((facet.pointIndex[2] == currentEdgeVertices[0] && facet.pointIndex[0] == currentEdgeVertices[1])
			||
			(facet.pointIndex[0] == currentEdgeVertices[0] && facet.pointIndex[2] == currentEdgeVertices[1])))
		{
			facetEdges[2] = edgesToSearch[edgeIndex];
			corrected[2] = TRUE;
		}
	}
}

void replaceFacetIndices(gcMesh *mesh, const unsigned long *facetEdges, const unsigned long oldFacetIndex, const unsigned long newFacetIndex)
{
	unsigned long edgeIndex;

	for (edgeIndex = 0; edgeIndex < 3; edgeIndex++)
	{
		long *faceIndices = mesh->edges[facetEdges[edgeIndex]].facetIndex;
		if (faceIndices[0] == oldFacetIndex && faceIndices[1] != newFacetIndex)
		{
			faceIndices[0] = newFacetIndex;
		}
		else if (faceIndices[1] == oldFacetIndex && faceIndices[0] != newFacetIndex)
		{
			faceIndices[1] = newFacetIndex;
		}
	}
}

unsigned long getEdgeWithoutPoint(const gcMesh *mesh, const unsigned long facetEdgeIndex, const unsigned long pointIndex)
{
	unsigned long retval = 0;
	const unsigned long *facetEdges = mesh->facetEdges[facetEdgeIndex].edgeIndex;
	const unsigned long *edge0 = mesh->edges[facetEdges[0]].pointIndex;
	const unsigned long *edge1 = mesh->edges[facetEdges[1]].pointIndex;
	const unsigned long *edge2 = mesh->edges[facetEdges[2]].pointIndex;

	if (edge0[0] != pointIndex && edge0[1] != pointIndex)
	{
		retval = facetEdges[0];
	}

	if (edge1[0] != pointIndex && edge1[1] != pointIndex)
	{
		retval = facetEdges[1];
	}

	if (edge2[0] != pointIndex && edge2[1] != pointIndex)
	{
		retval = facetEdges[2];
	}

	return retval;
}

void gcFillPoint(gcMapInfo *mapInfo, const unsigned long pointIndex, const unsigned long east, const unsigned long north) {
	mapInfo->mesh->points[pointIndex].east = east;
	mapInfo->mesh->points[pointIndex].north = north;
	mapInfo->mesh->points[pointIndex].elevation = gcGetElevationByIndex(mapInfo, gcGetElevationIndex(mapInfo, east, north));
}

void gcFillFacet(gcMapInfo *mapInfo, const unsigned long facetIndex, const unsigned long pt0, const unsigned long pt1, const unsigned long pt2) {
	mapInfo->mesh->facets[facetIndex].pointIndex[0] = pt0;
	mapInfo->mesh->facets[facetIndex].pointIndex[1] = pt1;
	mapInfo->mesh->facets[facetIndex].pointIndex[2] = pt2;
}

void gcFillEdge(gcMapInfo *mapInfo, const unsigned long edgeIndex, const unsigned long v0, const unsigned long v1, const unsigned long f0, const unsigned long f1)
{
	mapInfo->mesh->edges[edgeIndex].facetIndex[0] = f0;
	mapInfo->mesh->edges[edgeIndex].facetIndex[1] = f1;
	mapInfo->mesh->edges[edgeIndex].pointIndex[0] = v0;
	mapInfo->mesh->edges[edgeIndex].pointIndex[1] = v1;
}

void gcFillFacetEdge(gcMapInfo *mapInfo, const unsigned long currentFacetEdge, const unsigned long e0, const unsigned long e1, const unsigned long e2) {
	mapInfo->mesh->facetEdges[currentFacetEdge].edgeIndex[0] = e0;
	mapInfo->mesh->facetEdges[currentFacetEdge].edgeIndex[1] = e1;
	mapInfo->mesh->facetEdges[currentFacetEdge].edgeIndex[2] = e2;
}

void fillFacetSectionError(const gcMapInfo *mapInfo, const gcPoint *minp1, const gcPoint *minp2
							, const gcPoint *maxp1, const gcPoint *maxp2
							, const unsigned long minEast, const unsigned long maxEast, gcAdaptiveFacet *facet
							, const TVector4d *planeEquation, const double elevationStepAlongNorth) {
	double minEdgeNorth, minEdgeStepNorth;
	double maxEdgeNorth, maxEdgeStepNorth;
	double facetElevation;
	long east, north;

	minEdgeNorth = gcGetValueFromFarAlong(minp1->east, minp2->east, minEast, minp1->north, minp2->north);
	minEdgeStepNorth = gcGetValueFromFarAlong(minp1->east, minp2->east, minEast + 1, minp1->north, minp2->north);

	minEdgeStepNorth -= minEdgeNorth;

	maxEdgeNorth = gcGetValueFromFarAlong(maxp1->east, maxp2->east, minEast, maxp1->north, maxp2->north);
	maxEdgeStepNorth = gcGetValueFromFarAlong(maxp1->east, maxp2->east, minEast + 1, maxp1->north, maxp2->north);

	maxEdgeStepNorth -= maxEdgeNorth;


	for (east = minEast; east <= maxEast; east++, minEdgeNorth+=minEdgeStepNorth, maxEdgeNorth+=maxEdgeStepNorth)
	{
		unsigned long loopminy, loopmaxy;

		loopminy = ceil(minEdgeNorth);
		if (maxEdgeNorth < 0) 
		{
			maxEdgeNorth = 0;
		}
		else
		{
			loopmaxy = floor(maxEdgeNorth);
		}

		if (loopminy <= loopmaxy)
		{
			facetElevation = gcGetZFromPlaneEquation(planeEquation, east, loopminy);

			for (north = loopminy; north <= loopmaxy; north++, facetElevation+=elevationStepAlongNorth)
			{		
				double actualElevation = gcGetElevationByCoordinates(mapInfo, east, north);
				double localError = fabs(actualElevation - facetElevation);

				if (localError > ERROR_THRESHOLD && localError > facet->error)
				{
					facet->error = localError;
					facet->errorLocation[GC_EAST] = east;
					facet->errorLocation[GC_NORTH] = north;
				}
			}
		}
	}
}

void gcFillFacetError(gcMapInfo *mapInfo, gcAdaptiveFacet *facet) {
	unsigned long sortedEast[3] = {mapInfo->mesh->facets[facet->facetIndex].pointIndex[0], 
			mapInfo->mesh->facets[facet->facetIndex].pointIndex[1], 
			mapInfo->mesh->facets[facet->facetIndex].pointIndex[2]};
	boolean midPointHigh = FALSE;
	gcPoint *p[3];
	double tempy;
	double elevationStepAlongNorth;
	TVector4d planeEquation;

	//sort points ascending X hard coded bubble
	if (mapInfo->mesh->points[sortedEast[0]].east > mapInfo->mesh->points[sortedEast[1]].east)
	{
		unsigned long temp = sortedEast[1];
		sortedEast[1] = sortedEast[0];
		sortedEast[0] = temp;
	}
	if (mapInfo->mesh->points[sortedEast[1]].east > mapInfo->mesh->points[sortedEast[2]].east)
	{
		unsigned long temp = sortedEast[1];
		sortedEast[1] = sortedEast[2];
		sortedEast[2] = temp;
	}
	if (mapInfo->mesh->points[sortedEast[0]].east > mapInfo->mesh->points[sortedEast[1]].east)
	{
		unsigned long temp = sortedEast[1];
		sortedEast[1] = sortedEast[0];
		sortedEast[0] = temp;
	}
	p[0] = &mapInfo->mesh->points[sortedEast[0]];
	p[1] = &mapInfo->mesh->points[sortedEast[1]];
	p[2] = &mapInfo->mesh->points[sortedEast[2]];

	//is 1->3 higher than 2->3
	tempy = gcGetValueFromFarAlong(p[0]->east, p[2]->east, p[1]->east, p[0]->north, p[2]->north);
	if (tempy < p[1]->north)
	{
		midPointHigh = TRUE;
	}

	//clear the error
	facet->error = 0;

	gcFillPlaneEquation(p[0], p[1], p[2], &planeEquation);

	elevationStepAlongNorth =  gcGetZFromPlaneEquation(&planeEquation, 1, 2) - gcGetZFromPlaneEquation(&planeEquation, 1, 1);

	//do first leg
	if (p[0]->east != p[1]->east)
	{
		long minEast = p[0]->east + 1;
		long maxEast = p[1]->east;

		if (midPointHigh)
		{
			//min = 0->2
			//max = 0->1
			fillFacetSectionError(mapInfo, p[0], p[2]
				, p[0], p[1]
				, minEast, maxEast, facet, &planeEquation, elevationStepAlongNorth);
		}
		else
		{
			//min = 0->1
			//max = 0->2
			fillFacetSectionError(mapInfo, p[0], p[1]
				, p[0], p[2]
				, minEast, maxEast, facet, &planeEquation, elevationStepAlongNorth);
		}
	}

	//do second leg
	if (p[1]->east != p[2]->east)
	{
		long minEast = p[1]->east;
		long maxEast = p[2]->east - 1;

		//if mid point doesn't equal first point
		//the mid point has been covered in the previous 
		//leg, skip it
		if (p[0]->east != p[1]->east)
		{
			minEast++;
		}

		if (midPointHigh)
		{
			//min = 0->2
			//max = 1->2
			fillFacetSectionError(mapInfo, p[0], p[2]
				, p[1], p[2]
				, minEast, maxEast, facet, &planeEquation, elevationStepAlongNorth);
		}
		else
		{
			//min = 1->2
			//max = 0->2
			fillFacetSectionError(mapInfo, p[1], p[2]
				, p[0], p[2]
				, minEast, maxEast, facet, &planeEquation, elevationStepAlongNorth);
		}
	}


	//check to see if we're basically on an edge and keep track of which one
	facet->worstEdgeIndex = -1;
	if (facet->error > 0) 
	{
		double distance[3];
		distance[0] = getDistanceFromPointToLine( &mapInfo->mesh->points[mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facet->facetIndex].edgeIndex[0]].pointIndex[0]] 
			, &mapInfo->mesh->points[mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facet->facetIndex].edgeIndex[0]].pointIndex[1]]
			, facet->errorLocation[GC_EAST], facet->errorLocation[GC_NORTH] );
		distance[1] = getDistanceFromPointToLine( &mapInfo->mesh->points[mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facet->facetIndex].edgeIndex[1]].pointIndex[0]] 
			, &mapInfo->mesh->points[mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facet->facetIndex].edgeIndex[1]].pointIndex[1]]
			, facet->errorLocation[GC_EAST], facet->errorLocation[GC_NORTH] );
		distance[2] = getDistanceFromPointToLine( &mapInfo->mesh->points[mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facet->facetIndex].edgeIndex[2]].pointIndex[0]] 
			, &mapInfo->mesh->points[mapInfo->mesh->edges[mapInfo->mesh->facetEdges[facet->facetIndex].edgeIndex[2]].pointIndex[1]]
			, facet->errorLocation[GC_EAST], facet->errorLocation[GC_NORTH] );

		if (distance[0] <= distance[1] && distance[0] <= distance[2])
		{
			if (distance[0] < CLOSE_TO_EDGE_THRESHOLD)
			{
				facet->worstEdgeIndex = mapInfo->mesh->facetEdges[facet->facetIndex].edgeIndex[0];
			}
		}
		if (distance[1] <= distance[0] && distance[1] <= distance[2])
		{
			if (distance[1] < CLOSE_TO_EDGE_THRESHOLD)
			{
				facet->worstEdgeIndex = mapInfo->mesh->facetEdges[facet->facetIndex].edgeIndex[1];
			}
		}
		if (distance[2] <= distance[1] && distance[2] <= distance[0])
		{
			if (distance[2] < CLOSE_TO_EDGE_THRESHOLD)
			{
				facet->worstEdgeIndex = mapInfo->mesh->facetEdges[facet->facetIndex].edgeIndex[2];
			}
		}
	}
}

void gcSetAdaptiveFacetError(dcgBinaryTree *facetTree, gcMapInfo *mapInfo, gcAdaptiveFacet *facets, const unsigned long facetIndex) {

	facets[facetIndex].facetIndex = facetIndex;
	gcFillFacetError(mapInfo, &facets[facetIndex]);
	if (facets[facetIndex].error == 0)
	{
		//error is zero, I'll never pick it to split so don't bother adding
		return;
	}
	DCGBinaryTreeAddElem(facetTree, &facets[facetIndex]);
}

void replaceFaceIndex(const unsigned long oldFacet,const unsigned long newFacet, long *facetIndex)
{
	if ((facetIndex[0] == oldFacet && facetIndex[1] != newFacet))
	{
		facetIndex[0] = newFacet;
	}
	else if	(facetIndex[1] == oldFacet && facetIndex[0] != newFacet)
	{
		facetIndex[1] = newFacet;
	}
}

long getOppositeFacet(const unsigned long facetIndex, const long *facetIndices)
{
	if (facetIndices[0] == facetIndex)
	{
		return facetIndices[1];
	}
	else if (facetIndices[1] == facetIndex)
	{
		return facetIndices[0];
	}
	return -2;
}

//given a newly created edge, check to see if good split
void checkEdge(dcgBinaryTree *facetTree, gcAdaptiveFacet *facets, gcMapInfo *mapInfo,
		const unsigned long newFacetIndex, const unsigned long suspectEdgeIndex , const unsigned long newPointIndex) {

	long oldFacetIndex = getOppositeFacet(newFacetIndex, 
		mapInfo->mesh->edges[suspectEdgeIndex].facetIndex);

	gcPoint *pt0;
	gcPoint *pt1;
	gcPoint *pt2;

	TVector2d center, newpoint, pt0vec;

	double circumcircleRadius, distanceNewPoint;

	unsigned long oldFacetPointNotOnEdge;

	unsigned long edgesToSearch[6];

	//no opposing facet or error
	if (oldFacetIndex < 0)
	{
		return;
	}

	//get the radius of old facet, if distance to opposite point from suspect edge
	//on new facet is < radius then new facet is too thin
	//swap the edge and check the new suspect edges
	pt0 = &mapInfo->mesh->points[mapInfo->mesh->facets[oldFacetIndex].pointIndex[0]];
	pt1 = &mapInfo->mesh->points[mapInfo->mesh->facets[oldFacetIndex].pointIndex[1]];
	pt2 = &mapInfo->mesh->points[mapInfo->mesh->facets[oldFacetIndex].pointIndex[2]];

	center = getCircumcircleCenter(pt0, pt1, pt2);
	newpoint.east = mapInfo->mesh->points[newPointIndex].east;
	newpoint.north = mapInfo->mesh->points[newPointIndex].north;

	pt0vec.east = pt0->east;
	pt0vec.north = pt0->north;

	circumcircleRadius = sqrTVector2d(&center, &pt0vec);
	distanceNewPoint = sqrTVector2d(&center, &newpoint);

	if (circumcircleRadius < distanceNewPoint) 
	{
		return;
	}

	oldFacetPointNotOnEdge = getFacetPointNotOnEdge(&mapInfo->mesh->facets[oldFacetIndex], mapInfo->mesh->edges[suspectEdgeIndex].pointIndex);

	//reorder the points on our facets
	replaceVertexAfter(&mapInfo->mesh->facets[oldFacetIndex], oldFacetPointNotOnEdge, newPointIndex);

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacet (mapInfo, oldFacetIndex);
#endif

	replaceVertexAfter(&mapInfo->mesh->facets[newFacetIndex], newPointIndex, oldFacetPointNotOnEdge);

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacet (mapInfo, newFacetIndex);
#endif

	//move our suspect edge so that it's the opposite points it was
	mapInfo->mesh->edges[suspectEdgeIndex].pointIndex[0] = oldFacetPointNotOnEdge;
	mapInfo->mesh->edges[suspectEdgeIndex].pointIndex[1] = newPointIndex;

	//reorder the edges on our facets
	edgesToSearch[0] = mapInfo->mesh->facetEdges[oldFacetIndex].edgeIndex[0];
	edgesToSearch[1] = mapInfo->mesh->facetEdges[oldFacetIndex].edgeIndex[1];
	edgesToSearch[2] = mapInfo->mesh->facetEdges[oldFacetIndex].edgeIndex[2];
	edgesToSearch[3] = mapInfo->mesh->facetEdges[newFacetIndex].edgeIndex[0];
	edgesToSearch[4] = mapInfo->mesh->facetEdges[newFacetIndex].edgeIndex[1];
	edgesToSearch[5] = mapInfo->mesh->facetEdges[newFacetIndex].edgeIndex[2];

	fixFacetEdges(mapInfo->mesh, oldFacetIndex, mapInfo->mesh->facetEdges[oldFacetIndex].edgeIndex, edgesToSearch, 6);

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacetEdge (mapInfo, oldFacetIndex);
#endif

	fixFacetEdges(mapInfo->mesh, newFacetIndex, mapInfo->mesh->facetEdges[newFacetIndex].edgeIndex, edgesToSearch, 6);

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacetEdge (mapInfo, newFacetIndex);
#endif

	//change the facet indices on the reordered facets
	replaceFacetIndices(mapInfo->mesh, mapInfo->mesh->facetEdges[oldFacetIndex].edgeIndex, newFacetIndex, oldFacetIndex);
	replaceFacetIndices(mapInfo->mesh, mapInfo->mesh->facetEdges[newFacetIndex].edgeIndex, oldFacetIndex, newFacetIndex);

	//call checkEdge on our 2 new suspect edges
	checkEdge(facetTree, facets, mapInfo, oldFacetIndex, getEdgeWithoutPoint(mapInfo->mesh, oldFacetIndex, newPointIndex), newPointIndex);
	checkEdge(facetTree, facets, mapInfo, newFacetIndex, getEdgeWithoutPoint(mapInfo->mesh, newFacetIndex, newPointIndex), newPointIndex);

	//remove myself from the tree
	DCGBinaryTreeRemoveElement(facetTree, &facets[oldFacetIndex]);

	//and add the old facet back in with the new error
	gcSetAdaptiveFacetError(facetTree, mapInfo, facets, oldFacetIndex);
}



void splitFacet(dcgBinaryTree *facetTree, 
		gcAdaptiveFacet *facets, gcMapInfo *mapInfo, const unsigned long worstFacetIndex
		, unsigned long *currentFacetIndex, unsigned long *currentPointIndex, unsigned long *currentEdgeIndex) {

	unsigned long newFacet1Index = *currentFacetIndex;
	unsigned long newFacet2Index = *currentFacetIndex + 1;
	unsigned long newPointIndex = *currentPointIndex;
	unsigned long newEdgeIndex = *currentEdgeIndex;

	gcAdaptiveFacet *worstFacet = &facets[worstFacetIndex];

	gcFacetEdge *worstFacetEdges = &mapInfo->mesh->facetEdges[worstFacetIndex];

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacet (mapInfo, worstFacetIndex);
	validateFacetEdge (mapInfo, worstFacetIndex);
#endif

	//find the appropriate point to split at and add it
	gcFillPoint(mapInfo, newPointIndex, worstFacet->errorLocation[GC_EAST],  worstFacet->errorLocation[GC_NORTH]);

	//create the three new edges
	//currentPoint -> worstFacet.pt1
	gcFillEdge(mapInfo, newEdgeIndex, newPointIndex, mapInfo->mesh->facets[worstFacetIndex].pointIndex[0], newFacet1Index, newFacet2Index);
	//currentPoint -> worstFacet.pt2
	gcFillEdge(mapInfo, newEdgeIndex + 1, newPointIndex, mapInfo->mesh->facets[worstFacetIndex].pointIndex[1], newFacet1Index, worstFacetIndex);
	//currentPoint -> worstFacet.pt3
	gcFillEdge(mapInfo, newEdgeIndex + 2, newPointIndex, mapInfo->mesh->facets[worstFacetIndex].pointIndex[2], worstFacetIndex, newFacet2Index);

	//create the two new facets
	gcFillFacet(mapInfo, newFacet1Index
		, mapInfo->mesh->facets[worstFacetIndex].pointIndex[0]
		, mapInfo->mesh->facets[worstFacetIndex].pointIndex[1]
		, newPointIndex);

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacet (mapInfo, newFacet1Index);
#endif

	gcFillFacetEdge(mapInfo, newFacet1Index, worstFacetEdges->edgeIndex[0], newEdgeIndex + 1, newEdgeIndex + 0);

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacetEdge (mapInfo, newFacet1Index);
#endif

	replaceFaceIndex(worstFacetIndex, newFacet1Index, mapInfo->mesh->edges[worstFacetEdges->edgeIndex[0]].facetIndex);


	gcFillFacet(mapInfo, newFacet2Index
		, mapInfo->mesh->facets[worstFacetIndex].pointIndex[0]
		, newPointIndex
		, mapInfo->mesh->facets[worstFacetIndex].pointIndex[2]);

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacet (mapInfo, newFacet2Index);
#endif

	gcFillFacetEdge(mapInfo, newFacet2Index, newEdgeIndex + 0, newEdgeIndex + 2, worstFacetEdges->edgeIndex[2]);

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacetEdge (mapInfo, newFacet2Index);
#endif

	replaceFaceIndex(worstFacetIndex, newFacet2Index, mapInfo->mesh->edges[worstFacetEdges->edgeIndex[2]].facetIndex);

	//reassign the existing facet
	mapInfo->mesh->facets[worstFacetIndex].pointIndex[0] = newPointIndex;
	//worstFacet.pt2 = worstFacet.pt2;
	//worstFacet.pt3 = worstFacet.pt3;

	worstFacetEdges->edgeIndex[0] = newEdgeIndex + 1;
	//worstFacetEdges.y = worstFacetEdges.y;
	worstFacetEdges->edgeIndex[2] = newEdgeIndex + 2;

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacet (mapInfo, worstFacetIndex);
#endif

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacetEdge (mapInfo, worstFacetIndex);
#endif

	checkEdge(facetTree, facets, mapInfo, newFacet2Index, mapInfo->mesh->facetEdges[newFacet2Index].edgeIndex[2], newPointIndex);
	checkEdge(facetTree, facets, mapInfo, worstFacetIndex, mapInfo->mesh->facetEdges[worstFacetIndex].edgeIndex[1], newPointIndex);
	checkEdge(facetTree, facets, mapInfo, newFacet1Index, mapInfo->mesh->facetEdges[newFacet1Index].edgeIndex[0], newPointIndex);

	//add to facetTree
	gcSetAdaptiveFacetError(facetTree, mapInfo, facets, newFacet1Index);
	gcSetAdaptiveFacetError(facetTree, mapInfo, facets, newFacet2Index);
	gcSetAdaptiveFacetError(facetTree, mapInfo, facets, worstFacetIndex);

	*currentFacetIndex += 2;
	*currentPointIndex += 1;
	*currentEdgeIndex += 3;
}

void splitFacetOnEdge(dcgBinaryTree *facetTree, 
		gcAdaptiveFacet *facets, gcMapInfo *mapInfo
		, const unsigned long worstFacetIndex
		, unsigned long *currentFacetIndex, unsigned long *currentPointIndex, unsigned long *currentEdgeIndex) {

	unsigned int newPointIndex = *currentPointIndex;
	unsigned int newFacetIndex = *currentFacetIndex;
	unsigned int newEdgeIndex = *currentEdgeIndex;

	const unsigned long worstEdgeIndex = facets[worstFacetIndex].worstEdgeIndex;

	gcFacet *newFacet1 = &mapInfo->mesh->facets[newFacetIndex];
	gcFacet *worstFacet = &mapInfo->mesh->facets[worstFacetIndex];

#ifdef GC_EXTENDED_ERROR_CHECKING
	validateFacet (mapInfo, worstFacetIndex);
	validateFacetEdge (mapInfo, worstFacetIndex);
#endif

	//find the appropriate point to split at and add it
	gcFillPoint(mapInfo, newPointIndex, facets[worstFacetIndex].errorLocation[GC_EAST], facets[worstFacetIndex].errorLocation[GC_NORTH]);

	//create the new edges
	//first from new point to opposite point
	gcFillEdge(mapInfo, newEdgeIndex, newPointIndex, getFacetPointNotOnEdge(worstFacet, mapInfo->mesh->edges[worstEdgeIndex].pointIndex), newFacetIndex, worstFacetIndex);

	if (mapInfo->mesh->edges[worstEdgeIndex].facetIndex[1] == -1)
	{
		
		//from new point to end of original edge
		gcFillEdge(mapInfo, newEdgeIndex + 1, newPointIndex,  mapInfo->mesh->edges[worstEdgeIndex].pointIndex[1], newFacetIndex, -1);

		//create the new facet
		*newFacet1 = *worstFacet;

		replaceVertex(newFacet1, mapInfo->mesh->edges[worstEdgeIndex].pointIndex[0], newPointIndex);

#ifdef GC_EXTENDED_ERROR_CHECKING
		validateFacet (mapInfo, newFacetIndex);
#endif		
		//update the existing facet
		replaceVertex(worstFacet, mapInfo->mesh->edges[worstEdgeIndex].pointIndex[1], newPointIndex);

#ifdef GC_EXTENDED_ERROR_CHECKING
		validateFacet (mapInfo, worstFacetIndex);
#endif


		//update existing edge's end point
		mapInfo->mesh->edges[worstEdgeIndex].pointIndex[1] = newPointIndex;

		//reorder the edges on our facets
		{
			unsigned long edgesToSearch[5] = {mapInfo->mesh->facetEdges[worstFacetIndex].edgeIndex[0]
				, mapInfo->mesh->facetEdges[worstFacetIndex].edgeIndex[1]
				, mapInfo->mesh->facetEdges[worstFacetIndex].edgeIndex[2] 
				, newEdgeIndex, newEdgeIndex + 1};
			fixFacetEdges(mapInfo->mesh, worstFacetIndex, mapInfo->mesh->facetEdges[worstFacetIndex].edgeIndex, edgesToSearch, 5);

#ifdef GC_EXTENDED_ERROR_CHECKING
			validateFacetEdge (mapInfo, worstFacetIndex);
#endif					
			fixFacetEdges(mapInfo->mesh, newFacetIndex, mapInfo->mesh->facetEdges[newFacetIndex].edgeIndex, edgesToSearch, 5);

#ifdef GC_EXTENDED_ERROR_CHECKING
			validateFacetEdge (mapInfo, newFacetIndex);
#endif			
		}

		//update the face indexes for edges on the new facets
		replaceFaceIndex(worstFacetIndex, newFacetIndex
			, mapInfo->mesh->edges[getEdgeWithoutPoint(mapInfo->mesh, newFacetIndex, newPointIndex)].facetIndex);

		checkEdge(facetTree, facets, mapInfo, worstFacetIndex, 
			getEdgeWithoutPoint(mapInfo->mesh, worstFacetIndex, newPointIndex), newPointIndex);
		checkEdge(facetTree, facets, mapInfo, newFacetIndex, 
			getEdgeWithoutPoint(mapInfo->mesh, newFacetIndex, newPointIndex), newPointIndex);

		//add to facetTree
		gcSetAdaptiveFacetError(facetTree, mapInfo, facets, newFacetIndex);
		gcSetAdaptiveFacetError(facetTree, mapInfo, facets, worstFacetIndex);

		*currentFacetIndex += 1;
		*currentPointIndex += 1;
		*currentEdgeIndex += 2;
	}
	else
	{
		long oldFacetIndex = getOppositeFacet(worstFacetIndex, mapInfo->mesh->edges[worstEdgeIndex].facetIndex);
		gcFacet *newFacet2 = &mapInfo->mesh->facets[newFacetIndex + 1];
		gcFacet *oldFacet =  &mapInfo->mesh->facets[oldFacetIndex];

		//from new point to end of original edge
		gcFillEdge(mapInfo, newEdgeIndex + 1, newPointIndex, mapInfo->mesh->edges[worstEdgeIndex].pointIndex[1]
			, newFacetIndex, newFacetIndex + 1);

		//create the new edges
		//first from new point to opposite point
		gcFillEdge(mapInfo, newEdgeIndex + 2
			, newPointIndex, getFacetPointNotOnEdge(oldFacet, mapInfo->mesh->edges[worstEdgeIndex].pointIndex)
			, newFacetIndex + 1, oldFacetIndex);

		//create the new facet
		*newFacet1 = *worstFacet;
		*newFacet2 = *oldFacet;

		replaceVertex(newFacet1, mapInfo->mesh->edges[worstEdgeIndex].pointIndex[0], newPointIndex);

#ifdef GC_EXTENDED_ERROR_CHECKING
		validateFacet (mapInfo, newFacetIndex);
#endif

		replaceVertex(newFacet2, mapInfo->mesh->edges[worstEdgeIndex].pointIndex[0], newPointIndex);

#ifdef GC_EXTENDED_ERROR_CHECKING
		validateFacet (mapInfo, newFacetIndex + 1);
#endif

		//update the existing facet
		replaceVertex(worstFacet, mapInfo->mesh->edges[worstEdgeIndex].pointIndex[1], newPointIndex);

#ifdef GC_EXTENDED_ERROR_CHECKING
		validateFacet (mapInfo, worstFacetIndex);
#endif

		replaceVertex(oldFacet, mapInfo->mesh->edges[worstEdgeIndex].pointIndex[1], newPointIndex);

#ifdef GC_EXTENDED_ERROR_CHECKING
		validateFacet (mapInfo, oldFacetIndex);
#endif

		//update existing edge's end point
		mapInfo->mesh->edges[worstEdgeIndex].pointIndex[1] = newPointIndex;

		//reorder the edges on our facets
		{
			unsigned long edgesToSearchWorst[5] = {mapInfo->mesh->facetEdges[worstFacetIndex].edgeIndex[0]
				, mapInfo->mesh->facetEdges[worstFacetIndex].edgeIndex[1]
				, mapInfo->mesh->facetEdges[worstFacetIndex].edgeIndex[2] 
				, newEdgeIndex + 1, newEdgeIndex};
			fixFacetEdges(mapInfo->mesh, worstFacetIndex, mapInfo->mesh->facetEdges[worstFacetIndex].edgeIndex, edgesToSearchWorst, 5);

#ifdef GC_EXTENDED_ERROR_CHECKING
			validateFacetEdge (mapInfo, worstFacetIndex);
#endif

			fixFacetEdges(mapInfo->mesh, newFacetIndex, mapInfo->mesh->facetEdges[newFacetIndex].edgeIndex, edgesToSearchWorst, 5);

#ifdef GC_EXTENDED_ERROR_CHECKING
			validateFacetEdge (mapInfo, newFacetIndex);
#endif
		}
		{
			unsigned long edgesToSearchOld[5] = {mapInfo->mesh->facetEdges[oldFacetIndex].edgeIndex[0]
				, mapInfo->mesh->facetEdges[oldFacetIndex].edgeIndex[1]
				, mapInfo->mesh->facetEdges[oldFacetIndex].edgeIndex[2]  
				, newEdgeIndex + 1, newEdgeIndex + 2};

			fixFacetEdges(mapInfo->mesh, oldFacetIndex, mapInfo->mesh->facetEdges[oldFacetIndex].edgeIndex, edgesToSearchOld, 5);

#ifdef GC_EXTENDED_ERROR_CHECKING
			validateFacetEdge (mapInfo, oldFacetIndex);
#endif

			fixFacetEdges(mapInfo->mesh, newFacetIndex + 1, mapInfo->mesh->facetEdges[newFacetIndex + 1].edgeIndex, edgesToSearchOld, 5);

#ifdef GC_EXTENDED_ERROR_CHECKING
			validateFacetEdge (mapInfo, newFacetIndex + 1);
#endif
		}
		//update the face indexes for edges on the new facets
		replaceFaceIndex(oldFacetIndex, newFacetIndex + 1
			,  mapInfo->mesh->edges[getEdgeWithoutPoint(mapInfo->mesh, newFacetIndex + 1, newPointIndex)].facetIndex);
		replaceFaceIndex(worstFacetIndex, newFacetIndex
			,  mapInfo->mesh->edges[getEdgeWithoutPoint(mapInfo->mesh, newFacetIndex, newPointIndex)].facetIndex);

		checkEdge(facetTree, facets, mapInfo, worstFacetIndex, 
			getEdgeWithoutPoint(mapInfo->mesh, worstFacetIndex, newPointIndex), newPointIndex);
		checkEdge(facetTree, facets, mapInfo, oldFacetIndex, 
			getEdgeWithoutPoint(mapInfo->mesh, oldFacetIndex, newPointIndex), newPointIndex);
		checkEdge(facetTree, facets, mapInfo, newFacetIndex, 
			getEdgeWithoutPoint(mapInfo->mesh, newFacetIndex, newPointIndex), newPointIndex);
		checkEdge(facetTree, facets, mapInfo, newFacetIndex + 1, 
			getEdgeWithoutPoint(mapInfo->mesh, newFacetIndex + 1, newPointIndex), newPointIndex);

		DCGBinaryTreeRemoveElement(facetTree, &facets[oldFacetIndex]);

		//add to facetTree
		gcSetAdaptiveFacetError(facetTree, mapInfo, facets, worstFacetIndex);
		gcSetAdaptiveFacetError(facetTree, mapInfo, facets, newFacetIndex);
		gcSetAdaptiveFacetError(facetTree, mapInfo, facets, oldFacetIndex);
		gcSetAdaptiveFacetError(facetTree, mapInfo, facets, newFacetIndex + 1);

		*currentFacetIndex += 2;
		*currentPointIndex += 1;
		*currentEdgeIndex += 3;
	}
}

int gcResample(const gcMapInfo *source, const double scaleFactor, const unsigned long filtering, gcMapInfo *dest){
	gcPrivateData* destPrivateData = dest->privateData;

	float* EPercent = NULL;
	float* NPercent = NULL;
	int retval = GC_ERROR;
	unsigned long east, north;

	dest->samples[GC_EAST] = source->samples[GC_EAST] * scaleFactor;
	dest->samples[GC_NORTH] = source->samples[GC_NORTH] * scaleFactor;
	dest->bufferSize = dest->samples[GC_EAST] * dest->samples[GC_NORTH];

	destPrivateData->elevation = calloc( dest->bufferSize, sizeof( float ));
	if ( !destPrivateData->elevation ) goto Finish;

	EPercent = calloc( dest->samples[GC_EAST], sizeof( float ));
	if ( !EPercent ) goto Finish;

	NPercent = calloc( dest->samples[GC_NORTH], sizeof( float ));
	if ( !NPercent ) goto Finish;

	for (east = 0; east < dest->samples[GC_EAST]; east++){
		EPercent[east] = (float)east / (float)(dest->samples[GC_EAST] - 1);
	}
	for (north = 0; north < dest->samples[GC_NORTH]; north++){
		NPercent[north] = (float)north / (float)(dest->samples[GC_NORTH] - 1);
	}

	for (east = 0; east < dest->samples[GC_EAST]; east++){
		for (north = 0; north < dest->samples[GC_NORTH]; north++){
			unsigned long elevationIndex = gcGetElevationIndex(dest, east, north);
			destPrivateData->elevation[elevationIndex] = gcGetElevationBySample(source, filtering, EPercent[east], NPercent[north]);
		}
	}
	retval = GC_OK;
Finish:
	if (EPercent) free (EPercent);
	if (NPercent) free (NPercent);
	return retval;
}

int gcCreateGridMesh(gcMapInfo *mapInfo, const unsigned long targetPolyCount, const unsigned long meshType) {
	gcPrivateData* privateData = mapInfo->privateData;
	int retval = GC_ERROR;

	unsigned long east, north;

	unsigned long filtering = GC_POINT_FILTERING;
	unsigned long fullPolyCount = (mapInfo -> samples[GC_EAST] - 1) * (mapInfo -> samples[GC_NORTH] - 1) * 2;
	unsigned long facetIndex = 0;
	unsigned long progressChunkSize = 1;

	double scaleFactor = sqrt((double)targetPolyCount / (double)fullPolyCount);

	gcMapInfo *targetMapInfo;

	gcMapInfo scaledMapInfo;
	gcInitialize(&scaledMapInfo);

	if (meshType == GC_MESH_GRID_SMART || meshType == GC_MESH_GRID_BILINEAR) {
		filtering = GC_BILINEAR_FILTERING;
	}

	//if full, then just build our mesh off the inbound map
	if (targetPolyCount == fullPolyCount) {
		targetMapInfo = mapInfo;
	}
	else  {
		//otherwise resample
		if (gcResample(mapInfo, scaleFactor, filtering, &scaledMapInfo) == GC_ERROR) {
			goto Finish;
		}
		targetMapInfo = &scaledMapInfo;
	}

	//fill the points
	mapInfo->mesh->pointCount = targetMapInfo->bufferSize;
	mapInfo->mesh->points = calloc( mapInfo->mesh->pointCount, sizeof( gcPoint ));
	if ( !mapInfo->mesh->points ) goto Finish;

	for (east = 0; east < targetMapInfo->samples[GC_EAST]; east++){
		for (north = 0; north < targetMapInfo->samples[GC_NORTH]; north++){
			unsigned int elevationIndex = gcGetElevationIndex(targetMapInfo, east, north);
			mapInfo->mesh->points[elevationIndex].east = east;
			mapInfo->mesh->points[elevationIndex].north = north;
			mapInfo->mesh->points[elevationIndex].elevation = gcGetElevationByIndex(targetMapInfo, elevationIndex);
		}
	}

	mapInfo->mesh->samples[GC_EAST] = targetMapInfo->samples[GC_EAST];
	mapInfo->mesh->samples[GC_NORTH] = targetMapInfo->samples[GC_NORTH];

	mapInfo->mesh->EPercent = calloc( mapInfo->mesh->samples[GC_EAST], sizeof( float ));
	if ( !mapInfo->mesh->EPercent ) goto Finish;

	mapInfo->mesh->NPercent = calloc( mapInfo->mesh->samples[GC_NORTH], sizeof( float ));
	if ( !mapInfo->mesh->NPercent ) goto Finish;

	for (east = 0; east < mapInfo->mesh->samples[GC_EAST]; east++){
		mapInfo->mesh->EPercent[east] = (float)east / (float)(mapInfo->mesh->samples[GC_EAST] - 1);
	}
	for (north = 0; north < mapInfo->mesh->samples[GC_NORTH]; north++){
		mapInfo->mesh->NPercent[north] = (float)north / (float)(mapInfo->mesh->samples[GC_NORTH] - 1);
	}

	//fill the facets
	mapInfo->mesh->facetCount = (targetMapInfo->samples[GC_EAST] - 1) * (targetMapInfo->samples[GC_NORTH] - 1) * 2;
	mapInfo->mesh->facets = calloc(mapInfo->mesh->facetCount, sizeof( gcFacet));
	if ( !mapInfo->mesh->facets ) goto Finish;

	progressChunkSize = (unsigned long)(ceil((float)targetMapInfo->samples[GC_EAST] / 100.0f));

	for (east = 0; east < targetMapInfo->samples[GC_EAST] - 1; east++){
		if (east % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)east / (float)targetMapInfo->samples[GC_EAST];
			privateData->progressCallback(privateData->callbackData, farAlong);
		}
		for (north = 0; north < targetMapInfo->samples[GC_NORTH] - 1; north++){
			unsigned int corners[4];
			corners[GC_SOUTH_EAST] = gcGetElevationIndex(targetMapInfo, east, north);
			corners[GC_NORTH_EAST] = gcGetElevationIndex(targetMapInfo, east, north + 1);
			corners[GC_SOUTH_WEST] = gcGetElevationIndex(targetMapInfo, east + 1, north);
			corners[GC_NORTH_WEST] = gcGetElevationIndex(targetMapInfo, east + 1, north + 1);

			if (meshType == GC_MESH_GRID_SMART) {

				float elev1, elev2, middleElevation;

				elev1 = (gcGetElevationByIndex(targetMapInfo, corners[GC_NORTH_WEST]) + gcGetElevationByIndex(targetMapInfo, corners[GC_SOUTH_EAST])) * 0.5f;
				elev2 = (gcGetElevationByIndex(targetMapInfo, corners[GC_SOUTH_WEST]) + gcGetElevationByIndex(targetMapInfo, corners[GC_NORTH_EAST])) * 0.5f;

				middleElevation = gcGetElevationBySample(mapInfo, filtering, (mapInfo->mesh->EPercent[east] + mapInfo->mesh->EPercent[east + 1]) * 0.5f
					, (mapInfo->mesh->NPercent[north] + mapInfo->mesh->NPercent[north + 1]) * 0.5f);

				//figure out which way to split
				//calculate the elevation in the middle and see which way give us
				//the edge that passes closes to the elevation
				if (fabs(elev1 - middleElevation) < fabs(elev2 - middleElevation)) {
					mapInfo->mesh->facets[facetIndex].pointIndex[0] = corners[GC_NORTH_EAST];
					mapInfo->mesh->facets[facetIndex].pointIndex[1] = corners[GC_NORTH_WEST];
					mapInfo->mesh->facets[facetIndex].pointIndex[2] = corners[GC_SOUTH_EAST];
					facetIndex++;

					mapInfo->mesh->facets[facetIndex].pointIndex[0] = corners[GC_SOUTH_EAST];
					mapInfo->mesh->facets[facetIndex].pointIndex[1] = corners[GC_NORTH_WEST];
					mapInfo->mesh->facets[facetIndex].pointIndex[2] = corners[GC_SOUTH_WEST];
					facetIndex++;
				}
				else {
					mapInfo->mesh->facets[facetIndex].pointIndex[0] = corners[GC_NORTH_EAST];
					mapInfo->mesh->facets[facetIndex].pointIndex[1] = corners[GC_SOUTH_WEST];
					mapInfo->mesh->facets[facetIndex].pointIndex[2] = corners[GC_SOUTH_EAST];
					facetIndex++;

					mapInfo->mesh->facets[facetIndex].pointIndex[0] = corners[GC_NORTH_EAST];
					mapInfo->mesh->facets[facetIndex].pointIndex[1] = corners[GC_NORTH_WEST];
					mapInfo->mesh->facets[facetIndex].pointIndex[2] = corners[GC_SOUTH_WEST];
					facetIndex++;
				}				   
			}
			//blindly split
			else {
				mapInfo->mesh->facets[facetIndex].pointIndex[0] = corners[GC_NORTH_EAST] ;
				mapInfo->mesh->facets[facetIndex].pointIndex[1] = corners[GC_NORTH_WEST] ;
				mapInfo->mesh->facets[facetIndex].pointIndex[2] = corners[GC_SOUTH_EAST] ;
				facetIndex++;

				mapInfo->mesh->facets[facetIndex].pointIndex[0] = corners[GC_SOUTH_EAST];
				mapInfo->mesh->facets[facetIndex].pointIndex[1] = corners[GC_NORTH_WEST];
				mapInfo->mesh->facets[facetIndex].pointIndex[2] = corners[GC_SOUTH_WEST];
				facetIndex++;
			}

		}
	}

	retval = GC_OK;
Finish:
   gcCleanUp(&scaledMapInfo);

	return retval;
}




int gcCreateAdaptiveMesh(gcMapInfo *mapInfo, const unsigned long targetPolyCount, float adaptiveError) {
	gcPrivateData* privateData = mapInfo->privateData;
	unsigned long currentEdgeIndex = 0, currentFacetIndex = 0, currentPointIndex = 4;
	unsigned long split, east, north;
	int retval = GC_ERROR;
	unsigned long splits = (targetPolyCount - 2) * 0.5f;
	unsigned long vertexCount = 4 + splits;
	unsigned long edgeCount = 5 + 3 * splits;
	unsigned long facetCount = 2 + 2 * splits;
	unsigned long progressChunkSize = 1;

	dcgBinaryTree facetTree;
	gcAdaptiveFacet *facets;

	DCGBinaryTreeInit(&facetTree, compareAdaptiveFacetError);
	
	facets = calloc(facetCount, sizeof( gcAdaptiveFacet));
	if ( !facets ) goto Finish;

	mapInfo->mesh->points = calloc(vertexCount, sizeof( gcPoint ));
	if ( !mapInfo->mesh->points ) goto Finish;
	mapInfo->mesh->facets = calloc(facetCount, sizeof( gcFacet));
	if ( !mapInfo->mesh->facets ) goto Finish;
	mapInfo->mesh->facetEdges = calloc(facetCount, sizeof( gcFacetEdge));
	if ( !mapInfo->mesh->facetEdges ) goto Finish;
	mapInfo->mesh->edges = calloc(edgeCount, sizeof( gcEdge));
	if ( !mapInfo->mesh->edges ) goto Finish;

	mapInfo->mesh->samples[GC_EAST] = mapInfo->samples[GC_EAST];
	mapInfo->mesh->samples[GC_NORTH] = mapInfo->samples[GC_NORTH];


	mapInfo->mesh->EPercent = calloc( mapInfo->mesh->samples[GC_EAST], sizeof( float ));
	if ( !mapInfo->mesh->EPercent ) goto Finish;

	mapInfo->mesh->NPercent = calloc( mapInfo->mesh->samples[GC_NORTH], sizeof( float ));
	if ( !mapInfo->mesh->NPercent ) goto Finish;

	for (east = 0; east < mapInfo->mesh->samples[GC_EAST]; east++){
		mapInfo->mesh->EPercent[east] = (float)east / (float)(mapInfo->mesh->samples[GC_EAST] - 1);
	}
	for (north = 0; north < mapInfo->mesh->samples[GC_NORTH]; north++){
		mapInfo->mesh->NPercent[north] = (float)north / (float)(mapInfo->mesh->samples[GC_NORTH] - 1);
	}

	//build starter grid
	gcFillPoint(mapInfo, GC_SOUTH_EAST, mapInfo->samples[GC_EAST] - 1, 0);
	gcFillPoint(mapInfo, GC_NORTH_EAST, mapInfo->samples[GC_EAST] - 1, mapInfo->samples[GC_NORTH] - 1);
	gcFillPoint(mapInfo, GC_SOUTH_WEST, 0, 0);
	gcFillPoint(mapInfo, GC_NORTH_WEST, 0, mapInfo->samples[GC_NORTH] - 1);

	gcFillFacet(mapInfo, currentFacetIndex++, GC_NORTH_EAST, GC_SOUTH_EAST, GC_NORTH_WEST);
	gcFillFacet(mapInfo, currentFacetIndex++, GC_SOUTH_EAST, GC_SOUTH_WEST, GC_NORTH_WEST);

	gcFillEdge(mapInfo, currentEdgeIndex++, GC_NORTH_WEST, GC_NORTH_EAST, 0, -1);//0
	gcFillEdge(mapInfo, currentEdgeIndex++, GC_NORTH_EAST, GC_SOUTH_EAST, 0, -1);//1
	gcFillEdge(mapInfo, currentEdgeIndex++, GC_SOUTH_WEST, GC_NORTH_WEST, 1, -1);//2
	gcFillEdge(mapInfo, currentEdgeIndex++, GC_SOUTH_EAST, GC_SOUTH_WEST, 1, -1);//3
	gcFillEdge(mapInfo, currentEdgeIndex++, GC_SOUTH_EAST, GC_NORTH_WEST, 0, 1);//4

	gcFillFacetEdge(mapInfo, 0, 1, 4, 0);
	gcFillFacetEdge(mapInfo, 1, 3, 2, 4);

	gcSetAdaptiveFacetError(&facetTree, mapInfo, facets, 0);
	gcSetAdaptiveFacetError(&facetTree, mapInfo, facets, 1);

	mapInfo->mesh->pointCount = 4;
	mapInfo->mesh->facetCount = 2;

	progressChunkSize = (unsigned long)(ceil((float)splits / 100.0f));
	for (split = 0; split < splits; split++)
	{
		//find facet with greatest error
		gcAdaptiveFacet* worstFacet = DCGBinaryTreeGetTop(&facetTree);

		if (split % progressChunkSize == 0 && privateData->progressCallback != NULL) {
			float farAlong =  (float)split / (float)splits;
			privateData->progressCallback(privateData->callbackData, farAlong);
		}
		
		if (worstFacet == NULL)
		{
			split = splits;
		}
		else
		{
			unsigned long worstFacetIndex = worstFacet->facetIndex;

			DCGBinaryTreeRemoveTop(&facetTree);

			if (worstFacet->error > adaptiveError)
			{
				if (worstFacet->worstEdgeIndex >= 0)
				{
					splitFacetOnEdge(&facetTree, facets, mapInfo, worstFacetIndex, &currentFacetIndex, &currentPointIndex, &currentEdgeIndex);
				}
				else
				{
					splitFacet(&facetTree, facets, mapInfo, worstFacetIndex, &currentFacetIndex, &currentPointIndex, &currentEdgeIndex);
				}
			}
			else
			{
				split = splits;
			}
		}
	}

	mapInfo->mesh->pointCount = currentPointIndex;
	mapInfo->mesh->facetCount = currentFacetIndex;

	retval = GC_OK;
Finish:
	DCGBinaryTreeCleanUp(&facetTree);
	if (facets)
		free(facets);

	return retval;
}
