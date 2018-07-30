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
#include "PublicUtilities.h"
#include "GeminiDeformer.h"
#include "GeminiDLL.h"


enum Keep {kXPositive = 0, kXNegative = 1, kYPositive = 2, kYNegative = 3, kZPositive = 4, kZNegative = 5};

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_GeminiDeformer(R_CLSID_GeminiDeformer);
const MCGUID IID_GeminiDeformer(R_IID_GeminiDeformer);
#else
const MCGUID CLSID_GeminiDeformer={R_CLSID_GeminiDeformer};
const MCGUID IID_GeminiDeformer={R_IID_GeminiDeformer};
#endif

MCCOMErr GeminiDeformer::QueryInterface(const MCIID& riid, void** ppvObj)
{
 
	if (MCIsEqualIID(riid, IID_GeminiDeformer))
	{ 
		TMCCountedGetHelper<GeminiDeformer> result(ppvObj);
		result = (GeminiDeformer*)this;
		return MC_S_OK;
	}
	return TBasicDeformModifier::QueryInterface(riid, ppvObj);
}

GeminiDeformer::GeminiDeformer()
{
	fFlags.SetMasked(TModifierFlags::kDeformModifier);
	fData.bEnabled = true;
	fData.lKeep = kZNegative;
	fData.fMargin = 0.10f;
}

MCCOMErr GeminiDeformer::DeformFacetMesh(real lod,FacetMesh* in, FacetMesh** outMesh) {

	if (fData.bEnabled == false)
	{
		in->Clone(outMesh);
		return MC_S_OK; 
	}
	FacetMeshAccumulator accu;
	FacetMeshFacetIterator iter;
	TFacet3D aFacet;
	iter.Initialize(in);
	uint32 lFacetCount = 0;
	TVertex3D temp;

	switch (fData.lKeep)
	{

		case kYPositive:
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.y >= - fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.y >= - fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.y >= - fData.fMargin))
				{
					lFacetCount+=2;
				}
			}
			accu.PrepareAccumulation(lFacetCount);
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.y >= - fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.y >= - fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.y >= - fData.fMargin))
				{
					if (aFacet.fVertices[0].fVertex.y <= fData.fMargin)
					{
						aFacet.fVertices[0].fVertex.y = aFacet.fVertices[0].fNormal.y = 0;
					}
					if (aFacet.fVertices[1].fVertex.y <= fData.fMargin)
					{
						aFacet.fVertices[1].fVertex.y = aFacet.fVertices[1].fNormal.y = 0;
					}
					if (aFacet.fVertices[2].fVertex.y <= fData.fMargin)
					{
						aFacet.fVertices[2].fVertex.y = aFacet.fVertices[2].fNormal.y = 0;
					}

					accu.AccumulateFacet(&aFacet);
					aFacet.fVertices[0].fVertex.y = -aFacet.fVertices[0].fVertex.y;
					aFacet.fVertices[1].fVertex.y = -aFacet.fVertices[1].fVertex.y;
					aFacet.fVertices[2].fVertex.y = -aFacet.fVertices[2].fVertex.y;
					aFacet.fVertices[0].fNormal.y = -aFacet.fVertices[0].fNormal.y;
					aFacet.fVertices[1].fNormal.y = -aFacet.fVertices[1].fNormal.y;
					aFacet.fVertices[2].fNormal.y = -aFacet.fVertices[2].fNormal.y;
					temp = aFacet.fVertices[1];
					aFacet.fVertices[1] = aFacet.fVertices[2];
					aFacet.fVertices[2] = temp;
					accu.AccumulateFacet(&aFacet);
				}
			}
			break;
		case kYNegative:
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.y <= fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.y <= fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.y <= fData.fMargin))
				{
						lFacetCount+=2;
				}
			}
			accu.PrepareAccumulation(lFacetCount);
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.y <= fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.y <= fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.y <= fData.fMargin))
				{
					if (aFacet.fVertices[0].fVertex.y >= - fData.fMargin)
					{
						aFacet.fVertices[0].fVertex.y = aFacet.fVertices[0].fNormal.y = 0;
					}
					if (aFacet.fVertices[1].fVertex.y >= - fData.fMargin)
					{
						aFacet.fVertices[1].fVertex.y = aFacet.fVertices[1].fNormal.y = 0;
					}
					if (aFacet.fVertices[2].fVertex.y >= - fData.fMargin)
					{
						aFacet.fVertices[2].fVertex.y = aFacet.fVertices[2].fNormal.y = 0;
					}

					accu.AccumulateFacet(&aFacet);
					aFacet.fVertices[0].fVertex.y = -aFacet.fVertices[0].fVertex.y;
					aFacet.fVertices[1].fVertex.y = -aFacet.fVertices[1].fVertex.y;
					aFacet.fVertices[2].fVertex.y = -aFacet.fVertices[2].fVertex.y;
					aFacet.fVertices[0].fNormal.y = -aFacet.fVertices[0].fNormal.y;
					aFacet.fVertices[1].fNormal.y = -aFacet.fVertices[1].fNormal.y;
					aFacet.fVertices[2].fNormal.y = -aFacet.fVertices[2].fNormal.y;
					temp = aFacet.fVertices[1];
					aFacet.fVertices[1] = aFacet.fVertices[2];
					aFacet.fVertices[2] = temp;
					accu.AccumulateFacet(&aFacet);
				}
			}
			break;
		case kXPositive:
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.x >= - fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.x >= - fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.x >= - fData.fMargin))
				{
						lFacetCount+=2;
				}
			}
			accu.PrepareAccumulation(lFacetCount);
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.x >= - fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.x >= - fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.x >= - fData.fMargin))
				{
					if (aFacet.fVertices[0].fVertex.x <= fData.fMargin)
					{
						aFacet.fVertices[0].fVertex.x = aFacet.fVertices[0].fNormal.x = 0;
					}
					if (aFacet.fVertices[1].fVertex.x <= fData.fMargin)
					{
						aFacet.fVertices[1].fVertex.x = aFacet.fVertices[1].fNormal.x = 0;
					}
					if (aFacet.fVertices[2].fVertex.x <= fData.fMargin)
					{
						aFacet.fVertices[2].fVertex.x = aFacet.fVertices[2].fNormal.x = 0;
					}

					accu.AccumulateFacet(&aFacet);
					aFacet.fVertices[0].fVertex.x = -aFacet.fVertices[0].fVertex.x;
					aFacet.fVertices[1].fVertex.x = -aFacet.fVertices[1].fVertex.x;
					aFacet.fVertices[2].fVertex.x = -aFacet.fVertices[2].fVertex.x;
					aFacet.fVertices[0].fNormal.x = -aFacet.fVertices[0].fNormal.x;
					aFacet.fVertices[1].fNormal.x = -aFacet.fVertices[1].fNormal.x;
					aFacet.fVertices[2].fNormal.x = -aFacet.fVertices[2].fNormal.x;
					temp = aFacet.fVertices[1];
					aFacet.fVertices[1] = aFacet.fVertices[2];
					aFacet.fVertices[2] = temp;
					accu.AccumulateFacet(&aFacet);
				}
			}
			break;
		case kXNegative:
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.x <= fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.x <= fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.x <= fData.fMargin))
				{
						lFacetCount+=2;
				}
			}
			accu.PrepareAccumulation(lFacetCount);
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.x <= fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.x <= fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.x <= fData.fMargin))
				{
					if (aFacet.fVertices[0].fVertex.x >= - fData.fMargin)
					{
						aFacet.fVertices[0].fVertex.x = aFacet.fVertices[0].fNormal.x = 0;
					}
					if (aFacet.fVertices[1].fVertex.x >= - fData.fMargin)
					{
						aFacet.fVertices[1].fVertex.x = aFacet.fVertices[1].fNormal.x = 0;
					}
					if (aFacet.fVertices[2].fVertex.x >= - fData.fMargin)
					{
						aFacet.fVertices[2].fVertex.x = aFacet.fVertices[2].fNormal.x = 0;
					}

					accu.AccumulateFacet(&aFacet);
					aFacet.fVertices[0].fVertex.x = -aFacet.fVertices[0].fVertex.x;
					aFacet.fVertices[1].fVertex.x = -aFacet.fVertices[1].fVertex.x;
					aFacet.fVertices[2].fVertex.x = -aFacet.fVertices[2].fVertex.x;
					aFacet.fVertices[0].fNormal.x = -aFacet.fVertices[0].fNormal.x;
					aFacet.fVertices[1].fNormal.x = -aFacet.fVertices[1].fNormal.x;
					aFacet.fVertices[2].fNormal.x = -aFacet.fVertices[2].fNormal.x;
					temp = aFacet.fVertices[1];
					aFacet.fVertices[1] = aFacet.fVertices[2];
					aFacet.fVertices[2] = temp;
					accu.AccumulateFacet(&aFacet);
				}
			}
			break;
		case kZPositive:
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.z >= - fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.z >= - fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.z >= - fData.fMargin))
				{
						lFacetCount+=2;
				}
			}
			accu.PrepareAccumulation(lFacetCount);
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.z >= - fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.z >= - fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.z >= - fData.fMargin))
				{
					if (aFacet.fVertices[0].fVertex.z <= fData.fMargin)
					{
						aFacet.fVertices[0].fVertex.z = aFacet.fVertices[0].fNormal.z = 0;
					}
					if (aFacet.fVertices[1].fVertex.z <= fData.fMargin)
					{
						aFacet.fVertices[1].fVertex.z = aFacet.fVertices[1].fNormal.z = 0;
					}
					if (aFacet.fVertices[2].fVertex.z <= fData.fMargin)
					{
						aFacet.fVertices[2].fVertex.z = aFacet.fVertices[2].fNormal.z = 0;
					}

					accu.AccumulateFacet(&aFacet);
					aFacet.fVertices[0].fVertex.z = -aFacet.fVertices[0].fVertex.z;
					aFacet.fVertices[1].fVertex.z = -aFacet.fVertices[1].fVertex.z;
					aFacet.fVertices[2].fVertex.z = -aFacet.fVertices[2].fVertex.z;
					aFacet.fVertices[0].fNormal.z = -aFacet.fVertices[0].fNormal.z;
					aFacet.fVertices[1].fNormal.z = -aFacet.fVertices[1].fNormal.z;
					aFacet.fVertices[2].fNormal.z = -aFacet.fVertices[2].fNormal.z;
					temp = aFacet.fVertices[1];
					aFacet.fVertices[1] = aFacet.fVertices[2];
					aFacet.fVertices[2] = temp;
					accu.AccumulateFacet(&aFacet);
				}
			}
			break;
		case kZNegative:
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.z <= fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.z <= fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.z <= fData.fMargin))
				{
						lFacetCount+=2;
				}
			}
			accu.PrepareAccumulation(lFacetCount);
			for (iter.First(); iter.More(); iter.Next()) 
			{
				aFacet = iter.GetFacet();
				if ((aFacet.fVertices[0].fVertex.z <= fData.fMargin)
					&&(aFacet.fVertices[1].fVertex.z <= fData.fMargin)
					&&(aFacet.fVertices[2].fVertex.z <= fData.fMargin))
				{
					if (aFacet.fVertices[0].fVertex.z >= - fData.fMargin)
					{
						aFacet.fVertices[0].fVertex.z = aFacet.fVertices[0].fNormal.z = 0;
					}
					if (aFacet.fVertices[1].fVertex.z >= - fData.fMargin)
					{
						aFacet.fVertices[1].fVertex.z = aFacet.fVertices[1].fNormal.z = 0;
					}
					if (aFacet.fVertices[2].fVertex.z >= - fData.fMargin)
					{
						aFacet.fVertices[2].fVertex.z = aFacet.fVertices[2].fNormal.z = 0;
					}

					accu.AccumulateFacet(&aFacet);
					aFacet.fVertices[0].fVertex.z = -aFacet.fVertices[0].fVertex.z;
					aFacet.fVertices[1].fVertex.z = -aFacet.fVertices[1].fVertex.z;
					aFacet.fVertices[2].fVertex.z = -aFacet.fVertices[2].fVertex.z;
					aFacet.fVertices[0].fNormal.z = -aFacet.fVertices[0].fNormal.z;
					aFacet.fVertices[1].fNormal.z = -aFacet.fVertices[1].fNormal.z;
					aFacet.fVertices[2].fNormal.z = -aFacet.fVertices[2].fNormal.z;
					temp = aFacet.fVertices[1];
					aFacet.fVertices[1] = aFacet.fVertices[2];
					aFacet.fVertices[2] = temp;
					accu.AccumulateFacet(&aFacet);
				}
			}
			break;
	}
	
	accu.MakeFacetMesh(outMesh);

	return MC_S_OK; 
}


MCCOMErr GeminiDeformer::DeformBBox(const TBBox3D& in, TBBox3D& out)
{
	out = in;
	switch (fData.lKeep)
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

