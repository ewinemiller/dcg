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
#include "cAGrPrimCylinder.h"
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

cAGrPrimCylinder::cAGrPrimCylinder(const AGrPrimData& pfData) {
	VLines = NULL;
	VertLines = NULL;

	midULines = NULL;
	midVLines = NULL;
	midcosHorzLines = NULL;
	midsinHorzLines = NULL;
	midVertLines = NULL;

	sidepoints = NULL;
	sidedisplacement = NULL;

	toppoints = NULL;
	topdisplacement = NULL;

	bottompoints = NULL;
	bottomdisplacement = NULL;

	fData = pfData;
	//we need to make sure that we are some multiple 
	// of anything x 11
	if (fData.lV % 11 != 0) {
		fData.lV += 11 - fData.lV % 11;
		}
	//if we do a presmooth, the adaptive mesh algorithm will not
	//work, so just turn it off
	if (fData.bPreSmooth == true) {
		fData.bAdaptiveMesh = false;
		}
	topedge = 8 * fData.lV / 11;
	bottomedge = 3 * fData.lV / 11;
	}

void cAGrPrimCylinder::CleanUp() {
	if (VertLines != NULL) {
		MCfree(VertLines);
		VertLines = NULL;
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
	if (midVertLines != NULL) {
		MCfree(midVertLines);
		midVertLines = NULL;
		}
	if (midULines != NULL) {
		MCfree(midULines);
		midULines = NULL;
		}
	if (midVLines != NULL) {
		MCfree(midVLines);
		midVLines = NULL;
		}

	if (sidepoints != NULL) {
		MCfree(sidepoints);
		sidepoints = NULL;
		}
	if (sidedisplacement != NULL) {
		MCfree(sidedisplacement);
		sidedisplacement = NULL;
		}

	if (toppoints != NULL) {
		MCfree(toppoints);
		toppoints = NULL;
		}
	if (topdisplacement != NULL) {
		MCfree(topdisplacement);
		topdisplacement = NULL;
		}

	if (bottompoints != NULL) {
		MCfree(bottompoints);
		bottompoints = NULL;
		}
	if (bottomdisplacement != NULL) {
		MCfree(bottomdisplacement);
		bottomdisplacement = NULL;
		}
	
	shader = NULL;
	accu = NULL;
	}

cAGrPrimCylinder::~cAGrPrimCylinder() {
	CleanUp();
	}

void cAGrPrimCylinder::DoBasicMesh(FacetMesh** outMesh) {
	DCGFacetMeshAccumulator acc;
	TMCCountedPtr<I3DShShader> pShader;
	fData.lSplitMethod = SPLIT_DUMB;
	fData.lU = 8;
	fData.lV = 11;
	fData.bEmptyZero = false;
	topedge = 8;
	bottomedge = 3;
	DoMesh(&acc, pShader);
	acc.MakeFacetMesh(outMesh);
	}


void cAGrPrimCylinder::DoMesh(DCGFacetMeshAccumulator* pAccu, TMCCountedPtr<I3DShShader> pShader) {
	accu = pAccu;
	shader = pShader;

	//for doing the shading
	ShadingIn sideshadingIn;
	ShadingIn topshadingIn;
	ShadingIn bottomshadingIn;

	boolean fullarea = false;
	fUVSpaceID = 0;
	sideshadingIn.fUVSpaceID = fUVSpaceID;
	sideshadingIn.fCurrentCompletionMask = 0;

	topshadingIn.fUVSpaceID = fUVSpaceID;
	topshadingIn.fCurrentCompletionMask = 0;

	bottomshadingIn.fUVSpaceID = fUVSpaceID;
	bottomshadingIn.fCurrentCompletionMask = 0;
	real32 fValue = 0;


	uint32 u, v, vsize = 1;
	real32 oneoverU = 1.0 / (real32)fData.lU;
	real32 oneoverV = 1.0 / (real32)fData.lV;

	VLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
	VertLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
	
	if (fData.lSplitMethod == SPLIT_SMART) {
		midULines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
		midVLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
		midsinHorzLines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
		midcosHorzLines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
		midVertLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
		}

	if ((fData.bAdaptiveMesh == true)||(fData.bEmptyZero == true)) {
		sidedisplacement = (real32*)MCcalloc((fData.lV + 1) * (fData.lU + 1),sizeof(real32));
		topdisplacement = (real32*)MCcalloc((fData.lV + 1) * (fData.lU + 1),sizeof(real32));
		bottomdisplacement = (real32*)MCcalloc((fData.lV + 1) * (fData.lU + 1),sizeof(real32));
		}

	sidepoints = (TVertex3DLite*)MCcalloc((fData.lV + 1) * (fData.lU + 1) + 1,sizeof(TVertex3DLite));
	toppoints = (TVertex3DLite*)MCcalloc((fData.lV + 1) * (fData.lU + 1) + 1,sizeof(TVertex3DLite));
	bottompoints = (TVertex3DLite*)MCcalloc((fData.lV + 1) * (fData.lU + 1) + 1,sizeof(TVertex3DLite));

	scale.x = fData.sizeX * 0.5;
	scale.y = fData.sizeY * 0.5;
	scale.z = fData.sizeZ * 0.5;

	//calculate XY and UV slices
	for (v = 0; v <= fData.lV; v++) {
		VLines[v] = v * oneoverV;
		if (v > topedge) {
			VertLines[v] = scale.z;
			}
		else if (v < bottomedge) {
			VertLines[v] = -scale.z;
			}
		if ((v >= bottomedge)&&(v <= topedge)) {
			VertLines[v] = - fData.sizeZ * 0.5 + fData.sizeZ 
				* ((real32)(v - bottomedge) / (real32)(topedge - bottomedge));
			}

		if (fData.lSplitMethod == SPLIT_SMART) {
			midVLines[v] = (v + 0.5) * oneoverV;
			midVertLines[v] = - fData.sizeZ * 0.5 + fData.sizeZ * midVLines[v];
			if ((v + 0.5) > topedge) {
				midVertLines[v] = scale.z;
				}
			else if ((v + 0.5) < bottomedge) {
				midVertLines[v] = -scale.z;
				}
			if (((v + 0.5) >= bottomedge)&&((v + 0.5) <= topedge)) {
				midVertLines[v] = - fData.sizeZ * 0.5 + fData.sizeZ 
					* ((real32)((v + 0.5) - bottomedge) / (real32)(topedge - bottomedge));
				}
			}
		}

	//top normal
	topshadingIn.fNormalLoc.x = 0;
	topshadingIn.fNormalLoc.y = 0;
	topshadingIn.fNormalLoc.z = 1;
	//top global normal
	topshadingIn.fGNormal = topshadingIn.fNormalLoc;

	//bottom normal
	bottomshadingIn.fNormalLoc.x = 0;
	bottomshadingIn.fNormalLoc.y = 0;
	bottomshadingIn.fNormalLoc.z = -1;
	//bottom global normal
	bottomshadingIn.fGNormal = bottomshadingIn.fNormalLoc;

	//calculate the displacement
	for (u = 0; u <= fData.lU; u ++){
		TVector3 NormalStarter;

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

		//side local point
		sideshadingIn.fPointLoc.x = cosHorzLine * scale.x;
		sideshadingIn.fPointLoc.y = sinHorzLine * scale.y;

		//side normal
		sideshadingIn.fNormalLoc.x = sideshadingIn.fPointLoc.x;
		sideshadingIn.fNormalLoc.y = sideshadingIn.fPointLoc.y;
		sideshadingIn.fNormalLoc.z = 0;
		sideshadingIn.fNormalLoc.Normalize(sideshadingIn.fNormalLoc);
		if (fData.bPreSmooth) {
			NormalStarter = sideshadingIn.fNormalLoc;
			}

		//side U coordinate
		sideshadingIn.fUV.x = ULine;

		//side global normal
		sideshadingIn.fGNormal = sideshadingIn.fNormalLoc;

		//top U coordinate
		topshadingIn.fUV.x = ULine;

		//bottom U coordinate
		bottomshadingIn.fUV.x = ULine;

		for (v = 0; v <= fData.lV; v++) {
			//do the top
			if (v >= topedge) {
				real32 fFarAlong = (real32)(v - topedge) / (real32)(fData.lV - topedge);
				topshadingIn.fPointLoc.x = sideshadingIn.fPointLoc.x * (1 - fFarAlong);
				topshadingIn.fPointLoc.y = sideshadingIn.fPointLoc.y * (1 - fFarAlong);
				topshadingIn.fPointLoc.z = scale.z;
				topshadingIn.fUV.y = VLines[v];
				//global point
				topshadingIn.fPoint = topshadingIn.fPointLoc;
				if (fData.bPreSmooth) {
					topshadingIn.fNormalLoc = NormalStarter * (1 - fFarAlong);
					topshadingIn.fNormalLoc.z = 1;
					topshadingIn.fNormalLoc.Normalize(topshadingIn.fNormalLoc);

					topshadingIn.fGNormal = topshadingIn.fNormalLoc;
					}
				//get the shading 
				if (shader!=NULL) {
					shader->GetValue(fValue, fullarea, topshadingIn);
					fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
					}
				//now put it away
				toppoints[GetIndex(u, v)].fVertex = topshadingIn.fPointLoc;
				toppoints[GetIndex(u, v)].fUV = topshadingIn.fUV;

				toppoints[GetIndex(u, v)].fVertex += topshadingIn.fNormalLoc * fValue;
				if (topdisplacement!=NULL) {
					topdisplacement[GetIndex(u, v)] = fValue;
					}
				}

			//do the bottom
			if (v <= bottomedge) {
				real32 fFarAlong = (real32)v / (real32)bottomedge;
				bottomshadingIn.fPointLoc.x = sideshadingIn.fPointLoc.x * fFarAlong;
				bottomshadingIn.fPointLoc.y = sideshadingIn.fPointLoc.y * fFarAlong;
				bottomshadingIn.fPointLoc.z = - scale.z;
				bottomshadingIn.fUV.y = VLines[v];
				//global point
				bottomshadingIn.fPoint = bottomshadingIn.fPointLoc;
				if (fData.bPreSmooth) {
					bottomshadingIn.fNormalLoc = NormalStarter * fFarAlong;
					bottomshadingIn.fNormalLoc.z = - 1;
					bottomshadingIn.fNormalLoc.Normalize(bottomshadingIn.fNormalLoc);

					bottomshadingIn.fGNormal = bottomshadingIn.fNormalLoc;
					}
				//get the shading 
				if (shader!=NULL) {
					shader->GetValue(fValue, fullarea, bottomshadingIn);
					fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
					}
				//now put it away
				bottompoints[GetIndex(u, v)].fVertex = bottomshadingIn.fPointLoc;
				bottompoints[GetIndex(u, v)].fUV = bottomshadingIn.fUV;

				bottompoints[GetIndex(u, v)].fVertex +=bottomshadingIn.fNormalLoc * fValue;
				if (bottomdisplacement!=NULL) {
					bottomdisplacement[GetIndex(u, v)] = fValue;
					}
				}

			//do the walls
			if ((v >= bottomedge)&&(v <= topedge)) {
				sideshadingIn.fPointLoc.z = VertLines[v];
				sideshadingIn.fUV.y = VLines[v];
				sideshadingIn.fPoint = sideshadingIn.fPointLoc;
				if (fData.bPreSmooth) {
					sideshadingIn.fNormalLoc = NormalStarter;
					sideshadingIn.fNormalLoc.z = 
						2 * (real32)(v - bottomedge) / (real32)(topedge - bottomedge) - 1;
					sideshadingIn.fNormalLoc.Normalize(sideshadingIn.fNormalLoc);

					sideshadingIn.fGNormal = sideshadingIn.fNormalLoc;
					}

				//get the shading 
				if (shader!=NULL) {
					shader->GetValue(fValue, fullarea, sideshadingIn);
					fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
					}
				//now put it away
				sidepoints[GetIndex(u, v)].fVertex = sideshadingIn.fPointLoc;
				sidepoints[GetIndex(u, v)].fUV = sideshadingIn.fUV;
				sidepoints[GetIndex(u, v)].fVertex +=sideshadingIn.fNormalLoc * fValue;
				if (sidedisplacement!=NULL) {
					sidedisplacement[GetIndex(u, v)] = fValue;
					}
				}
			}
		}
	for (u = 0; u < fData.lU; u ++){
		for (v = 0; v < fData.lV; v+=vsize) {
			if ((v >= bottomedge)&&(v < topedge)) {
				if (fData.bAdaptiveMesh == true) {
					GetBlockSize(sidedisplacement, sidepoints, u, v, topedge, vsize);
					}
				AddQuad(sidedisplacement, sidepoints, u, v, vsize);
				}
			else if (v < bottomedge) {
				if (v == 0) {
					vsize = 1;
					TVertex3DLite bottomright, bottomleft, topleft, topright;
					int32 bottomrightindex = GetIndex(u + 1, 0)
						, bottomleftindex = GetIndex(u, 0)
						, topleftindex = GetIndex(u, 1)
						, toprightindex = GetIndex(u + 1, 1);

					bottomright=bottompoints[bottomrightindex];
					bottomleft=bottompoints[bottomleftindex];
					topleft=bottompoints[topleftindex];
					topright=bottompoints[toprightindex];

					if ((fData.bEmptyZero == false) 
						||(intcompare(bottomdisplacement[bottomrightindex]) != 0)
						||(intcompare(bottomdisplacement[topleftindex]) != 0)
						||(intcompare(bottomdisplacement[toprightindex]) != 0)) {
						AddFacet(bottomright
							, topleft
							, topright);
						}
					}
				else {
					if (fData.bAdaptiveMesh == true) {
						GetBlockSize(bottomdisplacement, bottompoints, u, v
							, bottomedge, vsize);
						}
					AddQuad(bottomdisplacement, bottompoints, u, v, vsize);
					}
				}
			else if (v >= topedge) {
				if (v == fData.lV - 1) {
					vsize = 1;
					TVertex3DLite bottomright, bottomleft, topleft, topright;
					int32 bottomrightindex = GetIndex(u + 1, fData.lV - 1)
						, bottomleftindex = GetIndex(u, fData.lV - 1)
						, topleftindex = GetIndex(u, fData.lV)
						, toprightindex = GetIndex(u + 1, fData.lV);

					bottomright=toppoints[bottomrightindex];
					bottomleft=toppoints[bottomleftindex];
					topleft=toppoints[topleftindex];
					topright=toppoints[toprightindex];

					if ((fData.bEmptyZero == false) 
						||(intcompare(topdisplacement[bottomrightindex]) != 0)
						||(intcompare(topdisplacement[bottomleftindex]) != 0)
						||(intcompare(topdisplacement[toprightindex]) != 0)) {
						AddFacet(bottomright
							, bottomleft
							, topright);
						}
					}
				else {
					if (fData.bAdaptiveMesh == true) {
						GetBlockSize(topdisplacement, toppoints, u, v, fData.lV - 1, vsize);
						}
					AddQuad(topdisplacement, toppoints, u, v, vsize);
					}
				}
			}
		}

	//clean up
	CleanUp();	
	}


void cAGrPrimCylinder::AddFacet(const TVertex3DLite& pt1,const TVertex3DLite& pt2,const TVertex3DLite& pt3) {
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


void cAGrPrimCylinder::AddQuad(const real32* displacement, const TVertex3DLite* points
							   , const int32& u, const int32& v, const int32& vsize) {
	TVertex3DLite bottomright, bottomleft, topleft, topright;
	int32 bottomrightindex = GetIndex(u + 1, v)
		, bottomleftindex = GetIndex(u, v)
		, topleftindex =  GetIndex(u, v + vsize)
		, toprightindex = GetIndex(u + 1, v + vsize);

	bottomright=points[bottomrightindex];
	bottomleft=points[bottomleftindex];
	topleft=points[topleftindex];
	topright=points[toprightindex];

	if ((vsize > 1)||(fData.lSplitMethod == SPLIT_DUMB)) {
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
	else if(fData.lSplitMethod == SPLIT_SMART) {
		boolean fullarea = false;
		TVector3 brtotl;
		TVector3 bltotr;
		TVertex3DLite middle;
		ShadingIn shadingIn;
		real32 fValue;
		if ((v + 0.5) > topedge) {
			real32 fFarAlong = ((v + 0.5) - topedge) / (real32)(fData.lV - topedge);
			//top
			shadingIn.fPointLoc.x = midcosHorzLines[u] * scale.x * (1 - fFarAlong);
			shadingIn.fPointLoc.y = midsinHorzLines[u] * scale.y * (1 - fFarAlong);
			shadingIn.fPointLoc.z = scale.z;
			shadingIn.fUV.y = midVLines[v];
			shadingIn.fUV.x = midULines[u];
			shadingIn.fPoint = shadingIn.fPointLoc;
			if (fData.bPreSmooth) {
				shadingIn.fNormalLoc.x = midcosHorzLines[u] * scale.x;
				shadingIn.fNormalLoc.y = midsinHorzLines[u] * scale.y;
				shadingIn.fNormalLoc.z = 0;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				shadingIn.fNormalLoc = shadingIn.fNormalLoc * (1 - fFarAlong);
				shadingIn.fNormalLoc.z = 1;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				}
			else {
				shadingIn.fNormalLoc.x = 0;
				shadingIn.fNormalLoc.y = 0;
				shadingIn.fNormalLoc.z = 1;
				}
			shadingIn.fGNormal = shadingIn.fNormalLoc;

			}
		else if ((v + 0.5) < bottomedge) {
			real32 fFarAlong = ((real32)v + 0.5) / (real32)bottomedge;
			//bottom
			shadingIn.fPointLoc.x = midcosHorzLines[u] * scale.x * fFarAlong;
			shadingIn.fPointLoc.y = midsinHorzLines[u] * scale.y * fFarAlong;
			shadingIn.fPointLoc.z = - scale.z;
			shadingIn.fUV.y = midVLines[v];
			shadingIn.fUV.x = midULines[u];
			shadingIn.fPoint = shadingIn.fPointLoc;
			if (fData.bPreSmooth) {
				shadingIn.fNormalLoc.x = midcosHorzLines[u] * scale.x;
				shadingIn.fNormalLoc.y = midsinHorzLines[u] * scale.y;
				shadingIn.fNormalLoc.z = 0;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				shadingIn.fNormalLoc = shadingIn.fNormalLoc * fFarAlong;
				shadingIn.fNormalLoc.z = -1;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				}
			else {
				shadingIn.fNormalLoc.x = 0;
				shadingIn.fNormalLoc.y = 0;
				shadingIn.fNormalLoc.z = -1;
				}
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			}
		else {
			//walls
			shadingIn.fPointLoc.x = midcosHorzLines[u] * scale.x;
			shadingIn.fPointLoc.y = midsinHorzLines[u] * scale.y;
			shadingIn.fPointLoc.z = midVertLines[v];
			shadingIn.fNormalLoc.x = shadingIn.fPointLoc.x;
			shadingIn.fNormalLoc.y = shadingIn.fPointLoc.y;
			shadingIn.fNormalLoc.z = 0;
			shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
			shadingIn.fUV.y = midVLines[v];
			shadingIn.fUV.x = midULines[u];

			shadingIn.fPoint = shadingIn.fPointLoc;
			if (fData.bPreSmooth) {
				shadingIn.fNormalLoc.z = 
					2 * (real32)(v - bottomedge) / (real32)(topedge - bottomedge) - 1;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);

				}
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			}

		//get the shading 
		shader->GetValue(fValue, fullarea, shadingIn);
		fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
		//no put it away
		middle.fVertex = shadingIn.fPointLoc;
		middle.fUV = shadingIn.fUV;
		middle.fVertex.x +=shadingIn.fNormalLoc.x * fValue;
		middle.fVertex.y +=shadingIn.fNormalLoc.y * fValue;

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
		}//END SPLIT_SMART
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
		}//END SPLIT_SHORT
	}

void cAGrPrimCylinder::GetBoundingBox(TBBox3D& bbox){
	bbox.fMin[0]=-(fData.sizeX * 0.5 + fData.fStop);
	bbox.fMax[0]=fData.sizeX * 0.5 + fData.fStop;
	bbox.fMin[1]=-(fData.sizeY * 0.5 + fData.fStop);
	bbox.fMax[1]=fData.sizeY * 0.5 + fData.fStop;
	bbox.fMin[2]=-fData.sizeZ * 0.5;
	bbox.fMax[2]=fData.sizeZ * 0.5;
	}

uint32 cAGrPrimCylinder::GetUVSpaceCount(){
	return 1;
	}

MCCOMErr cAGrPrimCylinder::GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo){
	if (uvSpaceID == 0)	{ 
		uvSpaceInfo->fWraparound[0] = false;  // No Wrap around
		uvSpaceInfo->fWraparound[1] = true;
		}
	return MC_S_OK;
	}

void cAGrPrimCylinder::GetBlockSize(const real32* displacement
	, const TVertex3DLite* points
	, const uint32& u, const uint32& v, const uint32& maxv, uint32& vsize) {

	real32 deltav;
	boolean bDone = false, bCanStepV = true;
	
	vsize = 1;

	deltav = intcompare(displacement[GetIndex(u, v + 1)] - displacement[GetIndex(u, v)]);

	//is the current block itself parallel, if not we can't go on

	if (intcompare(displacement[GetIndex(u + 1, v + 1)] - displacement[GetIndex(u + 1, v)]) ==  deltav) {
		

		while (bDone == false) {
			if (bCanStepV) {
				//see if we can take another step in the U direction
				if (v + vsize >= maxv) {
					bCanStepV = false;
					}
				else {
					if ((intcompare(displacement[GetIndex(u, v + vsize + 1)]
						- displacement[GetIndex(u, v + vsize)])
						!= deltav)
						||
						(intcompare(displacement[GetIndex(u + 1, v + vsize + 1)]
						- displacement[GetIndex(u + 1, v + vsize)])
						!= deltav)){

						bCanStepV = false;
						}
					}
				}

			if (bCanStepV) {
				vsize++;
				}
			else {
				bDone = true;
				}

			}

		}
	}
