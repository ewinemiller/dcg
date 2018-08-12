/*  Anything Goos - plug-in for LightWave
Copyright (C) 2009 Eric Winemiller

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
typedef struct WorkingEdge_t {
	unsigned int vertices[2];

} WorkingEdge, *WorkingEdge_p;

typedef struct WorkingPolygon_t {
	LWPolID id;
	unsigned int vertexCount;
	unsigned int* vertices;
	LWFVector normal;

} WorkingPolygon, *WorkingPolygon_p;

typedef struct WorkingPoint_t {
	LWPntID id;
	LWFVector vertex;
} WorkingPoint, *WorkingPoint_p;

typedef struct WorkingRawEdge_t {
	unsigned int vertices[2];
	unsigned int polygon;

} WorkingRawEdge, *WorkingRawEdge_p;

typedef struct WorkingMesh_t {
	unsigned int pointCount;        
	unsigned int polygonCount;
	unsigned int rawEdgeCount;

	WorkingPoint_p points;
	WorkingPolygon_p polygons;

} WorkingMesh, *WorkingMesh_p;

static void InitWorkingMesh(WorkingMesh_p workingMesh)
{
	workingMesh->points = NULL;
	workingMesh->polygons = NULL;
	workingMesh->pointCount = 0;
	workingMesh->polygonCount = 0;
}

static void FreePointsWorkingMesh(WorkingMesh_p workingMesh)
{
	if (workingMesh->points)
	{
		free(workingMesh->points);
		workingMesh->points = NULL;
	}
}

static void FreePolygonsWorkingMesh(WorkingMesh_p workingMesh)
{
	if (workingMesh->polygons)
	{
		unsigned int polygonIndex;

		for (polygonIndex = 0; polygonIndex < workingMesh->polygonCount; polygonIndex++) 
		{
			free(workingMesh->polygons[polygonIndex].vertices);
		}

		free(workingMesh->polygons);
		workingMesh->polygons = NULL;		
	}
}

static void CleanupWorkingMesh(WorkingMesh_p workingMesh)
{
	FreePointsWorkingMesh(workingMesh);
	FreePolygonsWorkingMesh(workingMesh);

	InitWorkingMesh(workingMesh);
}

int WorkingPointCompare(const void *elem1, const void *elem2)
{
	if (((WorkingPoint_p)elem1)->id < ((WorkingPoint_p)elem2)->id)
		return -1;
	else if (((WorkingPoint_p)elem1)->id > ((WorkingPoint_p)elem2)->id)
		return 1;
	else
	{
		return 0;
	}
}

int WorkingRawEdgeCompare(const void *elem1, const void *elem2)
{
	if (((WorkingRawEdge_p)elem1)->vertices[0] < ((WorkingRawEdge_p)elem2)->vertices[0])
		return -1;
	else if (((WorkingRawEdge_p)elem1)->vertices[0] > ((WorkingRawEdge_p)elem2)->vertices[0])
		return 1;
	else
	{
		if (((WorkingRawEdge_p)elem1)->vertices[1] < ((WorkingRawEdge_p)elem2)->vertices[1])
			return -1;
		else if (((WorkingRawEdge_p)elem1)->vertices[1] > ((WorkingRawEdge_p)elem2)->vertices[1])
			return 1;
		else
		{
			if (((WorkingRawEdge_p)elem1)->polygon < ((WorkingRawEdge_p)elem2)->polygon)
				return -1;
			else if (((WorkingRawEdge_p)elem1)->polygon > ((WorkingRawEdge_p)elem2)->polygon)
				return 1;
			else
			{
				return 0;
			}
		}
	}
}
