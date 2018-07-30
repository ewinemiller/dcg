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
#include "HairGenerator.h"
#include "stdlib.h"
#include "IMFTextPopupPart.h" 
#include "I3DShCamera.h"
#include "I3DShTreeElement.h"
#include "I3DRenderingModule.h"
#include "ISceneDocument.h"
#include "IShTextureMap.h"
#include "IShChannel.h"
#include "comutilities.h"
#include "apitypes.h"
#include "IMFPart.h"
#include "genrand.h"
#include "copyright.h"

inline real32 sqr(real32 pfIn){ 
	return pfIn * pfIn;
	}

inline real32 sqr(const TVector3 &first,const TVector3 &second){
	return sqr(first.x - second.x) 
		+ sqr(first.y - second.y) 
		+ sqr(first.z - second.z);
} 

inline real32 distance(const TVector3 &first,const TVector3 &second)
{
	return sqrt(sqr(first, second));
	
}

inline real32 intcompare(const real32 &in) {
	return floorf(in * 10000 + .5);
	}
inline real32 filter(const real32 &in) {
	return intcompare(in) / (real32)10000;
	}

void HairGenerator::BuildTransform(TTransform3D& transform, const real32& fZRotation, const real32& fYRotation)
{
	TTransform3D TempRotation;

	real32 costheta, sintheta;

	//rotate around the z-axis until we are lined up with xz plane
	sintheta = sin(fZRotation);
	costheta = cos(fZRotation);

	TempRotation.fRotationAndScale[0][0] = costheta;
	TempRotation.fRotationAndScale[0][1] = sintheta;
	TempRotation.fRotationAndScale[0][2] = 0;
	TempRotation.fRotationAndScale[1][0] = - sintheta;
	TempRotation.fRotationAndScale[1][1] = costheta;
	TempRotation.fRotationAndScale[1][2] = 0;
	TempRotation.fRotationAndScale[2][0] = 0;
	TempRotation.fRotationAndScale[2][1] = 0;
	TempRotation.fRotationAndScale[2][2] = 1;

	transform.fRotationAndScale = transform.fRotationAndScale * TempRotation.fRotationAndScale;

	//now use the working direction to figure out how far around the y axis we must rotate
	if (fYRotation != 0)
	{
		costheta = cos(fYRotation);
		sintheta = sin(fYRotation);

		TempRotation.fRotationAndScale[0][0] = costheta;
		TempRotation.fRotationAndScale[0][1] = 0;
		TempRotation.fRotationAndScale[0][2] = - sintheta;
		TempRotation.fRotationAndScale[1][0] = 0;
		TempRotation.fRotationAndScale[1][1] = 1;
		TempRotation.fRotationAndScale[1][2] = 0;
		TempRotation.fRotationAndScale[2][0] = sintheta;
		TempRotation.fRotationAndScale[2][1] = 0;
		TempRotation.fRotationAndScale[2][2] = costheta;

		transform.fRotationAndScale = transform.fRotationAndScale * TempRotation.fRotationAndScale;
	}

	//finally twist back around the z axis to go back to our original space
	sintheta = sin(-fZRotation);
	costheta = cos(-fZRotation);

	TempRotation.fRotationAndScale[0][0] = costheta;
	TempRotation.fRotationAndScale[0][1] = sintheta;
	TempRotation.fRotationAndScale[0][2] = 0;
	TempRotation.fRotationAndScale[1][0] = - sintheta;
	TempRotation.fRotationAndScale[1][1] = costheta;
	TempRotation.fRotationAndScale[1][2] = 0;
	TempRotation.fRotationAndScale[2][0] = 0;
	TempRotation.fRotationAndScale[2][1] = 0;
	TempRotation.fRotationAndScale[2][2] = 1;

	transform.fRotationAndScale = transform.fRotationAndScale * TempRotation.fRotationAndScale;

}

//take an existing facet and make it furry
void HairGenerator::AddStrands(const TFacet3D& aFacet, const TFacet3D& originalFacet, const uint32 lNumFacets, 
				AnythingGrowsStrands** strands, 
				GenerationMode generationMode, uint32& currentStrand, uint32& currentVertex) 
{
	TVertex3D originalstartingpoint;
	TVertex3D startingpoint;
	TVertex3D originalendpoint1;
	TVertex3D originalendpoint2; 
	TVertex3D endpoint1;
	TVertex3D endpoint2; 
	TVector3 startdirection;
	TVector3 currentdirection;
	TVector3 gravity = gravitydirection * fData.fGravity;
	TVector3 Wiggle(0, 0, 0); 
	TVector3 CustomStart(0, 0, 0); 
	TVertex3D originalpt[3]; 
	TVertex3D startpt[3]; 
	TTransform3D flattransform;
	TTransform3D TempRotation;

	real32 fLength = fData.fLength;
	real32 fWidth = fData.fWidth * 0.5f;
	real32 fStiffness = fData.fStiffness;
	real32 fCurrentLength;
	real32 fTempLength;
	real32 fAlong;
	real32 fEdgeLength[3];

	uint32 lNumSegments;
	uint32 lSegment;
	boolean bHaveStarting, bTipIsZero;
	real32 x, y, curvex, fSegmentLength;

	//move this facet to a flat plane so that we may generate 
	//random 2d coordinates on that plane easily
	fEdgeLength[0] = sqr(originalFacet.fVertices[0].fVertex, originalFacet.fVertices[1].fVertex);
	fEdgeLength[1] = sqr(originalFacet.fVertices[1].fVertex, originalFacet.fVertices[2].fVertex);
	fEdgeLength[2] = sqr(originalFacet.fVertices[2].fVertex, originalFacet.fVertices[0].fVertex);

	if ((fEdgeLength[0] > fEdgeLength[1])&&(fEdgeLength[0] > fEdgeLength[2])) 
	{
		originalpt[0] = originalFacet.fVertices[0];
		originalpt[1] = originalFacet.fVertices[1];
		originalpt[2] = originalFacet.fVertices[2];
		startpt[0] = aFacet.fVertices[0];
		startpt[1] = aFacet.fVertices[1];
		startpt[2] = aFacet.fVertices[2];

	}
	else if ((fEdgeLength[1] > fEdgeLength[0])&&(fEdgeLength[1] > fEdgeLength[2])) 
	{
		originalpt[0] = originalFacet.fVertices[1];
		originalpt[1] = originalFacet.fVertices[2];
		originalpt[2] = originalFacet.fVertices[0];
		startpt[0] = aFacet.fVertices[1];
		startpt[1] = aFacet.fVertices[2];
		startpt[2] = aFacet.fVertices[0];

	}
	else
	{
		originalpt[0] = originalFacet.fVertices[2];
		originalpt[1] = originalFacet.fVertices[0];
		originalpt[2] = originalFacet.fVertices[1];
		startpt[0] = aFacet.fVertices[2];
		startpt[1] = aFacet.fVertices[0];
		startpt[2] = aFacet.fVertices[1];

	}
	FillFlat(originalpt[0].fVertex, originalpt[1].fVertex, originalpt[2].fVertex, flattransform);

	//generate some random points in 2d space bounded by the edges of this facet
	//check to see if they are within the facet and use them if they are
	for (uint32 lHair = 0; lHair < lNumFacets; lHair++, currentStrand++)
	{
		bTipIsZero = false;
		bHaveStarting = false;
		do 
		{
			//get some random coordinates
			x = static_cast<real32>(rand()) / static_cast<real32>(RAND_MAX) * originalpt[1].fVertex.y;
			y = static_cast<real32>(rand()) / static_cast<real32>(RAND_MAX) * originalpt[1].fVertex.y;

			//see if they are under our curve
			if (y < originalpt[2].fVertex.y) 
			{
				fAlong = y / originalpt[2].fVertex.y;
				curvex = fAlong * originalpt[2].fVertex.x;
				if (x <= curvex)
				{
					//figure out normal and UV
					bHaveStarting = true;
					FillPoint(y / originalpt[1].fVertex.y, originalpt[0], originalpt[1], originalendpoint1);
					FillPoint(fAlong, originalpt[0], originalpt[2], originalendpoint2);
					FillPoint(x / originalendpoint2.fVertex.x, originalendpoint1, originalendpoint2, originalstartingpoint);

					FillPoint(y / originalpt[1].fVertex.y, startpt[0], startpt[1], endpoint1);
					FillPoint(fAlong, startpt[0], startpt[2], endpoint2);
					FillPoint(x / originalendpoint2.fVertex.x, endpoint1, endpoint2, startingpoint);
					
				}

			}
			else if (y > originalpt[2].fVertex.y)
			{
				fAlong = (originalpt[1].fVertex.y - y) / (originalpt[1].fVertex.y - originalpt[2].fVertex.y);
				curvex = fAlong * originalpt[2].fVertex.x;
				if (x <= curvex)
				{
					//figure out normal and UV
					bHaveStarting = true;
					FillPoint(y / originalpt[1].fVertex.y, originalpt[0], originalpt[1], originalendpoint1);
					FillPoint(fAlong, originalpt[1], originalpt[2], originalendpoint2);
					FillPoint(x / originalendpoint2.fVertex.x, originalendpoint1, originalendpoint2, originalstartingpoint);

					FillPoint(y / originalpt[1].fVertex.y, startpt[0], startpt[1], endpoint1);
					FillPoint(fAlong, startpt[1], startpt[2], endpoint2);
					FillPoint(x / originalendpoint2.fVertex.x, endpoint1, endpoint2, startingpoint);
				}
			}
			else if (x <= originalpt[2].fVertex.x) 
			{
				bHaveStarting = true;
				//figure out normal and UV
				FillPoint(y / originalpt[1].fVertex.y, originalpt[0], originalpt[1], originalendpoint1);
				FillPoint(x / originalpt[2].fVertex.x, originalendpoint1, originalpt[2], originalstartingpoint);

				FillPoint(y / originalpt[1].fVertex.y, startpt[0], startpt[1], endpoint1);
				FillPoint(x / originalpt[2].fVertex.x, endpoint1, startpt[2], startingpoint);
			}
			//if we have the starting, transform back to the original space
			if (bHaveStarting) 
			{
				TVector3 temppoint;
				LocalToGlobal(flattransform, originalstartingpoint.fVertex, temppoint);
				originalstartingpoint.fVertex = temppoint;

			}
		}
		while (bHaveStarting == false);

		FillShadingIn(startingpoint, originalstartingpoint);
		
		if (shader != NULL) 
		{
			fLength = GetLength() * fData.fLength;
		}

		if (widthshader != NULL) 
		{
			fWidth = GetWidth() * fData.fWidth * 0.5f;
		}

		if (stiffnessshader != NULL) 
		{
			fStiffness = GetStiffness() * fData.fStiffness;
		}

		if (fLength > 0) 
		{
			fCurrentLength = 0;
			//figure out the points on our profile
			if (fData.lSides > 2)
			{
				//call it and throw it away so that we match the # of rand calls as a 2 sided strand
				real32 fThrowawayAngle = static_cast<real32>(rand());
				if (fData.lUVMapping == uvBase)
				{
					for (uint32 lSide = 0; lSide < fData.lSides;lSide++)
					{
						//draw the points around the origin in XY plane
						startpoints[lSide].fUV = startingpoint.fUV;
					}
				}
			}
			else
			{
				//randomly generate an angle and twist to that point
				real32 fAngle = static_cast<real32>(rand()) / static_cast<real32>(RAND_MAX) * TWO_PI;
				real32 fCos = cos(fAngle), fSin = sin(fAngle);
				startpoints[0].fVertex.x = fCos;
				startpoints[0].fVertex.y = fSin;
				startpoints[0].fVertex.z = 0;
				startpoints[0].fNormal.x = -fSin; 
				startpoints[0].fNormal.y = fCos;
				startpoints[0].fNormal.z = 0; 
				if (fData.lUVMapping == uvBase)
				{
					startpoints[0].fUV = startingpoint.fUV;
				}

				fAngle += static_cast<real32>(PI);
				fCos = cos(fAngle);
				fSin = sin(fAngle);
				startpoints[1].fVertex.x = fCos;
				startpoints[1].fVertex.y = fSin;
				startpoints[1].fVertex.z = 0;
				startpoints[1].fNormal = startpoints[0].fNormal;

				if (fData.lUVMapping == uvBase)
				{
					startpoints[1].fUV = startingpoint.fUV;
				}
			}

			if (fData.fWiggle > 0) 
			{
				//if not overridden by the shaders, give a random wiggle
				Wiggle.x = (-1.0  + 2.0 * static_cast<real32>(rand()) / static_cast<real32>(RAND_MAX));
				Wiggle.y = (-1.0  + 2.0 * static_cast<real32>(rand()) / static_cast<real32>(RAND_MAX));
				Wiggle.z = (-1.0  + 2.0 * static_cast<real32>(rand()) / static_cast<real32>(RAND_MAX));

				if (xwiggleshader != NULL) 
				{
					Wiggle.x = (-1.0  + 2.0 * GetXWiggle());
				}
					
				if (ywiggleshader != NULL) 
				{
					Wiggle.y = (-1.0  + 2.0 * GetYWiggle());
				}
				
				if (zwiggleshader != NULL) 
				{
					Wiggle.z = (-1.0  + 2.0 * GetZWiggle());
				}

				Wiggle *= fData.fWiggle;

				//Wiggle.Normalize(Wiggle);
			}

			//initialize direction
			startdirection = startingpoint.fNormal;

			if (fData.fCustomVector > 0)
			{
				if (xcustomvectorshader != NULL) 
				{
					CustomStart.x = (-1.0  + 2.0 * GetXCustomVector());
				}
					
				if (ycustomvectorshader != NULL) 
				{
					CustomStart.y = (-1.0  + 2.0 * GetYCustomVector());
				}
				
				if (zcustomvectorshader != NULL) 
				{
					CustomStart.z = (-1.0  + 2.0 * GetZCustomVector());
				}

				if ((CustomStart.x != 0)||(CustomStart.y != 0)||(CustomStart.z != 0))
				{
					CustomStart.Normalize(CustomStart);
					TVector3 difference = originalstartingpoint.fNormal - CustomStart;
					CustomStart = startdirection - difference;
					CustomStart.Normalize(CustomStart);
					CustomStart = CustomStart * fData.fCustomVector + startdirection * (1 - fData.fCustomVector);
					startdirection = CustomStart;
				}
			}

			currentdirection = startdirection;

			fSegmentLength = fData.fLength / static_cast<real32>(fData.lSegments);

			//put aside the first set of points
			directionarray[0] = currentdirection;
			linecenter[0] = startingpoint.fVertex;

			real32 fTotalTaperEffect = (1.0 - fData.fTaper) / 2.0 * fData.fWidth;

			if (generationMode == GenerateStrands) {
				(*strands)->strandUV[currentStrand] = startingpoint.fUV;
				(*strands)->uvSpaceID[currentStrand] = aFacet.fUVSpace;
			}		

			//build build points and direction loop
			for (lSegment = 0; (lSegment < fData.lSegments)&&(fCurrentLength < fLength); lSegment++){
				real32 fTowardsDown;

				//keep track of the segments we processed
				lNumSegments = lSegment;

				if (fLength - fCurrentLength < fSegmentLength) 
				{
					fTempLength = fLength - fCurrentLength;
				}
				else
				{
					fTempLength = fSegmentLength;
				}

				//if there is gravity, set the direction to what it is for this segment
				fTowardsDown =  
						(static_cast<real32>(lSegment) - static_cast<real32>(fData.lStartEffects - 1)) 
						/ static_cast<real32>(fData.lSegments);
				if (fTowardsDown < 0) 
				{
					fTowardsDown = 0;
				}
				else
				if (fTowardsDown > 1)
				{
					fTowardsDown = 1;
				}

				fTowardsDown *= (1 - fStiffness);

				if (fData.fWiggle > 0 )
				{	
					currentdirection = fTowardsDown * Wiggle + (1 - fTowardsDown) * startdirection;
					currentdirection.Normalize(currentdirection);
				}
				if (gravity.GetMagnitude()  > 0)
				{
					currentdirection = fTowardsDown * gravity + (1 - fTowardsDown) * currentdirection;
					currentdirection.Normalize(currentdirection);
				}

				//put the points away for later use
				directionarray[lSegment + 1] = currentdirection;
				linecenter[lSegment + 1] = linecenter[lSegment] + directionarray[lSegment] * fTempLength;
				
				fCurrentLength += fTempLength;
			}//end build points and direction loop
			
			if (generationMode == GenerateStrands) {
				(*strands)->vertexCounts[currentStrand] = lNumSegments + 1;

				for (lSegment = 0; lSegment <= lNumSegments + 1; lSegment++, currentVertex++) {
					real32 fCurrentEffect;
					(*strands)->vertices[currentVertex] = linecenter[lSegment];
					if (fData.lTaperMode == tmPossible) 
					{
						fCurrentEffect = static_cast<real32>(lSegment)
							/static_cast<real32>(fData.lSegments);
					}
					else
					{
						fCurrentEffect = static_cast<real32>(lSegment)
							/static_cast<real32>(lNumSegments + 1);
					}
					(*strands)->vertexWidths[currentVertex] = fWidth * (fData.fTaper + (1 - fData.fTaper) * (1 - fCurrentEffect));
				}
			}
			
			if (generationMode == GenerateMesh) {
			
				//calculate the rotation
				for (lSegment = 0; lSegment <= lNumSegments + 1; lSegment++)
				{
					TTransform3D TempRotation;
					TVector3 workingdirection;
					TVector3 direction;
					real32 sintheta, costheta;

					direction = directionarray[lSegment];

					if ((filter(direction.x) != 0)||(filter(direction.y) != 0))
					{
						real32 fSin;			
						//rotate around the z-axis until we are lined up with xz plane
						fSin = direction.y / sqrt(sqr(direction.x) + sqr(direction.y));
						if (fSin > 1) {
							fSin = 1;
							}
						else if(fSin < -1) {
							fSin = -1;
							}
						fZRotation[lSegment] = asin(fSin);
						if (direction.x > 0){
							fZRotation[lSegment] = PI - fZRotation[lSegment];
						}

						sintheta = sin(fZRotation[lSegment]);
						costheta = cos(fZRotation[lSegment]);
						TempRotation.fRotationAndScale[0][0] = costheta;
						TempRotation.fRotationAndScale[0][1] = sintheta;
						TempRotation.fRotationAndScale[0][2] = 0;
						TempRotation.fRotationAndScale[1][0] = - sintheta;
						TempRotation.fRotationAndScale[1][1] = costheta;
						TempRotation.fRotationAndScale[1][2] = 0;
						TempRotation.fRotationAndScale[2][0] = 0;
						TempRotation.fRotationAndScale[2][1] = 0;
						TempRotation.fRotationAndScale[2][2] = 1;

						GlobalToLocalVector(TempRotation, direction, workingdirection);
						if ((filter(workingdirection.x) != 0)||(filter(workingdirection.z) != 0))
						{
							fSin = workingdirection.x / sqrt(sqr(workingdirection.x) + sqr(workingdirection.z));
							if (fSin > 1) 
							{
								fSin = 1;
							}
							else if(fSin < -1) 
							{
								fSin = -1;
							}

							fYRotation[lSegment] = -asin(fSin);
							if (workingdirection.z < 0) {
								fYRotation[lSegment] = PI - fYRotation[lSegment];
								}
						}
						else
						{
							fYRotation[lSegment] = 0;
						}
					}
					else
					{
						fZRotation[lSegment] = 0;
						if (direction.z < 0)
						{
							fYRotation[lSegment] = static_cast<real32>(PI);
						}
						else
						{
							fYRotation[lSegment] = 0;
						}
						
					}
				}

				//search through the rotation and see if there are any that are zero
				for (lSegment = 0; lSegment <= lNumSegments + 1; lSegment++)
				{
					uint32 lStartSegment, lEndSegment, lInnerSegment;

					if (fZRotation[lSegment] == 0)
					{
						if (lSegment == lNumSegments + 1)
						{
							//if it's the last one, just grab the previous
							fZRotation[lSegment] = fZRotation[lSegment - 1];
						} 
						else 
						{
							//if not, look for the end of the run
							lStartSegment = lSegment;
							lEndSegment = lSegment;
							for (lInnerSegment = lSegment + 1; lInnerSegment <= lNumSegments + 1; lInnerSegment++)
							{
								if (fZRotation[lInnerSegment] == 0)
								{
									lEndSegment = lInnerSegment;
								}
								else
								{
									lInnerSegment = lNumSegments + 1;
								}
							}
							//now that we have the start and end, do the appropriate thing
							if ((lStartSegment == 0)&&(lEndSegment == lNumSegments + 1))
							{
									//if they are all zeros, doesn't matter
							}
							else if (lStartSegment == 0)
							{
								//if zeros start at the beginning, copy end + 1 to everything zero
								for (lInnerSegment = lStartSegment; lInnerSegment <= lEndSegment; lInnerSegment++)
								{
									fZRotation[lInnerSegment] = fZRotation[lEndSegment + 1];
								}
							}
							else if (lEndSegment == lNumSegments + 1)
							{
								//if zeros fill until end, copy start - 1 to everything zero
								for (lInnerSegment = lStartSegment; lInnerSegment <= lEndSegment; lInnerSegment++)
								{
									fZRotation[lInnerSegment] = fZRotation[lStartSegment - 1];
								}

							}
							else
							{
								//make a gradient from start - 1 to end + 1
								for (lInnerSegment = lStartSegment; lInnerSegment <= lEndSegment; lInnerSegment++)
								{
									fZRotation[lInnerSegment] = fZRotation[lStartSegment - 1] 
										+ (fZRotation[lEndSegment + 1] - fZRotation[lStartSegment - 1]) 
										* (static_cast<real32>(lInnerSegment) - static_cast<real32>(lStartSegment))
										/(static_cast<real32>(lEndSegment) - static_cast<real32>(lInnerSegment));
								}
							}
							//push us past the zeros
							lSegment = lEndSegment;
						}
					}
				}

				if (fData.bShowStrands) 
				{

					//build the segments
					for (lSegment = 0; lSegment <= lNumSegments + 1; lSegment++){
						//put the points away for later use
						TTransform3D transform;
						real32 fCurrentEffect;
						transform.fTranslation = linecenter[lSegment];

						if (fData.fTaper < 1) 
						{
							if (fData.lTaperMode == tmPossible) 
							{
								fCurrentEffect = static_cast<real32>(lSegment)
									/static_cast<real32>(fData.lSegments);
							}
							else
							{
								fCurrentEffect = static_cast<real32>(lSegment)
									/static_cast<real32>(lNumSegments + 1);
							}
							transform.fRotationAndScale[0][0] 
								= transform.fRotationAndScale[1][1] 
								= transform.fRotationAndScale[2][2] 
								= fWidth * (fData.fTaper + (1 - fData.fTaper) * (1 - fCurrentEffect));
							//if this the last set of vertices and it's 100% tapered
							//rotate it an extra 1/2 of a side 
							if ((fCurrentEffect == 1)&&(fData.lSides > 2)&&(fData.fTaper == 0))
							{
								real32 theta, costheta, sintheta;

								theta = 1.0f / static_cast<real32>(fData.lSides) * static_cast<real32>(PI);

								sintheta = sin(theta);
								costheta = cos(theta);
								
								TempRotation.fRotationAndScale[0][0] = costheta;
								TempRotation.fRotationAndScale[0][1] = - sintheta;
								TempRotation.fRotationAndScale[0][2] = 0;
								TempRotation.fRotationAndScale[1][0] = sintheta;
								TempRotation.fRotationAndScale[1][1] = costheta;
								TempRotation.fRotationAndScale[1][2] = 0;
								TempRotation.fRotationAndScale[2][0] = 0;
								TempRotation.fRotationAndScale[2][1] = 0;
								TempRotation.fRotationAndScale[2][2] = 1;

								transform.fRotationAndScale = TempRotation.fRotationAndScale * transform.fRotationAndScale;
								bTipIsZero = true;
							}
						}
						else
						{
							transform.fRotationAndScale[0][0] 
								= transform.fRotationAndScale[1][1] 
								= transform.fRotationAndScale[2][2] 
								= fWidth;
						}

						if (lSegment == 0)
						{
							BuildTransform(transform, fZRotation[lSegment], fYRotation[lSegment]);
						}
						else
						{
							BuildTransform(transform, fZRotation[lSegment], (fYRotation[lSegment - 1] +  fYRotation[lSegment]) / 2.0);
						}
						for (uint32 lSide = 0; lSide < fData.lSides;lSide++)
						{
							LocalToGlobal(transform, startpoints[lSide].fVertex
								, points[(lSegment) * fData.lSides + lSide].fVertex);
							LocalToGlobalVector(transform, startpoints[lSide].fNormal 
								, points[(lSegment) * fData.lSides + lSide].fNormal);
						}
					}//end build segments loop

					real32 v;
					//add the vertices

					for (lSegment = 0; lSegment <= lNumSegments + 1; lSegment++)
					{
						if (fData.lVRunsAlong == tmActual)
						{
							if (lSegment == lNumSegments + 1)
							{
								v = 1.0f;
							}
							else
							{
								v = fSegmentLength * lSegment / fLength;
							}
						}
						else
						{
							if (lSegment == lNumSegments + 1)
							{
								v = fLength / fData.fLength;
							}
							else
							{
								v = VBySegment[lSegment];
							}
						}
						for (uint32 lSide = 0; lSide < fData.lSides;lSide++)
						{
							workingmesh->fVertices[lCurrentVertex + lSegment * lVertexCount + lSide] 
								= points[lSegment * fData.lSides + lSide].fVertex;
							workingmesh->fNormals[lCurrentVertex + lSegment * lVertexCount + lSide] 
								= points[lSegment * fData.lSides + lSide].fNormal;
							workingmesh->fuv[lCurrentVertex + lSegment * lVertexCount + lSide] 
								= startpoints[lSide].fUV;
							if (fData.lUVMapping == uvStrand)
							{
								workingmesh->fuv[lCurrentVertex + lSegment * lVertexCount + lSide].y = v;
							}
						}
						if ((fData.lUVMapping == uvStrand)&&(fData.lSides > 2))
						{
							workingmesh->fVertices[lCurrentVertex + lSegment * lVertexCount + fData.lSides] 
								= points[lSegment * fData.lSides + 0].fVertex;
							workingmesh->fNormals[lCurrentVertex + lSegment * lVertexCount + fData.lSides] 
								= points[lSegment * fData.lSides + 0].fNormal;
							workingmesh->fuv[lCurrentVertex + lSegment * lVertexCount + fData.lSides].x = 1.0f;
							workingmesh->fuv[lCurrentVertex + lSegment * lVertexCount + fData.lSides].y = v;
						}	

					}
					
					
					//add the facets
					for (lSegment = 0; lSegment <= lNumSegments; lSegment++)
					{
						//are we the last segment and do we come to a point
						if ((lSegment == lNumSegments)&&(bTipIsZero))
						{
							for (uint32 lSide = 0; lSide < fData.lSides - 1;lSide++)
							{
								
								workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
								workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lSegment * lVertexCount + lSide;
								workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + lSegment * lVertexCount + lSide + 1;
								workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lSegment + 1) * lVertexCount + lSide;
								lCurrentFacet++;
							}
							if (fData.lSides > 2)
							{
								if (fData.lUVMapping == uvStrand)
								{
									workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
									workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lSegment * lVertexCount + fData.lSides - 1;
									workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + lSegment * lVertexCount + fData.lSides;
									workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lSegment + 1) * lVertexCount + fData.lSides - 1;
									lCurrentFacet++;
								}
								else
								{
									workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
									workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lSegment * lVertexCount + fData.lSides - 1;
									workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + lSegment * lVertexCount + 0;
									workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lSegment + 1) * lVertexCount + fData.lSides - 1;
									lCurrentFacet++;
								}
							}
						}
						else
						{
							for (uint32 lSide = 0; lSide < fData.lSides - 1;lSide++)
							{
								
								workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
								workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lSegment * lVertexCount + lSide;
								workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + lSegment * lVertexCount + lSide + 1;
								workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lSegment + 1) * lVertexCount + lSide + 1;
								lCurrentFacet++;

								workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
								workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lSegment * lVertexCount + lSide;
								workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + (lSegment + 1) * lVertexCount + lSide + 1;
								workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lSegment + 1) * lVertexCount + lSide;
								lCurrentFacet++;

							}
							if (fData.lSides > 2)
							{
								if (fData.lUVMapping == uvStrand)
								{
									workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
									workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lSegment * lVertexCount + fData.lSides - 1;
									workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + lSegment * lVertexCount + fData.lSides;
									workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lSegment + 1) * lVertexCount + fData.lSides;
									lCurrentFacet++;

									workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
									workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lSegment * lVertexCount + fData.lSides - 1;
									workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + (lSegment + 1) * lVertexCount + fData.lSides;
									workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lSegment + 1) * lVertexCount + fData.lSides - 1;
									lCurrentFacet++;
								}
								else
								{
									workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
									workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lSegment * lVertexCount + fData.lSides - 1;
									workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + lSegment * lVertexCount + 0;
									workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lSegment + 1) * lVertexCount + 0;
									lCurrentFacet++;

									workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
									workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lSegment * lVertexCount + fData.lSides - 1;
									workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + (lSegment + 1) * lVertexCount + 0;
									workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lSegment + 1) * lVertexCount + fData.lSides - 1;
									lCurrentFacet++;
								}
							}
						}
					}

					//close off the tip if need be
					if ((fData.lSides > 2)&&(fData.fTaper != 0))
					{
						//figure out the facet in the middle
						TVector3 tipmiddle;
						tipmiddle.SetValues(0,0,0);
						for (uint32 lSide = 0; lSide < fData.lSides;lSide++)
						{
							tipmiddle += points[(lNumSegments + 1) * fData.lSides + lSide].fVertex;
						}
						tipmiddle /=  static_cast<real32>(fData.lSides);
						workingmesh->fVertices[lCurrentVertex + lVertexCount * (lNumSegments + 2)] = tipmiddle;

						workingmesh->fVertices[lCurrentVertex + lVertexCount * (lNumSegments + 2)] += directionarray[lNumSegments + 1] * fLength * 0.01f;
						//add the center facets
						workingmesh->fNormals[lCurrentVertex + lVertexCount * (lNumSegments + 2)] = directionarray[lNumSegments + 1];
						workingmesh->fuv[lCurrentVertex + lVertexCount * (lNumSegments + 2)] = shadingIn.fUV;
						if (fData.lUVMapping == uvStrand)
						{
							for (uint32 lSide = 0; lSide < fData.lSides - 1;lSide++)
							{
								//add the center vertices
								workingmesh->fVertices[lCurrentVertex + lVertexCount * (lNumSegments + 2) + lSide] = workingmesh->fVertices[lCurrentVertex + lVertexCount * (lNumSegments + 2)];
								workingmesh->fNormals[lCurrentVertex + lVertexCount * (lNumSegments + 2) + lSide] = directionarray[lNumSegments + 1];
								workingmesh->fuv[lCurrentVertex + lVertexCount * (lNumSegments + 2) + lSide] = startpoints[lSide].fUV;
								workingmesh->fuv[lCurrentVertex + lVertexCount * (lNumSegments + 2) + lSide].y = v;
								//add the center facets
								workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
								workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lVertexCount * (lNumSegments + 2)  + lSide;
								workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + (lNumSegments + 1) * lVertexCount + lSide;
								workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lNumSegments + 1) * lVertexCount + lSide + 1;
								lCurrentFacet++;
							}
							
							//add the center vertices
							workingmesh->fVertices[lCurrentVertex + lVertexCount * (lNumSegments + 2) + fData.lSides - 1] = workingmesh->fVertices[lCurrentVertex + lVertexCount * (lNumSegments + 2)];
							workingmesh->fNormals[lCurrentVertex + lVertexCount * (lNumSegments + 2) + fData.lSides - 1] = directionarray[lNumSegments + 1];
							workingmesh->fuv[lCurrentVertex + lVertexCount * (lNumSegments + 2) + fData.lSides - 1] = startpoints[fData.lSides - 1].fUV;
							workingmesh->fuv[lCurrentVertex + lVertexCount * (lNumSegments + 2) + fData.lSides - 1].y = v;
							//add the center facets
							workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
							workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lVertexCount * (lNumSegments + 2) + fData.lSides - 1;
							workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + (lNumSegments + 1) * lVertexCount + fData.lSides - 1;
							workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lNumSegments + 1) * lVertexCount + fData.lSides;
							lCurrentFacet++;
						}
						else
						{
							for (uint32 lSide = 0; lSide < fData.lSides - 1;lSide++)
							{
								workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
								workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lVertexCount * (lNumSegments + 2);
								workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + (lNumSegments + 1) * lVertexCount + lSide;
								workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lNumSegments + 1) * lVertexCount + lSide + 1;
								lCurrentFacet++;
							}
							
							workingmesh->fUVSpaceID[lCurrentFacet] = lStrandSpaceID;
							workingmesh->fFacets[lCurrentFacet].pt1 = lCurrentVertex + lVertexCount * (lNumSegments + 2);
							workingmesh->fFacets[lCurrentFacet].pt2 = lCurrentVertex + (lNumSegments + 1) * lVertexCount + fData.lSides - 1;
							workingmesh->fFacets[lCurrentFacet].pt3 = lCurrentVertex + (lNumSegments + 1) * lVertexCount + 0;
							lCurrentFacet++;
						}
					}
					//increment the vertex count
					lCurrentVertex += lVertexCount * (lNumSegments + 2);
					if (fData.lSides > 2)
					{
						//if there was more than two sides we also have the middle vertex			
						if (fData.lUVMapping == uvStrand)
						{
							lCurrentVertex+=fData.lSides;
						}
						else
						{
							lCurrentVertex++;
						}
					}				
				}
				//spin through stuff and check it out
	/*			for(uint i = 0; i < workingmesh->fFacets.GetElemCount(); i++)
				{
					uint32 pt1 = workingmesh->fFacets[i].pt1, pt2 = workingmesh->fFacets[i].pt2, pt3 = workingmesh->fFacets[i].pt3;
					int x = 2;
				}
				for(uint i = 0; i < workingmesh->fVertices.GetElemCount(); i++)
				{
					TVector3 test = workingmesh->fVertices[i];
					int x = 2;
				}
	*/			


				//add the tip object
				if (tipmesh != NULL) 
				{
					boolean bHasTwist = false;
					TVector3 workingdirection;
					TVector3 direction;
					TTransform3D TempRotation;
					TTransform3D transform;
					currentdirection = directionarray[lNumSegments + 1];
					transform.fTranslation = linecenter[lNumSegments + 1];
					uint32 lNumVertices = tipmesh->fVertices.GetElemCount();
					uint32 lNumFacets = tipmesh->fFacets.GetElemCount();
		
					real32 theta, costheta, sintheta;
					real32 scale = fData.fTipScale;
					real32 fZRotation = 0, fXRotation = 0, fYRotation = 0;

					//scale the tip object
					if (tipscaleshader != NULL) 
					{
						scale = GetTipScale() * fData.fTipScale;
					}

					transform.fRotationAndScale[0][0] = scale;
					transform.fRotationAndScale[0][1] = 0;
					transform.fRotationAndScale[0][2] = 0;
					transform.fRotationAndScale[1][0] = 0;
					transform.fRotationAndScale[1][1] = scale;
					transform.fRotationAndScale[1][2] = 0;
					transform.fRotationAndScale[2][0] = 0;
					transform.fRotationAndScale[2][1] = 0;
					transform.fRotationAndScale[2][2] = scale;

					//do tip twist shader if needed
					if (bFaceCamera)
					{

					}
					else if (tiptwistshader != NULL)
					{
						theta = - PI + GetTipTwist() * PI * 2.0;

						sintheta = sin(theta);
						costheta = cos(theta);
						
						TempRotation.fRotationAndScale[0][0] = costheta;
						TempRotation.fRotationAndScale[0][1] = sintheta;
						TempRotation.fRotationAndScale[0][2] = 0;
						TempRotation.fRotationAndScale[1][0] = - sintheta;
						TempRotation.fRotationAndScale[1][1] = costheta;
						TempRotation.fRotationAndScale[1][2] = 0;
						TempRotation.fRotationAndScale[2][0] = 0;
						TempRotation.fRotationAndScale[2][1] = 0;
						TempRotation.fRotationAndScale[2][2] = 1;

						transform.fRotationAndScale = transform.fRotationAndScale * TempRotation.fRotationAndScale;
					}


					//figure out the transform to put the tip object on the tip of the strand
					direction =  currentdirection;
					if ((filter(direction.x) != 0)||(filter(direction.y) != 0))
					{
						real32 fSin;			
						//rotate around the z-axis until we are lined up with xz plane
						fSin = direction.y / sqrt(sqr(direction.x) + sqr(direction.y));
						if (fSin > 1) {
							fSin = 1;
							}
						else if(fSin < -1) {
							fSin = -1;
							}
						fZRotation = asin(fSin);
						if (direction.x > 0){
							fZRotation = PI - fZRotation;
						}

						sintheta = sin(fZRotation);
						costheta = cos(fZRotation);
						TempRotation.fRotationAndScale[0][0] = costheta;
						TempRotation.fRotationAndScale[0][1] = - sintheta;
						TempRotation.fRotationAndScale[0][2] = 0;
						TempRotation.fRotationAndScale[1][0] = sintheta;
						TempRotation.fRotationAndScale[1][1] = costheta;
						TempRotation.fRotationAndScale[1][2] = 0;
						TempRotation.fRotationAndScale[2][0] = 0;
						TempRotation.fRotationAndScale[2][1] = 0;
						TempRotation.fRotationAndScale[2][2] = 1;

						LocalToGlobalVector(TempRotation, direction, workingdirection);
						if ((filter(workingdirection.x) != 0)||(filter(workingdirection.z) != 0))
						{
							fSin = workingdirection.x / sqrt(sqr(workingdirection.x) + sqr(workingdirection.z));
							if (fSin > 1) 
							{
								fSin = 1;
							}
							else if(fSin < -1) 
							{
								fSin = -1;
							}

							fYRotation = -asin(fSin);
							if (workingdirection.z < 0) {
								fYRotation = PI - fYRotation;
								}
						}
						else
						{
							fYRotation = 0;
						}
					}
					else
					{
						fZRotation = 0;
						if (direction.z < 0)
						{
							fYRotation = static_cast<real32>(PI);
						}
						else
						{
							fYRotation = 0;
						}
						
					}
					
					if (bFaceCamera)
					{
						TTransform3D temptransform;
						real32 fSin, theta;
						TVector3 cameraposition;
						temptransform.fTranslation = linecenter[lNumSegments + 1];
						BuildTransform(temptransform, fZRotation, fYRotation);

						GlobalToLocal(temptransform, vec3LocalCameraCenter, cameraposition);
						
						//rotate around the z-axis until we are lined up with xz plane
						fSin = cameraposition.x / sqrt(sqr(cameraposition.x) + sqr(cameraposition.y));
						if (fSin > 1) {
							fSin = 1;
							}
						else if(fSin < -1) {
							fSin = -1;
							}

						theta = -asin(fSin);

						if (cameraposition.y < 0){
							theta = PI - theta;
							}

						sintheta = sin(theta);
						costheta = cos(theta);
						TempRotation.fRotationAndScale[0][0] = costheta;
						TempRotation.fRotationAndScale[0][1] = - sintheta;
						TempRotation.fRotationAndScale[0][2] = 0;
						TempRotation.fRotationAndScale[1][0] = sintheta;
						TempRotation.fRotationAndScale[1][1] = costheta;
						TempRotation.fRotationAndScale[1][2] = 0;
						TempRotation.fRotationAndScale[2][0] = 0;
						TempRotation.fRotationAndScale[2][1] = 0;
						TempRotation.fRotationAndScale[2][2] = 1;

						transform.fRotationAndScale = TempRotation.fRotationAndScale * transform.fRotationAndScale ;
					}

					//apply the scale and twist 
					for (uint32 i = 0; i<lNumVertices;i++ ) 
					{
						//LocalToGlobal(transform, tipmesh->fVertices[i], workingmesh->fVertices[lCurrentVertex + i]);
						LocalToGlobalVector(transform, tipmesh->fVertices[i], workingmesh->fVertices[lCurrentVertex + i]);
						LocalToGlobalVector(transform, tipmesh->fNormals[i], workingmesh->fNormals[lCurrentVertex + i]);
					}
					
					transform.fRotationAndScale[0][0] = 1;
					transform.fRotationAndScale[0][1] = 0;
					transform.fRotationAndScale[0][2] = 0;
					transform.fRotationAndScale[1][0] = 0;
					transform.fRotationAndScale[1][1] = 1;
					transform.fRotationAndScale[1][2] = 0;
					transform.fRotationAndScale[2][0] = 0;
					transform.fRotationAndScale[2][1] = 0;
					transform.fRotationAndScale[2][2] = 1;

					BuildTransform(transform, fZRotation, fYRotation);

					for (uint32 i = 0; i<lNumVertices;i++ ) 
					{
						LocalToGlobal(transform, workingmesh->fVertices[lCurrentVertex  + i], workingmesh->fVertices[lCurrentVertex + i]);
						LocalToGlobalVector(transform, workingmesh->fNormals[lCurrentVertex + i], workingmesh->fNormals[lCurrentVertex + i]);
						workingmesh->fuv[lCurrentVertex + i] = tipmesh->fuv[i];
					}
					
					//add the facets
					for (uint32 i = 0; i<lNumFacets;i++ ) 
					{
						workingmesh->fFacets[lCurrentFacet].pt1 = tipmesh->fFacets[i].pt1 + lCurrentVertex;
						workingmesh->fFacets[lCurrentFacet].pt2 = tipmesh->fFacets[i].pt2 + lCurrentVertex;
						workingmesh->fFacets[lCurrentFacet].pt3 = tipmesh->fFacets[i].pt3 + lCurrentVertex;
						if (lStrandSpaceCount == 0)
						{
							workingmesh->fUVSpaceID[lCurrentFacet] = aFacet.fUVSpace; 
						}
						else
						{
							workingmesh->fUVSpaceID[lCurrentFacet] = tipmesh->fUVSpaceID[i] + lBaseSpaceCount + lStrandSpaceCount; 
						}

						lCurrentFacet++;
					}
					lCurrentVertex += lNumVertices;
				}
			
			}


		}//end if length > 0
	}
}

MCCOMErr HairGenerator::GrowHair(real lod, FacetMesh* in, FacetMesh* originalMesh, FacetMesh** outMesh, int32 lSingleDomain, 
				AnythingGrowsStrands** strands, 
				GenerationMode generationMode)
{
	TMCArray<real32> fSurfaceArea;
	TMCArray<uint32> lNumHairs;
	uint32 lFacet;
	try {

		FacetMeshFacetIterator iter;
		FacetMeshFacetIterator iterOriginal;
		TFacet3D aFacet;
		TFacet3D originalFacet;
		real32 fEdgeLength[3];
		int32 lNumFacets = in->fFacets.GetElemCount();
		real32 fTotalSurfaceArea = 0;
		real32 theta;
		uint32 lTotalHairs = 0;

		if (lNumFacets == 0)
		{
			fData.Warnings = "Empty base object.";
			instance = NULL;
			refinstance = NULL;
			return MC_S_OK;
		}

		srand(fData.lSeed);

		//reset warning stuff
		fData.Warnings = "No warnings.";
		if (!fData.bEnabled) 
		{
			fData.Warnings =  "Disabled.";
			in->Clone(outMesh);
			instance = NULL;
			refinstance = NULL;
			return MC_S_OK;
		}

		GetShaders();

		gravitydirection.x = fData.fGravityX;
		gravitydirection.y = fData.fGravityY;
		gravitydirection.z = fData.fGravityZ;
		gravitydirection.Normalize(gravitydirection);
		if (fData.lSpace == smGlobal)
		{
			TVector3 temppoint;
			GlobalToLocalVector(fTransform, gravitydirection, temppoint);
			gravitydirection = temppoint;
		}


		//setup the working arrays
		fSurfaceArea.SetElemCount(lNumFacets);
		lNumHairs.SetElemCount(lNumFacets);

		//figure out the total surface area
		iter.Initialize(originalMesh);
		for (iter.First(), lFacet = 0; iter.More(); iter.Next(), lFacet++) 
		{
			real32 cos;
			aFacet = iter.GetFacet();
			if (lSingleDomain < 0 || lSingleDomain == in->fUVSpaceID[lFacet] || fData.lStrandDistributionMode == sdTotalSurface) 
			{

				fEdgeLength[0] = sqrt(sqr(aFacet.fVertices[0].fVertex, aFacet.fVertices[1].fVertex));
				fEdgeLength[1] = sqrt(sqr(aFacet.fVertices[1].fVertex, aFacet.fVertices[2].fVertex));
				fEdgeLength[2] = sqrt(sqr(aFacet.fVertices[2].fVertex, aFacet.fVertices[0].fVertex));
				
				if ((fEdgeLength[1] == 0)||(fEdgeLength[2] == 0)||(fEdgeLength[2] == 0))
				{
					fSurfaceArea[lFacet] = 0;
				}
				else
				{
					cos = (sqr(fEdgeLength[1]) + sqr(fEdgeLength[2]) - sqr(fEdgeLength[0]))
						/ (2 * fEdgeLength[1] * fEdgeLength[2]);

					if (cos < -1.0) { cos = -1.0;}
					else if (cos > 1.0) {cos = 1.0;}
					theta = acos(cos);

					fSurfaceArea[lFacet] = 0.5 * fEdgeLength[1] * fEdgeLength[2] * sin(theta);
					fTotalSurfaceArea += fSurfaceArea[lFacet];
				}
			}
			else
			{
				fSurfaceArea[lFacet] = 0;
			}
		}
		if (fData.lStrandDistributionMode == sdTotalSurface)
		{
			//spin through the surface area and normalize it's position
			fSurfaceArea[0] = (fSurfaceArea[0] / fTotalSurfaceArea);
			for (lFacet = 1; lFacet < lNumFacets; lFacet++)
			{
				fSurfaceArea[lFacet] = (fSurfaceArea[lFacet] / fTotalSurfaceArea) + fSurfaceArea[lFacet - 1];
			}
			mtRand* myRand;
			myRand = new mtRand();
			myRand->init_genrand(fData.lSeed);
			uint32 MAX_UNSIGNED_LONG = 0xFFFFFFFF;

			//spin through the # of strands and allocate them over the entire surface
			uint32 lStrand = 0;
			while (lStrand < fData.lNumHair)
			{
				real32 fFarAlong = static_cast<real32>(myRand->genrand_int32()) / static_cast<real32>(MAX_UNSIGNED_LONG);
				if (fFarAlong <= fSurfaceArea[0])
				{
					if (lSingleDomain >= 0)
					{
						if (lSingleDomain == in->fUVSpaceID[0])
						{
							lStrand++;
							lNumHairs[0]++;
						}
					}
					else
					{
						lStrand++;
						lNumHairs[0]++;
					}
				}
				else
				{
					uint32 low = 1, high = lNumFacets - 1, mid = (low + high) / 2;
					while (low <= high)
					{
						if ((fFarAlong <= fSurfaceArea[mid])&&(fFarAlong > fSurfaceArea[mid - 1]))
						{
							low = high + 1;
							if (lSingleDomain >= 0)
							{
								if (lSingleDomain == in->fUVSpaceID[mid])
								{
									lStrand++;
									lNumHairs[mid]++;
								}
							}
							else
							{
								lStrand++;
								lNumHairs[mid]++;
							}
						}
						else if (fFarAlong <= fSurfaceArea[mid])
						{
							high = mid - 1;
						}
						else if (fFarAlong > fSurfaceArea[mid - 1])
						{
							low = mid + 1;
						}
						mid = (low + high) / 2;
					}
				}
			}
			lTotalHairs = fData.lNumHair;
			//todo: clean this up
			delete myRand;

		}
		else if (fData.lStrandDistributionMode == sdFacet)
		{
			for (lFacet = 0; lFacet < lNumFacets; lFacet++)
			{
				if (lSingleDomain < 0 || lSingleDomain == in->fUVSpaceID[lFacet]) {
					lNumHairs[lFacet] = fData.lNumHair * (fSurfaceArea[lFacet] / fTotalSurfaceArea) + 0.5f;
					if ((lNumHairs[lFacet] < 1)&&(fData.bAtLeastOneStrand == true))
					{
						lNumHairs[lFacet] = 1;
					}
					lTotalHairs += lNumHairs[lFacet];
				}
			}
		} 


		if (generationMode == GenerateMesh) {

			bFaceCamera = false;
			
			//if we are supposed to face the camera, see if we can get the render camera
			if (fData.bFaceCamera)
			{
				TMCCountedPtr<ISceneDocument> sceneDoc;
				scene->GetSceneDocument(&sceneDoc);

				if (MCVerify(sceneDoc))
					{
					TMCCountedPtr<I3DRenderingModule> renderingModule;
					sceneDoc->GetSceneRenderingModule(&renderingModule);

					if (MCVerify(renderingModule))
					{
						TMCCountedPtr<I3DShTreeElement> cameratree;
						TMCCountedPtr<I3DShCamera> camera;
						renderingModule->GetRenderingCamera(&camera);
						camera->QueryInterface(IID_I3DShTreeElement, (void**)&cameratree);
						if (MCVerify(cameratree))
						{
							TTransform3D cameratransform;
							//we found the camera we are supposed to face
							//get it's center and put it away
							bFaceCamera = true;
							cameratransform = cameratree->GetGlobalTransform3D(kCurrentFrame);

							vec3CameraCenter = cameratransform.fTranslation;
							GlobalToLocal(fTransform, vec3CameraCenter, vec3LocalCameraCenter);
						}
					}
				}
			}

			//get the tip object
			if (fData.sTipObjectName != kNullString)
			{
				TMCCountedPtr<I3DShTreeElement>	tree;
				TMCCountedPtr<I3DShInstance> instance;
				TMCCountedPtr<I3DShObject> object;
				TMCCountedPtr<I3DShPrimitive> primitive;

				GetTreeElementByName(tree, fData.sTipObjectName);
				if (tree != NULL)
				{

					if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance)== MC_S_OK) 
					{
						instance->GetDeformed3DObject(&object);
						ThrowIfNil(object);

						object->QueryInterface(IID_I3DShPrimitive, (void**) &primitive);
						ThrowIfNil(primitive);
						
						primitive->GetFMesh(.01f, &tipmesh);
						ThrowIfNil(tipmesh);
					}
				}
			}


			points = static_cast<TVertex3D*>(MCcalloc((fData.lSegments + 1) * fData.lSides, sizeof(TVertex3D)));
					
			//figure out the points on our profile
			if (fData.lSides > 2)
			{
				for (uint32 lSide = 0; lSide < fData.lSides;lSide++)
				{
					//draw the points around the origin in XY plane
					real32 fAngle = static_cast<real32>(lSide) / static_cast<real32>(fData.lSides) * TWO_PI;
					real32 fCos = cos(fAngle), fSin = sin(fAngle);
					startpoints[lSide].fVertex.x = fCos;
					startpoints[lSide].fVertex.y = fSin;
					startpoints[lSide].fVertex.z = 0;
					startpoints[lSide].fNormal.x = fCos;
					startpoints[lSide].fNormal.y = fSin;
					startpoints[lSide].fNormal.z = 0;
				}
				if (fData.lUVMapping == uvStrand)
				{
					startpoints[0].fUV.x = 0;
					for (uint32 lSide = 0; lSide < fData.lSides;lSide++)
					{
						//draw the points around the origin in XY plane
						startpoints[lSide].fUV.x = static_cast<real32>(lSide) / static_cast<real32>(fData.lSides);
					}
				}
			}
			else if ((fData.lSides == 2)&&(fData.lUVMapping == uvStrand))
			{
				startpoints[0].fUV.x = 0;
				startpoints[1].fUV.x = 1;
			}


			uint32 estVertices = 0;
			uint32 estFacets = 0;
			uint32 strandVertices = 0;
			uint32 strandFacets = 0;
			workingmesh = *outMesh;
			
			if (fData.bIncludeBaseMesh == true)
			{
				estVertices = in->fVertices.GetElemCount();
				estFacets = in->fFacets.GetElemCount();
			}
			lCurrentVertex = estVertices;
			lCurrentFacet = estFacets;

			//estimate how many polys and vertices we will have to allocate space in the mesh
			if ((fData.lUVMapping == uvBase)||(fData.lSides == 2))
			{
				strandVertices = (fData.lSegments  + 1) * fData.lSides;
			}
			else //fData.lUVMapping == eStrand
			{
				strandVertices = (fData.lSegments  + 1) * (fData.lSides + 1);
			}
			if(fData.lSides > 2)
			{
				if (fData.lUVMapping == uvStrand)
				{
					strandVertices+=fData.lSides;
				}
				else
				{
					strandVertices++;
				}
				strandFacets = fData.lSegments * fData.lSides * 2;
				strandFacets +=fData.lSides;
			}
			else
			{
				strandFacets = fData.lSegments *  2;
			}

			if (fData.bShowStrands) {
				estVertices += lTotalHairs * strandVertices;
				estFacets += lTotalHairs * strandFacets;
			}

			if (tipmesh)
			{
				estFacets += tipmesh->fFacets.GetElemCount() * lTotalHairs;
				estVertices += tipmesh->fVertices.GetElemCount() * lTotalHairs;
			} 
			workingmesh->SetFacetsCount(estFacets);
			workingmesh->SetVerticesCount(estVertices);

			if (fData.bIncludeBaseMesh == true) { 
				//safe clone of mesh, only copy the stuff we are going to fill out
				uint32 vertexCount = in->fVertices.GetElemCount();

				for (uint32 vertexIndex = 0; vertexIndex < vertexCount; vertexIndex++) {
					workingmesh->fVertices[vertexIndex]=in->fVertices[vertexIndex];
					workingmesh->fNormals[vertexIndex]=in->fNormals[vertexIndex];     
					workingmesh->fuv[vertexIndex]=in->fuv[vertexIndex];
				}

				uint32 facetCount = in->fFacets.GetElemCount();

				for (uint32 facetIndex = 0; facetIndex < facetCount; facetIndex++) {
					workingmesh->fUVSpaceID[facetIndex]=in->fUVSpaceID[facetIndex];
					workingmesh->fFacets[facetIndex]=in->fFacets[facetIndex];
				}

				if (lStrandSpaceCount > 0)
				{
					for (uint32 i = 0; i < estFacets; i++)
					{
						workingmesh->fUVSpaceID[i] += lStrandSpaceCount;
					}
				}
			
			}	
		}

		if (generationMode == GenerateStrands) {
			(*strands)->strandUV.SetElemCount(lTotalHairs);
			(*strands)->uvSpaceID.SetElemCount(lTotalHairs);
			(*strands)->vertexCounts.SetElemCount(lTotalHairs);
			(*strands)->vertices.SetElemCount(lTotalHairs * (fData.lSegments + 1));
			(*strands)->vertexWidths.SetElemCount(lTotalHairs * (fData.lSegments + 1));
		}
		
		uint32 currentStrand = 0;
		uint32 currentVertex = 0;
		iter.Initialize(in);
		iterOriginal.Initialize(originalMesh);
		iter.First();
		iterOriginal.First();
		lFacet = 0;	
		while (iterOriginal.More()&&iter.More())
		{
			aFacet = iter.GetFacet();
			originalFacet = iterOriginal.GetFacet();

			if (lNumHairs[lFacet] > 0)
			{
				fUVSpace = aFacet.fUVSpace;
				if (lStrandSpaceCount == 0)
				{
					lStrandSpaceID = aFacet.fUVSpace;
				}
				AddStrands(aFacet, originalFacet, lNumHairs[lFacet], 
					strands, 
					generationMode, 
					currentStrand, 
					currentVertex);
			}

			iter.Next();
			iterOriginal.Next();
			lFacet++;
		}
		//set the objects to what we actually did
		if (generationMode == GenerateStrands) {
			(*strands)->strandUV.SetElemCount(currentStrand);
			(*strands)->uvSpaceID.SetElemCount(currentStrand);
			(*strands)->vertexCounts.SetElemCount(currentStrand);
			(*strands)->vertices.SetElemCount(currentVertex);
			(*strands)->vertexWidths.SetElemCount(currentVertex);
		}
		if (generationMode == GenerateMesh) {
			workingmesh->SetFacetsCount(lCurrentFacet);
			workingmesh->SetVerticesCount(lCurrentVertex);
			workingmesh->fFacets.CompactArray();
			workingmesh->fUVSpaceID.CompactArray();
			workingmesh->fVertices.CompactArray();
			workingmesh->fNormals.CompactArray();
			workingmesh->fuv.CompactArray();
		}

		//clean up
		CleanUp();
		return MC_S_OK;
	}
	catch(...) {
		//clean up
		CleanUp();
		return MC_S_OK;
		}
}

void HairGenerator::FillShaderList (TMCCountedPtr<IMFPart> popuppart)
{
	TMCCountedPtr<IMFTextPopupPart> popup;
	TMCString255 name = "(Clear)";

	popuppart->QueryInterface(IID_IMFTextPopupPart, (void**)&popup);
	popup->RemoveAll();

	if (scene != NULL) 
	{
		TMCCountedPtr<I3DShMasterShader> mastershader;
		uint32 numshaders = scene->GetMasterShadersCount();
		uint32 nummenu = 0;
		popup->AppendMenuItem(name);
		popup->SetItemActionNumber(nummenu, -2);
		nummenu++;
		for (uint32 shaderindex = 0; shaderindex < numshaders; shaderindex++) 
		{
			scene->GetMasterShaderByIndex(&mastershader, shaderindex);
			mastershader->GetName(name);
			mastershader = NULL;
			popup->AppendMenuItem(name);
			popup->SetItemActionNumber(nummenu, shaderindex);
			nummenu++;
		}
	} 
}

void HairGenerator::FillDomainList (TMCCountedPtr<I3DShInstance> instance, TMCCountedPtr<IMFPart> popuppart)
{
	TMCCountedPtr<IMFTextPopupPart> popup;
	TMCCountedPtr<I3DShObject> object;
	TMCString255 name = "(Clear)";
	uint32 lBaseSpaceCount = 0;

	popuppart->QueryInterface(IID_IMFTextPopupPart, (void**)&popup);
	popup->RemoveAll();

	if (scene != NULL) 
	{
		uint32 nummenu = 0;
		popup->AppendMenuItem(name);
		popup->SetItemActionNumber(nummenu, -2);
		nummenu++;
		if (instance != NULL)
		{
			instance->GetDeformed3DObject(&object);
			ThrowIfNil(object);
			lBaseSpaceCount = object->GetUVSpaceCount();
			
			for (uint32 ldomainindex = 0; ldomainindex < lBaseSpaceCount; ldomainindex++) 
			{
				UVSpaceInfo uvSpaceInfo;				
				object->GetUVSpace(ldomainindex, &uvSpaceInfo);
				name = uvSpaceInfo.fName;
				popup->AppendMenuItem(name);
				popup->SetItemActionNumber(nummenu, ldomainindex);
				nummenu++;
			}
		}
	} 
}

MCCOMErr  HairGenerator::GetTreeElementByName(TMCCountedPtr<I3DShTreeElement>& treeElement, const TMCString& searchname)
{
	TMCString255 name;
	treeElement = NULL;
	if (scene != NULL) 
	{
		TMCCountedPtr<I3DShInstance> instance;
		TMCCountedPtr<I3DShTreeElement> tree;
		TMCCountedPtr<I3DShObject> object;
		uint32 numinstances = scene->GetInstanceListCount();
		for (uint32 instanceindex = 0; instanceindex < numinstances; instanceindex++) 
		{
			scene->GetInstanceByIndex(&instance, instanceindex);
			//am I primtive and not a volumetric
			if ((instance->GetInstanceKind() == 1)
					&&(instance->IsVolumeInstance() == false)) 
			{
				instance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
				tree->GetName(name);
				if (name.CompareQuick(searchname) == 0)
				{
					treeElement = tree;
					return MC_S_OK;
				}
			}//is prim and not volume
		}//for instances
	}//got a scene
	return MC_S_FALSE;
}


void HairGenerator::FillObjectList (I3DShObject* myobject, I3DShInstance* myinstance, IMFPart* popuppart)
{
	TMCCountedPtr<IMFTextPopupPart> popup;
	TMCString255 name = "(Clear)";

	popuppart->QueryInterface(IID_IMFTextPopupPart, (void**)&popup);
	popup->RemoveAll();

	if (scene != NULL) 
	{
		TMCCountedPtr<I3DShInstance> instance;
		TMCCountedPtr<I3DShTreeElement> tree;
		TMCCountedPtr<I3DShObject> object;
		uint32 numinstances = scene->GetInstanceListCount();
		uint32 nummenu = 0;
		popup->AppendMenuItem(name);
		popup->SetItemActionNumber(nummenu, -2);
		nummenu++;
		for (uint32 instanceindex = 0; instanceindex < numinstances; instanceindex++) 
		{
			scene->GetInstanceByIndex(&instance, instanceindex);
			//am I primtive and not a volumetric
			if ((instance->GetInstanceKind() == 1)
					&&(instance->IsVolumeInstance() == false)) 
			{
				if (instance != myinstance)
				{
					instance->Get3DObject(&object);
					if (myobject != object) 
					{
						instance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);
						tree->GetName(name);
						tree = NULL;
						popup->AppendMenuItem(name);
						popup->SetItemActionNumber(nummenu, instanceindex);
						nummenu++;
					}//myobject != object
				}
				object = NULL;
				instance = NULL;
			}//is prim and not volume
		}//for instances
	}//got a scene
}

void HairGenerator::FillFlat(TVector3& pt1, TVector3& pt2, TVector3& pt3, TTransform3D& Transform){
	
	
	TTransform3D TempRotation;

	Transform.fRotationAndScale.Reset();
	TVector3 temppt1, temppt2;

	real32 theta, sintheta, costheta;
	real32 d12, dSin, x, y, d13;
	
	Transform.fTranslation[0] = pt1.x;
	Transform.fTranslation[1] = pt1.y;
	Transform.fTranslation[2] = pt1.z;

	pt2.x -= pt1.x;	pt2.y -= pt1.y;	pt2.z -= pt1.z;
	pt3.x -= pt1.x;	pt3.y -= pt1.y;	pt3.z -= pt1.z;
	pt1.x = pt1.y = pt1.z = 0;
	//Rotate around the Z axis
	temppt1 = pt1; temppt2 = pt2;
	temppt1.z = temppt2.z = 0;
	d12 = distance(temppt1,temppt2);
	if (d12 == 0)
	{
		dSin = 0;
	}
	else
	{
		dSin = temppt2.x/d12;
	}
	if (dSin > 1) {
		dSin = 1;
		}
	else if(dSin < -1) {
		dSin = -1;
		}
	theta = asin(dSin);
	if (pt2.y < 0) {
		theta = PI - theta;
		}
	sintheta = sin(theta);
	costheta = cos(theta);
	x = pt2.x * costheta - pt2.y * sintheta;
	pt2.y = pt2.x * sintheta + pt2.y * costheta;
	pt2.x = x;
	x = pt3.x * costheta - pt3.y * sintheta;
	pt3.y = pt3.x * sintheta + pt3.y * costheta;
	pt3.x = x;
	
	TempRotation.fRotationAndScale[0][0] = costheta;
	TempRotation.fRotationAndScale[0][1] = sintheta;
	TempRotation.fRotationAndScale[0][2] = 0;
	TempRotation.fRotationAndScale[1][0] = - sintheta;
	TempRotation.fRotationAndScale[1][1] = costheta;
	TempRotation.fRotationAndScale[1][2] = 0;
	TempRotation.fRotationAndScale[2][0] = 0;
	TempRotation.fRotationAndScale[2][1] = 0;
	TempRotation.fRotationAndScale[2][2] = 1;


	Transform.fRotationAndScale =  Transform.fRotationAndScale * TempRotation.fRotationAndScale;
	//Rotate around the X axis
	d12 = distance(pt1,pt2);
	if (d12 == 0)
	{
		dSin = 0;
	}
	else
	{
		dSin = pt2.z/d12;
	}
	if (dSin > 1) {
		dSin = 1;
		}
	else if(dSin < -1) {
		dSin = -1;
		}

	theta = -asin(dSin);
	if (pt2.y < 0) {
		theta = PI - theta;
		}
	sintheta = sin(theta);
	costheta = cos(theta);
	y = pt2.y * costheta - pt2.z * sintheta;
	pt2.z = pt2.y * sintheta + pt2.z * costheta;
	pt2.y = y;
	y = pt3.y * costheta - pt3.z * sintheta;
	pt3.z = pt3.y * sintheta + pt3.z * costheta;
	pt3.y = y;

	TempRotation.fRotationAndScale[0][0] = 1;
	TempRotation.fRotationAndScale[0][1] = 0;
	TempRotation.fRotationAndScale[0][2] = 0;
	TempRotation.fRotationAndScale[1][0] = 0;
	TempRotation.fRotationAndScale[1][1] = costheta;
	TempRotation.fRotationAndScale[1][2] = sintheta;
	TempRotation.fRotationAndScale[2][0] = 0;
	TempRotation.fRotationAndScale[2][1] = - sintheta;
	TempRotation.fRotationAndScale[2][2] = costheta;


	Transform.fRotationAndScale =  Transform.fRotationAndScale * TempRotation.fRotationAndScale;

	//Rotate around the Y axis
	temppt1 = pt3; temppt2 = pt3;
	temppt1.x = temppt1.z = temppt2.z = 0;
	d13 = distance(temppt1, pt3);
	if (d13 == 0)
	{
		dSin = 0;
	}
	else
	{
		dSin = pt3.z / d13;
	}
	if (dSin > 1) {
		dSin = 1;
		}
	else if(dSin < -1) {
		dSin = -1;
		}
	theta = asin(dSin);
	if (pt3.x < 0) {
		theta = PI - theta;
		}

	sintheta = sin(theta);
	costheta = cos(theta);
	x = pt2.x * costheta + pt2.z * sintheta;
	pt2.z = - pt2.x * sintheta + pt2.z * costheta;
	pt2.x = x;
	x = pt3.x * costheta + pt3.z * sintheta;
	pt3.z = - pt3.x * sintheta + pt3.z * costheta;
	pt3.x = x;

	TempRotation.fRotationAndScale[0][0] = costheta;
	TempRotation.fRotationAndScale[0][1] = 0;
	TempRotation.fRotationAndScale[0][2] = - sintheta;
	TempRotation.fRotationAndScale[1][0] = 0;
	TempRotation.fRotationAndScale[1][1] = 1;
	TempRotation.fRotationAndScale[1][2] = 0;
	TempRotation.fRotationAndScale[2][0] = sintheta;
	TempRotation.fRotationAndScale[2][1] = 0;
	TempRotation.fRotationAndScale[2][2] = costheta;

	pt2.x = pt2.z = pt3.z = 0;

	Transform.fRotationAndScale =  Transform.fRotationAndScale * TempRotation.fRotationAndScale;
	
}

void HairGenerator::CleanUp ()
{
		workingmesh = NULL;
		shader = NULL;
		widthshader = NULL;
		stiffnessshader = NULL;
		xwiggleshader = NULL;
		ywiggleshader = NULL;
		zwiggleshader = NULL;
		xcustomvectorshader = NULL;
		ycustomvectorshader = NULL;
		zcustomvectorshader = NULL;
		tipscaleshader = NULL;
		tipmesh = NULL;
		tiptwistshader = NULL;

		if (points != NULL) 
		{
			MCfree(points);
			points = NULL;
		}
		instance = NULL;
		refinstance = NULL;
}
void HairGenerator::GetShaders()
{
	boolean bDoClone = false;

	TMCCountedPtr<I3DShMasterShader> mastershader;

	scene->GetMasterShaderByName(&mastershader, fData.MasterShader);
	if (!mastershader) 
	{
		shader = NULL;
	}
	else
	{
		mastershader->GetShader(&shader);
		bDoClone = true;
	}
	mastershader = NULL;

	scene->GetMasterShaderByName(&mastershader, fData.sWidthShader);
	if (!mastershader) 
	{
		widthshader = NULL;
	}
	else
	{
		mastershader->GetShader(&widthshader);
		bDoClone = true;
	}
	mastershader = NULL;

	scene->GetMasterShaderByName(&mastershader, fData.sStiffnessShader);
	if (!mastershader) 
	{
		stiffnessshader = NULL;
	}
	else
	{
		mastershader->GetShader(&stiffnessshader);
		bDoClone = true;
	}
	mastershader = NULL;

	scene->GetMasterShaderByName(&mastershader, fData.sXWiggleShader);
	if (!mastershader) 
	{
		xwiggleshader = NULL;
	}
	else
	{
		mastershader->GetShader(&xwiggleshader);
		bDoClone = true;
	}
	mastershader = NULL;

	scene->GetMasterShaderByName(&mastershader, fData.sYWiggleShader);
	if (!mastershader) 
	{
		ywiggleshader = NULL;
	}
	else
	{
		mastershader->GetShader(&ywiggleshader);
		bDoClone = true;
	}
	mastershader = NULL;

	scene->GetMasterShaderByName(&mastershader, fData.sZWiggleShader);
	if (!mastershader) 
	{
		zwiggleshader = NULL;
	}
	else
	{
		mastershader->GetShader(&zwiggleshader);
		bDoClone = true;
	}
	mastershader = NULL;

	scene->GetMasterShaderByName(&mastershader, fData.sXCustomVectorShader);
	if (!mastershader) 
	{
		xcustomvectorshader = NULL;
	}
	else
	{
		mastershader->GetShader(&xcustomvectorshader);
		bDoClone = true;
	}
	mastershader = NULL;

	scene->GetMasterShaderByName(&mastershader, fData.sYCustomVectorShader);
	if (!mastershader) 
	{
		ycustomvectorshader = NULL;
	}
	else
	{
		mastershader->GetShader(&ycustomvectorshader);
		bDoClone = true;
	}
	mastershader = NULL;

	scene->GetMasterShaderByName(&mastershader, fData.sZCustomVectorShader);
	if (!mastershader) 
	{
		zcustomvectorshader = NULL;
	}
	else
	{
		mastershader->GetShader(&zcustomvectorshader);
		bDoClone = true;
	}
	mastershader = NULL;

	scene->GetMasterShaderByName(&mastershader, fData.sTipScaleShader);
	if (!mastershader) 
	{
		tipscaleshader = NULL;
	}
	else
	{
		mastershader->GetShader(&tipscaleshader);
		bDoClone = true;
	}
	mastershader = NULL;
	
	scene->GetMasterShaderByName(&mastershader, fData.sTipTwistShader);
	if (!mastershader) 
	{
		tiptwistshader = NULL;
	}
	else
	{
		mastershader->GetShader(&tiptwistshader);
		bDoClone = true;
	}
	mastershader = NULL;

	//if there are any shaders then take our base object and attach it to a fake instance
	TMCCountedPtr<I3DShObject> baseobject;
	if (instance->Get3DObject(&baseobject)==MC_S_OK) 
	{
		gComponentUtilities->CoCreateInstance(CLSID_StandardInstance, NULL, MC_CLSCTX_INPROC_SERVER, IID_I3DShInstance, (void**)&instance);	
		ThrowIfNil(instance);
		instance->Set3DObject(baseobject);
	}

}
