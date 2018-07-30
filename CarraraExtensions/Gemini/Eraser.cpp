/*  Project Gemini - plug-in for Carrara
    Copyright (C) 2002 Eric Winemiller

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
#include "Eraser.h"
#include "GeminiDLL.h"


enum Keep {kXPositive = 0, kXNegative = 1, kYPositive = 2, kYNegative = 3, kZPositive = 4, kZNegative = 5};

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Eraser(R_CLSID_Eraser);
#else
const MCGUID CLSID_Eraser={R_CLSID_Eraser};
#endif

inline void FillPoint(const real32& fFarAlong, const TVertex3D& pt1, const TVertex3D& pt2, TVertex3D& result)
{
	//do the UV
	result.fUV = pt1.fUV + fFarAlong * (pt2.fUV - pt1.fUV);

	//do the 3D point
	result.fVertex = pt1.fVertex + fFarAlong * (pt2.fVertex - pt1.fVertex);

	//do the normal
	result.fNormal = pt1.fNormal + fFarAlong * (pt2.fNormal - pt1.fNormal);
};

Eraser::Eraser()
{
	fFlags.SetMasked(TModifierFlags::kDeformModifier);
	fData.bEnabled = true;
	fData.lStartFrom = kZNegative;
	fData.fErase = 0.5f;
}

MCCOMErr Eraser::DeformFacetMesh(real lod,FacetMesh* in, FacetMesh** outMesh) {

	if (fData.bEnabled == false)
	{
		in->Clone(outMesh);
		return MC_S_OK; 
	}

	FacetMeshAccumulator accu;
	FacetMeshFacetIterator iter;

	iter.Initialize(in);
	TBBox3D range;

	in->CalcBBox(range);
	if (fData.fErase < 1.0f)
	{
		switch (fData.lStartFrom)
		{

			case kYPositive:
				DoPositiveDirection(1, range, accu, iter);
				break;
			case kYNegative:
				DoNegativeDirection(1, range, accu, iter);
				break;
			case kXPositive:
				DoPositiveDirection(0, range, accu, iter);
				break;
			case kXNegative:
				DoNegativeDirection(0, range, accu, iter);
				break;
			case kZPositive:
				DoPositiveDirection(2, range, accu, iter);
				break;
			case kZNegative:
				DoNegativeDirection(2, range, accu, iter);
				break;
		}
	}
	accu.MakeFacetMesh(outMesh);

	return MC_S_OK; 
}


MCCOMErr Eraser::DeformBBox(const TBBox3D& in, TBBox3D& out)
{
	out = in;
	switch (fData.lStartFrom)
	{
		case kYPositive:
			out.fMin.y = - out.fMax.y;
			break;
		case kYNegative:
			out.fMax.y = - out.fMin.y;
			break;
		case kXPositive:
			out.fMin.x = - out.fMax.x;
			break;
		case kXNegative:
			out.fMax.x = - out.fMin.x;
			break;
		case kZPositive:
			out.fMin.z = - out.fMax.z;
			break;
		case kZNegative:
			out.fMax.z = - out.fMin.z;
			break;
	}
	return MC_S_OK;
}

void Eraser::DoNegativeDirection(const uint& lIndex,const TBBox3D& range, FacetMeshAccumulator& accu, FacetMeshFacetIterator& iter)
{
	real32 fCutOff, fFarAlong;
	TFacet3D aFacet;
	TVertex3D pt02, pt01, pt12;
	uint32 lOnLine, lUnderLine, lOverLine, lFacetCount = 0;

	fCutOff = range.fMin[lIndex] - (range.fMin[lIndex] - range.fMax[lIndex]) * fData.fErase;
	for (iter.First(); iter.More(); iter.Next()) 
	{
		aFacet = iter.GetFacet();
		lOnLine = 0; lUnderLine = 0; lOverLine = 0;
		if (aFacet.fVertices[0].fVertex[lIndex] > fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[0].fVertex[lIndex] < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (aFacet.fVertices[1].fVertex[lIndex] > fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[1].fVertex[lIndex] < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (aFacet.fVertices[2].fVertex[lIndex] > fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[2].fVertex[lIndex] < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}


		if (lUnderLine + lOnLine == 3)
		{
			//we grab the full facet					
			lFacetCount++;
		}
		else if (lUnderLine == 2)
		{
			//we chop down the facet and do a partial
			lFacetCount+=2;
		}
		else if (lUnderLine == 1)
		{
			lFacetCount++;
		}
		//else 
		//{
		//either all are over the line, or some are online and some are overline and we still throw it away
		//}
	}
	accu.PrepareAccumulation(lFacetCount);
	for (iter.First(); iter.More(); iter.Next()) 
	{
		aFacet = iter.GetFacet();
		lOnLine = 0; lUnderLine = 0; lOverLine = 0;
		if (aFacet.fVertices[0].fVertex[lIndex] > fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[0].fVertex[lIndex] < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (aFacet.fVertices[1].fVertex[lIndex] > fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[1].fVertex[lIndex] < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (aFacet.fVertices[2].fVertex[lIndex] > fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[2].fVertex[lIndex] < fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}


		if (lUnderLine + lOnLine == 3)
		{
			//we grab the full facet					
			accu.AccumulateFacet(&aFacet);
		}
		else if (lUnderLine == 2)
		{
			//we chop down the facet and do a partial
			if (aFacet.fVertices[0].fVertex[lIndex] <= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[0].fVertex[lIndex]) / (aFacet.fVertices[1].fVertex[lIndex] - aFacet.fVertices[0].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[0], aFacet.fVertices[1], pt01);

				fFarAlong = (fCutOff - aFacet.fVertices[0].fVertex[lIndex]) / (aFacet.fVertices[2].fVertex[lIndex] - aFacet.fVertices[0].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[0], aFacet.fVertices[2], pt02);							

				aFacet.fVertices[0] = pt01;
				accu.AccumulateFacet(&aFacet);

				aFacet.fVertices[1] = aFacet.fVertices[2];
				aFacet.fVertices[2] = pt02;
				accu.AccumulateFacet(&aFacet);

			}
			else if (aFacet.fVertices[1].fVertex[lIndex] <= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[1].fVertex[lIndex]) / (aFacet.fVertices[0].fVertex[lIndex] - aFacet.fVertices[1].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[1], aFacet.fVertices[0], pt01);

				fFarAlong = (fCutOff - aFacet.fVertices[1].fVertex[lIndex]) / (aFacet.fVertices[2].fVertex[lIndex] - aFacet.fVertices[1].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[1], aFacet.fVertices[2], pt12);							

				aFacet.fVertices[1] = pt12;
				accu.AccumulateFacet(&aFacet);

				aFacet.fVertices[1] = pt01;
				aFacet.fVertices[2] = pt12;
				accu.AccumulateFacet(&aFacet);
			}
			else if (aFacet.fVertices[2].fVertex[lIndex] <= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[2].fVertex[lIndex]) / (aFacet.fVertices[1].fVertex[lIndex] - aFacet.fVertices[2].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[2], aFacet.fVertices[1], pt12);

				fFarAlong = (fCutOff - aFacet.fVertices[2].fVertex[lIndex]) / (aFacet.fVertices[0].fVertex[lIndex] - aFacet.fVertices[2].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[2], aFacet.fVertices[0], pt02);							

				aFacet.fVertices[2] = pt02;
				accu.AccumulateFacet(&aFacet);

				aFacet.fVertices[0] = pt02;
				aFacet.fVertices[2] = pt12;
				accu.AccumulateFacet(&aFacet);
			}
		}
		else if (lUnderLine == 1)
		{
			//we chop down the facet and do a partial
			if (aFacet.fVertices[0].fVertex[lIndex] >= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[0].fVertex[lIndex]) / (aFacet.fVertices[1].fVertex[lIndex] - aFacet.fVertices[0].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[0], aFacet.fVertices[1], aFacet.fVertices[1]);

				fFarAlong = (fCutOff - aFacet.fVertices[0].fVertex[lIndex]) / (aFacet.fVertices[2].fVertex[lIndex] - aFacet.fVertices[0].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[0], aFacet.fVertices[2], aFacet.fVertices[2]);							

				accu.AccumulateFacet(&aFacet);
			}
			else if (aFacet.fVertices[1].fVertex[lIndex] >= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[1].fVertex[lIndex]) / (aFacet.fVertices[0].fVertex[lIndex] - aFacet.fVertices[1].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[1], aFacet.fVertices[0], aFacet.fVertices[0]);

				fFarAlong = (fCutOff - aFacet.fVertices[1].fVertex[lIndex]) / (aFacet.fVertices[2].fVertex[lIndex] - aFacet.fVertices[1].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[1], aFacet.fVertices[2], aFacet.fVertices[2]);							

				accu.AccumulateFacet(&aFacet);
			}
			else if (aFacet.fVertices[2].fVertex[lIndex] >= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[2].fVertex[lIndex]) / (aFacet.fVertices[1].fVertex[lIndex] - aFacet.fVertices[2].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[2], aFacet.fVertices[1], aFacet.fVertices[1]);

				fFarAlong = (fCutOff - aFacet.fVertices[2].fVertex[lIndex]) / (aFacet.fVertices[0].fVertex[lIndex] - aFacet.fVertices[2].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[2], aFacet.fVertices[0], aFacet.fVertices[0]);							

				accu.AccumulateFacet(&aFacet);
			}
			
		}
		//else 
		//{
		//either all are over the line, or some are online and some are overline and we still throw it away
		//}
	}

}

void Eraser::DoPositiveDirection(const uint& lIndex,const TBBox3D& range, FacetMeshAccumulator& accu, FacetMeshFacetIterator& iter)
{
	real32 fCutOff, fFarAlong;
	TFacet3D aFacet;
	TVertex3D pt02, pt01, pt12;
	uint32 lOnLine, lUnderLine, lOverLine, lFacetCount = 0;

	fCutOff = range.fMax[lIndex] - (range.fMax[lIndex] - range.fMin[lIndex]) * fData.fErase;
	for (iter.First(); iter.More(); iter.Next()) 
	{
		aFacet = iter.GetFacet();
		lOnLine = 0; lUnderLine = 0; lOverLine = 0;
		if (aFacet.fVertices[0].fVertex[lIndex] < fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[0].fVertex[lIndex] > fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (aFacet.fVertices[1].fVertex[lIndex] < fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[1].fVertex[lIndex] > fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (aFacet.fVertices[2].fVertex[lIndex] < fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[2].fVertex[lIndex] > fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}


		if (lUnderLine + lOnLine == 3)
		{
			//we grab the full facet					
			lFacetCount++;
		}
		else if (lUnderLine == 2)
		{
			//we chop down the facet and do a partial
			lFacetCount+=2;
		}
		else if (lUnderLine == 1)
		{
			lFacetCount++;
		}
		//else 
		//{
		//either all are over the line, or some are online and some are overline and we still throw it away
		//}
	}
	accu.PrepareAccumulation(lFacetCount);
	for (iter.First(); iter.More(); iter.Next()) 
	{
		aFacet = iter.GetFacet();
		lOnLine = 0; lUnderLine = 0; lOverLine = 0;
		if (aFacet.fVertices[0].fVertex[lIndex] < fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[0].fVertex[lIndex] > fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (aFacet.fVertices[1].fVertex[lIndex] < fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[1].fVertex[lIndex] > fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}
		if (aFacet.fVertices[2].fVertex[lIndex] < fCutOff)
		{
			lUnderLine++;
		}
		else if (aFacet.fVertices[2].fVertex[lIndex] > fCutOff)
		{
			lOverLine++;
		}
		else
		{
			lOnLine++;
		}


		if (lUnderLine + lOnLine == 3)
		{
			//we grab the full facet					
			accu.AccumulateFacet(&aFacet);
		}
		else if (lUnderLine == 2)
		{
			//we chop down the facet and do a partial
			if (aFacet.fVertices[0].fVertex[lIndex] >= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[0].fVertex[lIndex]) / (aFacet.fVertices[1].fVertex[lIndex] - aFacet.fVertices[0].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[0], aFacet.fVertices[1], pt01);

				fFarAlong = (fCutOff - aFacet.fVertices[0].fVertex[lIndex]) / (aFacet.fVertices[2].fVertex[lIndex] - aFacet.fVertices[0].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[0], aFacet.fVertices[2], pt02);							

				aFacet.fVertices[0] = pt01;
				accu.AccumulateFacet(&aFacet);

				aFacet.fVertices[1] = aFacet.fVertices[2];
				aFacet.fVertices[2] = pt02;
				accu.AccumulateFacet(&aFacet);

			}
			else if (aFacet.fVertices[1].fVertex[lIndex] >= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[1].fVertex[lIndex]) / (aFacet.fVertices[0].fVertex[lIndex] - aFacet.fVertices[1].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[1], aFacet.fVertices[0], pt01);

				fFarAlong = (fCutOff - aFacet.fVertices[1].fVertex[lIndex]) / (aFacet.fVertices[2].fVertex[lIndex] - aFacet.fVertices[1].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[1], aFacet.fVertices[2], pt12);							

				aFacet.fVertices[1] = pt12;
				accu.AccumulateFacet(&aFacet);

				aFacet.fVertices[1] = pt01;
				aFacet.fVertices[2] = pt12;
				accu.AccumulateFacet(&aFacet);
			}
			else if (aFacet.fVertices[2].fVertex[lIndex] >= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[2].fVertex[lIndex]) / (aFacet.fVertices[1].fVertex[lIndex] - aFacet.fVertices[2].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[2], aFacet.fVertices[1], pt12);

				fFarAlong = (fCutOff - aFacet.fVertices[2].fVertex[lIndex]) / (aFacet.fVertices[0].fVertex[lIndex] - aFacet.fVertices[2].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[2], aFacet.fVertices[0], pt02);							

				aFacet.fVertices[2] = pt02;
				accu.AccumulateFacet(&aFacet);

				aFacet.fVertices[0] = pt02;
				aFacet.fVertices[2] = pt12;
				accu.AccumulateFacet(&aFacet);
			}
		}
		else if (lUnderLine == 1)
		{
			//we chop down the facet and do a partial
			if (aFacet.fVertices[0].fVertex[lIndex] <= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[0].fVertex[lIndex]) / (aFacet.fVertices[1].fVertex[lIndex] - aFacet.fVertices[0].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[0], aFacet.fVertices[1], aFacet.fVertices[1]);

				fFarAlong = (fCutOff - aFacet.fVertices[0].fVertex[lIndex]) / (aFacet.fVertices[2].fVertex[lIndex] - aFacet.fVertices[0].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[0], aFacet.fVertices[2], aFacet.fVertices[2]);							

				accu.AccumulateFacet(&aFacet);
			}
			else if (aFacet.fVertices[1].fVertex[lIndex] <= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[1].fVertex[lIndex]) / (aFacet.fVertices[0].fVertex[lIndex] - aFacet.fVertices[1].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[1], aFacet.fVertices[0], aFacet.fVertices[0]);

				fFarAlong = (fCutOff - aFacet.fVertices[1].fVertex[lIndex]) / (aFacet.fVertices[2].fVertex[lIndex] - aFacet.fVertices[1].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[1], aFacet.fVertices[2], aFacet.fVertices[2]);							

				accu.AccumulateFacet(&aFacet);
			}
			else if (aFacet.fVertices[2].fVertex[lIndex] <= fCutOff)
			{
				fFarAlong = (fCutOff - aFacet.fVertices[2].fVertex[lIndex]) / (aFacet.fVertices[1].fVertex[lIndex] - aFacet.fVertices[2].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[2], aFacet.fVertices[1], aFacet.fVertices[1]);

				fFarAlong = (fCutOff - aFacet.fVertices[2].fVertex[lIndex]) / (aFacet.fVertices[0].fVertex[lIndex] - aFacet.fVertices[2].fVertex[lIndex]);
				FillPoint(fFarAlong, aFacet.fVertices[2], aFacet.fVertices[0], aFacet.fVertices[0]);							

				accu.AccumulateFacet(&aFacet);
			}
			
		}
		//else 
		//{
		//either all are over the line, or some are online and some are overline and we still throw it away
		//}
	}
}