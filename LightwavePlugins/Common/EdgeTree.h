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
#include "Boolean.h"
#include "BoundingBox.h"
#include "MathHelper.h"
#include <math.h>

//how many edges do we have before trying to search
//via the octtree
#define OCT_TREE_LIMIT 100
#define MAX_THREADS 64

typedef struct Edge_t {
	unsigned int vertices[2];
} Edge, *Edge_p;

typedef struct EdgeNode_t {
	unsigned int edgeCount;
	unsigned int* edges;
	LWFVector center;
	float radius;
} EdgeNode, *EdgeNode_p;

static void InitEdgeNode(EdgeNode_p edgeNode)
{
	edgeNode->edgeCount = 0;
	edgeNode->edges = NULL;

}

typedef struct EdgeNodeTree_t *EdgeNodeTree_p;

typedef struct EdgeNodeTree_t {
	EdgeNode_p node;
	BoundingBox bbox;
	LWFVector center;
	EdgeNodeTree_p child[8];
} EdgeNodeTree;

static void InitEdgeNodeTree(EdgeNodeTree_p edgeNodeTree)
{

	edgeNodeTree->node = NULL;
	edgeNodeTree->child[0] = NULL;
	edgeNodeTree->child[1] = NULL;
	edgeNodeTree->child[2] = NULL;
	edgeNodeTree->child[3] = NULL;
	edgeNodeTree->child[4] = NULL;
	edgeNodeTree->child[5] = NULL;
	edgeNodeTree->child[6] = NULL;
	edgeNodeTree->child[7] = NULL;

}

static void CleanupEdgeNodeTree(EdgeNodeTree_p edgeNodeTree)
{
	unsigned int childIndex;

	for (childIndex = 0; childIndex < 8; childIndex++)
	{
		if (edgeNodeTree->child[childIndex] != NULL)
		{
			CleanupEdgeNodeTree(edgeNodeTree->child[childIndex]);
			free(edgeNodeTree->child[childIndex]);
		}
	}
	InitEdgeNodeTree(edgeNodeTree);
}


typedef struct EdgeList_t *EdgeList_p;

typedef struct EdgeList_t {

	LWItemID		objID;

	unsigned int edgeCount;
	unsigned int pointCount;
	unsigned int nodeCount;

	bool isFilled;
	bool isDirty;

	LWFVector* p2minusp1;
	float* oneOverLineMagnitude;
	LWFVector* points;
	Edge_p edges;
	EdgeNode_p *nodeList;
	double* nodeDistance[MAX_THREADS];
	LWDVector lastPos[MAX_THREADS];
	double lastValue[MAX_THREADS];

	EdgeList_p next;
} EdgeList;



static void InitEdgeList(EdgeList_p edgeList)
{
	unsigned int threadIndex;

	for (threadIndex = 0;threadIndex < MAX_THREADS; threadIndex++)
	{
		edgeList->nodeDistance[threadIndex] = NULL;
		edgeList->lastPos[threadIndex][0] = edgeList->lastPos[threadIndex][1] = edgeList->lastPos[threadIndex][2] = DPOSINF;
		edgeList->lastValue[threadIndex] = DPOSINF;
	}

	edgeList->edgeCount = 0;
	edgeList->pointCount = 0;
	edgeList->nodeCount = 0;

	edgeList->objID = 0;

	edgeList->isFilled = false;
	edgeList->isDirty = false;

	edgeList->p2minusp1 = NULL;
	edgeList->oneOverLineMagnitude = NULL;
	edgeList->points = NULL;
	edgeList->edges = NULL;
	edgeList->nodeList = NULL;
	edgeList->next = NULL;
}

static void CleanupEdgeList(EdgeList_p edgeList)
{
	unsigned int threadIndex;

	if (!edgeList)
		return;

	for (threadIndex = 0;threadIndex < MAX_THREADS; threadIndex++)
	{
		if (edgeList->nodeDistance[threadIndex] != NULL)
		{
			free(edgeList->nodeDistance[threadIndex]);
		}
	}

	if (edgeList->p2minusp1) 
	{
		free(edgeList->p2minusp1);
	}
	if (edgeList->oneOverLineMagnitude) 
	{
		free(edgeList->oneOverLineMagnitude);
	}
	if (edgeList->points) 
	{
		free(edgeList->points);
	}
	if (edgeList->edges) 
	{
		free(edgeList->edges);
	}
	if (edgeList->nodeList) 
	{
		free(edgeList->nodeList);
	}

	InitEdgeList(edgeList);
}

static void getBoundingBoxFromEdgeList(const EdgeList_p edgeList, BoundingBox_p boundingBox)
{
	unsigned int pointIndex;
	for (pointIndex = 0; pointIndex < edgeList->pointCount; pointIndex++)
	{
		if (pointIndex == 0) 
		{
			addFirstPointToBoundingBox(boundingBox, &edgeList->points[pointIndex]);
		}
		else
		{
			addPointToBoundingBox(boundingBox, &edgeList->points[pointIndex]);
		}
	}
}

static void calculateCenterAndRadius(EdgeList_p edgeList, unsigned int nodeIndex)
{
	unsigned int nodeEdgeIndex;
	BoundingBox boundingBox;
	EdgeNode_p edgeNode = edgeList->nodeList[nodeIndex];


	for (nodeEdgeIndex = 0; nodeEdgeIndex < edgeNode->edgeCount; nodeEdgeIndex++)
	{
		if (nodeEdgeIndex == 0) 
		{
			addFirstPointToBoundingBox(&boundingBox, 
				&edgeList->points[edgeList->edges[edgeNode->edges[nodeEdgeIndex]].vertices[0]]);
		}
		else
		{
			addPointToBoundingBox(&boundingBox, 
				&edgeList->points[edgeList->edges[edgeNode->edges[nodeEdgeIndex]].vertices[0]]);
		}
		addPointToBoundingBox(&boundingBox, 
			&edgeList->points[edgeList->edges[edgeNode->edges[nodeEdgeIndex]].vertices[1]]);
	}
	getBoundingBoxCenter(&boundingBox, &edgeNode->center);

	edgeNode->radius = (float)(sqrt(vsqr(edgeNode->center, boundingBox.min)));
}

static bool countNodeTree(EdgeNodeTree_p nodeTree, EdgeList_p edgeList, unsigned int edgeIndex)
{
	LWFVector *pt1 = NULL, *pt2 = NULL;
	unsigned int axis, location = 0;
	unsigned int bitfield[3] = {1, 2, 4};

	pt1 = &edgeList->points[edgeList->edges[edgeIndex].vertices[0]];
	pt2 = &edgeList->points[edgeList->edges[edgeIndex].vertices[1]];

	for (axis = 0; axis < 3; axis++)
	{
		if (((*pt1)[axis] < nodeTree->center[axis])
			&&((*pt2)[axis] < nodeTree->center[axis])) 
		{
			//low do nothing
		}
		else 
		if (((*pt1)[axis] >= nodeTree->center[axis])
			&&((*pt2)[axis] >= nodeTree->center[axis]))
		{
			//high add the bit field value 
			location += bitfield[axis];
		}
		else 
		{
			//mixed add it to ours
			if (!nodeTree->node)
			{
				nodeTree->node = calloc(1, sizeof(EdgeNode));
				if (!nodeTree->node)
				{
					return false;
				}
				InitEdgeNode(nodeTree->node);
			}
				
			nodeTree->node->edgeCount++;
			return true;
		}
	}
	if (!nodeTree->child[location])
	{
		EdgeNodeTree_p childTree = NULL;
		nodeTree->child[location] = calloc(1, sizeof(EdgeNodeTree));
		if (!nodeTree->child[location])
		{
			return false;
		}
		childTree = nodeTree->child[location];

		InitEdgeNodeTree(childTree);

		//set the new child's bounding box
		for (axis = 0; axis < 3; axis++)
		{
			if ((location & bitfield[axis]) == bitfield[axis])
			{
				//high block
				childTree->bbox.min[axis] = nodeTree->center[axis];
				childTree->bbox.max[axis] = nodeTree->bbox.max[axis];
			}
			else
			{
				//low block
				childTree->bbox.min[axis] = nodeTree->bbox.min[axis];
				childTree->bbox.max[axis] = nodeTree->center[axis];
			}
		}
		getBoundingBoxCenter(&childTree->bbox, &childTree->center); 
	}

	return countNodeTree(nodeTree->child[location], edgeList, edgeIndex);
}

static bool allocateNodeTree(EdgeNodeTree_p nodeTree, unsigned int *nodeCount)
{
	unsigned int childIndex;

	if (nodeTree->node)
	{
		nodeTree->node->edges = calloc(nodeTree->node->edgeCount, sizeof(unsigned int));
		if (!nodeTree->node->edges)
		{
			return false;
		}
		(*nodeCount)++;
		//reset the edge count so we can use
		//it as a current position
		nodeTree->node->edgeCount = 0;
	}
	for (childIndex = 0; childIndex < 8; childIndex++)
	{
		if (nodeTree->child[childIndex])
		{
			if (!allocateNodeTree(nodeTree->child[childIndex], nodeCount))
			{
				return false;
			}
		}
	}
	return true;
}

static void addToNodeTree(EdgeNodeTree_p nodeTree, EdgeList_p edgeList, unsigned int edgeIndex)
{
	LWFVector *pt1 = NULL, *pt2 = NULL;
	unsigned int axis, location = 0;
	unsigned int bitfield[3] = {1, 2, 4};

	pt1 = &edgeList->points[edgeList->edges[edgeIndex].vertices[0]];
	pt2 = &edgeList->points[edgeList->edges[edgeIndex].vertices[1]];

	for (axis = 0; axis < 3; axis++)
	{
		if (((*pt1)[axis] < nodeTree->center[axis])
			&&((*pt2)[axis] < nodeTree->center[axis])) 
		{
			//low do nothing
		}
		else 
		if (((*pt1)[axis] >= nodeTree->center[axis])
			&&((*pt2)[axis] >= nodeTree->center[axis]))
		{
			//high add the bit field value 
			location += bitfield[axis];
		}
		else 
		{
			nodeTree->node->edges[nodeTree->node->edgeCount] = edgeIndex;			
			nodeTree->node->edgeCount++;
			return;
		}
	}

	addToNodeTree(nodeTree->child[location], edgeList, edgeIndex);
}

static void fillFlatList(EdgeNodeTree_p nodeTree, EdgeNode_p *nodeList, unsigned int *nodeIndex)
{
	unsigned int childIndex;

	if (nodeTree->node) 
	{
		nodeList[*nodeIndex] = nodeTree->node;
		(*nodeIndex)++;
	}
	for (childIndex = 0; childIndex < 8; childIndex++)
	{
		if (nodeTree->child[childIndex]) 
		{
			fillFlatList(nodeTree->child[childIndex], nodeList, nodeIndex);
		}
	}
}


static double getDistanceToEdgeFlatList(EdgeList_p edgeList, LWDVector pos) {
	unsigned int edgeIndex;
	double minDistance = DPOSINF;

	for (edgeIndex = 0; edgeIndex < edgeList->edgeCount; edgeIndex++) {
		unsigned int p1Index = edgeList->edges[edgeIndex].vertices[0]
			, p2Index = edgeList->edges[edgeIndex].vertices[1];
		double u;
			
		u = (((pos[0] - edgeList->points[p1Index][0]) * edgeList->p2minusp1[edgeIndex][0]) 
			+ ((pos[1] - edgeList->points[p1Index][1]) * edgeList->p2minusp1[edgeIndex][1]) 
			+ ((pos[2] - edgeList->points[p1Index][2]) * edgeList->p2minusp1[edgeIndex][2]))
			* edgeList->oneOverLineMagnitude[edgeIndex];
		if (u < 0)
		{
			double distance = sqrt(vdfsqr(pos, edgeList->points[p1Index]));
			if (distance < minDistance)
			{
				minDistance = distance;
			}
		}
		else if (u > 1.0f)
		{
			double distance = sqrt(vdfsqr(pos, edgeList->points[p2Index]));
			if (distance < minDistance)
			{
				minDistance = distance;
			}
		}
		else if (( u >= 0)&&(u <= 1.0f))
		{
			LWDVector intersection;

			double distance; 

			intersection[0] = edgeList->points[p1Index][0] + u * (edgeList->p2minusp1[edgeIndex][0]);
			intersection[1] = edgeList->points[p1Index][1] + u * (edgeList->p2minusp1[edgeIndex][1]);
			intersection[2] = edgeList->points[p1Index][2] + u * (edgeList->p2minusp1[edgeIndex][2]);
			distance = sqrt(vddsqr(pos, intersection));

			if (distance < minDistance)
			{
				minDistance = distance;
			}

		}

	
	}

	return minDistance;
}

static double getDistanceToEdgeNode(EdgeList_p edgeList, LWDVector pos, unsigned int nodeIndex) {
	unsigned int nodeEdgeIndex;
	double minDistance = DPOSINF;

	for (nodeEdgeIndex = 0; nodeEdgeIndex < edgeList->nodeList[nodeIndex]->edgeCount; nodeEdgeIndex++) {
		unsigned int edgeIndex = edgeList->nodeList[nodeIndex]->edges[nodeEdgeIndex];
		unsigned int p1Index = edgeList->edges[edgeIndex].vertices[0]
			, p2Index = edgeList->edges[edgeIndex].vertices[1];
		double u;
			
		u = (((pos[0] - edgeList->points[p1Index][0]) * edgeList->p2minusp1[edgeIndex][0]) 
			+ ((pos[1] - edgeList->points[p1Index][1]) * edgeList->p2minusp1[edgeIndex][1]) 
			+ ((pos[2] - edgeList->points[p1Index][2]) * edgeList->p2minusp1[edgeIndex][2]))
			* edgeList->oneOverLineMagnitude[edgeIndex];
		if (u < 0)
		{
			double distance = sqrt(vdfsqr(pos, edgeList->points[p1Index]));
			if (distance < minDistance)
			{
				minDistance = distance;
			}
		}
		else if (u > 1.0f)
		{
			double distance = sqrt(vdfsqr(pos, edgeList->points[p2Index]));
			if (distance < minDistance)
			{
				minDistance = distance;
			}
		}
		else if (( u >= 0)&&(u <= 1.0f))
		{
			LWDVector intersection;

			double distance; 

			intersection[0] = edgeList->points[p1Index][0] + u * (edgeList->p2minusp1[edgeIndex][0]);
			intersection[1] = edgeList->points[p1Index][1] + u * (edgeList->p2minusp1[edgeIndex][1]);
			intersection[2] = edgeList->points[p1Index][2] + u * (edgeList->p2minusp1[edgeIndex][2]);
			distance = sqrt(vddsqr(pos, intersection));

			if (distance < minDistance)
			{
				minDistance = distance;
			}

		}

	}

	return minDistance;
}


static double getDistanceToEdgeOctTree(EdgeList_p edgeList, LWDVector pos, int threadIndex) {
	unsigned int nodeIndex, minNodeIndex;
	double* nodeDistance;
	double minDistance = DPOSINF, currentDistance, tempDistance;

	//figure out the distances to our nodes
	if (edgeList->nodeDistance[threadIndex] == NULL)
	{
		edgeList->nodeDistance[threadIndex] = calloc(edgeList->nodeCount, sizeof(double));
	}
	nodeDistance = edgeList->nodeDistance[threadIndex];
	for (nodeIndex = 0; nodeIndex < edgeList->nodeCount; nodeIndex++)
	{
		nodeDistance[nodeIndex] = sqrt(vdfsqr(pos, edgeList->nodeList[nodeIndex]->center));
		tempDistance = nodeDistance[nodeIndex] + edgeList->nodeList[nodeIndex]->radius;
		if (tempDistance < minDistance)
		{
			minDistance = tempDistance;
			minNodeIndex = nodeIndex;
		}
	}
	
	//check the node with the minimum distance first, it's likely to be the one
	currentDistance = getDistanceToEdgeNode(edgeList, pos, minNodeIndex);
	if (currentDistance < minDistance)
	{
		minDistance = currentDistance;
	}

	//spin through the nodes and find the ones we need to check based on distance
	for (nodeIndex = 0; nodeIndex < edgeList->nodeCount; nodeIndex++)
	{
		if (nodeIndex != minNodeIndex 
				&& nodeDistance[nodeIndex] - edgeList->nodeList[nodeIndex]->radius <= currentDistance)
		{
			currentDistance = getDistanceToEdgeNode(edgeList, pos, nodeIndex);
			if (currentDistance < minDistance)
			{
				minDistance = currentDistance;
			}
		}
	}

	return minDistance;

}

//public stuff
double getDistanceToEdge(EdgeList_p edgeList, LWDVector pos, int threadIndex) {
	if (edgeList->edgeCount > OCT_TREE_LIMIT)
	{
		return getDistanceToEdgeOctTree(edgeList, pos, threadIndex);
	}
	else
	{
		return getDistanceToEdgeFlatList(edgeList, pos);
	}
}

bool fillNodeTree(EdgeList_p edgeList) {
	unsigned int edgeIndex;
	unsigned int nodeIndex = 0;

	EdgeNodeTree nodeTree;

	InitEdgeNodeTree(&nodeTree);

	//prime the first node
	getBoundingBoxFromEdgeList(edgeList, &nodeTree.bbox);
	getBoundingBoxCenter(&nodeTree.bbox, &nodeTree.center);

	for (edgeIndex = 0; edgeIndex < edgeList->edgeCount; edgeIndex++)
	{
		if (!countNodeTree(&nodeTree, edgeList, edgeIndex))
		{
			CleanupEdgeNodeTree(&nodeTree);
			return false;
		}
	}
	if (!allocateNodeTree(&nodeTree, &edgeList->nodeCount)) 
	{
		CleanupEdgeNodeTree(&nodeTree);
		return false;
	}
	for (edgeIndex = 0; edgeIndex < edgeList->edgeCount; edgeIndex++)
	{
		addToNodeTree(&nodeTree, edgeList, edgeIndex);
	}
	edgeList->nodeList = calloc(edgeList->nodeCount, sizeof(EdgeNode_p));
	if (!edgeList->nodeList) 
	{
		CleanupEdgeNodeTree(&nodeTree);
		return false;
	}

	fillFlatList(&nodeTree, edgeList->nodeList, &nodeIndex);

	for (nodeIndex = 0; nodeIndex < edgeList->nodeCount; nodeIndex++)
	{
		calculateCenterAndRadius(edgeList, nodeIndex);
	}

	CleanupEdgeNodeTree(&nodeTree);

	return true;
}
