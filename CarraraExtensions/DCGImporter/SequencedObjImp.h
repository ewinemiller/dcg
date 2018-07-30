/*  DCG Importer - plug-in for Carrara
    Copyright (C) 2012 Eric Winemiller

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
#ifndef __SequencedObjImp__
#define __SequencedObjImp__

#include "SequencedObjImpDef.h"
#include "APITypes.h"
#include "COMUtilities.h"
#include "I3DShGroup.h"
#include "I3DShScene.h"
#include "I3DShObject.h"
#include "I3DShTreeElement.h"
#include "MCCompObj.h"
#include "MCBasicTypes.h"

#include "Basic3DImportExport.h"
#include "BasicCOMExtImplementations.h"
#include "BasicEnvironment.h"

extern const MCGUID CLSID_SequencedObjImp;

struct SequencedObjImpData {
	boolean mapObjectYToCarraraZ;
	real32 conversion;
	real32 smoothing;
	int32 objectType;
	boolean uvBetween0And1;
}; 

class SequencedObjImp : public TBasic3DImportFilter
{
public:
	STANDARD_RELEASE;

	SequencedObjImp();
	~SequencedObjImp();

	virtual int32   MCCOMAPI GetParamsBufferSize    ()
		const        { return sizeof(SequencedObjImpData); }

	//data exchanger calls
	virtual void*	 MCCOMAPI GetExtensionDataBuffer()
	{
		return &fData;
	};

	virtual MCCOMErr	MCCOMAPI DoImport(IMCFile * file, I3DShScene* scene, I3DShTreeElement* fatherTree);	

	virtual boolean 	MCCOMAPI WantsOptionDialog	()  
	{
		return true;
	};

protected:
	I3DShMasterGroup* fMasterGroup;
private:
	SequencedObjImpData fData;
	void buildName(const TMCDynamicString& fileName, TMCDynamicString& name);
	void buildBasicScene(I3DShScene *scene, I3DShTreeElement *fatherTree, TMCCountedPtr<I3DShTreeElement> &topTree);
	void synchShaders(I3DShScene* sourceScene, I3DShScene* destScene, I3DShInstance* destInstance);

};

#endif