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
#ifndef __GCGenerator__
#define __GCGenerator__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "GCGeneratorDef.h"
#include "BasicTerrainFilter.h"
#include "dem.h"
#include "IShTokenStream.h"
#include "ElevationMesh.h"

struct GCGeneratorPublicData
{
	TMCString255 sDescription;
	TMCString255 sResolution;
	boolean bUseFileHeight;
	real32 fBottom;
	real32 fTop;
	ActionNumber lFiltering;
	real32 northSize;
	real32 eastSize;
};

extern const MCGUID CLSID_GCGenerator;


class GCGenerator : public TBasicTerrainFilter, public IExStreamIO, public ElevationMesh
{
	public:
		GCGenerator()
		{
			fData.bUseFileHeight = true;
			fData.fTop = 12000;
			fData.fBottom = 0;
			fData.lFiltering = fiBilinear;
			fData.eastSize = 120000;
			fData.northSize = 120000;
		};
		~GCGenerator()
		{
		};
		virtual MCCOMErr 	MCCOMAPI QueryInterface			(const MCIID &riid, void** ppvObj);
	 	virtual uint32 		MCCOMAPI AddRef 				();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(GCGeneratorPublicData); }
		virtual void MCCOMAPI Filter(TMCArray<real>& heightField, TVector2& heightBound,
								 const TIndex2& size, const TVector2& cellSize);
#if (VERSIONNUMBER >= 0x060000)
		virtual void MCCOMAPI SetDefaultSizes(real &sizex, real &sizey) {};
#endif

		virtual boolean MCCOMAPI CanBuildPreview()
			{return true;};
		virtual void*		MCCOMAPI GetExtensionDataBuffer(){return &fData;};
		virtual void		MCCOMAPI Clone(IExDataExchanger**,IMCUnknown* pUnkOuter);
		virtual MCCOMErr	MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);  

		// IExStreamIO calls
		virtual MCCOMErr			MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
		virtual MCCOMErr			MCCOMAPI Write(IShTokenStream* stream);
#if (VERSIONNUMBER >= 0x060000)
		virtual MCCOMErr MCCOMAPI FinishRead(IStreamContext* streamContext) {return MC_S_OK;};
#endif

	protected:
		GCGeneratorPublicData fData;

};

#endif