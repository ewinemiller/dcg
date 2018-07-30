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
#ifndef __AGrPrim__
#define __AGrPrim__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "AGRPrimDef.h"
#include "BasicPrimitive.h"
#include "I3DShShader.h"
#include "AGrPrimData.h"
#include	"IMFPart.h"
#include	"IMFResponder.h"

// Define the AGrPrim CLSID ( see the AGrPrimDef.h file to get R_CLSID_AGrPrim value )

//MCDEFINE_GUID2(CLSID_AGrPrim, R_CLSID_AGrPrim);
//MCDEFINE_GUID2 (IID_AGrPrim, R_IID_AGrPrim);
extern const MCGUID CLSID_AGrPrim;
extern const MCGUID IID_AGrPrim;

#define PRIMITIVE_CONE 0
#define PRIMITIVE_CUBE 1
#define PRIMITIVE_CYLINDER 2
#define PRIMITIVE_PLANE 3
#define PRIMITIVE_SPHERE 4

class AGrPrim : public TBasicPrimitive
{
	public :  
		AGrPrim();
		~AGrPrim();
		
		STANDARD_RELEASE;
		virtual MCCOMErr	MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);

  		// I3DExDataExchanger methods :
		virtual void*	 MCCOMAPI GetExtensionDataBuffer();
		virtual int16	 MCCOMAPI GetResID();
		virtual MCCOMErr MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);  
		virtual MCCOMErr MCCOMAPI ExtensionDataChanged();
		//virtual void	 MCCOMAPI Clone(IExDataExchanger**,IMCUnknown* pUnkOuter);
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(AGrPrimData); }

		// Geometric Calls
		virtual void	 MCCOMAPI GetBoundingBox	(TBBox3D& bbox);
		virtual MCCOMErr MCCOMAPI GetNbrLOD			(int16& nbrLod);
		virtual MCCOMErr   MCCOMAPI GetLOD			(int16 lodIndex, real& lod);
		virtual MCCOMErr MCCOMAPI GetFacetMesh		(uint32 lodIndex, FacetMesh** outMesh);	// lod = level of detail. See theory text in .cpp
		virtual boolean	 MCCOMAPI AutoSwitchToModeler() const;


		// Shading Calls
		virtual uint32	 MCCOMAPI GetUVSpaceCount	();
		virtual MCCOMErr MCCOMAPI GetUVSpace		(uint32 uvSpaceID, UVSpaceInfo* uvSpaceInfo);
		
		TMCString255 GetMasterShaderName();
	private:
		TMCCountedPtr<I3DShScene> scene;
		AGrPrimData fData;
		TMCCountedPtr<IMFPart> warnings;

		void UpdateWarnings();
};

#endif // __AGrPrim__