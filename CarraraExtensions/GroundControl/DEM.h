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
#ifndef __DEM__
#define __DEM__

#include "DEMDef.h"
#include "APITypes.h"
#include "COMUtilities.h"
#include "I3DShScene.h"
#include "I3DShObject.h"
#include "I3DShTreeElement.h"
#include "MCCompObj.h"
#include "MCBasicTypes.h"
#include "IShTextureMap.h"
#include "ElevationMesh.h"
#include "copyright.h"

#include "Basic3DImportExport.h"
#include "BasicCOMExtImplementations.h"
#include "BasicEnvironment.h"


struct IShFileStream;
struct I3DShScene;
struct I3DShTreeElement;
struct I3DShObject;

extern const MCGUID CLSID_DEM;
extern const MCGUID CLSID_r32;
extern const MCGUID CLSID_TER;
extern const MCGUID CLSID_BIL;
extern const MCGUID CLSID_HGT;
extern const MCGUID CLSID_PGM;
extern const MCGUID CLSID_IMG;
extern const MCGUID CLSID_ASC;
extern const MCGUID CLSID_FLT;
extern const MCGUID CLSID_PDS;
extern const MCGUID CLSID_StandardExternalPrimitive;


struct I3DShMasterGroup;

enum ImportAs {iaDEM = 2, iaMesh = 3, iaGenerator = 5};
enum DemFormat {dfUnknown = 0, dfUSGS = 1, dfVistaPro = 2, dfWorldMachine = 3, dfTerragen = 4, dfHGT = 5, dfBIL = 6, dfPGM = 7, dfErdasImagine = 8, dfArcInfoASCII};
enum GCShading {gcsNone = 1, gcsTexture = 2, gcsShader = 3};
enum GCScaling {gcscaleNone = 1, gcscaleProportion = 2, gcscaleReal = 3};
enum MeshType {gcGridPointSampling = 1, gcGridBilinearFiltering = 2, gcGridSmartSplit = 3, gcAdaptive = 4};

struct DEMImpData {
	ActionNumber	lImportAs;
	real32 fResolution;
	real32 sizeX;
	real32 sizeY;
	real32 sizeZ;
	ActionNumber shading; 
	ActionNumber scaling;
	ActionNumber meshType;
	real32 adaptiveError;
	boolean lookForSequenced;
	};

struct DEMGapAccumulator {
	uint32 row, col;
	real32 value;
};


class DEM : public TBasic3DImportFilter, public ElevationMesh
{

public:
	STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(DEMImpData); }

	DEM();
	~DEM();

	virtual MCCOMErr	MCCOMAPI DoImport(IMCFile * file, I3DShScene* scene, I3DShTreeElement* fatherTree);
	void buildBasicScene(I3DShScene *scene, I3DShTreeElement *fatherTree, TMCCountedPtr<I3DShTreeElement> &topTree);
		
	virtual boolean 	MCCOMAPI WantsOptionDialog	()  
	{
		return true;
	};
	
	virtual int16		MCCOMAPI GetResID()
	{
		return 130;
	};
	
	virtual void* MCCOMAPI GetExtensionDataBuffer()
	{
		return &fData;
	};

public:
	DEMImpData fData;
	TVector2 fullElevationRange;
	

protected:
	void readFile(uint32 frame, TMCDynamicString fileName);
	void buildDescriptionAndName();
	void buildAndShadeObject(I3DShScene* scene, I3DShTreeElement* topTree);


	void TextureFromBuffers(TMCArray<uint8>& r, TMCArray<uint8>& g, TMCArray<uint8>& b);
	void cleanUp();

	TMCCountedPtr<IShTextureMap> texture;

	I3DShMasterGroup*		fMasterGroup;

	TMCString1023 fullPathName;
	
	boolean bRequiresTexture;

	boolean bDoTexture;

	TVector2 importSize;
	TMCString255 name;

};

#endif