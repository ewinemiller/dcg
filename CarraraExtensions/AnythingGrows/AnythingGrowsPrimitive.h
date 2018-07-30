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
#ifndef __AnythingGrowsPrimitive__
#define __AnythingGrowsPrimitive__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "AnythingGrowsPrimitiveDef.h"
#include "BasicPrimitive.h"
#include "I3DShShader.h"
#include "IMFPart.h"
#include "IMFResponder.h"
#include "HairGenerator.h"
#include "copyright.h"


extern const MCGUID CLSID_AnythingGrowsPrimitive;
extern const MCGUID IID_AnythingGrowsPrimitive;

class AnythingGrowsPrimitive : public TBasicPrimitive, public HairGenerator, public IAnythingGrowsStrandGenerator
{
	public :  
		AnythingGrowsPrimitive()
		{
			fData.lPreviewMode = pmLow;
			lStrandSpaceCount = 1;
		#if VERSIONNUMBER >= 0x050000
			bInGetBoundingBox = false;
		#endif
		};
		~AnythingGrowsPrimitive()
		{
			warnings = NULL;
		};
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(AnythingGrowsData); }
		
		STANDARD_RELEASE;
		virtual MCCOMErr	MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj)
		{
			if (MCIsEqualIID(riid, IID_AnythingGrowsPrimitive))
			{
				TMCCountedGetHelper<AnythingGrowsPrimitive> result(ppvObj);
				result = (AnythingGrowsPrimitive*)this;
				return MC_S_OK;
			} else if (MCIsEqualIID(riid, IID_IAnythingGrowsStrandGenerator)) {
				TMCCountedGetHelper<IAnythingGrowsStrandGenerator> result(ppvObj);
				result = (IAnythingGrowsStrandGenerator*)this;
				return MC_S_OK;
			}
			return TBasicPrimitive::QueryInterface(riid, ppvObj);
		};

		virtual uint32 			MCCOMAPI AddRef() {
			return TBasicPrimitive::AddRef();
		};

  		// I3DExDataExchanger methods :
		virtual void* MCCOMAPI GetExtensionDataBuffer()
		{
			return &fData;
		};

		virtual int16	 MCCOMAPI GetResID()
		{
			return 310;
		};

		virtual MCCOMErr MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);  

		virtual MCCOMErr MCCOMAPI ExtensionDataChanged()
		{
			if (fData.lDomainPicker == -2) 
			{
				fData.sDomain = "";
				fData.lDomainPicker = -1;
			}
			else if (fData.lDomainPicker != -1)  
			{
				TMCString255 name;
				TMCCountedPtr<I3DShInstance> instance;
				TMCCountedPtr<I3DShObject> object;
				TMCCountedPtr<I3DShTreeElement>	tree;
				if (fData.sObjectName.Length() > 0) 
				{
					GetTreeElementByName(tree, fData.sObjectName);
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
				}
				fData.lDomainPicker = -1;
			}
			return HairGenerator::ExtensionDataChanged();
		};

		//virtual void	 MCCOMAPI Clone(IExDataExchanger**,IMCUnknown* pUnkOuter);

		// Geometric Calls
		virtual void	 MCCOMAPI GetBoundingBox	(TBBox3D& bbox);
		virtual MCCOMErr MCCOMAPI GetNbrLOD			(int16& nbrLod)
		{
			nbrLod = 2;
			return MC_S_OK;
		};

		virtual MCCOMErr MCCOMAPI GetLOD			(int16 lodIndex, real& lod)
		{
			switch(lodIndex) {
				case 0: 
					lod = 0;
					break;
				case 1:
					lod = .000001f;
					break;
				}
			return MC_S_OK;
		};

		virtual MCCOMErr MCCOMAPI GetFacetMesh(uint32 lodIndex, FacetMesh** outMesh);	
		virtual MCCOMErr MCCOMAPI Grow(uint32 lodIndex, 
				FacetMesh** outMesh, 
				AnythingGrowsStrands** strands, 
				GenerationMode generationMode);	
		virtual boolean	 MCCOMAPI AutoSwitchToModeler() const
		{	
			return true;
		};


		// Shading Calls
		virtual uint32	 MCCOMAPI GetUVSpaceCount	();

		virtual MCCOMErr MCCOMAPI GetUVSpace		(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo);
		
		TMCString255 GetMasterShaderName();

		virtual MCCOMErr MCCOMAPI GetStrands(AnythingGrowsStrands** strands);
	private:
		TMCCountedPtr<IMFPart> warnings;

		void UpdateWarnings();
		#if VERSIONNUMBER >= 0x050000
		boolean bInGetBoundingBox;
		#endif
		void setScene();

};

#endif // __AnythingGrowsPrimitive__