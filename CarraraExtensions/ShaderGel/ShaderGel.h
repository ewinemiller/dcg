/*  Shader Gel and Strobe - plug-in for Carrara
    Copyright (C) 2003 Michael Clarke

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

#ifndef __SHADERGEL__
#define __SHADERGEL__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include	"BasicCameraLightGel.h"
#include	"APITypes.h"
#include	"MCColorRGBA.h"
#include	"ShaderGelDef.h"
#include	"IShComponent.h"
#include	"I3DShScene.h"
#include	"IMFPart.h"
#include	"IMFTextPopupPart.h"


// Define the Gel CLSID ( see the GelDef.h file to get the R_CLSID_GEL value )

extern const MCGUID CLSID_ShaderGel;

// Defines for the shader popup menu

#define SHADER_POPUP_CURRENT	30001
#define SHADER_POPUP_CLEAR	30002
#define SHADER_POPUP_IGNORE	30003
#define SHADER_POPUP_REFRESH	30004

// Gel Data structure                            
struct ShaderGelData {

	real32		offsetX;
	real32		offsetY;
	real32		scaleX;
	real32		scaleY;
	real32		repeatX;
	real32		repeatY;

	real32		rotation;
	
	boolean		radialMapping;
	
	boolean		rotationFC;
	boolean		offsetXFC;
	boolean		offsetYFC;
	boolean		scaleXFC;
	boolean		scaleYFC;
	boolean		repeatXFC;
	boolean		repeatYFC;

	TMCString255	Shader; 
	
	ActionNumber		popup;
};
 
// Gel Object :

class ShaderGel : public TBasicGel {

	public:  
		ShaderGel();
		STANDARD_RELEASE;

		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(ShaderGelData); }
		// Query Interface
		
		MCCOMErr		MCCOMAPI QueryInterface(const MCIID& riid, void** ppvObj);
		
		// IExDataExchanger methods :
		virtual void*	 MCCOMAPI	GetExtensionDataBuffer();
		virtual MCCOMErr MCCOMAPI	ExtensionDataChanged();
		virtual int16	 MCCOMAPI	GetResID();
		virtual void     MCCOMAPI	Clone(IExDataExchanger** newOne, IMCUnknown* pUnkOuter);				

		// Dialog message handler
		virtual MCCOMErr MCCOMAPI	HandleEvent(MessageID message, IMFResponder* source, void* data);
		
		// I3DExLightsourceGel methods
		virtual boolean	 MCCOMAPI	GetGelValues(const TVector2& gelScreenPosition, TMCColorRGB &result);
	protected:	
//		void	CloneData(TBasicDataExchanger *destExchanger, IMCUnknown* pUnkOuter);
		
	private:
		ShaderGelData      fData;           // Gel Data (see definition of GelData structure)

		double theta;
		
		double sin_theta, cos_theta;

		double offsetX, offsetY, rotation, scaleX, scaleY, repeatX, repeatY;
		
		MicroTick	lastTime;
		
		TMCCountedPtr<I3DShScene> 	scene;
		TMCCountedPtr<ShaderGel> 	parent;
		TMCCountedPtr<IMFTextPopupPart> myPopup;

		void FillShaderList(TMCCountedPtr<IMFPart> popuppart);
		void FillShaderList2(TMCCountedPtr<IMFTextPopupPart> popuppart);

		void const CopyData(ShaderGel* dest);
		 
};
#endif 

