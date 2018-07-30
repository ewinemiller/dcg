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
#include "cAGrPrimCube.h"
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

cAGrPrimCube::cAGrPrimCube(const AGrPrimData& pfData) {
	toppoints = NULL;
	bottompoints = NULL;
	rightpoints = NULL;
	leftpoints = NULL;
	frontpoints = NULL;
	backpoints = NULL;

	midtoppoints = NULL;
	midbottompoints = NULL;
	midrightpoints = NULL;
	midleftpoints = NULL;
	midfrontpoints = NULL;
	midbackpoints = NULL;

	topfilled = NULL;
	bottomfilled = NULL;
	rightfilled = NULL;
	leftfilled = NULL;
	frontfilled = NULL;
	backfilled = NULL;

	ULines = NULL;
	VLines = NULL;

	midULines = NULL;
	midVLines = NULL;

	topdisplacement = NULL;
	bottomdisplacement = NULL;
	leftdisplacement = NULL;
	rightdisplacement = NULL;
	frontdisplacement = NULL;
	backdisplacement = NULL;
	fData = pfData;

	//we need to make sure that we are some multiple 
	// of 3 x 4
	if (fData.lU % 3 != 0) {
		fData.lU += 3 - fData.lU % 3;
		}
	if (fData.lV % 4 != 0) {
		fData.lV += 4 - fData.lV % 4;
		}
	//if we do a presmooth, the adaptive mesh algorithm will not
	//work, so just turn it off
	if (fData.bPreSmooth == true) {
		fData.bAdaptiveMesh = false;
		}

	}

void cAGrPrimCube::CleanUp () {
	if (toppoints != NULL) {
		MCfree(toppoints);
		toppoints = NULL;
		}
	if (bottompoints != NULL) {
		MCfree(bottompoints);
		bottompoints = NULL;
		}
	if (rightpoints != NULL) {
		MCfree(rightpoints);
		rightpoints = NULL;
		}
	if (leftpoints != NULL) {
		MCfree(leftpoints);
		leftpoints = NULL;
		}
	if (backpoints != NULL) {
		MCfree(backpoints);
		backpoints = NULL;
		}
	if (frontpoints != NULL) {
		MCfree(frontpoints);
		frontpoints = NULL;
		}

	if (midtoppoints != NULL) {
		MCfree(midtoppoints);
		midtoppoints = NULL;
		}
	if (midbottompoints != NULL) {
		MCfree(midbottompoints);
		midbottompoints = NULL;
		}
	if (midrightpoints != NULL) {
		MCfree(midrightpoints);
		midrightpoints = NULL;
		}
	if (midleftpoints != NULL) {
		MCfree(midleftpoints);
		midleftpoints = NULL;
		}
	if (midbackpoints != NULL) {
		MCfree(midbackpoints);
		midbackpoints = NULL;
		}
	if (midfrontpoints != NULL) {
		MCfree(midfrontpoints);
		midfrontpoints = NULL;
		}

	if (topfilled != NULL) {
		MCfree(topfilled);
		topfilled = NULL;
		}
	if (bottomfilled != NULL) {
		MCfree(bottomfilled);
		bottomfilled = NULL;
		}
	if (rightfilled != NULL) {
		MCfree(rightfilled);
		rightfilled = NULL;
		}
	if (leftfilled != NULL) {
		MCfree(leftfilled);
		leftfilled = NULL;
		}
	if (backfilled != NULL) {
		MCfree(backfilled);
		backfilled = NULL;
		}
	if (frontfilled != NULL) {
		MCfree(frontfilled);
		frontfilled = NULL;
		}

	if (topdisplacement != NULL) {
		MCfree(topdisplacement);
		topdisplacement = NULL;
		}
	if (bottomdisplacement != NULL) {
		MCfree(bottomdisplacement);
		bottomdisplacement = NULL;
		}
	if (rightdisplacement != NULL) {
		MCfree(rightdisplacement);
		rightdisplacement = NULL;
		}
	if (leftdisplacement != NULL) {
		MCfree(leftdisplacement);
		leftdisplacement = NULL;
		}
	if (backdisplacement != NULL) {
		MCfree(backdisplacement);
		backdisplacement = NULL;
		}
	if (frontdisplacement != NULL) {
		MCfree(frontdisplacement);
		frontdisplacement = NULL;
		}

	if (ULines != NULL) {
		MCfree(ULines);
		ULines = NULL;
		}
	if (VLines != NULL) {
		MCfree(VLines);
		VLines = NULL;
		}

	if (midULines != NULL) {
		MCfree(midULines);
		midULines = NULL;
		}
	if (midVLines != NULL) {
		MCfree(midVLines);
		midVLines = NULL;
		}
	
	shader = NULL;
	accu = NULL;
	}

cAGrPrimCube::~cAGrPrimCube() {
	CleanUp();
	}

void cAGrPrimCube::DoBasicMesh(FacetMesh** outMesh) {
	DCGFacetMeshAccumulator acc;
	TMCCountedPtr<I3DShShader> pShader;
	fData.lSplitMethod = SPLIT_DUMB;
	fData.lU = 3;
	fData.lV = 4;
	DoMesh(&acc, pShader);
	acc.MakeFacetMesh(outMesh);
	}

void cAGrPrimCube::GetBlockSize(boolean* bFilled, const real32* displacement
	, const uint32& u, const uint32& v, uint32& usize, uint32& vsize) {

	real32 deltau, deltav;
	uint32 tempu, tempv, tempusize, tempvsize;
	boolean bDone = false, bCanStepU = true, bCanStepV = true, bTrySingleDirection = false;
	
	usize = 1;
	vsize = 1;

	deltau = intcompare(displacement[(u + 1) * (chunkv + 1) + v] - displacement[u * (chunkv + 1) + v]);
	deltav = intcompare(displacement[u * (chunkv + 1) + v + 1] - displacement[u * (chunkv + 1) + v]);

	//is the current block itself parallel, if not we can't go on

	if ((intcompare(displacement[(u + 1) * (chunkv + 1) + v + 1] - displacement[(u + 1) * (chunkv + 1) + v]) ==  deltav)
		&&(intcompare(displacement[(u + 1) * (chunkv + 1) + v + 1] - displacement[u * (chunkv + 1) + v + 1]) == deltau)) {
		

		while (bDone == false) {
			if (bCanStepU) {
				//see if we can take another step in the U direction
				if (u + usize >= chunku) {
					bCanStepU = false;
					}
				else {
					for(tempv = v; tempv <= v + vsize; tempv++) {
						if (//not flat
							(intcompare(
							displacement[(u + usize + 1) * (chunkv + 1) + tempv]
							- displacement[(u + usize) * (chunkv + 1) + tempv])
							!= deltau)||
							//or already filled
							((bFilled[(u + usize) * (chunkv + 1) + tempv])
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
				if (v + vsize >= chunkv) {
					bCanStepV = false;
					}
				else {
					for(tempu = u; tempu <= u + usize; tempu++) {
						if (//not flat
							(intcompare(displacement[tempu * (chunkv + 1) + v + vsize + 1]
							- displacement[tempu * (chunkv + 1) + v + vsize])
							!= deltav)
							//or already filled
							||((bFilled[tempu * (chunkv + 1) + v + vsize])
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
				if ((intcompare(displacement[(u + usize +  1) * (chunkv + 1) + v + vsize + 1]
					-displacement[(u + usize) * (chunkv + 1) + v + vsize + 1]) == deltau)
					&&(intcompare(displacement[(u + usize +  1) * (chunkv + 1) + v + vsize + 1]
					- displacement[(u + usize +  1) * (chunkv + 1) + v + vsize]) == deltav)
					&&(!bFilled[(u + usize) * (chunkv + 1) + v + vsize]) ) {
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
				if (u + tempusize >= chunku) {
					bCanStepU = false;
					}
				else {
					for(tempv = v; tempv <= v + vsize; tempv++) {
						if (//not flat
							(intcompare(
							displacement[(u + tempusize + 1) * (chunkv + 1) + tempv]
							- displacement[(u + tempusize) * (chunkv + 1) + tempv])
							!= deltau)||
							//or already filled
							((bFilled[(u + tempusize) * (chunkv + 1) + tempv])
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
				if (v + tempvsize >= chunkv) {
					bCanStepV = false;
					}
				else {
					for(tempu = u; tempu <= u + usize; tempu++) {
						if (//not flat
							(intcompare(displacement[tempu * (chunkv + 1) + v + tempvsize + 1]
							- displacement[tempu * (chunkv + 1) + v + tempvsize])
							!= deltav)
							//or already filled
							||((bFilled[tempu * (chunkv + 1) + v + tempvsize])
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
			bFilled[tempu * (chunkv + 1) + tempv] = true;
			}
		}
	
	}

void cAGrPrimCube::DoMesh(DCGFacetMeshAccumulator* pAccu, TMCCountedPtr<I3DShShader> pShader) {
	accu = pAccu;
	shader = pShader;

	//for doing the shading
	ShadingIn shadingIn;
	boolean fullarea = false;
	fUVSpaceID = 0;
	shadingIn.fNormalLoc.x = 0;
	shadingIn.fNormalLoc.y = 0;
	shadingIn.fNormalLoc.z = 0;
	shadingIn.fUVSpaceID = fUVSpaceID;
	shadingIn.fCurrentCompletionMask = 0;

	//normals
	TVector3 topnormal(0,0,1), leftnormal(1, 0, 0)
		, rightnormal(-1, 0, 0), bottomnormal(0, 0, -1)
		, backnormal(0, -1, 0), frontnormal(0, 1, 0);


	uint32 u, v;
	chunku = fData.lU / 3;
	chunkv = fData.lV / 4;
	uint32 usize = 1, vsize = 1;
	real32 oneoverU = 1.0 / (real32)fData.lU;
	real32 oneoverV = 1.0 / (real32)fData.lV;

	ULines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
	VLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
	if (fData.lSplitMethod == SPLIT_SMART) {
		midULines = (real32*)MCcalloc(fData.lU + 1,sizeof(real32));
		midVLines = (real32*)MCcalloc(fData.lV + 1,sizeof(real32));
		midtoppoints = (TVector3*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVector3));
		midbottompoints = (TVector3*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVector3));
		midrightpoints = (TVector3*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVector3));
		midleftpoints = (TVector3*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVector3));
		midbackpoints = (TVector3*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVector3));
		midfrontpoints = (TVector3*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVector3));
		}

	topdisplacement = (real32*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(real32));
	bottomdisplacement = (real32*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(real32));
	rightdisplacement = (real32*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(real32));
	leftdisplacement = (real32*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(real32));
	frontdisplacement = (real32*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(real32));
	backdisplacement = (real32*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(real32));

	topfilled = (boolean*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(boolean));
	bottomfilled = (boolean*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(boolean));
	rightfilled = (boolean*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(boolean));
	leftfilled = (boolean*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(boolean));
	backfilled = (boolean*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(boolean));
	frontfilled = (boolean*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(boolean));

	toppoints = (TVertex3DLite*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVertex3DLite));
	bottompoints = (TVertex3DLite*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVertex3DLite));
	rightpoints = (TVertex3DLite*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVertex3DLite));
	leftpoints = (TVertex3DLite*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVertex3DLite));
	backpoints = (TVertex3DLite*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVertex3DLite));
	frontpoints = (TVertex3DLite*)MCcalloc((chunku + 1) * (chunkv + 1),sizeof(TVertex3DLite));

	//calculate XY and UV slices
	for (u = 0; u <= fData.lU; u ++){
		ULines[u] = u * oneoverU;
		if (fData.lSplitMethod == SPLIT_SMART) {
			midULines[u] = (u + 0.5) * oneoverU;
			}
		}
	for (v = 0; v <= fData.lV; v++) {
		VLines[v] = v * oneoverV;
		if (fData.lSplitMethod == SPLIT_SMART) {
			midVLines[v] = (v + 0.5) * oneoverV;
			}
		}
	//calculate the displacement
	for (u = 0; u <= chunku; u ++){
		for (v = 0; v <= chunkv; v++) {
			real32 fValue;
			uint32 workingu;
			uint32 workingv;
			uint32 index2d;
			real32 valong = (real32)v / (real32)chunkv;
			real32 ualong = (real32)u / (real32)chunku;
			real32 vvector = 1.0 - valong * 2.0;
			real32 uvector = 1.0 - ualong * 2.0;
			real32 midvalong = ((real32)v + 0.5) / (real32)chunkv;
			real32 midualong = ((real32)u + 0.5)/ (real32)chunku;
			real32 midvvector = 1.0 - midvalong * 2.0;
			real32 miduvector = 1.0 - midualong * 2.0;
			index2d = u * (chunkv + 1) + v;
			//do the top
			workingu = chunku + u;
			workingv = chunkv * 2 + v;
			shadingIn.fUV.x = ULines[workingu];
			shadingIn.fUV.y = VLines[workingv];
			shadingIn.fPointLoc.x = fData.sizeX * 0.5 - fData.sizeX * valong;
			shadingIn.fPointLoc.y = - fData.sizeY * 0.5 + fData.sizeY * ualong;
			shadingIn.fPointLoc.z = fData.sizeZ * 0.5;
			if (fData.bPreSmooth == true) {
				shadingIn.fNormalLoc.y = - uvector;
				shadingIn.fNormalLoc.x = vvector;
				shadingIn.fNormalLoc.z = 1.0;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				}
			else {
				shadingIn.fNormalLoc = topnormal;
				}
			shadingIn.fPoint = shadingIn.fPointLoc;
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			toppoints[index2d].fUV = shadingIn.fUV;
			if (shader != NULL) {
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				topdisplacement[index2d] = fValue;
				toppoints[index2d].fVertex = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			else {
				topdisplacement[index2d] = 0;
				toppoints[index2d].fVertex = shadingIn.fPointLoc;
				}
			topfilled[index2d] = false;
			if (fData.lSplitMethod == SPLIT_SMART) {
				shadingIn.fUV.x = midULines[workingu];
				shadingIn.fUV.y = midVLines[workingv];
				shadingIn.fPointLoc.x = fData.sizeX * 0.5 - fData.sizeX * midvalong;
				shadingIn.fPointLoc.y = - fData.sizeY * 0.5 + fData.sizeY * midualong;
				shadingIn.fPointLoc.z = fData.sizeZ * 0.5;
				if (fData.bPreSmooth == true) {
					shadingIn.fNormalLoc.y = - miduvector;
					shadingIn.fNormalLoc.x = midvvector;
					shadingIn.fNormalLoc.z = 1.0;
					shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
				else {
					shadingIn.fNormalLoc = topnormal;
					}
				shadingIn.fPoint = shadingIn.fPointLoc;
				shadingIn.fGNormal = shadingIn.fNormalLoc;
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				midtoppoints[index2d] = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;

				}
			//do the left
			workingu = chunku + u;
			workingv = chunkv + v;
			shadingIn.fUV.x = ULines[workingu];
			shadingIn.fUV.y = VLines[workingv];
			shadingIn.fPointLoc.x = fData.sizeX * 0.5;
			shadingIn.fPointLoc.y = - fData.sizeY * 0.5 + fData.sizeY * ualong;
			shadingIn.fPointLoc.z = - fData.sizeZ * 0.5 + fData.sizeZ * valong;
			if (fData.bPreSmooth == true) {
				shadingIn.fNormalLoc.y = - uvector;
				shadingIn.fNormalLoc.z = - vvector;
				shadingIn.fNormalLoc.x = 1.0;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				}
			else {
				shadingIn.fNormalLoc = leftnormal;
				}
			shadingIn.fPoint = shadingIn.fPointLoc;
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			leftpoints[index2d].fUV = shadingIn.fUV;
			if (shader != NULL) {
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				leftdisplacement[index2d] = fValue;
				leftpoints[index2d].fVertex = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			else {
				leftdisplacement[index2d] = 0;
				leftpoints[index2d].fVertex = shadingIn.fPointLoc;
				}
			leftfilled[index2d] = false;
			if (fData.lSplitMethod == SPLIT_SMART) {
				shadingIn.fUV.x = midULines[workingu];
				shadingIn.fUV.y = midVLines[workingv];
				shadingIn.fPointLoc.x = fData.sizeX * 0.5;
				shadingIn.fPointLoc.y = - fData.sizeY * 0.5 + fData.sizeY * midualong;
				shadingIn.fPointLoc.z = - fData.sizeZ * 0.5 + fData.sizeZ * midvalong;
				if (fData.bPreSmooth == true) {
					shadingIn.fNormalLoc.y = - miduvector;
					shadingIn.fNormalLoc.z = - midvvector;
					shadingIn.fNormalLoc.x = 1.0;
					shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
				else {
					shadingIn.fNormalLoc = leftnormal;
					}
				shadingIn.fPoint = shadingIn.fPointLoc;
				shadingIn.fGNormal = shadingIn.fNormalLoc;
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				midleftpoints[index2d] = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			//do the right
			workingu = chunku + u;
			workingv = chunkv * 3 + v;
			shadingIn.fUV.x = ULines[workingu];
			shadingIn.fUV.y = VLines[workingv];
			shadingIn.fPointLoc.x =  - fData.sizeX * 0.5;
			shadingIn.fPointLoc.y = - fData.sizeY * 0.5 + fData.sizeY * ualong;
			shadingIn.fPointLoc.z = fData.sizeZ * 0.5 - fData.sizeZ * valong;
			if (fData.bPreSmooth == true) {
				shadingIn.fNormalLoc.y = - uvector;
				shadingIn.fNormalLoc.z = vvector;
				shadingIn.fNormalLoc.x = -1.0;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				}
			else {
				shadingIn.fNormalLoc = rightnormal;
				}
			shadingIn.fPoint = shadingIn.fPointLoc;
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			rightpoints[index2d].fUV = shadingIn.fUV;
			if (shader != NULL) {
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				rightdisplacement[index2d] = fValue;
				rightpoints[index2d].fVertex = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			else {
				rightdisplacement[index2d] = 0;
				rightpoints[index2d].fVertex = shadingIn.fPointLoc;
				}
			rightfilled[index2d] = false;
			if (fData.lSplitMethod == SPLIT_SMART) {
				shadingIn.fUV.x = midULines[workingu];
				shadingIn.fUV.y = midVLines[workingv];
				shadingIn.fPointLoc.x =  - fData.sizeX * 0.5;
				shadingIn.fPointLoc.y = - fData.sizeY * 0.5 + fData.sizeY * midualong;
				shadingIn.fPointLoc.z = fData.sizeZ * 0.5 - fData.sizeZ * midvalong;
				if (fData.bPreSmooth == true) {
					shadingIn.fNormalLoc.y = - miduvector;
					shadingIn.fNormalLoc.z = midvvector;
					shadingIn.fNormalLoc.x = -1.0;
					shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
				else {
					shadingIn.fNormalLoc = rightnormal;
					}
				shadingIn.fPoint = shadingIn.fPointLoc;
				shadingIn.fGNormal = shadingIn.fNormalLoc;
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				midrightpoints[index2d] = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			//do the bottom
			workingu = chunku + u;
			workingv = v;
			shadingIn.fUV.x = ULines[workingu];
			shadingIn.fUV.y = VLines[workingv];
			shadingIn.fPointLoc.x = - fData.sizeX * 0.5 + fData.sizeX * valong;
			shadingIn.fPointLoc.y = - fData.sizeY * 0.5 + fData.sizeY * ualong;
			shadingIn.fPointLoc.z = - fData.sizeZ * 0.5;
			if (fData.bPreSmooth == true) {
				shadingIn.fNormalLoc.y = - uvector;
				shadingIn.fNormalLoc.x = - vvector;
				shadingIn.fNormalLoc.z = - 1.0;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				}
			else {
				shadingIn.fNormalLoc = bottomnormal;
				}
			shadingIn.fPoint = shadingIn.fPointLoc;
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			bottompoints[index2d].fUV = shadingIn.fUV;
			if (shader != NULL) {
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				bottomdisplacement[index2d] = fValue;
				bottompoints[index2d].fVertex = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			else {
				bottomdisplacement[index2d] = 0;
				bottompoints[index2d].fVertex = shadingIn.fPointLoc;
				}
			bottomfilled[index2d] = false;
			if (fData.lSplitMethod == SPLIT_SMART) {
				shadingIn.fUV.x = midULines[workingu];
				shadingIn.fUV.y = midVLines[workingv];
				shadingIn.fPointLoc.x = - fData.sizeX * 0.5 + fData.sizeX * midvalong;
				shadingIn.fPointLoc.y = - fData.sizeY * 0.5 + fData.sizeY * midualong;
				shadingIn.fPointLoc.z = - fData.sizeZ * 0.5;
				if (fData.bPreSmooth == true) {
					shadingIn.fNormalLoc.y = - miduvector;
					shadingIn.fNormalLoc.x = - midvvector;
					shadingIn.fNormalLoc.z = - 1.0;
					shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
				else {
					shadingIn.fNormalLoc = bottomnormal;
					}
				shadingIn.fPoint = shadingIn.fPointLoc;
				shadingIn.fGNormal = shadingIn.fNormalLoc;
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				midbottompoints[index2d] = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			//do the front
			workingu = chunku * 2 + u;
			workingv = chunkv + v;
			shadingIn.fUV.x = ULines[workingu];
			shadingIn.fUV.y = VLines[workingv];
			shadingIn.fPointLoc.x = fData.sizeX * 0.5 - fData.sizeX * ualong;
			shadingIn.fPointLoc.y = fData.sizeY * 0.5;
			shadingIn.fPointLoc.z = - fData.sizeZ * 0.5 + fData.sizeZ * valong;
			if (fData.bPreSmooth == true) {
				shadingIn.fNormalLoc.x = uvector;
				shadingIn.fNormalLoc.z = - vvector;
				shadingIn.fNormalLoc.y =1.0;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				}
			else {
				shadingIn.fNormalLoc = frontnormal;
				}
			shadingIn.fPoint = shadingIn.fPointLoc;
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			frontpoints[index2d].fUV = shadingIn.fUV;
			if (shader != NULL) {
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				frontdisplacement[index2d] = fValue;
				frontpoints[index2d].fVertex = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			else {
				frontdisplacement[index2d] = 0;
				frontpoints[index2d].fVertex = shadingIn.fPointLoc;
				}
			frontfilled[index2d] = false;
			if (fData.lSplitMethod == SPLIT_SMART) {
				shadingIn.fUV.x = midULines[workingu];
				shadingIn.fUV.y = midVLines[workingv];
				shadingIn.fPointLoc.x = fData.sizeX * 0.5 - fData.sizeX * midualong;
				shadingIn.fPointLoc.y = fData.sizeY * 0.5;
				shadingIn.fPointLoc.z = - fData.sizeZ * 0.5 + fData.sizeZ * midvalong;
				if (fData.bPreSmooth == true) {
					shadingIn.fNormalLoc.x = miduvector;
					shadingIn.fNormalLoc.z = - midvvector;
					shadingIn.fNormalLoc.y =1.0;
					shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
				else {
					shadingIn.fNormalLoc = frontnormal;
					}
				shadingIn.fPoint = shadingIn.fPointLoc;
				shadingIn.fGNormal = shadingIn.fNormalLoc;
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				midfrontpoints[index2d] = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			//do the back
			workingu = u;
			workingv = chunkv + v;
			shadingIn.fUV.x = ULines[workingu];
			shadingIn.fUV.y = VLines[workingv];
			shadingIn.fPointLoc.x = - fData.sizeX * 0.5 + fData.sizeX * ualong;
			shadingIn.fPointLoc.y = - fData.sizeY * 0.5;
			shadingIn.fPointLoc.z = - fData.sizeZ * 0.5 + fData.sizeZ * valong;
			if (fData.bPreSmooth == true) {
				shadingIn.fNormalLoc.x = - uvector;
				shadingIn.fNormalLoc.z = - vvector;
				shadingIn.fNormalLoc.y = -1.0;
				shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
				}
			else {
				shadingIn.fNormalLoc = backnormal;
				}
			shadingIn.fPoint = shadingIn.fPointLoc;
			shadingIn.fGNormal = shadingIn.fNormalLoc;
			backpoints[index2d].fUV = shadingIn.fUV;
			if (shader != NULL) {
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				backdisplacement[index2d] = fValue;
				backpoints[index2d].fVertex = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			else {
				backdisplacement[index2d] = 0;
				backpoints[index2d].fVertex = shadingIn.fPointLoc;
				}
			backfilled[index2d] = false;
			if (fData.lSplitMethod == SPLIT_SMART) {
				shadingIn.fUV.x = midULines[workingu];
				shadingIn.fUV.y = midVLines[workingv];
				shadingIn.fPointLoc.x = - fData.sizeX * 0.5 + fData.sizeX * midualong;
				shadingIn.fPointLoc.y = - fData.sizeY * 0.5;
				shadingIn.fPointLoc.z = - fData.sizeZ * 0.5 + fData.sizeZ * midvalong;
				if (fData.bPreSmooth == true) {
					shadingIn.fNormalLoc.x = - miduvector;
					shadingIn.fNormalLoc.z = - midvvector;
					shadingIn.fNormalLoc.y = -1.0;
					shadingIn.fNormalLoc.Normalize(shadingIn.fNormalLoc);
					}
				else {
					shadingIn.fNormalLoc = backnormal;
					}
				shadingIn.fPoint = shadingIn.fPointLoc;
				shadingIn.fGNormal = shadingIn.fNormalLoc;
				shader->GetValue(fValue, fullarea, shadingIn);
				fValue = fValue * fData.fStop + (1 - fValue) * fData.fStart;
				midbackpoints[index2d] = shadingIn.fPointLoc +  shadingIn.fNormalLoc * fValue;
				}
			}
		}

	if (fData.bAdaptiveMesh) {
		for (u = 0; u < chunku; u ++){
			for (v = 0; v < chunkv; v++) {
				if (topfilled[u * (chunkv + 1) + v] == false) {
					GetBlockSize(topfilled, topdisplacement, u, v, usize, vsize);
					AddQuad(topdisplacement, toppoints, u, v, usize, vsize, midtoppoints);
					}
				if (bottomfilled[u * (chunkv + 1) + v] == false) {
					GetBlockSize(bottomfilled, bottomdisplacement, u, v, usize, vsize);
					AddQuad(bottomdisplacement, bottompoints, u, v, usize, vsize, midbottompoints);
					}
				if (rightfilled[u * (chunkv + 1) + v] == false) {
					GetBlockSize(rightfilled, rightdisplacement, u, v, usize, vsize);
					AddQuad(rightdisplacement, rightpoints, u, v, usize, vsize, midrightpoints);
					}
				if (leftfilled[u * (chunkv + 1) + v] == false) {
					GetBlockSize(leftfilled, leftdisplacement, u, v, usize, vsize);
					AddQuad(leftdisplacement, leftpoints, u, v, usize, vsize, midleftpoints);
					}
				if (frontfilled[u * (chunkv + 1) + v] == false) {
					GetBlockSize(frontfilled, frontdisplacement, u, v, usize, vsize);
					AddQuad(frontdisplacement, frontpoints, u, v, usize, vsize, midfrontpoints);
					}
				if (backfilled[u * (chunkv + 1) + v] == false) {
					GetBlockSize(backfilled, backdisplacement, u, v, usize, vsize);
					AddQuad(backdisplacement, backpoints, u, v, usize, vsize, midbackpoints);
					}
				}
			}

		}
	else {//not adaptive mesh
		for (u = 0; u < chunku; u ++){
			for (v = 0; v < chunkv; v++) {
				//do the top
				AddQuad(topdisplacement, toppoints, u, v, 1, 1, midtoppoints);
				//do the left
				AddQuad(leftdisplacement, leftpoints, u, v, 1, 1, midleftpoints);
				//do the right
				AddQuad(rightdisplacement, rightpoints, u, v, 1, 1, midrightpoints);
				//do the bottom
				AddQuad(bottomdisplacement, bottompoints, u, v, 1, 1, midbottompoints);
				//do the front
				AddQuad(frontdisplacement, frontpoints, u, v, 1, 1, midfrontpoints);
				//do the back
				AddQuad(backdisplacement, backpoints, u, v, 1, 1, midbackpoints);
				}
			}
		}

	//clean up
	CleanUp();
	}


inline void cAGrPrimCube::AddFacet(const TVertex3DLite& pt1,const TVertex3DLite& pt2,const TVertex3DLite& pt3) {
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

void cAGrPrimCube::AddQuad(const real32* displacement, const TVertex3DLite* points, const int32& u, const int32& v
						   , const int32& usize, const int32& vsize
						   , const TVector3* midpoints) {
	TVertex3DLite bottomright, bottomleft, topleft, topright;

	int32 bottomrightindex = (u + usize) * (chunkv + 1) + v
		, bottomleftindex = u * (chunkv + 1) + v
		, topleftindex = u * (chunkv + 1) + v + vsize
		, toprightindex = (u + usize) * (chunkv + 1) + v + vsize;

	bottomright = points[bottomrightindex];

	bottomleft = points[bottomleftindex];

	topleft = points[topleftindex];

	topright = points[toprightindex];

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
		TVector3 brtotl;
		TVector3 bltotr;

		brtotl = (bottomright.fVertex + topleft.fVertex) * 0.5;
		bltotr = (bottomleft.fVertex + topright.fVertex) * 0.5;
		if (sqr(bltotr,midpoints[bottomleftindex]) 
			< sqr(brtotl, midpoints[bottomleftindex])) {
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

void cAGrPrimCube::GetBoundingBox(TBBox3D& bbox)
{
	bbox.fMin[0]= -fData.sizeX * 0.5 + fData.fStop;
	bbox.fMax[0]= fData.sizeX * 0.5 + fData.fStop;
	bbox.fMin[1]= -fData.sizeY * 0.5 + fData.fStop;
	bbox.fMax[1]= fData.sizeY * 0.5 + fData.fStop;
	bbox.fMin[2]= -fData.sizeZ * 0.5 + fData.fStop;
	bbox.fMax[2]= fData.sizeZ * 0.5 + fData.fStop;
}

uint32 cAGrPrimCube::GetUVSpaceCount(){
	return 1;
	}

MCCOMErr cAGrPrimCube::GetUVSpace(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo)
{
	if (uvSpaceID == 0)
	{ 
		uvSpaceInfo->fWraparound[0] = false;  // No Wrap around
		uvSpaceInfo->fWraparound[1] = false;
	}
	return MC_S_OK;
}

