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
#include "cAGrPrimSphere.h"
#include "AnythingGroovesDLL.h"

inline real32 sqr(real32 pfIn){
	return pfIn * pfIn;
	}

inline real32 sqr(const TVector3 &first,const TVector3 &second){
	return sqr(first.x - second.x) 
		+ sqr(first.y - second.y) 
		+ sqr(first.z - second.z);
	}

inline int32 intcompare(real32 in) {
	return in * 10000 + .5;
	}

cAGrPrimSphere::cAGrPrimSphere(const AGrPrimData& pfData) {
	VLines = NULL;
	sinVertLines = NULL;
	cosVertLines = NULL;
	midULines = NULL;
	midVLines = NULL;
	midsinHorzLines = NULL;
	midcosHorzLines = NULL;
	midsinVertLines = NULL;
	midcosVertLines = NULL;
	points = NULL;
	displacement = NULL;
	fData = pfData;
	}

void cAGrPrimSphere::CleanUp () {
	if (sinVertLines != NULL) {
		MCfree(sinVertLines);
		sinVertLines = NULL;
		}
	if (cosVertLines != NULL) {
		MCfree(cosVertLines);
		cosVertLines = NULL;
		}
	if (VLines != NULL) {
		MCfree(VLines);
		VLines = NULL;
		}

	if (midsinHorzLines != NULL) {
		MCfree(midsinHorzLines);
		midsinHorzLines = NULL;
		}
	if (midcosHorzLines != NULL) {
		MCfree(midcosHorzLines);
		midcosHorzLines = NULL;
		}
	if (midsinVertLines != NULL) {
		MCfree(midsinVertLines);
		midsinVertLines = NULL;
		}
	if (midcosVertLines != NULL) {
		MCfree(midcosVertLines);
		midcosVertLines = NULL;
		}
	if (midULines != NULL) {
		MCfree(midULines);
		midULines = NULL;
		}
	if (midVLines != NULL) {
		MCfree(midVLines);
		midVLines = NULL;
		}

	if (points != NULL) {
		MCfree(points);
		points = NULL;
		}
	if (displacement != NULL) {
		MCfree(displacement);
		displacement = NULL;
		}

	shader = NULL;
	accu = NULL;
	}


cAGrPrimSphere::~cAGrPrimSphere() {
	CleanUp();
	}

void cAGrPrimSphere::DoBasicMesh(FacetMesh** outMesh) {
	DCGFacetMeshAccumulator acc;
	TMCCountedPtr<I3DShShader> pShader;
	fData.lSplitMethod = SPLIT_DUMB;
	fData.lU = 8;
	fData.lV = 6;
	fData.bEmptyZero = false;
	DoMesh(&acc, pShader);
	acc.MakeFacetMesh(outMesh);
	}


void cAGrPrimSphere::DoMesh(DCGFacetMeshAccumulator* pAccu, TMCCountedPtr<I3DShShader> pShader) {
	accu = pAccu;
	shader = pShader;

	//for doing the shading
	ShadingIn shadingIn;

	boolean fullarea = false;
	fUVSpaceID = 0;
	shadingIn.fUVSpaceID = fUVSpaceID;
	shadingIn.fCurrentCompletionMask = 0;
	real32 fValue = 0;
	uint32 lFacetCount = 0;

	int32 u, v;
	oneoverU = 1.0 / (real32)fData.lU;
	oneoverV = 1.0 / (real32)fData.lV;

	VLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
	sinVertLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
	cosVertLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
	if (fData.lSplitMethod == SPLIT_SMART) {
		midULines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
		midVLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
		midsinHorzLines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
		midcosHorzLines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
		midsinVertLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
		midcosVertLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
		}
	if (fData.bEmptyZero == true) {
		displacement = (real32*)MCcalloc((fData.lV + 1) * (fData.lU + 1),sizeof(real32));
		}
	points = (TVertex3DLite*)MCcalloc((fData.lV + 1) * (fData.lU + 1) + 1,sizeof(TVertex3DLite));

	scale.x = fData.sizeX * 0.5;
	scale.y = fData.sizeY * 0.5;
	scale.z = fData.sizeZ * 0.5;

	//calculate XY and UV slices
	for (v = 0; v <= fData.lV; v++) {
		VLines[v] = v * oneoverV;
		real32 VertLine = - PI * 0.5 + PI * VLines[v];
		sinVertLines[v] = sin(VertLine);
		cosVertLines[v] = cos(VertLine);
		if (fData.lSplitMethod == SPLIT_SMART) {
			midVLines[v] = (v + 0.5) * oneoverV;
			real32 midVertLine = - PI * 0.5 + PI * midVLines[v];
			midsinVertLines[v] = sin(midVertLine);
			midcosVertLines[v] = cos(midVertLine);
			}
		}

	//calculate the displacement
	for (u = 0; u <= fData.lU; u ++){
		real32 ULine = u * oneoverU;
		real32 HorzLine = 2 * PI * ULine;
		real32 sinHorzLine = sin(HorzLine);
		real32 cosHorzLine = cos(HorzLine);
		if (fData.lSplitMethod == SPLIT_SMART) {
			midULines[u] = (u + 0.5) * oneoverU;
			real32 midHorzLine = 2 * PI * midULines[u];
			midsinHorzLines[u] = sin(midHorzLine);
			midcosHorzLines[u] = cos(midHorzLine);
			}
		shadingIn.fUV.x = ULine;
		for (v = 0; v <= fData.lV; v++) {
			shadingIn.fNormalLoc.x = cosHorzLine * cosVertLines[v];
			shadingIn.fNormalLoc.y = sinHorzLine * cosVertLines[v];
			shadingIn.fNormalLoc.z = sinVertLines[v];
			shadingIn.fPointLoc.x = shadingIn.fNormalLoc.x * scale.x;
			shadingIn.fPointLoc.y = shadingIn.fNormalLoc.y * scale.y;
			shadingIn.fPointLoc.z = shadingIn.fNormalLoc.z * scale.z;
			shadingIn.fNormalLoc = shadingIn.fPointLoc;
			shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
			shadingIn.fUV.y = VLines[v];
			shadingIn.fPoint = shadingIn.fPointLoc;
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			//get the shading 
			if (shader!=NULL) {
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				}
			//no put it away
			points[u * (fData.lV + 1) + v].fVertex = shadingIn.fPointLoc;
			points[u * (fData.lV + 1) + v].fUV = shadingIn.fUV;
			points[u * (fData.lV + 1) + v].fVertex
				+= shadingIn.fNormalLoc * fValue;
			if (fData.bEmptyZero == true) {
				displacement[u * (fData.lV + 1) + v] = fValue;
				}
			}
		}
	//set up the expected # of facets
	lFacetCount = fData.lU * (fData.lV - 1) * 2;
	accu->PrepareAccumulation(lFacetCount);
	for (u = 0; u < fData.lU; u ++){
			TVertex3DLite bottomright, bottomleft, topleft, topright;
			//first do the bottom row facets
			int32 bottomrightindex = (u + 1) * (fData.lV + 1) + 0
				, bottomleftindex = u * (fData.lV + 1) + 0
				, topleftindex = u * (fData.lV + 1) + 0 + 1
				, toprightindex = (u + 1) * (fData.lV + 1) + 0 + 1;

			bottomright=points[bottomrightindex];
			bottomleft=points[bottomleftindex];
			topleft=points[topleftindex];
			topright=points[toprightindex];

			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomrightindex]) != 0)
				||(intcompare(displacement[topleftindex]) != 0)
				||(intcompare(displacement[toprightindex]) != 0)) {
				AddFacet(bottomright
					, topleft
					, topright);
				}

			//do all the middle stuff
			for (v = 1; v < fData.lV - 1; v++) {
				AddQuad(u, v);
				}

			//do the top row
			bottomrightindex = (u + 1) * (fData.lV + 1) + fData.lV - 1;
			bottomleftindex = u * (fData.lV + 1) + fData.lV - 1;
			topleftindex = u * (fData.lV + 1) + fData.lV - 1 + 1;
			toprightindex = (u + 1) * (fData.lV + 1) + fData.lV - 1 + 1;

			bottomright=points[bottomrightindex];
			bottomleft=points[bottomleftindex];
			topleft=points[topleftindex];
			topright=points[toprightindex];

			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomrightindex]) != 0)
				||(intcompare(displacement[bottomleftindex]) != 0)
				||(intcompare(displacement[toprightindex]) != 0)) {
				AddFacet(bottomright
					, bottomleft
					, topright);
				}


		}

	//clean up
	CleanUp();
	}


void cAGrPrimSphere::AddFacet(const TVertex3DLite& pt1,const TVertex3DLite& pt2,const TVertex3DLite& pt3) {
	TFacet3D newfacet;
	newfacet.fUVSpace = fUVSpaceID;
	newfacet.fReserved = 0;
	newfacet.fVertices[0].fVertex = pt1.fVertex;
	newfacet.fVertices[1].fVertex = pt3.fVertex;
	newfacet.fVertices[2].fVertex = pt2.fVertex;
	newfacet.fVertices[0].fUV = pt1.fUV;
	newfacet.fVertices[1].fUV = pt3.fUV;
	newfacet.fVertices[2].fUV = pt2.fUV;
	
	newfacet.fVertices[2].fNormal = 
		(newfacet.fVertices[1].fVertex - newfacet.fVertices[0].fVertex)
		^ (newfacet.fVertices[2].fVertex - newfacet.fVertices[1].fVertex);
	newfacet.fVertices[2].fNormal.Normalize(newfacet.fVertices[2].fNormal);
	newfacet.fVertices[0].fNormal = newfacet.fVertices[1].fNormal = newfacet.fVertices[2].fNormal;

	accu->AccumulateFacet(&newfacet);

	}

void cAGrPrimSphere::AddQuad(const int32& u, const int32& v) {
	TVertex3DLite bottomright, bottomleft, topleft, topright;

	int32 bottomrightindex = (u + 1) * (fData.lV + 1) + v
		, bottomleftindex = u * (fData.lV + 1) + v
		, topleftindex = u * (fData.lV + 1) + v + 1
		, toprightindex = (u + 1) * (fData.lV + 1) + v + 1;

	bottomright=points[bottomrightindex];
	bottomleft=points[bottomleftindex];
	topleft=points[topleftindex];
	topright=points[toprightindex];

	if (fData.lSplitMethod == SPLIT_DUMB) {
		if ((fData.bEmptyZero == false) 
			||(intcompare(displacement[bottomrightindex]) != 0)
			||(intcompare(displacement[bottomleftindex]) != 0)
			||(intcompare(displacement[toprightindex]) != 0)) {
			AddFacet(bottomright
				, bottomleft
				, topright);
			}
		if ((fData.bEmptyZero == false) 
			||(intcompare(displacement[bottomleftindex]) != 0)
			||(intcompare(displacement[topleftindex]) != 0)
			||(intcompare(displacement[toprightindex]) != 0)) {
			AddFacet(bottomleft
				, topleft
				, topright);
			}
		}
	else if (fData.lSplitMethod == SPLIT_SMART) {
		boolean fullarea = false;
		TVector3 brtotl;
		TVector3 bltotr;
		TVertex3DLite middle;
		ShadingIn shadingIn;
		real32 fValue;
		shadingIn.fNormalLoc.x = midcosHorzLines[u] * midcosVertLines[v];
		shadingIn.fNormalLoc.y = midsinHorzLines[u] * midcosVertLines[v];
		shadingIn.fNormalLoc.z = midsinVertLines[v];
		shadingIn.fPointLoc.x = shadingIn.fNormalLoc.x * scale.x;
		shadingIn.fPointLoc.y = shadingIn.fNormalLoc.y * scale.y;
		shadingIn.fPointLoc.z = shadingIn.fNormalLoc.z * scale.z;
		shadingIn.fNormalLoc = shadingIn.fPointLoc;
		shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
		shadingIn.fUV.x = midULines[u];
		shadingIn.fUV.y = midVLines[v];
		shadingIn.fPoint = shadingIn.fPointLoc;
		shadingIn.fGNormal = shadingIn.fNormalLoc;
		//get the shading 
		shader->GetValue(fValue, fullarea, shadingIn);
		fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
		//no put it away
		middle.fVertex = shadingIn.fPointLoc;
		middle.fUV = shadingIn.fUV;
		middle.fVertex +=shadingIn.fNormalLoc * fValue;

		brtotl = (bottomright.fVertex + topleft.fVertex) * 0.5;
		bltotr = (bottomleft.fVertex + topright.fVertex) * 0.5;
		if (sqr(bltotr, middle.fVertex) < sqr(brtotl, middle.fVertex)) {
			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomrightindex]) != 0)
				||(intcompare(displacement[bottomleftindex]) != 0)
				||(intcompare(displacement[toprightindex]) != 0)) {
				AddFacet(bottomright
					, bottomleft
					, topright);
				}
			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomleftindex]) != 0)
				||(intcompare(displacement[topleftindex]) != 0)
				||(intcompare(displacement[toprightindex]) != 0)) {
				AddFacet(bottomleft
					, topleft
					, topright);
				}
			}
		else {
			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomrightindex]) != 0)
				||(intcompare(displacement[bottomleftindex]) != 0)
				||(intcompare(displacement[topleftindex]) != 0)) {
				AddFacet(bottomright
					, bottomleft
					, topleft);
				}
			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomrightindex]) != 0)
				||(intcompare(displacement[topleftindex]) != 0)
				||(intcompare(displacement[toprightindex]) != 0)) {
				AddFacet(bottomright
					, topleft
					, topright);
				}
			}
		}
	else if (fData.lSplitMethod == SPLIT_SHORT) {
		if (sqr(bottomleft.fVertex, topright.fVertex) < sqr(bottomright.fVertex, topleft.fVertex)) {
			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomrightindex]) != 0)
				||(intcompare(displacement[bottomleftindex]) != 0)
				||(intcompare(displacement[toprightindex]) != 0)) {
				AddFacet(bottomright
					, bottomleft
					, topright);
				}
			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomleftindex]) != 0)
				||(intcompare(displacement[topleftindex]) != 0)
				||(intcompare(displacement[toprightindex]) != 0)) {
				AddFacet(bottomleft
					, topleft
					, topright);
				}
			}
		else {
			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomrightindex]) != 0)
				||(intcompare(displacement[bottomleftindex]) != 0)
				||(intcompare(displacement[topleftindex]) != 0)) {
				AddFacet(bottomright
					, bottomleft
					, topleft);
				}
			if ((fData.bEmptyZero == false) 
				||(intcompare(displacement[bottomrightindex]) != 0)
				||(intcompare(displacement[topleftindex]) != 0)
				||(intcompare(displacement[toprightindex]) != 0)) {
				AddFacet(bottomright
					, topleft
					, topright);
				}
			}
		}

	}

void cAGrPrimSphere::GetBoundingBox(TBBox3D& bbox){
	bbox.fMin[0]=-(fData.sizeX * 0.5 + fData.fStop);
	bbox.fMax[0]=fData.sizeX * 0.5 + fData.fStop;
	bbox.fMin[1]=-(fData.sizeY * 0.5 + fData.fStop);
	bbox.fMax[1]=fData.sizeY * 0.5 + fData.fStop;
	bbox.fMin[2]=-(fData.sizeZ * 0.5 + fData.fStop);
	bbox.fMax[2]=fData.sizeZ * 0.5 + fData.fStop;
	}

uint32 cAGrPrimSphere::GetUVSpaceCount(){
	return 1;
	}

MCCOMErr cAGrPrimSphere::GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo){
	if (uvSpaceID == 0)	{ 
		uvSpaceInfo->fWraparound[0] = true; 
		uvSpaceInfo->fWraparound[1] = false;
		}
	return MC_S_OK;
	}
