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
#include "GCShapefile.h"
#include "DEMDLL.h"
#include "MCCountedPtrHelper.h"
#include <algorithm>

#include "IMFPart.h"
#include "IMFResponder.h"
#include "MFPartMessages.h"
#include "COMUtilities.h"
#include "IShUtilities.h"
#if VERSIONNUMBER >= 0x050000
#include "COMSafeUtilities.h"
#endif

#include "dcguiutil.h"

int ShapeFileEdgeCompare(const void *elem1, const void *elem2)
{
	//sort by y on first point
	real32 temp = (static_cast<const ShapeFileEdge*>(elem1))->p1.y - (static_cast<const ShapeFileEdge*>(elem2))->p1.y;

	if (temp < 0)
		return -1;
	else if (temp > 0)
		return 1;
	else
	{
		return 0;
	}
}

inline boolean lineSegmentIntersection(const TVector2& p1,const TVector2& p2,const TVector2& p3,const TVector2& p4)
{
	real32 denom = (p4.y - p3.y) * (p2.x - p1.x) - (p4.x - p3.x) * (p2.y - p1.y);
	if (denom == 0)
		return false;
	real32 ua = (p4.x - p3.x) * (p1.y - p3.y) - (p4.y - p3.y) * (p1.x - p3.x);
	ua /= denom;
	if (ua < 0 || ua > 1.0f)
		return false;
	real32 ub = (p2.x - p1.x) * (p1.y - p3.y) - (p2.y - p1.y) * (p1.x - p3.x);
	ub /= denom;
	if (ub < 0 || ub > 1.0f)
		return false;
	return true;
}


uint32 EdgeNode::tallyIntersect(const TVector2& p1, const TVector2& p2)
{
	uint32 intersectTally = 0;

	if (p1.y < midPoint && p2.y < midPoint && left != NULL)
	{
		intersectTally += left->tallyIntersect(p1, p2);
	}
	else if (p1.y > midPoint && p2.y > midPoint && right != NULL)
	{
		intersectTally += right->tallyIntersect(p1, p2);
	}

	uint32 maxEdges = edges.GetElemCount();
	for (uint32 edgeIndex = 0; edgeIndex < maxEdges; edgeIndex++)
	{
		ShapeFileEdge& currentEdge = edges[edgeIndex];
		
		//if (currentEdge.p2.y >= p1.y && currentEdge.p1.y <= p1.y)
		//{
			if (lineSegmentIntersection(p1, p2, currentEdge.p1, currentEdge.p2))
			{
				intersectTally++;
			}
		/*}
		else
		if (p1.y < currentEdge.p1.y)
		{
			edgeIndex = maxEdges;
		}*/
	}

	return intersectTally;

}



#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_GCShapefile(R_CLSID_GCShapefile);
#else
const MCGUID CLSID_GCShapefile={R_CLSID_GCShapefile};
#endif

GCShapefile::GCShapefile() 
{
	fData.bInvert = false;
	fData.latitude.SetValues(-90, 90);
	fData.longitude.SetValues(-180, 180);
}

GCShapefile::~GCShapefile()
{
}

void* GCShapefile::GetExtensionDataBuffer()
{
	return ((void*) &(fData));
}

MCCOMErr GCShapefile::ExtensionDataChanged()
{
	bDirty = true;
	return MC_S_OK;
}

boolean	GCShapefile::IsEqualTo(I3DExShader* aShader)
{

  return (false); 
}  

MCCOMErr GCShapefile::GetShadingFlags(ShadingFlags& theFlags)
{
	theFlags.fNeedsUV = true;
	theFlags.fConstantChannelsMask = kNoChannel;
	return MC_S_OK;
}

EShaderOutput GCShapefile::GetImplementedOutput()
{
	return static_cast<EShaderOutput>(kUsesGetValue);
}

#if VERSIONNUMBER >= 0x030000
real 
#else
MCCOMErr
#endif
GCShapefile::GetValue(real& result,boolean& fullArea,ShadingIn& shadingIn)
{
	uint32 intersections = 0;
	uint32 maxEdges = shapefile.Edges.GetElemCount();

	if (maxEdges > 0)
	{
		TVector2 samplepoint;
		TVector2 exteriorpoint;


		//samplepoint.x = shapefile.range.fMin.x + (1.0f - shadingIn.fUV.x) * (shapefile.range.fMax.x - shapefile.range.fMin.x);
		//samplepoint.y = shapefile.range.fMin.y + (1.0f - shadingIn.fUV.y) * (shapefile.range.fMax.y - shapefile.range.fMin.y);

		samplepoint.x = fData.longitude.x + (1.0f - shadingIn.fUV.x) * (fData.longitude.y - fData.longitude.x);
		samplepoint.y = fData.latitude.x + (1.0f - shadingIn.fUV.y) * (fData.latitude.y - fData.latitude.x);

		//find the closest side for our exterior point
		//real32 mindistance = samplepoint.x - shapefile.range.fMin.x;
		//exteriorpoint.SetValues(shapefile.range.fMin.x, samplepoint.y);
		//if (mindistance > shapefile.range.fMax.x - samplepoint.x)
		//{
		//	mindistance = shapefile.range.fMax.x - samplepoint.x;
		//	exteriorpoint.SetValues(shapefile.range.fMax.x, samplepoint.y);
		//}
		real32 mindistance = samplepoint.x - fData.longitude.x;
		exteriorpoint.SetValues(fData.longitude.x, samplepoint.y);
		if (mindistance > fData.longitude.y - samplepoint.x)
		{
			mindistance = fData.longitude.y - samplepoint.x;
			exteriorpoint.SetValues(fData.longitude.y, samplepoint.y);
		}
		intersections = edgeTree.tallyIntersect(samplepoint, exteriorpoint);

	}//has some edges
	result = intersections % 2;
	if (fData.bInvert)
		result = 1.0f - result;
	return 1.0f;
}


void GCShapefile::readFile(TMCDynamicString sFilename)
{
	bDirty = false;
	shapefile.Edges.ArrayFree();
	if (sFilename.Length() > 0)
	{
		TMCArray<PolygonType*> polygons;
		int32 filelength = 0;
		TMCfstream*  stream = NULL;

		//open the file
#if VERSIONNUMBER >= 0x050000
		stream = new TMCfstream(sFilename.StrGet(),kAnsiTranscoder,TMCiostream::in);
#else
		stream = new TMCfstream(sFilename.StrGet(),TMCiostream::in);
#endif
		if (stream->fail() == 2)
		{
			return;
		}
		//file length
		stream->seek(24);
		filelength = GetIntBigEndian(stream) * 2;
		//type of shapes
		stream->seek(32);
		shapefile.shapeType = static_cast<spShapeType>(GetIntLittleEndian(stream));
		//get bounds
		shapefile.range.fMin.x = GetDoubleLittleEndian(stream);
		shapefile.range.fMin.y = GetDoubleLittleEndian(stream);
		shapefile.range.fMax.x = GetDoubleLittleEndian(stream);
		shapefile.range.fMax.y = GetDoubleLittleEndian(stream);
		//shapefile.range.fMin.x = floor(shapefile.range.fMin.x);
		//shapefile.range.fMin.y = floor(shapefile.range.fMin.y);
		//shapefile.range.fMax.x = ceil(shapefile.range.fMax.x);
		//shapefile.range.fMax.y = ceil(shapefile.range.fMax.y);

		fData.longitude.SetValues(shapefile.range.fMin.x, shapefile.range.fMax.x);
		fData.latitude.SetValues(shapefile.range.fMin.y, shapefile.range.fMax.y);

		//first spin through the records and count how many we have
		uint32 currentpos = 100;
		int32 maxrecords = 0;
		while (currentpos < filelength)
		{
			stream->seek(currentpos);
			maxrecords = GetIntBigEndian(stream);
			int32 recordlength = (GetIntBigEndian(stream) * 2) + 8;
			currentpos += recordlength;
		}
		polygons.SetElemCount(maxrecords);

		//spin through the records
		currentpos = 100;
		while (currentpos < filelength)
		{
			stream->seek(currentpos);
			int32 recordnumber = GetIntBigEndian(stream);
			int32 recordlength = (GetIntBigEndian(stream) * 2) + 8;
			int32 shapetype = static_cast<spShapeType>(GetIntLittleEndian(stream));
			switch (shapetype)
			{
			case PolygonShape:
			case PolygonZ:
				{
					polygons[recordnumber - 1] = new PolygonType();
					PolygonType& polygon = *polygons[recordnumber - 1];

					polygon.range.fMin.x = GetDoubleLittleEndian(stream);
					polygon.range.fMin.y = GetDoubleLittleEndian(stream);
					polygon.range.fMax.x = GetDoubleLittleEndian(stream);
					polygon.range.fMax.y = GetDoubleLittleEndian(stream);
					polygon.NumParts = GetIntLittleEndian(stream);
					polygon.NumPoints = GetIntLittleEndian(stream);
					polygon.Parts.SetElemCount(polygon.NumParts);
					polygon.Points.SetElemCount(polygon.NumPoints);
					for (uint32 i = 0; i < polygon.NumParts; i++)
					{
						int32& part = polygon.Parts[i];
						part = GetIntLittleEndian(stream);
					}
					for (uint32 i = 0; i < polygon.NumPoints; i++)
					{
						TVector2& point = polygon.Points[i];
						point.x = GetDoubleLittleEndian(stream);
						point.y = GetDoubleLittleEndian(stream);
					}
				}
				break;
			}
			currentpos += recordlength;
		}
		
		delete stream;
		//count the edges and allocate fill an array of edges
		//sort them so that first point on edge is minimum y 
		uint32 edgeCount = 0;
		for (uint32 polygonIndex = 0; polygonIndex < polygons.GetElemCount(); polygonIndex++)
		{
			PolygonType& polygon = *polygons[polygonIndex];
			for (uint32 partIndex = 0; partIndex < polygon.NumParts; partIndex++)
			{
				uint32 maxPointIndex;
				if (partIndex + 1 < polygon.NumParts)
				{
					maxPointIndex = polygon.Parts[partIndex + 1] - 1;
				}
				else
				{
					maxPointIndex = polygon.NumPoints - 1;
				}
				edgeCount += maxPointIndex - polygon.Parts[partIndex];
			}

		}
		//fill the edge list
		uint32 edgeIndex = 0;
		shapefile.Edges.SetElemCount(edgeCount);
		for (uint32 polygonIndex = 0; polygonIndex < polygons.GetElemCount(); polygonIndex++)
		{
			PolygonType& polygon = *polygons[polygonIndex];
			for (uint32 partIndex = 0; partIndex < polygon.NumParts; partIndex++)
			{
				uint32 maxPointIndex;
				if (partIndex + 1 < polygon.NumParts)
				{
					maxPointIndex = polygon.Parts[partIndex + 1] - 1;
				}
				else
				{
					maxPointIndex = polygon.NumPoints - 1;
				}
				for (uint32 pointIndex = polygon.Parts[partIndex]; pointIndex < maxPointIndex; pointIndex++)
				{
					TVector2& p1 = shapefile.Edges[edgeIndex].p1;
					TVector2& p2 = shapefile.Edges[edgeIndex].p2;
					if (polygon.Points[pointIndex].y < polygon.Points[pointIndex + 1].y)
					{
						p1 = polygon.Points[pointIndex];
						p2 = polygon.Points[pointIndex + 1];
					}
					else
					{
						p1 = polygon.Points[pointIndex + 1];
						p2 = polygon.Points[pointIndex];
					}
					edgeIndex++;
				}
			}

		}
		//clean up the polygons we allocated
		for (uint32 i = 0; i < polygons.GetElemCount(); i++)
		{
			delete polygons[i];
		}
		//sort these so that can skip chunks later
		qsort(shapefile.Edges.Pointer(0), shapefile.Edges.GetElemCount(), sizeof(ShapeFileEdge), ShapeFileEdgeCompare);
		
		fillEdgeTree();
	}
}

void GCShapefile::Clone(IExDataExchanger**res,IMCUnknown* pUnkOuter)
{
     TMCCountedCreateHelper<IExDataExchanger> result(res);
     GCShapefile* clone = new GCShapefile();
     result = (IExDataExchanger*)clone;

     clone->CopyDataFrom(this);

     clone->SetControllingUnknown(pUnkOuter);
}

void GCShapefile::CopyDataFrom(const GCShapefile* source)
{
	fData = source->fData;
	bDirty = source->bDirty;
	shapefile.range = source->shapefile.range;
	shapefile.Edges.Append(source->shapefile.Edges);
	fillEdgeTree();
}

MCCOMErr GCShapefile::HandleEvent(MessageID message, IMFResponder* source, void* data)
{
	IDType sourceID;
	TMCCountedPtr<IMFPart> sourcePart;

	source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	ThrowIfNil(sourcePart);
	sourceID = sourcePart->GetIMFPartID();

	if ((sourceID == 'OPEN')&&(message == EMFPartMessage::kMsg_PartValueChanged)) 
	{
		TMCCountedPtr<IMCFile> file;
		gFileUtilities->CreateIMCFile(&file);
		IDType fileformat = 0, typeselection = 0;
		TMCArray<IDType> filetypes;
		TMCString255 filedialogtitle;
		gResourceUtilities->GetIndString('3Din','DDEM',filedialogtitle, 130, 4);
		filetypes.SetElemCount(1);
		filetypes.SetElem(0, 'SHPF');
		if (MC_S_OK == gFileFormatUtilities->OpenFileDialog(filedialogtitle, file, filetypes, typeselection, fileformat))
		{
			TMCDynamicString fullPathName;
			file->GetFileFullPathName(fullPathName);
			readFile(fullPathName);
			TMCCountedPtr<IMFPart> parentPart;
			sourcePart->GetPartParent(&parentPart);
		}//file not null
	}

	if ((sourceID == 'RESE')&&(message == EMFPartMessage::kMsg_PartValueChanged)) 
	{
		fData.longitude.SetValues(shapefile.range.fMin.x, shapefile.range.fMax.x);
		fData.latitude.SetValues(shapefile.range.fMin.y, shapefile.range.fMax.y);
	}	
	return MC_S_OK;
}

MCCOMErr GCShapefile::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_IExStreamIO))
	{
		TMCCountedGetHelper<IExStreamIO> result(ppvObj);
		result = (IExStreamIO*) this;
		return MC_S_OK;
	}
	else
		return TBasicShader::QueryInterface(riid, ppvObj);	
}
uint32 GCShapefile::AddRef()
{
	return TBasicShader::AddRef();
}

MCCOMErr GCShapefile::Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData)
{
	// the keyword has been read
	int8 token[256];
	int32 NumElements;
	stream->GetBegin();

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
					case 'fMin':
						 stream->GetPoint2D(&shapefile.range.fMin.x, &shapefile.range.fMin.y);
						break;
					case 'fMax':
						 stream->GetPoint2D(&shapefile.range.fMax.x, &shapefile.range.fMax.y);
						break;
					case 'Nele':

#if VERSIONNUMBER >= 0x080000
						NumElements = stream->GetInt32Token();
#else
						stream->GetLong(&NumElements);
#endif
						shapefile.Edges.SetElemCount(NumElements / 4);
						break;
					case 'ELEM':
						stream->GetArray(shapefile.Edges.GetElemCount() * 4, reinterpret_cast<real*>(shapefile.Edges.Pointer(0)));
						break;
					default:
						readUnknown(keyword, stream, privData);
				}

			}
		}
		while (!stream->IsEndToken(token));
	}
	catch (TMCException& exception)
	{
		throw exception;
	}

	fillEdgeTree();

	return MC_S_OK;

}

void GCShapefile::fillEdgeTree()
{
	uint32 maxEdges = shapefile.Edges.GetElemCount();

	edgeTree.clearNodes();

	edgeTree.setRange(shapefile.range.fMin.y, shapefile.range.fMax.y);
	
	for (uint32 edgeIndex = 0; edgeIndex < maxEdges; edgeIndex++)
	{
		ShapeFileEdge& currentEdge = shapefile.Edges[edgeIndex];
		edgeTree.countEdge(currentEdge.p1, currentEdge.p2);
	}

	edgeTree.allocNodes();

	for (uint32 edgeIndex = 0; edgeIndex < maxEdges; edgeIndex++)
	{
		ShapeFileEdge& currentEdge = shapefile.Edges[edgeIndex];
		edgeTree.addEdge(currentEdge.p1, currentEdge.p2);
	}

}

MCCOMErr GCShapefile::Write(IShTokenStream* stream)
{
	//next the ElevationMesh's stuff
	stream->Indent();
	stream->PutKeyword('fMin');
	stream->PutPoint2D(shapefile.range.fMin.x, shapefile.range.fMin.y);
	stream->Indent();
	stream->PutKeyword('fMax');
	stream->PutPoint2D(shapefile.range.fMax.x, shapefile.range.fMax.y);
	stream->Indent();
#if VERSIONNUMBER >= 0x080000
	stream->PutInt32Attribute('Nele', shapefile.Edges.GetElemCount() * 4);
#else
	stream->PutKeyword('Nele');
	stream->PutLong(shapefile.Edges.GetElemCount() * 4);
#endif
	stream->Indent();
	stream->PutKeyword('ELEM');
	stream->PutArray(shapefile.Edges.GetElemCount() * 4, reinterpret_cast<real*>(shapefile.Edges.Pointer(0)));

	
	return MC_S_OK;
}
