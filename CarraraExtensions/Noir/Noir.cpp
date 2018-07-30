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
#include "Noir.h"

#include "IShRasterLayer.h"
#include "TBBox.h"

// Define my class id

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Noir(R_CLSID_Noir);
#else
const MCGUID CLSID_Noir={R_CLSID_Noir};
#endif

// Serial Numbers

#include "I3DShPostRenderer.h"

// Extension initialization and cleanup

void Extension3DInit(IMCUnknown* utilities) {

}

void Extension3DCleanup() {

}

// Create an instance of the class

TBasicUnknown* MakeCOMObject(const MCCLSID& classId) {
	
	TBasicUnknown* res = NULL;

	//check serial if you haven't already when creating the key extensions in your project

	if (classId == CLSID_Noir) {
		
		res = new Noir; 
		
	}

	return res;
}

// Constructor

Noir::Noir() {

	Initialize();

	Initialize_Apply();
	
	fData.fPreset = PRESET_NEUTRAL;
	fData.fOldPreset = fData.fPreset;

	return;
}

// Destructor
 
Noir::~Noir() {

	return;
}

// Parms changed

MCCOMErr Noir::ExtensionDataChanged() {

	if (fData.fPreset != fData.fOldPreset) {
		Preset(fData.fPreset);
		fData.fOldPreset = fData.fPreset;
	}	

	switch (fData.fDistFx) {

		case DIST_FX_CIRC_GRAD:

			grad_len = sqrt(0.5);

			break;
			
		case DIST_FX_LIN_GRAD:

			sin_theta = sin(-1.0 * (fData.fGradAngle/360.0) * TWO_PI);
			cos_theta = cos(-1.0 * (fData.fGradAngle/360.0) * TWO_PI);

			if (fData.fGradAngle < -315 ) {
				grad_len = 1.0/cos_theta;
			} else if (fData.fGradAngle < -225) {
				grad_len = 1.0/sin_theta;
			} else if (fData.fGradAngle < -135 ) {
				grad_len = 1.0/cos_theta;
			} else if (fData.fGradAngle < -45) {
				grad_len = 1.0/sin_theta;
			} else if (fData.fGradAngle < 45 ) {
				grad_len = 1.0/cos_theta;
			} else if (fData.fGradAngle < 135) {
				grad_len = 1.0/sin_theta;
			} else if (fData.fGradAngle < 225 ) {
				grad_len = 1.0/cos_theta;
			} else if (fData.fGradAngle < 315) {
				grad_len = 1.0/sin_theta;
			} else {	
				grad_len = 1.0/cos_theta;
			}	

			grad_len = fabs(grad_len) * 2.0;

			break;
	}		
	
	return MC_S_OK;
}

// Return the extensions buffer

void* Noir::GetExtensionDataBuffer() {

	return &fData;
}

// What do we need?
#if VERSIONNUMBER >= 0x070000
void Noir::GetBufferNeeds(RenderFilterNeeds& needs, void* renderer)
{
	needs.Set(k32Distance);
}
#else
void Noir::GetBufferNeeds(uint32 &needs, void *renderer) {

	//Uses the Distance Channel
	
	needs=(1<<k32Distance);

	return;
}
#endif

// ***************************************************************
//				Noir::PrepareDraw
//
// called for each frame before beginning the postrenderering
// by tile
//
// input   : is a rasterLayer with 3 channels (R,G,B) in 16 bits
// bounds  : is the rectangle of the full image
// buffers : is the array of pointer to the GBuffers
// scene   : is a pointer to the current scene being renderered
// uvBox   : is the coordinates of the screen in screen coordinates
//
// ***************************************************************

MCCOMErr Noir::PrepareDraw(	IShRasterLayer* input,
				const TMCRect& bounds,
				IShChannel* buffers[],
				I3DShScene* scene,
				I3DShCamera* renderingCamera,
				const TBBox2D& uvBox,
				const TBBox2D& productionFrame) {
	
	TMCCountedPtr<I3DShMasterShader>	appMaskMaster;

	//fColorOffScreen is assigned to the input buffer

	fColorOffscreen=input;

	//fZBuffer is set to the Distance Channel

	fZBuffer=buffers[k32Distance];

	fDistanceBuffer = new real;

	// Where's the screen?
	
	screen_x0 = productionFrame.fMin.x;
	screen_y0 = productionFrame.fMin.y;

	screen_w = productionFrame.fMax.x - productionFrame.fMin.x;
	screen_h = productionFrame.fMax.y - productionFrame.fMin.y;

	// Can we find the shader?

	appMask = 0;
	appMaskMaster = 0;
	
	if (scene != 0) {

		scene->GetMasterShaderByName(&appMaskMaster, fData.fAppShader);

		if (appMaskMaster != 0) {
			appMaskMaster->GetShader(&appMask);
		}

		if (appMask != 0) {
			appMaskOut = appMask->GetImplementedOutput();

			/*if ( appMaskOut != kUsesGetValue
			  && appMaskOut != kUsesGetColor 
			  && appMaskOut != kUsesDoShade ) {
				appMask = 0;
			}*/	
		}
	}	
	
	// All done
	
	return MC_S_OK;
}

//
// Graphics Subroutine - Interpolate a value
//

double inline Noir::Interp(double base, double low, double high, double key) {

	return low + (high - low) * (base - key) * 10.0;
}

//
// Initialize Graphics variables
//

void Noir::Initialize() {

	int i;
	
	// PreProcess
	
	fData.fPrePro = false;			// No preprocessing
	
	fData.fPreGray		= 0.0f;		// Pre GrayScale strength 0
	fData.fPreNeg		= 0.0f;		// Pre Negative strength 0
	fData.fPreSol		= 0.0f;		// Pre Solarize strength 0
	fData.fPreSolThrs	= 0.5f;		// Pre Solarize Threshold 0.5
	fData.fPreQnt		= 0.0f;		// Pre Quantize strength 0
	fData.fPreQntLvls	= 16;		// Pre Quantize Levels 16  
	fData.fPreTint		= 0.0f;		// Pre Tint strength 0
	
	fData.fPreTintColour.red = 0.67f;	
	fData.fPreTintColour.green = 0.32f;
	fData.fPreTintColour.blue = 0.14f;
	
	// Exposure
	
	fData.fEqualize = false;		// No equalization
	
	fData.fExpStrength = 0.0f;		// Exposure Strength
	
	fData.fExpMaster   = 1.0f;		// Exposure Master

	fData.fExp[0] = 0.0f;			// Exposure 0
	
	for( i = 1; i < 11; i++) {
		fData.fExp[i] = 1.0f;		// Exposure Sliders
	}	
	
	// Red
	
	fData.fRedStrength = 0.0f;		// Red Strength
	
	fData.fRedMaster   = 1.0f;		// Red Master

	fData.fRed[0] = 0.0f;			// Red 0
	
	for( i = 1; i < 11; i++) {
		fData.fRed[i] = 1.0f;		// Red Sliders
	}	
	
	// Green
	
	fData.fGreenStrength = 0.0f;		// Green Strength
	
	fData.fGreenMaster   = 1.0f;		// Green Master

	fData.fGreen[0] = 0.0f;			// Green 0
	
	for( i = 1; i < 11; i++) {
		fData.fGreen[i] = 1.0f;		// Green Sliders
	}	
	
	// Blue
	
	fData.fBlueStrength = 0.0f;		// Blue Strength
	
	fData.fBlueMaster   = 1.0f;		// Blue Master

	fData.fBlue[0] = 0.0f;			// Blue 0
	
	for( i = 1; i < 11; i++) {
		fData.fBlue[i] = 1.0f;		// Blue Sliders
	}	
	
	// PreProcess
	
	fData.fPstPro = false;			// No post processing
	
	fData.fPstGray		= 0.0f;		// Post GrayScale strength 0
	fData.fPstNeg		= 0.0f;		// Post Negative strength 0
	fData.fPstSol		= 0.0f;		// Post Solarize strength 0
	fData.fPstSolThrs	= 0.5f;		// Post Solarize Threshold 0.5
	fData.fPstQnt		= 0.0f;		// Post Quantize strength 0
	fData.fPstQntLvls	= 16;		// Post Quantize Levels 16  
	fData.fPstTint		= 0.0f;		// Post Tint strength 0
	
	fData.fPstTintColour.red = 0.65f;	// Sepia?
	fData.fPstTintColour.green = 0.40f;
	fData.fPstTintColour.blue = 0.20f;
	
	return;
}

//
// Initialize Graphics variables
//

void Noir::Initialize_Apply() {

	// Application Method
	
	fData.fDistFx	  = DIST_FX_NONE;	// No distance effect
	fData.fDistTarget = DIST_TGT_BOTH;	// Background and Foreground

	// Distance paameters
	
	fData.fDistFade   = 100.0;		// Fade in/out distance
	fData.fDistLead   = 40.0;		// Fade in/out lead

	// Shader mask
	
	fData.fAppShader;
	fData.fInvertMask = false;

	appMask = 0;
	appMaskOut = kNotImplemented;
	
	// Screen Parameters
	
	screen_x0 = 0.0;
	screen_y0 = 0.0;

	screen_w = 1.0;
	screen_h = 1.0;

	// Gradient parameters
	
	fData.fGradAngle = 0.0;

	fData.fGradLead = 0.0;
	fData.fGradWidth = 1.0;
	
	fData.fGradCenterX = 0.0;
	fData.fGradCenterY = 0.0;

	fData.fGradTrueCirc = true;
	fData.fGradInvert = false;
	fData.fGradMirror = false;
	
	sin_theta = 0.0;
	cos_theta = 1.0;

	grad_len = 2.0;
	
	// All Done
	
	return;
}

//
// Set variables accoring to preset
// 

void Noir::Preset(long preset) {

	// Start off with a clean slate
	
	Initialize();

	// Make the changes
	
	switch (preset) {

		/* Grayscale */
		
		case PRESET_GRAYSCALE:
			
			fData.fPreGray = 1.00f;
			
			fData.fPrePro = true;
			
			break;
		
		case PRESET_SMALL_GRAYSCALE:
			
			fData.fPreGray = 0.50f;
			
			fData.fPrePro = true;
			
			break;
		
		/* Negatives */	
			
		case PRESET_NEGATIVE:
			
			fData.fPreNeg = 1.00f;
			
			fData.fPrePro = true;
			
			break;
	
		case PRESET_GRAY_NEGATIVE:
			
			fData.fPreGray = 1.00f;
			
			fData.fPreNeg = 1.00f;
			
			fData.fPrePro = true;
			
			break;
		
		/* Solarize */	
			
		case PRESET_SOLARIZE:
			
			fData.fPreSol = 1.00f;
			
			fData.fPrePro = true;
			
			break;
		
		case PRESET_SMALL_SOLARIZE:
			
			fData.fPreSol = 0.50f;
			
			fData.fPrePro = true;
			
			break;
		
		case PRESET_GRAY_SOLARIZE:
			
			fData.fPreGray = 1.00f;
			
			fData.fPreSol = 1.00f;
			
			fData.fPrePro = true;
			
			break;
		
		/* Sepia */	
			
		case PRESET_GRAY_SEPIA:
			
			fData.fPreGray = 1.00f;
			
			fData.fPreTint = 0.40f;
			
			fData.fPreTintColour.red = 0.67f;	
			fData.fPreTintColour.green = 0.32f;
			fData.fPreTintColour.blue = 0.14f;
	
			fData.fPrePro = true;
		
			fData.fExpStrength = 1.00f;

			fData.fExp[1] = 0.80f;
			fData.fExp[2] = 0.80f;
			fData.fExp[3] = 0.85f;
			fData.fExp[4] = 0.92f;
			fData.fExp[6] = 1.08f;
			fData.fExp[7] = 1.15f;
			fData.fExp[8] = 1.15f;
			fData.fExp[9] = 1.10f;
			
			fData.fEqualize = true;
		
			break;
		
		case PRESET_COLOUR_SEPIA:
			
			fData.fPreTint = 0.40f;
			
			fData.fPreTintColour.red = 0.67f;	
			fData.fPreTintColour.green = 0.32f;
			fData.fPreTintColour.blue = 0.14f;
	
			fData.fPrePro = true;
			
			fData.fExpStrength = 1.00f;

			fData.fExp[1] = 0.80f;
			fData.fExp[2] = 0.80f;
			fData.fExp[3] = 0.85f;
			fData.fExp[4] = 0.92f;
			fData.fExp[6] = 1.08f;
			fData.fExp[7] = 1.15f;
			fData.fExp[8] = 1.15f;
			fData.fExp[9] = 1.10f;
			
			fData.fEqualize = true;
		
			break;
	
		/* Noir */	
			
		case PRESET_NOIR:
			
			fData.fExpStrength = 1.00f;

			fData.fExp[1] = 0.80f;
			fData.fExp[2] = 0.80f;
			fData.fExp[3] = 0.85f;
			fData.fExp[4] = 0.92f;
			fData.fExp[6] = 1.08f;
			fData.fExp[7] = 1.15f;
			fData.fExp[8] = 1.15f;
			fData.fExp[9] = 1.10f;
			
			fData.fEqualize = true;
		
			break;
		
		case PRESET_GRAY_NOIR:
			
			fData.fPreGray = 1.00f;
			
			fData.fPrePro = true;
		
			fData.fExpStrength = 1.00f;

			fData.fExp[1] = 0.80f;
			fData.fExp[2] = 0.80f;
			fData.fExp[3] = 0.85f;
			fData.fExp[4] = 0.92f;
			fData.fExp[6] = 1.08f;
			fData.fExp[7] = 1.15f;
			fData.fExp[8] = 1.15f;
			fData.fExp[9] = 1.10f;
			
			fData.fEqualize = true;
		
			break;
		
		case PRESET_COLOUR_NOIR:
			
			fData.fRedStrength = 1.00f;

			fData.fRed[1] = 0.80f;
			fData.fRed[2] = 0.80f;
			fData.fRed[3] = 0.85f;
			fData.fRed[4] = 0.92f;
			fData.fRed[6] = 1.08f;
			fData.fRed[7] = 1.15f;
			fData.fRed[8] = 1.15f;
			fData.fRed[9] = 1.10f;
			
			fData.fGreenStrength = 1.00f;

			fData.fGreen[1] = 0.80f;
			fData.fGreen[2] = 0.80f;
			fData.fGreen[3] = 0.85f;
			fData.fGreen[4] = 0.92f;
			fData.fGreen[6] = 1.08f;
			fData.fGreen[7] = 1.15f;
			fData.fGreen[8] = 1.15f;
			fData.fGreen[9] = 1.10f;
			
			fData.fBlueStrength = 1.00f;

			fData.fBlue[1] = 0.80f;
			fData.fBlue[2] = 0.80f;
			fData.fBlue[3] = 0.85f;
			fData.fBlue[4] = 0.92f;
			fData.fBlue[6] = 1.08f;
			fData.fBlue[7] = 1.15f;
			fData.fBlue[8] = 1.15f;
			fData.fBlue[9] = 1.10f;
			
			fData.fEqualize = true;
		
			break;
		
		/* Noir Grosse */	
			
		case PRESET_BIG_NOIR:
			
			fData.fExpStrength = 1.00f;

			fData.fExp[1] = 0.60f;
			fData.fExp[2] = 0.60f;
			fData.fExp[3] = 0.70f;
			fData.fExp[4] = 0.88f;
			fData.fExp[6] = 1.13f;
			fData.fExp[7] = 1.30f;
			fData.fExp[8] = 1.30f;
			fData.fExp[9] = 1.15f;
			
			fData.fEqualize = true;
		
			break;
		
		case PRESET_BIG_GRAY_NOIR:
			
			fData.fPreGray = 1.00f;
			
			fData.fPrePro = true;
		
			fData.fExpStrength = 1.00f;

			fData.fExp[1] = 0.60f;
			fData.fExp[2] = 0.60f;
			fData.fExp[3] = 0.70f;
			fData.fExp[4] = 0.88f;
			fData.fExp[6] = 1.13f;
			fData.fExp[7] = 1.30f;
			fData.fExp[8] = 1.30f;
			fData.fExp[9] = 1.15f;
			
			fData.fEqualize = true;
		
			break;
		
		case PRESET_BIG_COLOUR_NOIR:
			
			fData.fRedStrength = 1.00f;

			fData.fRed[1] = 0.60f;
			fData.fRed[2] = 0.60f;
			fData.fRed[3] = 0.70f;
			fData.fRed[4] = 0.88f;
			fData.fRed[6] = 1.13f;
			fData.fRed[7] = 1.30f;
			fData.fRed[8] = 1.30f;
			fData.fRed[9] = 1.15f;
			
			fData.fGreenStrength = 1.00f;

			fData.fGreen[1] = 0.60f;
			fData.fGreen[2] = 0.60f;
			fData.fGreen[3] = 0.70f;
			fData.fGreen[4] = 0.88f;
			fData.fGreen[6] = 1.13f;
			fData.fGreen[7] = 1.30f;
			fData.fGreen[8] = 1.30f;
			fData.fGreen[9] = 1.15f;
			
			fData.fBlueStrength = 1.00f;

			fData.fBlue[1] = 0.60f;
			fData.fBlue[2] = 0.60f;
			fData.fBlue[3] = 0.70f;
			fData.fBlue[4] = 0.88f;
			fData.fBlue[6] = 1.13f;
			fData.fBlue[7] = 1.30f;
			fData.fBlue[8] = 1.30f;
			fData.fBlue[9] = 1.15f;
			
			fData.fEqualize = true;
		
			break;

		/* Exposure */	
			
		case PRESET_OVER_EXPOSED:
			
			fData.fExpStrength = 1.00f;

			fData.fExpMaster = 2.00f;

			fData.fEqualize = true;
		
			break;
		
		case PRESET_UNDER_EXPOSED:
			
			fData.fExpStrength = 1.00f;

			fData.fExpMaster = 0.40f;

			fData.fEqualize = true;
		
			break;
			
		/* Quantize */	
			
		case PRESET_QUANTIZE:
			
			fData.fPreQnt = 1.00f;
			
			fData.fPrePro = true;
			
			break;
		
		case PRESET_SMALL_QUANTIZE:
			
			fData.fPreQnt = 1.00f;
			
			fData.fPreQntLvls = 32;
			
			fData.fPrePro = true;
			
			break;
		
		case PRESET_BIG_QUANTIZE:
			
			fData.fPreQnt = 1.00f;
			
			fData.fPreQntLvls = 8;
			
			fData.fPrePro = true;
			
			break;
		
		case PRESET_VERY_BIG_QUANTIZE:
			
			fData.fPreQnt = 1.00f;
			
			fData.fPreQntLvls = 4;
			
			fData.fPrePro = true;
			
			break;
	
		/* COlours */	
			
		case PRESET_HUE_RED:
			
			fData.fRedStrength = 1.00f;

			fData.fRedMaster = 1.00f;

			fData.fRed[0] = 0.05f;
			fData.fRed[1] = 1.30f;
			fData.fRed[2] = 1.15f;
			fData.fRed[3] = 1.10f;
			fData.fRed[4] = 1.05f;
			
			fData.fGreenStrength = 1.00f;

			fData.fGreenMaster = 0.20f;

			fData.fBlueStrength = 1.00f;

			fData.fBlueMaster = 0.20f;

			fData.fEqualize = true;
		
			break;
		
		case PRESET_HUE_GREEN:
			
			fData.fGreenStrength = 1.00f;

			fData.fGreenMaster = 1.00f;

			fData.fGreen[0] = 0.05f;
			fData.fGreen[1] = 1.30f;
			fData.fGreen[2] = 1.15f;
			fData.fGreen[3] = 1.10f;
			fData.fGreen[4] = 1.05f;
			
			fData.fRedStrength = 1.00f;

			fData.fRedMaster = 0.20f;

			fData.fBlueStrength = 1.00f;

			fData.fBlueMaster = 0.20f;

			fData.fEqualize = true;
		
			break;
		
		case PRESET_HUE_BLUE:
			
			fData.fBlueStrength = 1.00f;

			fData.fBlueMaster = 1.00f;

			fData.fBlue[0] = 0.05f;
			fData.fBlue[1] = 1.30f;
			fData.fBlue[2] = 1.15f;
			fData.fBlue[3] = 1.10f;
			fData.fBlue[4] = 1.05f;
			
			fData.fRedStrength = 1.00f;

			fData.fRedMaster = 0.20f;

			fData.fGreenStrength = 1.00f;

			fData.fGreenMaster = 0.20f;

			fData.fEqualize = true;
		
			break;
		
		case PRESET_HUE_YELLOW:
			
			fData.fRedStrength = 1.00f;

			fData.fRedMaster = 1.00f;

			fData.fRed[0] = 0.05f;
			fData.fRed[1] = 1.30f;
			fData.fRed[2] = 1.15f;
			fData.fRed[3] = 1.10f;
			fData.fRed[4] = 1.05f;
			
			fData.fGreenStrength = 1.00f;

			fData.fGreenMaster = 1.00f;

			fData.fGreen[0] = 0.05f;
			fData.fGreen[1] = 1.30f;
			fData.fGreen[2] = 1.15f;
			fData.fGreen[3] = 1.10f;
			fData.fGreen[4] = 1.05f;
			
			fData.fBlueStrength = 1.00f;

			fData.fBlueMaster = 0.20f;

			fData.fEqualize = true;
		
			break;
		
		case PRESET_HUE_TURQUOISE:
			
			fData.fGreenStrength = 1.00f;

			fData.fGreenMaster = 1.00f;

			fData.fGreen[0] = 0.05f;
			fData.fGreen[1] = 1.30f;
			fData.fGreen[2] = 1.15f;
			fData.fGreen[3] = 1.10f;
			fData.fGreen[4] = 1.05f;
			
			fData.fBlueStrength = 1.00f;

			fData.fBlueMaster = 1.00f;

			fData.fBlue[0] = 0.05f;
			fData.fBlue[1] = 1.30f;
			fData.fBlue[2] = 1.15f;
			fData.fBlue[3] = 1.10f;
			fData.fBlue[4] = 1.05f;
			
			fData.fRedStrength = 1.00f;

			fData.fRedMaster = 0.20f;

			fData.fEqualize = true;
		
			break;
		
		case PRESET_HUE_PURPLE:
			
			fData.fBlueStrength = 1.00f;

			fData.fBlueMaster = 1.00f;

			fData.fBlue[0] = 0.05f;
			fData.fBlue[1] = 1.30f;
			fData.fBlue[2] = 1.15f;
			fData.fBlue[3] = 1.10f;
			fData.fBlue[4] = 1.05f;
			
			fData.fRedStrength = 1.00f;

			fData.fRedMaster = 1.00f;

			fData.fRed[0] = 0.05f;
			fData.fRed[1] = 1.30f;
			fData.fRed[2] = 1.15f;
			fData.fRed[3] = 1.10f;
			fData.fRed[4] = 1.05f;
			
			fData.fGreenStrength = 1.00f;

			fData.fGreenMaster = 0.20f;

			fData.fEqualize = true;
		
			break;
		
		/* Neutral */	
			
		case PRESET_NEUTRAL:	
		default:
			break;
	}	

	// All Done
	
	return;
}	

//
// Graphics Subroutine - Boundry check a pixel
//

void Noir::Bounds(pixel &pix) {

	if ( pix.red < 0.0) {
		pix.red = 0.0;
	} else if ( pix.red > 1.0) {
		pix.red = 1.0;
	}	

	if ( pix.green < 0.0) {
		pix.green = 0.0;
	} else if ( pix.green > 1.0) {
		pix.green = 1.0;
	}	

	if ( pix.blue < 0.0) {
		pix.blue = 0.0;
	} else if ( pix.blue > 1.0) {
		pix.blue = 1.0;
	}	

	// All done
	
	return;
}

//
// Graphics Subroutine - Pixel to GrayScale
//

void Noir::GrayScale(pixel &pix, double level) {

	double total;

	// Work out the average colour value
	 
	total = (pix.red + pix.blue + pix.green)/3.0;

	// Apply fade
	
	level *= pix.fade;

	// Apply
	
	pix.red += ((total - pix.red) * level);
	
	pix.green += ((total - pix.green) * level);
	
	pix.blue += ((total - pix.blue) * level);
	
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Photo Negative of a Pixel
//

void Noir::Negative(pixel &pix, double level) {

	// Apply fade
	
	level *= pix.fade;
	
	// Apply
	
	pix.red += (1.0 - pix.red - pix.red) * level;
	
	pix.green += (1.0 - pix.green - pix.green) * level;
	
	pix.blue += (1.0 - pix.blue - pix.blue) * level;
	
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Solarize a Pixel
//

void Noir::Solarize(pixel &pix, double level, double threshold) {

	// Apply fade
	
	level *= pix.fade;
	
	// Apply
	
	if (pix.red < threshold) {
		pix.red += (1.0 - pix.red - pix.red) * level;
	}	
	
	if (pix.green < threshold) {
		pix.green += (1.0 - pix.green - pix.green) * level;
	}	
	
	if (pix.blue < threshold) {
		pix.blue += (1.0 - pix.blue - pix.blue) * level;
	}	
	
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Quantize a Pixel
//

void Noir::Quantize(pixel &pix, double level, double levels) {

	double effect;
	
	// Apply fade
	
	level *= pix.fade;
	
	// Apply
	
	effect = floor(((1.01 * pix.red) - 0.005) * levels)/levels;
	
	pix.red += (effect - pix.red) * level;
	
	effect = floor(((1.01 * pix.green) - 0.005) * levels)/levels;
	
	pix.green += (effect - pix.green) * level;
	
	effect = floor(((1.01 * pix.blue) - 0.005) * levels)/levels;
	
	pix.blue += (effect - pix.blue) * level;
	
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Tint a Pixel
//

void Noir::Tint(pixel &pix, double level, TMCColorRGBA &colour) {

	// Apply fade
	
	level *= pix.fade;
	
	// Apply
	
	pix.red += ((colour.red - pix.red) * level);
	
	pix.green += ((colour.green - pix.green) * level);
	
	pix.blue += ((colour.blue - pix.blue) * level);
		
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Adjust Pixel Exposure
//

void Noir::Expose(pixel &pix) {

	int i;
	
	double level, effect, total;

	// Strength of effect
	
	level = pix.fade * fData.fExpStrength;

	// Apply Master

	if (fData.fExpMaster != 1.0) {
		
		pix.red += (pix.red * fData.fExpMaster - pix.red) * level;
	
		pix.green += (pix.green * fData.fExpMaster - pix.green) * level;
		
		pix.blue += (pix.blue * fData.fExpMaster - pix.blue) * level;
	
		Bounds(pix);
	}	
		
	// Apply sliders
	
	total = (pix.red + pix.green + pix.blue)/3.0;

	i = total * 10;

	if (i <= 0) {
		pix.red = fData.fExp[0];
		pix.green = fData.fExp[0];
		pix.blue = fData.fExp[0];
		effect = -1.0;
	} else if (i >= 10) {
		effect = fData.fExp[10];
	} else {
		effect = Interp(total, fData.fExp[i], fData.fExp[i+1], i/10.0);
	}	
	
	if (effect != -1.0) {
		pix.red += (pix.red * effect - pix.red) * level;
	
		pix.green += (pix.green * effect - pix.green) * level;
		
		pix.blue += (pix.blue * effect - pix.blue) * level;
	}	

	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Boost Pixel Red
//

void Noir::RedBoost(pixel &pix) {

	int i;
	
	double level, effect;

	// Strength of effect
	
	level = pix.fade * fData.fRedStrength;

	// Apply Master

	if (fData.fRedMaster != 1.0) {
		
		pix.red += (pix.red * fData.fRedMaster - pix.red) * level;
	
		Bounds(pix);
	}	
		
	// Apply sliders
	
	i = pix.red * 10;

	if (i <= 0) {
		pix.red = fData.fRed[0];
	} else if (i >= 10) {
		pix.red += (pix.red * fData.fRed[10] - pix.red) * level;
	} else {
		effect = Interp(pix.red, fData.fRed[i], fData.fRed[i+1], i/10.0);
		pix.red += (pix.red * effect - pix.red) * level;
	}	
	
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Boost Pixel Green
//

void Noir::GreenBoost(pixel &pix) {

	int i;
	
	double level, effect;

	// Strength of effect
	
	level = pix.fade * fData.fGreenStrength;

	// Apply Master

	if (fData.fGreenMaster != 1.0) {
		
		pix.green += (pix.green * fData.fGreenMaster - pix.green) * level;
	
		Bounds(pix);
	}	
		
	// Apply sliders
	
	i = pix.green * 10;

	if (i <= 0) {
		pix.green = fData.fGreen[0];
	} else if (i >= 10) {
		pix.green += (pix.green * fData.fGreen[10] - pix.green) * level;
	} else {
		effect = Interp(pix.green, fData.fGreen[i], fData.fGreen[i+1], i/10.0);
		pix.green += (pix.green * effect - pix.green) * level;
	}	
	
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Boost Pixel Blue
//

void Noir::BlueBoost(pixel &pix) {

	int i;
	
	double level, effect;

	// Strength of effect
	
	level = pix.fade * fData.fBlueStrength;

	// Apply Master

	if (fData.fBlueMaster != 1.0) {
		
		pix.blue += (pix.blue * fData.fBlueMaster - pix.blue) * level;
	
		Bounds(pix);
	}	
		
	// Apply sliders
	
	i = pix.blue * 10;

	if (i <= 0) {
		pix.blue = fData.fBlue[0];
	} else if (i >= 10) {
		pix.blue += (pix.blue * fData.fBlue[10] - pix.blue) * level;
	} else {
		effect = Interp(pix.blue, fData.fBlue[i], fData.fBlue[i+1], i/10.0);
		pix.blue += (pix.blue * effect - pix.blue) * level;
	}	
	
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Calculate gradient
//
// Input is in screen coordinates
//
// Screen width is in screen_w
// Screen height is in screen_h
// Screen left is in screen_x0
// Screen bottom is in screen_y0
// 

double Noir::Gradient(double sx, double sy) {

	double grad = 1.0;

	// Which sort of gradient
	
	switch (fData.fDistFx) {

		case DIST_FX_CIRC_GRAD:

			// Normalize
			
			sx /= screen_w;
			sy /= screen_h;
	
			// Adjust for aspect ratio?

			if (fData.fGradTrueCirc) {
				if (screen_w < screen_h) {
					sy = (sy*screen_h)/screen_w;
				} else {
					sx = (sx*screen_w)/screen_h;
				}	
			}	
			
			// Move to center
		
			sx -= fData.fGradCenterX;
			sy -= fData.fGradCenterY;
			
			// Calc distance
			
			grad = sqrt( sx * sx + sy * sy);

			// Scale back

			grad /= grad_len;
			
			break;
			
		default:	
		case DIST_FX_LIN_GRAD:
			
			double nx;

			// Normalize
	
			sx = (sx / screen_w ) * 2.0;
			sy = (sy / screen_h ) * 2.0;
	
			// Apply rotation 

			if (fData.fGradAngle != 0.0) {
		
				nx = sx * sin_theta + sy * cos_theta;

				sy = sy * cos_theta - sx * sin_theta;

				sx = nx;
			}	

			// Convert to left/right gradient
	
			grad = 0.5 + (sy/grad_len);

			break;
	}		
	
	// Now we got a position of the grad line
	
	// Adjust for lead and width

	if (grad < fData.fGradLead) {
		grad = 0.0;
	} else if (grad > fData.fGradLead + fData.fGradWidth) {
		grad = 1.0;
	} else {
		grad = (grad - fData.fGradLead)/fData.fGradWidth;
	}	
	
	// Adjust for mirror

	if (fData.fGradMirror) {
		if (grad < 0.5) {
			grad *= 2.0;
		} else {
			grad = (1.0 - grad) * 2.0;
		}	
	}	
	
	// Look up the curve
	
	// Invert if requested

	if (fData.fGradInvert) {
		grad = 1.0 - grad;
	}	
	
	// All Done...
	
	return grad;
}	

// ***************************************************************
//				Noir::DrawRect
//
// called for each tile of the image
//
// outputRect   : is the rectangle of the tile in the image
// red			: is a pointer to the output rasterLayer ( 3 channels of 16 bits for R,G,B )
//
// ***************************************************************


#if VERSIONNUMBER >= 0x080000
MCCOMErr Noir::DrawRect(const TMCRect&	outputRect,
				 const TChannelDataBucket*	outputTile[],
				 const TChannelDataBucket* compositedOutputTile[],
				 const TBBox2D&				uvBox)



#else
MCCOMErr Noir::DrawRect(const TMCRect& outputRect,
				const TChannelDataBucket* output[],
				const TBBox2D& uvBox) 


#endif

{
	real *dist=NULL;

	int32 x,y;

	double uvw, uvh, uvu, uvv, tuvu, tuvv;

	ShadingIn 	shIn;

	real		mask;
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA	shColor;
#else
	TMCColorRGB	shColor;
#endif 

	boolean 	maskFull;
	
	// Get width and height
	
	int32 width=outputRect.GetWidth();
	int32 height=outputRect.GetHeight();

	// Get colour data
	
	const TChannelDataBucket *color[3];
	uint16 index[3]={0,1,2};
	
	fColorOffscreen->GetTile(outputRect, color, index, 3, eTileRead);

	// Get distance data
	
	void *baseAddress=(void*)fDistanceBuffer;
#if (VERSIONNUMBER >= 0x070000)
	fZBuffer->GetData(outputRect, &fDistance, baseAddress, eTileRead, NULL);
#else
	fZBuffer->GetData(outputRect, &fDistance, baseAddress, eTileRead);
#endif

	// Pixel data
	
	pixel pix;

	// Screen coords
	
	uvw = uvBox.fMax.x - uvBox.fMin.x;
	uvh = uvBox.fMax.y - uvBox.fMin.y;
	
	// Prime shIn values...
	
	shIn.fPoint[0] = 0;
	shIn.fPoint[1] = 0;
	shIn.fPoint[2] = 0;
	
	shIn.fGNormal[0] = 0;
	shIn.fGNormal[1] = 0;
	shIn.fGNormal[2] = 1;
				
	shIn.fPointLoc[0] = 0;
	shIn.fPointLoc[1] = 0;
	shIn.fPointLoc[2] = 0;
				
	shIn.fNormalLoc[0] = 0;
	shIn.fNormalLoc[1] = 0;
	shIn.fNormalLoc[2] = 1;
				
	shIn.fUV[0] = 0;
	shIn.fUV[1] = 0;
	
#if (VERSIONNUMBER == 0x010000)
		ShadingOut shOut;
		shOut.SetDefaultValues(shIn);
#elif (VERSIONNUMBER == 0x020000)
		ShadingOut shOut(shIn);
#elif (VERSIONNUMBER >= 0x030000)
		ShadingOut shOut;
		shOut.SetDefaultValues();
#endif	
	
	// Loop over rows
	
	for (y=0;y<height;y++) {

		// Calc screen coord...
		
		uvv = uvBox.fMin.y + uvh * (1.0 - ((1.0 * y)/(1.0 * height)));
		
		tuvv = 0.5 + (uvv / screen_h);
		
		// Find output buffer for the row
#if (VERSIONNUMBER >= 0x080000)
		uint8 *newRed	 = (uint8*)outputTile[0]->RowPtr(y);
		uint8 *newGreen = (uint8*)outputTile[1]->RowPtr(y);
		uint8 *newBlue	 = (uint8*)outputTile[2]->RowPtr(y);

		// Find input buffer for the row
		 
		uint8 *oldRed   = (uint8*)color[0]->RowPtr(y);
		uint8 *oldGreen = (uint8*)color[1]->RowPtr(y);
		uint8 *oldBlue  = (uint8*)color[2]->RowPtr(y);
#elif (VERSIONNUMBER >= 0x030000)
		uint8 *newRed	 = (uint8*)output[0]->RowPtr(y);
		uint8 *newGreen = (uint8*)output[1]->RowPtr(y);
		uint8 *newBlue	 = (uint8*)output[2]->RowPtr(y);

		// Find input buffer for the row
		 
		uint8 *oldRed   = (uint8*)color[0]->RowPtr(y);
		uint8 *oldGreen = (uint8*)color[1]->RowPtr(y);
		uint8 *oldBlue  = (uint8*)color[2]->RowPtr(y);
#else
		uint16 *newRed	 = (uint16*)output[0]->RowPtr16(y);
		uint16 *newGreen = (uint16*)output[1]->RowPtr16(y);
		uint16 *newBlue	 = (uint16*)output[2]->RowPtr16(y);

		// Find input buffer for the row
		 
		uint16 *oldRed   = (uint16*)color[0]->RowPtr16(y);
		uint16 *oldGreen = (uint16*)color[1]->RowPtr16(y);
		uint16 *oldBlue  = (uint16*)color[2]->RowPtr16(y);
#endif
		// Find distance buffer for the row
		
		dist = (real*)fDistance.RowPtr32(y);

		// Loop over the pixels in the row
		
		for (x=0;x<width;x++) {
		
			// Calc screen coord...
		
			uvu = uvBox.fMin.x + uvw * ((1.0 * x)/(1.0 * width));
		
			// In the Distance Channel, the background color is 0x1e20
			 
			// Do we want a fade effect?
			
			switch (fData.fDistFx) {

				// Default is to apply it equally
				
				default:
				case DIST_FX_NONE:
					pix.fade = 1.0;
					break;

				// Fade in as distance increases	
					
				case DIST_FX_FADE_IN:
					if (dist[x] < fData.fDistLead) {
						pix.fade = 0.0;
					} else if (dist[x] < fData.fDistLead + fData.fDistFade) {
						pix.fade = (dist[x] - fData.fDistLead)/fData.fDistFade;
					} else {
						pix.fade = 1.0f;
					}
					break;

				// Fade out as distance increases	
					
				case DIST_FX_FADE_OUT:
					if (dist[x] < fData.fDistLead) {
						pix.fade = 1.0;
					} else if (dist[x] < fData.fDistLead + fData.fDistFade) {
						pix.fade = 1.0 - ((dist[x] - fData.fDistLead)/fData.fDistFade);
					} else {
						pix.fade = 0.0f;
					}
					break;

				// Gradient?	
					
				case DIST_FX_LIN_GRAD:
				case DIST_FX_CIRC_GRAD:
					
					pix.fade = Gradient(uvu, uvv);
					
					break;
			}
	
			// What are we affecting - forground, background or both?
			
			switch (fData.fDistTarget) {
				
				// Only apply to background	
					
				case DIST_TGT_FOREGROUND:
					if (dist[x] == 1e20f) {
						pix.fade = 0.0;
					} 
					break;

				// Only apply to background
					
				case DIST_TGT_BACKGROUND:	
					if (dist[x] != 1e20f) {
						pix.fade = 0.0f;
					}
					break;

				// Default is both

				default:
				case DIST_TGT_BOTH:
					break;
			}		

			// Masked?

			if ( appMask != 0
			  && pix.fade > 0.0 ) {

				// Fake shader input
				
				tuvu = 0.5 + (uvu / screen_w);
				
				shIn.fPoint[0] = tuvu;
				shIn.fPoint[1] = tuvv;
				// shIn.fPoint[2] = 0;
				
				// shIn.fGNormal[0] = 0;
				// shIn.fGNormal[1] = 0;
				// shIn.fGNormal[2] = 1;
				
				shIn.fPointLoc[0] = tuvu;
				shIn.fPointLoc[1] = tuvv;
				// shIn.fPointLoc[2] = 0;
				
				// shIn.fNormalLoc[0] = 0;
				// shIn.fNormalLoc[1] = 0;
				// shIn.fNormalLoc[2] = 1;
				
				shIn.fUV[0] = tuvu;
				shIn.fUV[1] = tuvv;
				
				// Go find the mask value
				
				if ((appMaskOut & kUsesGetColor)==kUsesGetColor)
				{

						appMask->GetColor(shColor, maskFull, shIn);
						
						mask = ((shColor.red + shColor.green + shColor.blue)/3.0);
				}
				else
				if ((appMaskOut & kUsesDoShade)==kUsesDoShade)
				{
						appMask->DoShade(shOut, shIn);
						
						mask = ((shOut.fColor.red + shOut.fColor.green + shOut.fColor.blue)/3.0);
				}
				else
				if ((appMaskOut & kUsesGetValue)==kUsesGetValue)
				{
						appMask->GetValue(mask, maskFull, shIn);
				}
				else
				{
					mask = 1.0;
				}		

				// Invert the mask?
				
				if (fData.fInvertMask) {
					mask = 1.0 - mask;
				}

				// Apply the mask
				
				pix.fade *= mask;
			}	
			
						
			// Affect the pixel
			
			if (pix.fade == 0.0) {
				
				// Fastpath
		
				newRed[x]   = oldRed[x];
				newGreen[x] = oldGreen[x];
				newBlue[x]  = oldBlue[x];
				
			} else {
				// Extract the working values
				
#if (VERSIONNUMBER >= 0x030000)
				pix.red   = (1.0 * oldRed[x])/0xFF;
				pix.green = (1.0 * oldGreen[x])/0xFF;
				pix.blue  = (1.0 * oldBlue[x])/0xFF ;
#else
				pix.red   = (1.0 * oldRed[x])/0x4000;
				pix.green = (1.0 * oldGreen[x])/0x4000;
				pix.blue  = (1.0 * oldBlue[x])/0x4000;
#endif				
				// PreProcess

				if (fData.fPrePro) {

					// PreGray
					
					if (fData.fPreGray != 0.0) {
						GrayScale(pix, fData.fPreGray);
					}	
					
					// PreNegate
					
					if (fData.fPreNeg != 0.0) {
						Negative(pix, fData.fPreNeg);
					}	
			
					// PreTint
					
					if (fData.fPreTint != 0.0) {
						Tint(pix, fData.fPreTint, fData.fPreTintColour);
					}	
			
					// PreSolar
					
					if (fData.fPreSol != 0.0) {
						Solarize(pix, fData.fPreSol, fData.fPreSolThrs);
					}	
			
					// PreQuantizer
					
					if (fData.fPreQnt != 0.0) {
						Quantize(pix, fData.fPreQnt, fData.fPreQntLvls);
					}	
				}	
				
				// Apply the equalizers

				if (fData.fEqualize) {
					
					if (fData.fExpStrength != 0.0) {
						Expose(pix);
					}	
				
					if (fData.fRedStrength != 0.0) {
						RedBoost(pix);
					}	
				
					if (fData.fGreenStrength != 0.0) {
						GreenBoost(pix);
					}	
				
					if (fData.fBlueStrength != 0.0) {
						BlueBoost(pix);
					}
				}	
				
				// PostProcess

				if (fData.fPstPro) {

					// PstGray
					
					if (fData.fPstGray != 0.0) {
						GrayScale(pix, fData.fPstGray);
					}	
					
					// PstNegate
					
					if (fData.fPstNeg != 0.0) {
						Negative(pix, fData.fPstNeg);
					}	
			
					// PstTint
					
					if (fData.fPstTint != 0.0) {
						Tint(pix, fData.fPstTint, fData.fPstTintColour);
					}	
			
					// PstSolar
					
					if (fData.fPstSol != 0.0) {
						Solarize(pix, fData.fPstSol, fData.fPstSolThrs);
					}	
			
					// PostQuantizer
					
					if (fData.fPstQnt != 0.0) {
						Quantize(pix, fData.fPstQnt, fData.fPstQntLvls);
					}	
				}	
				
				// Output the updated values

#if (VERSIONNUMBER >= 0x030000)
				newRed[x]   = 0xFF * pix.red;
				newGreen[x] = 0xFF * pix.green;
				newBlue[x]  = 0xFF * pix.blue;

#else
				newRed[x]   = 0x4000 * pix.red;
				newGreen[x] = 0x4000 * pix.green;
				newBlue[x]  = 0x4000 * pix.blue;
#endif
			}
		}
	}
	
	// Free distance buffer
	 
	fZBuffer->UnGetData(&fDistance,false);

	// Free colour buffer
	 
	fColorOffscreen->UnGetTile(color,index,3,false);

	// All done
	
	return MC_S_OK;
}

// ***************************************************************
//				Noir::FinishDraw
//
// called for each frame after the postrenderering
//
//
// ***************************************************************
 
MCCOMErr Noir::FinishDraw() {

	fColorOffscreen=NULL;
	fZBuffer=NULL;

	delete [] fDistanceBuffer;
	fDistanceBuffer=NULL;

	return MC_S_OK;
}
