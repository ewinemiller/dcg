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
#include "copyright.h"
#include "DEMDLL.h"
#include "ElevationMesh.h"
#include "IShUtilities.h"
#include "COMUtilities.h"
#include "COMSafeUtilities.h"
#include "ElevationMeshHelper.h"


ElevationMesh::ElevationMesh()
{
	gcInitialize(&fullMapInfo);
	currentFrame = 0;
	completeElevationRange.SetValues(0, 1.0f);
}

ElevationMesh::~ElevationMesh() 
{
	gcCleanUp(&fullMapInfo);
}

real32 ElevationMesh::getShaderValue(const TVector2& uv, const Filtering& fiFiltering)
{
	//return (elevationRange[getCurrentFrame()].x + getValue(uv, fiFiltering) * (elevationRange[getCurrentFrame()].y - elevationRange[getCurrentFrame()].x) 
	//	- completeElevationRange.x) / (completeElevationRange.y - completeElevationRange.x);

	return (getValue(uv, fiFiltering) - completeElevationRange.x) / (completeElevationRange.y - completeElevationRange.x);
}

real32 ElevationMesh::getValue(const TVector2& uv, const Filtering& fiFiltering)
{
	gcSetExternalBuffer(&fullMapInfo, fElevation[getCurrentFrame()]->getDataPointer(0));

	real32 value = 0;

	TVector2 localUV = uv;

	if (localUV.x < 0)
	{
		localUV.x = 0;
	}
	else if(localUV.x > 1.0f)
	{
		localUV.x = 1.0f;
	}

	if (localUV.y < 0)
	{
		localUV.y = 0;
	}
	else if(localUV.y > 1.0f)
	{
		localUV.y = 1.0f;
	}

	value = gcGetElevationBySample(&fullMapInfo, fiFiltering, localUV.y, 1.0f - localUV.x);

	return value;
}

void ElevationMesh::BuildMesh(FacetMesh* theMesh
								, const real32 fSizeX,const real32 fSizeY,const real32 fSizeZ, const real32 baseElevation, const int meshType
								, unsigned long targetPolyCount, const real32 maxError)
{
	updateProgressBar pb;
	TMCString255 progressMsg;
	gResourceUtilities->GetIndString('3Din','DDEM',progressMsg, 130, 2);
	gShellUtilities->BeginProgress(progressMsg, &pb.progressKey);
	unsigned long east, north, point, facet;

	gcSetExternalBuffer(&fullMapInfo, fElevation[getCurrentFrame()]->getDataPointer(0));
	gcRegisterProgressCallback(&fullMapInfo, updateProgressBarCallback, &pb);

	if (gcCreateMesh(&fullMapInfo, targetPolyCount, meshType, maxError) == GC_ERROR) {
		return;
	}
	theMesh->SetVerticesCount(fullMapInfo.mesh->pointCount);
	theMesh->SetFacetsCount(fullMapInfo.mesh->facetCount);

	TMCArray<real32> XLines;
	TMCArray<real32> YLines;
	TMCArray<real32> ULines;
	TMCArray<real32> VLines;
	TMCArray<uint8> normalTally;

	VLines.SetElemCount(fullMapInfo.mesh->samples[GC_EAST]);
	YLines.SetElemCount(fullMapInfo.mesh->samples[GC_EAST]);

	ULines.SetElemCount(fullMapInfo.mesh->samples[GC_NORTH]);
	XLines.SetElemCount(fullMapInfo.mesh->samples[GC_NORTH]);

	for (east = 0; east < fullMapInfo.mesh->samples[GC_EAST]; east++){
		VLines[east] = fullMapInfo.mesh->EPercent[east];
		YLines[east] = fSizeY * VLines[east] - fSizeY * 0.5f;
	}

	for (north = 0; north < fullMapInfo.mesh->samples[GC_NORTH]; north++){
		ULines[north] = fullMapInfo.mesh->NPercent[fullMapInfo.mesh->samples[GC_NORTH] - north - 1];
		XLines[north] = fSizeX * ULines[north] - fSizeX * 0.5f;
	}

	for (point = 0; point < fullMapInfo.mesh->pointCount; point++) {
		theMesh->fVertices[point].x = XLines[fullMapInfo.mesh->points[point].north];
		theMesh->fVertices[point].y = YLines[fullMapInfo.mesh->points[point].east];
		theMesh->fVertices[point].z = baseElevation + fullMapInfo.mesh->points[point].elevation * fSizeZ;
		theMesh->fNormals[point].x = 0;
		theMesh->fNormals[point].y = 0;
		theMesh->fNormals[point].z = 0;
		theMesh->fuv[point].x = ULines[fullMapInfo.mesh->points[point].north];
		theMesh->fuv[point].y = VLines[fullMapInfo.mesh->points[point].east];
	}

	for (facet = 0; facet < fullMapInfo.mesh->facetCount; facet++) {
		theMesh->fFacets[facet].pt1 = fullMapInfo.mesh->facets[facet].pointIndex[0];
		theMesh->fFacets[facet].pt2 = fullMapInfo.mesh->facets[facet].pointIndex[2];
		theMesh->fFacets[facet].pt3 = fullMapInfo.mesh->facets[facet].pointIndex[1];

		//build the normal and accumulate on fNormals
		TVector3 normal = (theMesh->fVertices[theMesh->fFacets[facet].pt2] 
						- theMesh->fVertices[theMesh->fFacets[facet].pt1])
						^ (theMesh->fVertices[theMesh->fFacets[facet].pt3] 
						- theMesh->fVertices[theMesh->fFacets[facet].pt2]);
		normal.Normalize(normal);

		real32 area = GetArea(theMesh->fVertices[theMesh->fFacets[facet].pt1]
			, theMesh->fVertices[theMesh->fFacets[facet].pt2]
			, theMesh->fVertices[theMesh->fFacets[facet].pt3]);

		theMesh->fNormals[fullMapInfo.mesh->facets[facet].pointIndex[0]] += normal * area;
		theMesh->fNormals[fullMapInfo.mesh->facets[facet].pointIndex[1]] += normal * area;
		theMesh->fNormals[fullMapInfo.mesh->facets[facet].pointIndex[2]] += normal * area;
	}

	for (point = 0; point < fullMapInfo.mesh->pointCount; point++) {
		theMesh->fNormals[point].Normalize(theMesh->fNormals[point]);
	}

	gcCleanUpMesh(&fullMapInfo);
	gcDeregisterProgressCallback(&fullMapInfo);
	gShellUtilities->EndProgress(pb.progressKey);
}

MCCOMErr ElevationMesh::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{
	// the keyword has been read
	int8 token[256];
	int32 ELEH, ELEW;
	stream->GetBegin();
	boolean secondELEW = false;
	uint32 readFrame = 0;
	int32 version = 1;
	try
	{
		do
		{
			int16 err=stream->GetNextToken(token);
			//todo: update error handling
			//if(err!=0) throw TMCException(err,0);

			if (!stream->IsEndToken(token))
			{
				int32 keyword;

				stream->CompactAttribute(token, &keyword);

				switch (keyword)
				{
				case 'VERS':
#if VERSIONNUMBER >= 0x080000
					version = stream->GetInt32Token();
#else
					stream->GetLong((int32*)&version);
#endif
					break;
				case 'ELEH':
#if VERSIONNUMBER >= 0x080000
					ELEH = stream->GetInt32Token();
#else
					stream->GetLong((int32*)&ELEH);
#endif
					fullMapInfo.samples[GC_NORTH] = ELEH;
					break;
				case 'ELEW':
#if VERSIONNUMBER >= 0x080000
					if (secondELEW)
					{
						ELEH = ELEW;
						fullMapInfo.samples[GC_NORTH] = ELEH;
					}
					ELEW = stream->GetInt32Token();
					secondELEW = true;
#else
					stream->GetLong((int32*)&ELEW);
#endif
					fullMapInfo.samples[GC_EAST] = ELEW;
					break;
				case 'ELEF':
					readFrame = stream->GetInt32Token();
					break;
				case 'ELEM':
					{
						TMCCountedPtr<CountedRealArray> currentFrameElevation;
						CountedRealArray::Create(&currentFrameElevation);
						if (fElevation.GetElemCount() < readFrame + 1)
						{
							fElevation.SetElemCount(readFrame + 1);
							elevationRange.SetElemCount(readFrame + 1);
							elevationRange[readFrame].SetValues(0, 1.0f);
						}
						fElevation.SetElem(readFrame, currentFrameElevation);
						fElevation[readFrame]->setDataElemCount(ELEH * ELEW);
						if (version == 2) {
							stream->GetArray(fElevation[readFrame]->getDataElemCount(), reinterpret_cast<real*>(fElevation[readFrame]->getDataPointer(0)));
						}
						else if (version == 1) {
							TMCArray<real> oldElevation;
							oldElevation.SetElemCount(fElevation[readFrame]->getDataElemCount());
							stream->GetArray(fElevation[readFrame]->getDataElemCount(), reinterpret_cast<real*>(oldElevation.BaseAddress()));
							//this is an old save, load in buffer than transform to new layout
							for (unsigned int x = 0; x < fullMapInfo.samples[GC_NORTH]; x++) {
								unsigned int north = fullMapInfo.samples[GC_NORTH] - x - 1;
								for (unsigned int east = 0; east < fullMapInfo.samples[GC_EAST]; east++) {
									fElevation[readFrame]->setData(gcGetElevationIndex(&fullMapInfo, east, north), oldElevation[east * fullMapInfo.samples[GC_NORTH] + x]);

								}
							}

						}
					}
					break;
				case 'ELER':
					{
						TVector2 tempPoint;
						stream->GetPoint2D(&tempPoint.x, &tempPoint.y);
						elevationRange[readFrame] = tempPoint;
					}
					break;
				default:
					readUnknown(keyword, stream, privData);
				}

			}
		}
		while (!stream->IsEndToken(token));

		setCompleteElevationRange();
	}
	catch (TMCException& exception)
	{
		throw exception;
	}
	return MC_S_OK;

}
MCCOMErr ElevationMesh::Write(IShTokenStream* stream)
{
	//next the ElevationMesh's stuff
	stream->Indent();
#if VERSIONNUMBER >= 0x080000
	stream->PutInt32Attribute('VERS', 2);
#else
	stream->PutKeyword('VERS');
	stream->PutLong(2);
#endif
	stream->Indent();
#if VERSIONNUMBER >= 0x080000
	stream->PutInt32Attribute('ELEH', fullMapInfo.samples[GC_NORTH]);
#else
	stream->PutKeyword('ELEH');
	stream->PutLong(fullMapInfo.samples[GC_NORTH]);
#endif
	stream->Indent();
#if VERSIONNUMBER >= 0x080000
	stream->PutInt32Attribute('ELEW', fullMapInfo.samples[GC_EAST]);
#else
	stream->PutKeyword('ELEW');
	stream->PutLong(fullMapInfo.samples[GC_EAST]);
#endif
	for (uint32 writeFrame = 0; writeFrame < fElevation.GetElemCount(); writeFrame++)
	{
		stream->Indent();
		stream->PutInt32Attribute('ELEF', writeFrame);
		stream->Indent();
		stream->PutKeyword('ELEM');
		stream->PutArray(fElevation[writeFrame]->getDataElemCount(), reinterpret_cast<real*>(fElevation[writeFrame]->getDataPointer(0)));
		if (elevationRange.GetElemCount() > writeFrame)
		{
			stream->Indent();
			stream->PutKeyword('ELER');
			stream->PutPoint2D(elevationRange[writeFrame].x, elevationRange[writeFrame].y);
		}
	}

	return MC_S_OK;
}

void ElevationMesh::setCompleteElevationRange()
{
	completeElevationRange = elevationRange[0];
	for (uint32 index = 1; index < elevationRange.GetElemCount(); index++)
	{
		if (elevationRange[index].x < completeElevationRange.x)
			completeElevationRange.x = elevationRange[index].x;

		if (elevationRange[index].y > completeElevationRange.y)
			completeElevationRange.y = elevationRange[index].y;
	}

}
