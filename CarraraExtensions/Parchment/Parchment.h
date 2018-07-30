/*  Parchment - plug-in for Carrara
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


#ifndef __PARCHMENT__
#define __PARCHMENT__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "BasicPostRenderer.h"
#include "ParchmentDef.h"
#include "IShChannel.h"
#include "IShTextureMap.h"
#include "ITextureMapPart.h"
#include "I3DShShader.h"
#include "MCColorRGBA.h"
#include "copyright.h"


// Define the Parchment CLSID ( see the ParchmentDef.h file to get the R_CLSID_Parchment value )


extern const MCGUID CLSID_Parchment;

// Data storage of our extension :
struct ParchmentData {	

		// Texture Strength
	
		real32		fTextStrength;		 // Texture Strength
		real32		fTextOverlay;		 // Texture Overlay Threshold
	
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

		// Texture Method
		
		int32		fTexSource;		 // Texture Source;
		int32		fTexMethod;		 // Texturing Method;
		int32		fTexChannel;		 // Texturing Channel;

		// Chromakey Colour
		
		TMCColorRGBA	fTexChroma;		// Texture Chromakey Overlay Colour
		
		
		// Gradient
		
		boolean		fGradTrueCirc;		 // True Circular gradient;
		boolean		fGradInvert;		 // Invert gradient;
		boolean		fGradMirror;		 // Reflexive gradient;
		
		// Apply mask

		boolean		fInvertMask;		 // Invert shader mask
		TMCString255	fAppShader;		 // Apply shader mask
		
		// Apply mask

		boolean		fInvertShader;		 // Invert shader texture
		TMCString255	fShaderTex;		 // Shader texture

		// Snap, Crackle & Pop
		//
		// General parms

		int32		fSCPSeed;		 // PRN Seed
		real32		fSCPLength;		 // Length of sequence

		// Flicker Parms

		real32		fSCPFInterval;		 // Interval between flickers
		real32		fSCPFDuration;		 // Duration of flicker
		real32		fSCPFIntensity;		 // Intensity of flicker
		TMCColorRGBA	fSCPFColour;		 // Flicker colour
		// Texture map
		
		IShTextureMap	*texture;		 // Texture Map
};


#define DIST_FX_NONE		0
#define DIST_FX_FADE_IN		1
#define DIST_FX_FADE_OUT	2	
#define DIST_FX_LIN_GRAD	3	
#define DIST_FX_CIRC_GRAD	4	

#define DIST_TGT_BOTH		0
#define DIST_TGT_FOREGROUND	1
#define DIST_TGT_BACKGROUND	2

#define TEX_SOURCE_BITMAP	0
#define TEX_SOURCE_SHADER	1

#define TEX_METHOD_TEXTURE		 0
#define TEX_METHOD_LIGHTEN 		 1
#define TEX_METHOD_DARKEN		 2
#define TEX_METHOD_ADD      		 3
#define TEX_METHOD_MULTIPLY		 4
#define TEX_METHOD_DIVIDE_I  		 5
#define TEX_METHOD_DIVIDE_T  		 6
#define TEX_METHOD_SUBTRACT_T 		 7
#define TEX_METHOD_SUBTRACT_I 		 8
#define TEX_METHOD_OVERLAY_ABOVE	 9
#define TEX_METHOD_OVERLAY_BELOW	10
#define TEX_METHOD_OVERLAY_CHROMA	11
#define TEX_METHOD_OVERLAY_CHROMA_AAA	12

#define TEX_CHANNEL_INTENSITY	0
#define TEX_CHANNEL_COLOUR  	1
#define TEX_CHANNEL_RED  	2
#define TEX_CHANNEL_GREEN  	3
#define TEX_CHANNEL_BLUE  	4
#define TEX_CHANNEL_YELLOW  	5
#define TEX_CHANNEL_CYAN  	6
#define TEX_CHANNEL_MAGENTA  	7

typedef struct pixel_struct pixel;

struct pixel_struct {
		double		red;
		double		green;
		double		blue;
		double		fade;
};		

#define SCP_EVENT_FLICKER	0
#define SCP_EVENT_SCRATCH	1
#define SCP_EVENT_SPOT		2

class SCP_Event {

public:
	SCP_Event(long new_type, long new_when, long new_width);
	SCP_Event(long new_type, long new_when, long new_width,
			double new_x, double new_y, double new_size);
	~SCP_Event();

	void index(SCP_Event *new_event);
			
	void getEffect(double &intensity, TMCColorRGB& colour,
					double u, double v, double t);
	
	void	addRef();

	void	delRef();
	
protected:	
	void effect(double &intensity, TMCColorRGB& colour,
					double u, double v, double t);
	
	long		type;
	double		when;
	double		width;
	double		x;
	double		y;
	double		size;

	int	refCount;

	SCP_Event *left, *right;
};	

// Parchment Object :

class Parchment : public TBasicPostRenderer {
	
public :

	// Constructor
	
	Parchment();

	// Destructor
	 
	~Parchment();

	virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(ParchmentData); }
	// IUnknown methods
	 
	STANDARD_RELEASE;

	// IExDataExchanger methods 
	 
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

	TMCCountedPtr<I3DShShader>	shdText;
	EShaderOutput			shdTextOut;

	real*				fDistanceBuffer;

	TChannelDataBucket		fDistance;

	void				Initialize();
	void				Initialize_Apply();
	
	double inline			Interp(double base, double low, double high, double key);
	void				Bounds(pixel &pix);
	void				GrayScale(pixel &pix, double level);
	void				Negative(pixel &pix, double level);
	void				Solarize(pixel &pix, double level, double threshold);
	void				Quantize(pixel &pix, double level, double levels);
	void				Tint(pixel &pix, double level, TMCColorRGBA &colour);

	void				Texture(pixel &pix, double level, double sx, double sy);

	double				screen_x0, screen_y0;
	double				screen_w, screen_h;

	double				Gradient(double u, double v);

	double				sin_theta, cos_theta;

	double				grad_len;

	double				total_cr_low, total_cr_high, total_cr;

	TMCColorRGBA			cr_low, cr_high;
	
	ParchmentData			fData;

};

#endif
