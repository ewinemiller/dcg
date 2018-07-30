/*  Noir - plug-in for Carrara
    Copyright (C) 2002 Michael Clarke

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

#ifndef __Noir__
#define __Noir__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicPostRenderer.h"
#include "NoirDef.h"
#include "IShChannel.h"
#include "I3DShShader.h"
#include "MCColorRGBA.h"
#include "copyright.h"

// Define the Noir CLSID ( see the NoirDef.h file to get the R_CLSID_Noir value )


extern const MCGUID CLSID_Noir;

// Data storage of our extension :
struct NoirData {	

		// Exposure
		 
		real32		fExpStrength;	         // Exposure Strength
		real32		fExpMaster;	         // Exposure Master
		real32		fExp[11];	         // Exposure values   

		// Red
		
		real32		fRedStrength;	         // Red Strength
		real32		fRedMaster;	         // Red Master
		real32		fRed[11];	         // Red values   

		// Green
		
		real32		fGreenStrength;	         // Green Strength
		real32		fGreenMaster;	         // Green Master
		real32		fGreen[11];	         // Green values   

		// Blue
		
		real32		fBlueStrength;	         // Blue Strength
		real32		fBlueMaster;	         // Blue Master
		real32		fBlue[11];	         // Blue values   

		// Pre-Procress
		
		real32		fPreGray;	         // Pre-GrayScale Intensity
		real32		fPreNeg;	         // Pre-Negative Intensity
		real32		fPreSol;	         // Pre-Solar Intensity
		real32		fPreSolThrs;	         // Pre-Solar Threshold
		real32		fPreQnt;	         // Pre-Quantize Intensity
		int32		fPreQntLvls;	         // Pre-Quantize Levels
		real32		fPreTint;	         // Pre-Tint Intensity
		TMCColorRGBA	fPreTintColour;		 // Colour for PreTint

		// Post-Procress
		
		real32		fPstGray;	         // Post-GrayScale Intensity
		real32		fPstNeg;	         // Post-Negative Intensity
		real32		fPstSol;	         // Post-Solar Intensity
		real32		fPstSolThrs;	         // Post-Solar Threshold
		real32		fPstQnt;	         // Post-Quantize Intensity
		int32		fPstQntLvls;	         // Post-Quantize Levels
		real32		fPstTint;	         // Post-Tint Intensity
		TMCColorRGBA	fPstTintColour;		 // Colour for PostTint

		// Gradient
		
		real32		fGradAngle;              // Gradient Angle
		real32		fGradLead;               // Gradient Lead
		real32		fGradWidth;              // Gradient Width
		real32		fGradCenterX;            // Gradient Center X
		real32		fGradCenterY;            // Gradient Center Y

		// Distance
		
		real32		fDistFade;               // Distance for Fade in/out
		real32		fDistLead;               // Distance for Fade in/out lead

		// Apply method
		
		int32		fDistFx;		 // Ignore Background;
		int32		fDistTarget;		 // Ignore Background;
	
		// Preset
		
		int32		fPreset;		 // Preset
		int32		fOldPreset;		 // Preset
		
		// Flags
		
		boolean		fPrePro;		 // PreProcess;
		boolean		fEqualize;		 // Equalizers;
		boolean		fPstPro;		 // PostProcess;

		// Gradient
		
		boolean		fGradTrueCirc;		 // True Circular gradient;
		boolean		fGradInvert;		 // True Circular gradient;
		boolean		fGradMirror;		 // True Circular gradient;
		
		// Apply mask

		boolean		fInvertMask;		 // True Circular gradient;
		TMCString255	fAppShader;		 // Apply shader mask
};

#define DIST_FX_NONE		0
#define DIST_FX_FADE_IN		1
#define DIST_FX_FADE_OUT	2	
#define DIST_FX_LIN_GRAD	3	
#define DIST_FX_CIRC_GRAD	4	

#define DIST_TGT_BOTH		0
#define DIST_TGT_FOREGROUND	1
#define DIST_TGT_BACKGROUND	2

#define PRESET_NEUTRAL			 0

#define PRESET_GRAYSCALE		 1

#define PRESET_NEGATIVE			 2
#define PRESET_GRAY_NEGATIVE		 3

#define PRESET_SOLARIZE			 4
#define PRESET_GRAY_SOLARIZE		 5

#define PRESET_GRAY_SEPIA		 6
#define PRESET_COLOUR_SEPIA		 7

#define PRESET_NOIR			 8
#define PRESET_GRAY_NOIR		 9
#define PRESET_COLOUR_NOIR		10

#define PRESET_BIG_NOIR			11
#define PRESET_BIG_GRAY_NOIR		12
#define PRESET_BIG_COLOUR_NOIR		13

#define PRESET_SMALL_GRAYSCALE		14
#define PRESET_SMALL_SOLARIZE 		15

#define PRESET_OVER_EXPOSED		16
#define PRESET_UNDER_EXPOSED		17

#define PRESET_QUANTIZE			18
#define PRESET_SMALL_QUANTIZE		19
#define PRESET_BIG_QUANTIZE		20
#define PRESET_VERY_BIG_QUANTIZE	21

#define PRESET_HUE_RED			22
#define PRESET_HUE_GREEN		23
#define PRESET_HUE_BLUE			24
#define PRESET_HUE_YELLOW		25
#define PRESET_HUE_TURQUOISE		26
#define PRESET_HUE_PURPLE		27

typedef struct pixel_struct pixel;

struct pixel_struct {
		double		red;
		double		green;
		double		blue;
		double		fade;
};		

// Noir Object :

class Noir : public TBasicPostRenderer {
	
public :

	// Constructor
	
	Noir();

	// Destructor
	 
	~Noir();

	// IUnknown methods
	 
	STANDARD_RELEASE;

	// IExDataExchanger methods 
	virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(NoirData); }

	virtual void*	 MCCOMAPI GetExtensionDataBuffer();
	
	virtual	MCCOMErr MCCOMAPI ExtensionDataChanged();

	// I3DExPostRenderer methods
	 
#if VERSIONNUMBER >= 0x070000
	virtual void MCCOMAPI GetBufferNeeds(RenderFilterNeeds& needs, void* renderer);
#else 
	virtual void MCCOMAPI GetBufferNeeds(uint32& needs, void* renderer);
#endif
	
	virtual	MCCOMErr MCCOMAPI PrepareDraw(	IShRasterLayer* input,
						const TMCRect& bounds,
						IShChannel* buffers[],
						I3DShScene* scene,
						I3DShCamera* renderingCamera,
						const TBBox2D& uvBox,
						const TBBox2D& productionFrame);

#if VERSIONNUMBER >= 0x080000
		virtual MCCOMErr MCCOMAPI DrawRect			(const TMCRect&				outputRect,
													 const TChannelDataBucket*	outputTile[],
													 const TChannelDataBucket* compositedOutputTile[],
													 const TBBox2D&				uvBox);
#else

		virtual	MCCOMErr MCCOMAPI DrawRect(	const TMCRect& outputRect,
						const TChannelDataBucket* output[],
						const TBBox2D& uvBox);
#endif

	virtual	MCCOMErr MCCOMAPI FinishDraw();

protected :
	
	TMCCountedPtr<IShRasterLayer>	fColorOffscreen;
	TMCCountedPtr<IShChannel>	fZBuffer;

	TMCCountedPtr<I3DShShader>	appMask;
	EShaderOutput			appMaskOut;

	real*				fDistanceBuffer;

	TChannelDataBucket		fDistance;

	void				Initialize();
	void				Initialize_Apply();
	void				Preset(long preset);
	
	double inline			Interp(double base, double low, double high, double key);
	void				Bounds(pixel &pix);
	void				GrayScale(pixel &pix, double level);
	void				Negative(pixel &pix, double level);
	void				Solarize(pixel &pix, double level, double threshold);
	void				Quantize(pixel &pix, double level, double levels);
	void				Tint(pixel &pix, double level, TMCColorRGBA &colour);
	void				Expose(pixel &pix);
	void				RedBoost(pixel &pix);
	void				GreenBoost(pixel &pix);
	void				BlueBoost(pixel &pix);

	double				screen_x0, screen_y0;
	double				screen_w, screen_h;

	double				Gradient(double u, double v);

	double				sin_theta, cos_theta;

	double				grad_len;
	
	NoirData			fData;

};

#endif
