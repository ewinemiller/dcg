/*  Cognito - plug-in for Carrara
    Copyright (C) 2001 Michael Clarke

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
#ifndef __CognitoData__
#define __CognitoData__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "IShRasterLayerUtilities.h"
#include "CognitoDef.h"
#include "COMUtilities.h"
#include "BasicModifiers.h"  
#include "BasicDataComponent.h"
#include "BasicCOMImplementations.h"

extern const MCGUID CLSID_CognitoData;
extern const MCGUID IID_CognitoData;
extern const MCGUID IID_CognitoDataExt;

#define ACT_LINK_GEAR			 0
#define	ACT_LINK_AXEL			 1
#define	ACT_LINK_CHAIN			 2
#define	ACT_LINK_XCHAIN			 3
#define	ACT_LINK_RACK			 4
#define	ACT_LINK_CAM			 5
#define	ACT_LINK_WORM			 6
#define	ACT_LINK_RAD_OFFSET		 7
#define	ACT_LINK_PUSHROD		 8
#define	ACT_LINK_PISTON			 9
#define	ACT_LINK_PUSHROD_PISTON		10
#define	ACT_LINK_PUSHROD_CYLINDER	11
#define	ACT_LINK_ROCKER          	12
#define	ACT_LINK_TRIP_HAMMER     	13

struct CognitoDataBuffer {

	real32		 rotation;	// Motor rotation
	real32	 	 pitch;		// Worm drive pitch
	real32		 offset;	// Offset
	int32		 linkage;	// How connected
	int32		 teeth;		// Number of teeth
	boolean		 motor;		// Is a motor?
	boolean		 slave;		// Is a slave?
	TMCString255	 driver;	// Name of driver object
};


class CognitoData : public TBasicDataComponent {
	
public :  
	STANDARD_RELEASE;
		
	CognitoData();
		
	MCCOMErr		MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	virtual void*		MCCOMAPI GetExtensionDataBuffer();
	virtual MCCOMErr	MCCOMAPI ExtensionDataChanged();

	virtual MCCOMErr	MCCOMAPI GetRotation(I3DShScene *scene, double &rotation);
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(CognitoDataBuffer); }
	
private :

	void SaveData();	
	
	int32	depth;

	CognitoDataBuffer fData;      // Live data
	CognitoDataBuffer fSave;      // Copy used to detect changes
};

#endif
#ifndef __Cognito__
#define __Cognito__  

#if CP_PRAGMA_ONCE
#pragma once
#endif


// define the Cognito CLSID

extern const MCGUID CLSID_Cognito;


struct CognitoBuffer {
	real32		vectorX;	
	real32		vectorY;
	real32		vectorZ;
	real32		radius;
	boolean		around_x;
	boolean		around_y;
	boolean		around_z;
	TMCString255	hint_vector;	// Hint for vector
	TMCString255	hint_parm;	// Hint for parameter
	TMCString255	hint_hot;	// Hint for hot point
	TMCString255	hint_parm2;	// Hint for parameter
	real32		radius2;
};


class Cognito : public TBasicTreeModifier {

public :  
	STANDARD_RELEASE;
	
	Cognito();
	
	MCCOMErr		MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
	virtual void*		MCCOMAPI GetExtensionDataBuffer();
	virtual MCCOMErr	MCCOMAPI ExtensionDataChanged();
	virtual MCCOMErr	MCCOMAPI Apply(I3DShTreeElement* tree);
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(CognitoBuffer); }
	
private :

	CognitoBuffer *GetBuffer(I3DShTreeElement * tree);

	CognitoDataBuffer *GetDataBuffer(I3DShTreeElement * tree);

	double	GetRotation(I3DShTreeElement * tree);
	
	double	findAngle(double opp, double adj);
	double	AngleFromVertical(double x, double y, double z);

	int32	GetLinkage(I3DShTreeElement * tree, CognitoDataBuffer * treeCDB);
		
	TMCCountedPtr<I3DShTreeElement> GetDriver(I3DShTreeElement *tree, CognitoDataBuffer *treeCogDataBuf);
	
	CognitoBuffer fData;

	int32	depth;

	TTreeTransform		lastTree;

	bool checkTree;

	MicroTick		checkTime;
};

#endif


