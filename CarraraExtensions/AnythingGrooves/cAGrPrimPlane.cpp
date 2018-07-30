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
#include "cAGrPrimPlane.h"
#include "AnythingGroovesDLL.h"
#include "MCMemoryCopy.h"
#include "Copyright.h"

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

cAGrPrimPlane::cAGrPrimPlane(const AGrPrimData& pfData) {
	XLines = NULL;
	YLines = NULL;
	ULines = NULL;
	VLines = NULL;

	midXLines = NULL;
	midYLines = NULL;
	midULines = NULL;
	midVLines = NULL;

	displacement = NULL;
	bFilled = NULL;
	iBlocksU = NULL;
	iBlocksV = NULL;
	fData = pfData;
	}

void cAGrPrimPlane::CleanUp () {
	if (XLines != NULL) {
		MCfree(XLines);
		XLines = NULL;
		}
	if (YLines != NULL) {
		MCfree(YLines);
		YLines = NULL;
		}
	if (ULines != NULL) {
		MCfree(ULines);
		ULines = NULL;
		}
	if (VLines != NULL) {
		MCfree(VLines);
		VLines = NULL;
		}

	if (midXLines != NULL) {
		MCfree(midXLines);
		midXLines = NULL;
		}
	if (midYLines != NULL) {
		MCfree(midYLines);
		midYLines = NULL;
		}
	if (midULines != NULL) {
		MCfree(midULines);
		midULines = NULL;
		}
	if (midVLines != NULL) {
		MCfree(midVLines);
		midVLines = NULL;
		}

	if (displacement != NULL) {
		MCfree(displacement);
		displacement = NULL;
		}
	if (bFilled != NULL) {
		MCfree(bFilled);
		bFilled = NULL;
		}
	if (iBlocksU != NULL) {
		MCfree(iBlocksU);
		iBlocksU = NULL;
		}
	if (iBlocksV != NULL) {
		MCfree(iBlocksV);
		iBlocksV = NULL;
		}

	shader = NULL;
	accu = NULL;
	}

cAGrPrimPlane::~cAGrPrimPlane() {
	CleanUp();
	}

void cAGrPrimPlane::DoBasicMesh(FacetMesh** outMesh) {
	DCGFacetMeshAccumulator acc;
	TMCCountedPtr<I3DShShader> pShader;
	fData.lSplitMethod = SPLIT_DUMB;
	fData.lU = 1;
	fData.lV = 1;
	DoMesh(&acc, pShader);
	acc.MakeFacetMesh(outMesh);
	}

void cAGrPrimPlane::GetBlockSize(const uint32& u, const uint32& v, uint32& usize, uint32& vsize) {

	real32 deltau, deltav;
	uint32 tempu, tempv, tempusize, tempvsize;
	boolean bDone = false, bCanStepU = true, bCanStepV = true, bTrySingleDirection = false;
	
	usize = 1;
	vsize = 1;

	deltau = intcompare(displacement[(u + 1) * (fData.lV + 1) + v] - displacement[u * (fData.lV + 1) + v]);
	deltav = intcompare(displacement[u * (fData.lV + 1) + v + 1] - displacement[u * (fData.lV + 1) + v]);

	//is the current block itself parallel, if not we can't go on

	if ((intcompare(displacement[(u + 1) * (fData.lV + 1) + v + 1] - displacement[(u + 1) * (fData.lV + 1) + v]) ==  deltav)
		&&(intcompare(displacement[(u + 1) * (fData.lV + 1) + v + 1] - displacement[u * (fData.lV + 1) + v + 1]) == deltau)) {
		

		while (bDone == false) {
			if (bCanStepU) {
				//see if we can take another step in the U direction
				if (u + usize >= fData.lU) {
					bCanStepU = false;
					}
				else {
					for(tempv = v; tempv <= v + vsize; tempv++) {
						if (//not flat
							(intcompare(
							displacement[(u + usize + 1) * (fData.lV + 1) + tempv]
							- displacement[(u + usize) * (fData.lV + 1) + tempv])
							!= deltau)||
							//or already filled
							((bFilled[(u + usize) * (fData.lV + 1) + tempv])
								&&(tempv != v + vsize))){
							bCanStepU = false;
							//get us out of this loop
							tempv = v + vsize + 1;
							}	
						}
					}
				}
			if (bCanStepV) {
				//see if we can take another step in the U direction
				if (v + vsize >= fData.lV) {
					bCanStepV = false;
					}
				else {
					for(tempu = u; tempu <= u + usize; tempu++) {
						if (//not flat
							(intcompare(displacement[tempu * (fData.lV + 1) + v + vsize + 1]
							- displacement[tempu * (fData.lV + 1) + v + vsize])
							!= deltav)
							//or already filled
							||((bFilled[tempu * (fData.lV + 1) + v + vsize])
								&&(tempu != u + usize))){
							bCanStepV = false;
							tempu = u + usize + 1;
							}
						}
					}
				}

			if ((bCanStepV)&&(bCanStepU)) {
				//we can go both directions, but can we do the little corner
				//if not we'll just go in the U direction
				if ((intcompare(displacement[(u + usize +  1) * (fData.lV + 1) + v + vsize + 1]
					-displacement[(u + usize) * (fData.lV + 1) + v + vsize + 1]) == deltau)
					&&(intcompare(displacement[(u + usize +  1) * (fData.lV + 1) + v + vsize + 1]
					- displacement[(u + usize +  1) * (fData.lV + 1) + v + vsize]) == deltav)
					&&(!bFilled[(u + usize) * (fData.lV + 1) + v + vsize]) ) {
					vsize++;
					usize++;
					}
				else {
					bTrySingleDirection = true;
					bDone = true;
					}
				}
			else if (bCanStepV) {
				vsize++;
				}
			else if (bCanStepU) {
				usize++;
				}
			else {
				bDone = true;
				}

			}//end while not done

		}
	
	if (bTrySingleDirection) {
		//first keep going in the U direction
		tempusize = usize + 1;
		bDone = false;
		while (bDone == false) {
			if (bCanStepU) {
				//see if we can take another step in the U direction
				if (u + tempusize >= fData.lU) {
					bCanStepU = false;
					}
				else {
					for(tempv = v; tempv <= v + vsize; tempv++) {
						if (//not flat
							(intcompare(
							displacement[(u + tempusize + 1) * (fData.lV + 1) + tempv]
							- displacement[(u + tempusize) * (fData.lV + 1) + tempv])
							!= deltau)||
							//or already filled
							((bFilled[(u + tempusize) * (fData.lV + 1) + tempv])
							&&(tempv != v + vsize))){
							bCanStepU = false;
							//get us out of this loop
							tempv = v + vsize + 1;
							}	
						}
					}
				}

			if (bCanStepU) {
				tempusize++;
				}
			else {
				bDone = true;
				}

			}//end while not done

		tempvsize = vsize + 1;
		bDone = false;
		//next keep going in the V direction
		while (bDone == false) {
			if (bCanStepV) {
				//see if we can take another step in the U direction
				if (v + tempvsize >= fData.lV) {
					bCanStepV = false;
					}
				else {
					for(tempu = u; tempu <= u + usize; tempu++) {
						if (//not flat
							(intcompare(displacement[tempu * (fData.lV + 1) + v + tempvsize + 1]
							- displacement[tempu * (fData.lV + 1) + v + tempvsize])
							!= deltav)
							//or already filled
							||((bFilled[tempu * (fData.lV + 1) + v + tempvsize])
							&&(tempu != u + usize))){
							bCanStepV = false;
							tempu = u + usize + 1;
							}
						}
					}
				}

			if (bCanStepV) {
				tempvsize++;
				}
			else {
				bDone = true;
				}

			}//end while not done

		//which one gave us the biggest block
		if (usize * tempvsize > tempusize * vsize) {
			vsize = tempvsize;
			}
		else {
			usize = tempusize;
			}
		}//end try single

	//mark everything in this block as filled
	for (tempu = u; tempu < u + usize; tempu++) {
		for (tempv = v; tempv < v + vsize; tempv++) {
			bFilled[tempu * (fData.lV + 1) + tempv] = true;
			}
		}
	
	}

void cAGrPrimPlane::DoMesh(DCGFacetMeshAccumulator* pAccu, TMCCountedPtr<I3DShShader> pShader) {
	accu = pAccu;
	shader = pShader;

	//for doing the shading
	ShadingIn shadingIn;
	boolean fullarea = false;
	fUVSpaceID = 0;
	shadingIn.fNormalLoc.x = 0;
	shadingIn.fNormalLoc.y = 0;
	shadingIn.fNormalLoc.z = 1;
	shadingIn.fUVSpaceID = fUVSpaceID;
	shadingIn.fCurrentCompletionMask = 0;


	uint32 u, v;
	uint32 usize = 1, vsize = 1;
	real32 oneoverU = 1.0 / (real32)fData.lU;
	real32 oneoverV = 1.0 / (real32)fData.lV;

	XLines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
	YLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
	ULines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
	VLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
	if (fData.lSplitMethod == SPLIT_SMART) {
		midXLines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
		midYLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
		midULines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
		midVLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
		}


	displacement = (real32*)MCcalloc((fData.lV + 1) * (fData.lU + 1),sizeof(real32));

	//calculate XY and UV slices
	for (u = 0; u <= fData.lU; u ++){
		ULines[u] = u * oneoverU;
		XLines[u] = -fData.sizeX * 0.5 + fData.sizeX * ULines[u];
		if (fData.lSplitMethod == SPLIT_SMART) {
			midULines[u] = (u + 0.5) * oneoverU;
			midXLines[u] = -fData.sizeX * 0.5 + fData.sizeX * midULines[u];
			}
		}
	for (v = 0; v <= fData.lV; v++) {
		VLines[v] = v * oneoverV;
		YLines[v] = -fData.sizeY * 0.5 + fData.sizeY * VLines[v];
		if (fData.lSplitMethod == SPLIT_SMART) {
			midVLines[v] = (v + 0.5) * oneoverV;
			midYLines[v] = -fData.sizeY * 0.5 + fData.sizeY * midVLines[v];
			}
		}

	//calculate the displacement
	for (u = 0; u <= fData.lU; u ++){
		for (v = 0; v <= fData.lV; v++) {
			real32 fValue;
			shadingIn.fUV.x = ULines[u];
			shadingIn.fUV.y = VLines[v];
			shadingIn.fPointLoc.x = XLines[u];
			shadingIn.fPointLoc.y = YLines[v];
			shadingIn.fPoint = shadingIn.fPointLoc;
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			if (shader != NULL) {
				shader->GetValue(fValue, fullarea, shadingIn);
				displacement[u * (fData.lV + 1) + v] = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				}
			else {
				displacement[u * (fData.lV + 1) + v] = 0;
				}
			//bFilled[u * (fData.lV + 1) + v] = false;
			}
		}


	if (fData.bAdaptiveMesh) {
		bFilled = (boolean*)MCcalloc((fData.lV + 1) * (fData.lU + 1) + 1,sizeof(boolean));
		iBlocksU = (uint32*)MCcalloc((fData.lV + 1) * (fData.lU + 1) + 1,sizeof(uint32));
		iBlocksV = (uint32*)MCcalloc((fData.lV + 1) * (fData.lU + 1) + 1,sizeof(uint32));
		uint32 quads = 0;
		uint32 iBlockU;
		uint32 iBlockV;
#if (VERSIONNUMBER >= 0x080000)
		for (int i = 0; i < (fData.lV + 1) * (fData.lU + 1) + 1;i++)
		{
			bFilled[i] = false;
		}
#else
		MCmemfill(false, bFilled, (fData.lV + 1) * (fData.lU + 1) + 1);
#endif
		for (u = 0; u < fData.lU; u ++)
		{
			for (v = 0; v < fData.lV; v++) 
			{
				if (bFilled[u * (fData.lV + 1) + v] == false) 
				{
					GetBlockSize(u, v, usize, vsize);
					iBlocksU[u * (fData.lV + 1) + v] = usize;
					iBlocksV[u * (fData.lV + 1) + v] = vsize;
					quads++;
				}
				else
				{
					iBlocksU[u * (fData.lV + 1) + v] = 0;
					iBlocksV[u * (fData.lV + 1) + v] = 0;
				}
			}
		}

		//memset(bFilled, false, (fData.lV + 1) * (fData.lU + 1) + 1);
		accu->PrepareAccumulation(quads * 2);
		for (u = 0; u < fData.lU; u ++){
			for (v = 0; v < fData.lV; v++) {
				iBlockU = iBlocksU[u * (fData.lV + 1) + v];
				iBlockV = iBlocksV[u * (fData.lV + 1) + v];
				if ((iBlockU != 0)&&(iBlockV != 0)) {
					AddQuad(u, v, iBlockU, iBlockV);
					}
				}
			}
	}

	else {//not adaptive mesh
		accu->PrepareAccumulation(fData.lU * fData.lV * 2);
		for (u = 0; u < fData.lU; u ++){
			for (v = 0; v < fData.lV; v++) {
				AddQuad(u, v, 1, 1);
				}
			}
		}
	//clean up
	CleanUp();

	}


inline void cAGrPrimPlane::AddFacet(const TVertex3DLite& pt1,const TVertex3DLite& pt2,const TVertex3DLite& pt3) {
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

void cAGrPrimPlane::AddQuad(const int32& u, const int32& v, const int32& usize, const int32& vsize) {
	TVertex3DLite bottomright, bottomleft, topleft, topright;

	int32 bottomrightindex = (u + usize) * (fData.lV + 1) + v
		, bottomleftindex = u * (fData.lV + 1) + v
		, topleftindex = u * (fData.lV + 1) + v + vsize
		, toprightindex = (u + usize) * (fData.lV + 1) + v + vsize;

	bottomright.fVertex[0]=XLines[u + usize];
	bottomright.fVertex[1]=YLines[v];
	bottomright.fVertex[2]=displacement[bottomrightindex];
	bottomright.fUV[0]=ULines[u + usize];
	bottomright.fUV[1]=VLines[v];

	bottomleft.fVertex[0]=XLines[u];
	bottomleft.fVertex[1]=YLines[v];
	bottomleft.fVertex[2]=displacement[bottomleftindex];
	bottomleft.fUV[0]=ULines[u];
	bottomleft.fUV[1]=VLines[v];

	topleft.fVertex[0]=XLines[u];
	topleft.fVertex[1]=YLines[v + vsize];
	topleft.fVertex[2]=displacement[topleftindex];
	topleft.fUV[0]=ULines[u];
	topleft.fUV[1]=VLines[v + vsize];

	topright.fVertex[0]=XLines[u + usize];
	topright.fVertex[1]=YLines[v + vsize];
	topright.fVertex[2]=displacement[toprightindex];
	topright.fUV[0]=ULines[u + usize];
	topright.fUV[1]=VLines[v + vsize];

	if ((usize > 1)||(vsize > 1)||(fData.lSplitMethod == SPLIT_DUMB)) {
		//if usize or vsize is greater than 1
		//then we know it's a adaptive mesh optimization
		//and is therefore flat
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
		ShadingIn shadingIn;
		boolean fullarea = false;
		real32 fValue;
		TVertex3DLite middle;
		TVector3 brtotl;
		TVector3 bltotr;

		shadingIn.fNormalLoc.x = 0;
		shadingIn.fNormalLoc.y = 0;
		shadingIn.fNormalLoc.z = 1;
		shadingIn.fUVSpaceID = fUVSpaceID;
		shadingIn.fCurrentCompletionMask = 0;
		shadingIn.fUV.x = midULines[u];
		shadingIn.fUV.y = midVLines[v];
		shadingIn.fPointLoc.x = midXLines[u];
		shadingIn.fPointLoc.y = midYLines[v];
		shadingIn.fPoint = shadingIn.fPointLoc;
		shadingIn.fGNormal = shadingIn.fNormalLoc;
		shader->GetValue(fValue, fullarea, shadingIn);
		middle.fVertex.x = midXLines[u];
		middle.fVertex.y = midYLines[v];
		middle.fVertex.z = fValue * fData.fStop + (1 - fValue) * fData.fStart;

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
		}//SPLIT_SHORT
	}

void cAGrPrimPlane::GetBoundingBox(TBBox3D& bbox)
{
	bbox.fMin[0]=-fData.sizeX * 0.5;
	bbox.fMax[0]=fData.sizeX * 0.5;
	bbox.fMin[1]=-fData.sizeY * 0.5;
	bbox.fMax[1]=fData.sizeY * 0.5;
	bbox.fMin[2]= fData.fStart;
	bbox.fMax[2]= fData.fStop;
}

uint32 cAGrPrimPlane::GetUVSpaceCount(){
	return 1;
	}

MCCOMErr cAGrPrimPlane::GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo)
{
	if (uvSpaceID == 0)
	{ 
		uvSpaceInfo->fWraparound[0] = false;  // No Wrap around
		uvSpaceInfo->fWraparound[1] = false;
	}
	return MC_S_OK;
}

