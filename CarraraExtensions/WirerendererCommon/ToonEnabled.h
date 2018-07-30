/*  Wirerender utilities for Carrara
    Copyright (C) 2004 Eric Winemiller

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
#ifndef __TOONENABLED__
#define __TOONENABLED__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicDataComponent.h"
#include "ToonEnabledDef.h"
#include "WireRendererData.h"
#include "IShTokenStream.h"
#include "I3dShScene.h"
#include "IMFListPart.h"
#include "ToonSettings.h"
#include <string.h>

extern const MCGUID CLSID_ToonEnabled;
extern const MCGUID CLSID_WireframeEnabled;
extern const MCGUID IID_ToonEnabled;
extern const MCGUID IID_WireframeEnabled;

struct ToonEnabledData
{
	boolean bOverriden;
	real32 fLine;
	boolean synchAll;
	boolean  operator==	(const ToonEnabledData& rhs) 
	{ 
		return memcmp(this, &rhs, sizeof(ToonEnabledData)) == 0;
	}
	ToonEnabledData()
	{
		bOverriden = false;
		fLine = 1.0f;
		synchAll = true;
	};
};


class ToonEnabled: public TBasicDataComponent
	, public IExStreamIO{
public :

	// IUnknown methods
	STANDARD_RELEASE;
	virtual int32 MCCOMAPI GetParamsBufferSize ()
		const { return sizeof(ToonEnabledData); }
	ToonEnabled(RenderStyle renderStyle);
	~ToonEnabled();
	virtual void MCCOMAPI Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter);
	virtual boolean MCCOMAPI IsEqual(IExDataExchanger * otherDataExchanger);
	virtual MCCOMErr MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	virtual void* MCCOMAPI GetExtensionDataBuffer();
	virtual MCCOMErr MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);
	virtual MCCOMErr MCCOMAPI Read(IShTokenStream* stream, ReadAttributeProc readUnknown, void* privData);
	virtual MCCOMErr MCCOMAPI Write(IShTokenStream* stream);
	virtual boolean MCCOMAPI IsActive(I3DShTreeElement* tree);

#if (VERSIONNUMBER >= 0x060000)
	virtual MCCOMErr MCCOMAPI FinishRead(IStreamContext* streamContext) {return MC_S_OK;};
#endif
	virtual uint32 MCCOMAPI AddRef();

	ToonSettings toonSettings;
	void mergeLevelsWithScene();

private:
	ToonEnabledData fData; 
	ActionNumber lRenderStyle;
	TMCCountedPtr<I3DShTreeElement> tree;
	void CopyDataFrom(const ToonEnabled* source);
	void fillShadingDomainList(TMCCountedPtr<IMFListPart> list, uint32 selection);
	void enableDomainControls(TMCCountedPtr<IMFPart> parentPart, boolean enabled);

	boolean ignoreSelectionChange;
};
#endif