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
#ifndef __AnythingGrowsDeformer__
#define __AnythingGrowsDeformer__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicModifiers.h"   
#include "../AnythingGrows/AnythingGrowsDeformerDef.h"
#include "IMFDialogPart.h"
#include "IMFPart.h"
#include "IMFResponder.h"
#include "../AnythingGrows/HairGenerator.h"


//****** Globally Unique Id *************************************************
extern const MCGUID CLSID_AnythingGrowsDeformer;
extern const MCGUID IID_AnythingGrowsDeformer;

// Deformer Object :
class AnythingGrowsDeformer : public TBasicDeformModifier, public HairGenerator
{
	private:
		TMCCountedPtr<I3DShTreeElement>	tree;

		void CopyData(AnythingGrowsDeformer* dest) const
		{
			dest->fData = fData;
			if (scene != NULL) {
				dest->scene = scene;
				}
			if (tree != NULL) 
			{
				dest->tree = tree;
			}
		};

	public :  

		AnythingGrowsDeformer()
		{
			fFlags.SetMasked(TModifierFlags::kDeformModifier);
		};
		~AnythingGrowsDeformer()
		{
			tree = NULL;
		}
		// IUnknown Interface :
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(AnythingGrowsData); }
		virtual MCCOMErr	MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
		
  		// I3DExDataExchanger methods :
		virtual void*		MCCOMAPI GetExtensionDataBuffer()
		{
			return &fData;	
		};

		virtual int16		MCCOMAPI GetResID()
		{
			return 308;
		};

		virtual MCCOMErr MCCOMAPI ExtensionDataChanged()
		{
			if (fData.lDomainPicker == -2) 
			{
				fData.sDomain = "";
				fData.lDomainPicker = -1;
			}
			else if (fData.lDomainPicker != -1)  {
				TMCString255 name;
				TMCCountedPtr<I3DShInstance> instance;
				TMCCountedPtr<I3DShObject> object;
				if (tree != NULL) 
				{
					if (tree->QueryInterface(IID_I3DShInstance, (void**)&instance) == MC_S_OK) 
					{
						if (instance->Get3DObject(&object) == MC_S_OK)
						{
							UVSpaceInfo uvSpaceInfo;
							object->GetUVSpace(fData.lDomainPicker, &uvSpaceInfo);
							fData.sDomain = uvSpaceInfo.fName;
						}
					}
				}
				fData.lDomainPicker = -1;
				}


			return HairGenerator::ExtensionDataChanged();
		};

		virtual MCCOMErr	MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);  
		virtual void		MCCOMAPI Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter)
		{
			TMCCountedCreateHelper<IExDataExchanger>	result(res);

			AnythingGrowsDeformer* theClone = new AnythingGrowsDeformer();
			ThrowIfNil(theClone);
			theClone->SetControllingUnknown(pUnkOuter);
			CopyData(theClone);

			result = (IExDataExchanger*) theClone;
		};

		// I3DExDeformer methods :
		virtual void			MCCOMAPI SetBoundingBox		(const TBBox3D& bbox)
		{
		};

		virtual MCCOMErr		MCCOMAPI DeformPoint		(const TVector3& point, TVector3& result)
		{
			return MC_S_OK;
		};

		virtual MCCOMErr		MCCOMAPI DeformFacetMesh	(real lod,FacetMesh* in, FacetMesh** outMesh);

		virtual MCCOMErr		MCCOMAPI DeformBBox			(const TBBox3D& in, TBBox3D& out)
		{
			out = in;
			out.fMin.x -= fData.fLength;
			out.fMin.y -= fData.fLength;
			out.fMin.z -= fData.fLength;
			out.fMax.x += fData.fLength;
			out.fMax.y += fData.fLength;
			out.fMax.z += fData.fLength;
			return MC_S_OK;
		};
		
		virtual TModifierFlags	MCCOMAPI GetModifierFlags	()	
		{
			return fFlags; 
		};


		TMCString255 GetMasterShaderName()
		{
			return fData.MasterShader;
		};

};

#endif