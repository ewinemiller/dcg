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


#include "Parchment.h"

#include "IShRasterLayer.h"
#include "TBBox.h"

#include "MCCountedPtrHelper.h"
#include "ITextureMapPart.h"
#include "IShTextureMap.h"
#include "I3DShPostRenderer.h"

// #include "IMFDialogPart.h"
// #include "IMFPart.h"
// #include "IMFResponder.h"
// #include "IShUtilities.h"
// #include "COMUtilities.h"

// Define my class id

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Parchment(R_CLSID_Parchment);
#else
const MCGUID CLSID_Parchment={R_CLSID_Parchment};
#endif

// Extension initialization and cleanup

void Extension3DInit(IMCUnknown* utilities) {
	
}

void Extension3DCleanup() {

}

// Create an instance of the class

TBasicUnknown* MakeCOMObject(const MCCLSID& classId) {
	
	TBasicUnknown* res = NULL;

	//check serial if you haven't already when creating the key extensions in your project

	if (classId == CLSID_Parchment) {
		
		res = new Parchment; 
		
	}

	return res;
}

// Constructor

Parchment::Parchment() {

	Initialize();

	Initialize_Apply();

	fData.texture = 0;
	
	return;
}

// Destructor
 
Parchment::~Parchment() {

	return;
}

// Parms changed

MCCOMErr Parchment::ExtensionDataChanged() {

	// Precalc for gradient
	
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

	// Precalc for chroma key

	total_cr = (fData.fTexChroma.red + fData.fTexChroma.green + fData.fTexChroma.blue)/3.0;
	
	cr_low.red   = fData.fTexChroma.red * (1.0 - fData.fTextOverlay); 
	cr_low.green = fData.fTexChroma.green * (1.0 - fData.fTextOverlay); 
	cr_low.blue  = fData.fTexChroma.blue * (1.0 - fData.fTextOverlay); 
	
	total_cr_low = (cr_low.red + cr_low.green + cr_low.blue)/3.0;
	
	cr_high.red   = fData.fTexChroma.red * (1.0 + fData.fTextOverlay); 
	cr_high.green = fData.fTexChroma.green * (1.0 + fData.fTextOverlay); 
	cr_high.blue  = fData.fTexChroma.blue * (1.0 + fData.fTextOverlay); 
	
	total_cr_high = (cr_high.red + cr_high.green + cr_high.blue)/3.0;
	
	// All done...
	
	return MC_S_OK;
}

// Return the extensions buffer

void* Parchment::GetExtensionDataBuffer() {

	return &fData;
}

// What do we need?
#if VERSIONNUMBER >= 0x070000
void Parchment::GetBufferNeeds(RenderFilterNeeds& needs, void* renderer)
{
	needs.Set(k32Distance);
}
#else
void Parchment::GetBufferNeeds(uint32 &needs, void *renderer) {

	//Uses the Distance Channel
	
	needs=(1<<k32Distance);

	return;
}
#endif

// ***************************************************************
//				Parchment::PrepareDraw
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

MCCOMErr Parchment::PrepareDraw(	IShRasterLayer* input,
				const TMCRect& bounds,
				IShChannel* buffers[],
				I3DShScene* scene,
				I3DShCamera* renderingCamera,
				const TBBox2D& uvBox,
				const TBBox2D& productionFrame) {

	TMCCountedPtr<I3DShMasterShader>	appMaskMaster;
	TMCCountedPtr<I3DShMasterShader>	shdTextMaster;

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
	
	// Can we find the mask shader?

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
			}	*/
		}
	}	
	
	// Can we find the texture shader?

	shdText = 0;
	shdTextMaster = 0;
	
	if (scene != 0) {

		scene->GetMasterShaderByName(&shdTextMaster, fData.fShaderTex);

		if (shdTextMaster != 0) {
			shdTextMaster->GetShader(&shdText);
		}

		if (shdText != 0) {
			shdTextOut = shdText->GetImplementedOutput();

			/*(if ( shdTextOut != kUsesGetValue
			  && shdTextOut != kUsesGetColor 
			  && shdTextOut != kUsesDoShade ) {
				shdText = 0;
			}*/	
		}
	}	
	
	// All done
	
	return MC_S_OK;
}

//
// Graphics Subroutine - Interpolate a value
//

double inline Parchment::Interp(double base, double low, double high, double key) {

	return low + (high - low) * (base - key) * 10.0;
}

//
// Initialize Graphics variables
//

void Parchment::Initialize() {

	fData.fTextStrength	= 0.15f;
	fData.fTextOverlay	= 0.5f;

	fData.fTexChroma.red	= 0.0f;
	fData.fTexChroma.green	= 0.0f;
	fData.fTexChroma.blue	= 1.0f;
	
	fData.fTexSource = TEX_SOURCE_BITMAP;
	fData.fTexMethod = TEX_METHOD_TEXTURE;
	fData.fTexChannel = TEX_CHANNEL_INTENSITY;
	
	shdText = 0;
	shdTextOut = kNotImplemented;

	// Snap, Crackle & Pop
	
	fData.fSCPSeed		= 1;

	fData.fSCPLength	= 6.0f;
	
	fData.fSCPFInterval	= 1.0f;
	fData.fSCPFDuration	= 0.3f;
	fData.fSCPFIntensity	= 1.0f;

	fData.fSCPFColour.red   = 0.0f;
	fData.fSCPFColour.green = 0.0f;
	fData.fSCPFColour.blue  = 0.0f;
	
	return;
}

//
// Initialize Graphics variables
//

void Parchment::Initialize_Apply() {

	// Application Method
	
	fData.fDistFx	  = DIST_FX_NONE;	// No distance effect
	fData.fDistTarget = DIST_TGT_BOTH;	// Background and Foreground

	// Distance paameters
	
	fData.fDistFade   = 100.0;		// Fade in/out distance
	fData.fDistLead   = 40.0;		// Fade in/out lead

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
// Graphics Subroutine - Boundry check a pixel
//

void Parchment::Bounds(pixel &pix) {

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

void Parchment::GrayScale(pixel &pix, double level) {

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

void Parchment::Negative(pixel &pix, double level) {

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

void Parchment::Solarize(pixel &pix, double level, double threshold) {

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

void Parchment::Quantize(pixel &pix, double level, double levels) {

	double effect;
	
	// Apply fade
	
	level *= pix.fade;
	
	// Apply
	
	effect = floor(pix.red * levels)/levels;
	
	pix.red += (effect - pix.red) * level;
	
	effect = floor(pix.green * levels)/levels;
	
	pix.green += (effect - pix.green) * level;
	
	effect = floor(pix.blue * levels)/levels;
	
	pix.blue += (effect - pix.blue) * level;
	
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

//
// Graphics Subroutine - Tint a Pixel
//

void Parchment::Tint(pixel &pix, double level, TMCColorRGBA &colour) {

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
// Graphics Subroutine - Calculate gradient
//
// Input is in screen coordinates
//
// Screen width is in screen_w
// Screen height is in screen_h
// Screen left is in screen_x0
// Screen bottom is in screen_y0
// 

double Parchment::Gradient(double sx, double sy) {

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

//
// Graphics Subroutine - Texture a pixel
//

void Parchment::Texture(pixel &pix, double level, double sx, double sy) {

#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA colour;
#else
	TMCColorRGB	colour;
#endif
	
	TVector2	pos;

	double		new_red, new_green, new_blue;
	
	double		total;

	double		delta;
	
	double		tuvu, tuvv;
	
	ShadingIn 	shIn;

	real		mask;

	boolean 	maskFull;
	
	// Where are we?
	
	pos.x = (sx - screen_x0)/screen_w;
	pos.y = (sy - screen_y0)/screen_h;
	
	// What's the textures colour?
	
	switch (fData.fTexSource) {
	
		default:
		case TEX_SOURCE_BITMAP:	
			// Nothing if we don't have a texture...
			if (fData.texture == 0) {
				return;
			}
			if (fData.texture->GetColor(colour, pos) != MC_S_OK) {
				return;
			}

			break;

		case TEX_SOURCE_SHADER:

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
#else
		ShadingOut shOut;
		shOut.SetDefaultValues();
#endif
			
			// Fake shader input

			tuvu = 0.5 + (sx / screen_w);
			tuvv = 0.5 + (sy / screen_h);
				
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
			
			if ((shdTextOut & kUsesGetColor) == kUsesGetColor)
			{
				shdText->GetColor(colour, maskFull, shIn);
			}
			else if((shdTextOut & kUsesDoShade) == kUsesDoShade)
			{
				shdText->DoShade(shOut, shIn);
					
				colour = shOut.fColor;
						
			}
			else if((shdTextOut & kUsesGetValue) == kUsesGetValue)
			{
				shdText->GetValue(mask, maskFull, shIn);
		
				colour.red   = mask;
				colour.green = mask;
				colour.blue  = mask;
					
			}
			else
			{
				colour.red   = 1.0f;
				colour.green = 0.0f;
				colour.blue  = 0.0f;
					
			}		

			// Invert the mask?
				
			if (fData.fInvertShader) {
				colour.red   = 1.0 - colour.red;
				colour.green = 1.0 - colour.green;
				colour.blue  = 1.0 - colour.blue;
			}

			break;
	}		

	// Initialize the output colours
	
	new_red = pix.red;
	new_green = pix.green;
	new_blue = pix.blue;
	
	// Ok, how do we apply it?
	
	switch (fData.fTexMethod) {

		// Texture, Lighten and Darken
		
		default:
		case TEX_METHOD_TEXTURE:
		case TEX_METHOD_LIGHTEN:
		case TEX_METHOD_DARKEN:

			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					new_red = 2.0 * (total - 0.5);
	
					new_green = 2.0 * (total - 0.5);
	
					new_blue = 2.0 * (total - 0.5);
	
					break;

				case TEX_CHANNEL_COLOUR:

					new_red = 2.0 * (colour.red - 0.5);
	
					new_green = 2.0 * (colour.green - 0.5);
	
					new_blue = 2.0 * (colour.blue - 0.5);
	
					break;

				case TEX_CHANNEL_RED:

					new_red = 2.0 * (colour.red - 0.5);
	
					break;

				case TEX_CHANNEL_GREEN:

					new_green = 2.0 * (colour.green - 0.5);
	
					break;

				case TEX_CHANNEL_BLUE:

					new_blue = 2.0 * (colour.blue - 0.5);
	
					break;

				case TEX_CHANNEL_YELLOW:

					new_red = 2.0 * (colour.red - 0.5);
	
					new_green = 2.0 * (colour.green - 0.5);
	
					break;

				case TEX_CHANNEL_CYAN:

					new_green = 2.0 * (colour.green - 0.5);
	
					new_blue = 2.0 * (colour.blue - 0.5);
	
					break;

				case TEX_CHANNEL_MAGENTA:

					new_red = 2.0 * (colour.red - 0.5);
	
					new_blue = 2.0 * (colour.blue - 0.5);
	
					break;

			}

			// Only want some of the changes for Lighten and Darken
			
			switch (fData.fTexMethod) {

				default:
				case TEX_METHOD_TEXTURE:
					break;
					
				case TEX_METHOD_LIGHTEN:
					
					if (new_red < pix.red) {
						new_red = pix.red;
					}	
					
					if (new_green < pix.green) {
						new_green = pix.green;
					}	
					
					if (new_blue < pix.blue) {
						new_blue = pix.blue;
					}	
					
					break;
					
				case TEX_METHOD_DARKEN:
					
					if (new_red > pix.red) {
						new_red = pix.red;
					}	
					
					if (new_green > pix.green) {
						new_green = pix.green;
					}	
					
					if (new_blue > pix.blue) {
						new_blue = pix.blue;
					}	
					
					break;
			}

			break;

		// Add texture to image	
			
		case TEX_METHOD_ADD:

			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					new_red += total;
	
					new_green += total;
	
					new_blue += total;
	
					break;

				case TEX_CHANNEL_COLOUR:

					new_red += colour.red;
	
					new_green += colour.green;
	
					new_blue += colour.blue;
	
					break;

				case TEX_CHANNEL_RED:

					new_red += colour.red;
	
					break;

				case TEX_CHANNEL_GREEN:

					new_green += colour.green;
	
					break;

				case TEX_CHANNEL_BLUE:

					new_blue += colour.blue;
	
					break;

				case TEX_CHANNEL_YELLOW:

					new_red += colour.red;
	
					new_green += colour.green;
	
					break;

				case TEX_CHANNEL_CYAN:

					new_green += colour.green;
	
					new_blue += colour.blue;
	
					break;

				case TEX_CHANNEL_MAGENTA:

					new_red += colour.red;
	
					new_blue += colour.blue;
	
					break;

			}

			break;

		// Multiply texture and image	
			
		case TEX_METHOD_MULTIPLY:

			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					new_red *= total;
	
					new_green *= total;
	
					new_blue *= total;
	
					break;

				case TEX_CHANNEL_COLOUR:

					new_red *= colour.red;
	
					new_green *= colour.green;
	
					new_blue *= colour.blue;
	
					break;

				case TEX_CHANNEL_RED:

					new_red *= colour.red;
	
					break;

				case TEX_CHANNEL_GREEN:

					new_green *= colour.green;
	
					break;

				case TEX_CHANNEL_BLUE:

					new_blue *= colour.blue;
	
					break;

				case TEX_CHANNEL_YELLOW:

					new_red *= colour.red;
	
					new_green *= colour.green;
	
					break;

				case TEX_CHANNEL_CYAN:

					new_green *= colour.green;
	
					new_blue *= colour.blue;
	
					break;

				case TEX_CHANNEL_MAGENTA:

					new_red *= colour.red;
	
					new_blue *= colour.blue;
	
					break;

			}
			
			break;

		// Divide image by texture	
			
		case TEX_METHOD_DIVIDE_I:

			if (colour.red < 0.000001) {
				colour.red = 0.000001f;
			}	
			
			if (colour.green < 0.000001) {
				colour.green = 0.000001f;
			}	
			
			if (colour.blue < 0.000001) {
				colour.blue = 0.000001f;
			}	
			
			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					new_red /= total;
	
					new_green /= total;
	
					new_blue /= total;
	
					break;

				case TEX_CHANNEL_COLOUR:

					new_red /= colour.red;
	
					new_green /= colour.green;
	
					new_blue /= colour.blue;
	
					break;

				case TEX_CHANNEL_RED:

					new_red /= colour.red;
	
					break;

				case TEX_CHANNEL_GREEN:

					new_green /= colour.green;
	
					break;

				case TEX_CHANNEL_BLUE:

					new_blue /= colour.blue;
	
					break;

				case TEX_CHANNEL_YELLOW:

					new_red /= colour.red;
	
					new_green /= colour.green;
	
					break;

				case TEX_CHANNEL_CYAN:

					new_green /= colour.green;
	
					new_blue /= colour.blue;
	
					break;

				case TEX_CHANNEL_MAGENTA:

					new_red /= colour.red;
	
					new_blue /= colour.blue;
	
					break;

			}

			break;

		// Divide texture by image
			
		case TEX_METHOD_DIVIDE_T:

			if (pix.red < 0.000001) {
				pix.red = 0.000001;
			}	
			
			if (pix.green <= 0.000001) {
				pix.green = 0.000001;
			}	
			
			if (pix.blue <= 0.000001) {
				pix.blue = 0.000001;
			}	
			
			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					new_red = total/pix.red;
	
					new_green = total/pix.green;
	
					new_blue = total/pix.blue;
	
					break;

				case TEX_CHANNEL_COLOUR:

					new_red = colour.red/pix.red;
	
					new_green = colour.green/pix.green;
	
					new_blue = colour.blue/pix.blue;
	
					break;

				case TEX_CHANNEL_RED:

					new_red = colour.red/pix.red;
	
					break;

				case TEX_CHANNEL_GREEN:

					new_green = colour.green/pix.green;
	
					break;

				case TEX_CHANNEL_BLUE:

					new_blue = colour.blue/pix.blue;
	
					break;

				case TEX_CHANNEL_YELLOW:

					new_red = colour.red/pix.red;
	
					new_green = colour.green/pix.green;
	
					break;

				case TEX_CHANNEL_CYAN:

					new_green = colour.green/pix.green;
	
					new_blue = colour.blue/pix.blue;
	
					break;

				case TEX_CHANNEL_MAGENTA:

					new_red = colour.red/pix.red;
	
					new_blue = colour.blue/pix.blue;
	
					break;

			}

			break;

		// Subtract texture from image	
			
		case TEX_METHOD_SUBTRACT_T:

			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					new_red -= total;
	
					new_green -= total;
	
					new_blue -= total;
	
					break;

				case TEX_CHANNEL_COLOUR:

					new_red -= colour.red;
	
					new_green -= colour.green;
	
					new_blue -= colour.blue;
	
					break;

				case TEX_CHANNEL_RED:

					new_red -= colour.red;
	
					break;

				case TEX_CHANNEL_GREEN:

					new_green -= colour.green;
	
					break;

				case TEX_CHANNEL_BLUE:

					new_blue -= colour.blue;
	
					break;

				case TEX_CHANNEL_YELLOW:

					new_red -= colour.red;
	
					new_green -= colour.green;
	
					break;

				case TEX_CHANNEL_CYAN:

					new_green -= colour.green;
	
					new_blue -= colour.blue;
	
					break;

				case TEX_CHANNEL_MAGENTA:

					new_red -= colour.red;
	
					new_blue -= colour.blue;
	
					break;

			}

			break;

		// Subtract image from texture	
			
		case TEX_METHOD_SUBTRACT_I:

			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					new_red = total - pix.red;
	
					new_green = total - pix.green;
	
					new_blue = total - pix.blue;
	
					break;

				case TEX_CHANNEL_COLOUR:

					new_red = colour.red - pix.red;
	
					new_green = colour.green - pix.green;
	
					new_blue = colour.blue - pix.blue;
	
					break;

				case TEX_CHANNEL_RED:

					new_red = colour.red - pix.red;
	
					break;

				case TEX_CHANNEL_GREEN:

					new_green = colour.green - pix.green;
	
					break;

				case TEX_CHANNEL_BLUE:

					new_blue = colour.blue - pix.blue;
	
					break;

				case TEX_CHANNEL_YELLOW:

					new_red = colour.red - pix.red;
	
					new_green = colour.green - pix.green;
	
					break;

				case TEX_CHANNEL_CYAN:

					new_green = colour.green - pix.green;
	
					new_blue = colour.blue - pix.blue;
	
					break;

				case TEX_CHANNEL_MAGENTA:

					new_red = colour.red - pix.red;
	
					new_blue = colour.blue - pix.blue;
	
					break;

			}

			break;

		// Overlay, with texture dark being transparent	
			
		case TEX_METHOD_OVERLAY_ABOVE:

			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					if (total > fData.fTextOverlay) {
						
						new_red = total;
	
						new_green = total;
	
						new_blue = total;
					}	
	
					break;

				case TEX_CHANNEL_COLOUR:

					if (colour.red > fData.fTextOverlay) {
						new_red = colour.red;
					}	
	
					if (colour.green > fData.fTextOverlay) {	
						new_green = colour.green;
					}	
	
					if (colour.blue > fData.fTextOverlay) {
						new_blue = colour.blue;
					}	
	
					break;

				case TEX_CHANNEL_RED:
					
					if (colour.red > fData.fTextOverlay) {
						new_red = colour.red;
					}	
	
					break;

				case TEX_CHANNEL_GREEN:

					if (colour.green > fData.fTextOverlay) {	
						new_green = colour.green;
					}	
	
					break;

				case TEX_CHANNEL_BLUE:

					if (colour.blue > fData.fTextOverlay) {
						new_blue = colour.blue;
					}	
	
					break;

				case TEX_CHANNEL_YELLOW:

					if (colour.red > fData.fTextOverlay) {
						new_red = colour.red;
					}	
	
					if (colour.green > fData.fTextOverlay) {	
						new_green = colour.green;
					}	
	
					break;

				case TEX_CHANNEL_CYAN:

					if (colour.green > fData.fTextOverlay) {	
						new_green = colour.green;
					}	
	
					if (colour.blue > fData.fTextOverlay) {
						new_blue = colour.blue;
					}	
	
					break;

				case TEX_CHANNEL_MAGENTA:

					if (colour.red > fData.fTextOverlay) {
						new_red = colour.red;
					}	
	
					if (colour.blue > fData.fTextOverlay) {
						new_blue = colour.blue;
					}	
	
					break;

			}

			break;

		// Overlay, with texture light being transparent	
			
		case TEX_METHOD_OVERLAY_BELOW:

			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					if (total < fData.fTextOverlay) {
						
						new_red = total;
	
						new_green = total;
	
						new_blue = total;
					}	
	
					break;

				case TEX_CHANNEL_COLOUR:

					if (colour.red < fData.fTextOverlay) {
						new_red = colour.red;
					}	
	
					if (colour.green < fData.fTextOverlay) {	
						new_green = colour.green;
					}	
	
					if (colour.blue < fData.fTextOverlay) {
						new_blue = colour.blue;
					}	
	
					break;

				case TEX_CHANNEL_RED:
					
					if (colour.red < fData.fTextOverlay) {
						new_red = colour.red;
					}	
	
					break;

				case TEX_CHANNEL_GREEN:

					if (colour.green < fData.fTextOverlay) {	
						new_green = colour.green;
					}	
	
					break;

				case TEX_CHANNEL_BLUE:

					if (colour.blue < fData.fTextOverlay) {
						new_blue = colour.blue;
					}	
	
					break;

				case TEX_CHANNEL_YELLOW:

					if (colour.red < fData.fTextOverlay) {
						new_red = colour.red;
					}	
	
					if (colour.green < fData.fTextOverlay) {	
						new_green = colour.green;
					}	
	
					break;

				case TEX_CHANNEL_CYAN:

					if (colour.green < fData.fTextOverlay) {	
						new_green = colour.green;
					}	
	
					if (colour.blue < fData.fTextOverlay) {
						new_blue = colour.blue;
					}	
	
					break;

				case TEX_CHANNEL_MAGENTA:

					if (colour.red < fData.fTextOverlay) {
						new_red = colour.red;
					}	
	
					if (colour.blue < fData.fTextOverlay) {
						new_blue = colour.blue;
					}	
	
					break;

			}

			break;

		// Overlay, with chroma key transparency	
			
		case TEX_METHOD_OVERLAY_CHROMA:

			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					if (total < total_cr_low || total > total_cr_high) {
						
						new_red = total;
	
						new_green = total;
	
						new_blue = total;
					}	
	
					break;

				case TEX_CHANNEL_COLOUR:

					if (colour.red < cr_low.red || colour.red > cr_high.red) {
						new_red = colour.red;
					}	
	
					if (colour.green < cr_low.green || colour.green > cr_high.green) {	
						new_green = colour.green;
					}	
	
					if (colour.blue < cr_low.blue || colour.blue > cr_high.blue) {
						new_blue = colour.blue;
					}	
	
					break;

				case TEX_CHANNEL_RED:
					
					if (colour.red < cr_low.red || colour.red > cr_high.red) {
						new_red = colour.red;
					}	
	
					break;

				case TEX_CHANNEL_GREEN:

					if (colour.green < cr_low.green || colour.green > cr_high.green) {	
						new_green = colour.green;
					}	
	
					break;

				case TEX_CHANNEL_BLUE:

					if (colour.blue < cr_low.blue || colour.blue > cr_high.blue) {
						new_blue = colour.blue;
					}	
	
					break;

				case TEX_CHANNEL_YELLOW:

					if (colour.red < cr_low.red || colour.red > cr_high.red) {
						new_red = colour.red;
					}	
	
					if (colour.green < cr_low.green || colour.green > cr_high.green) {	
						new_green = colour.green;
					}	
	
					break;

				case TEX_CHANNEL_CYAN:

					if (colour.green < cr_low.green || colour.green > cr_high.green) {	
						new_green = colour.green;
					}	
	
					if (colour.blue < cr_low.blue || colour.blue > cr_high.blue) {
						new_blue = colour.blue;
					}	
	
					break;

				case TEX_CHANNEL_MAGENTA:

					if (colour.red < cr_low.red || colour.red > cr_high.red) {
						new_red = colour.red;
					}	
	
					if (colour.blue < cr_low.blue || colour.blue > cr_high.blue) {
						new_blue = colour.blue;
					}	
	
					break;

			}

			break;

		// Overlay, with chroma key transparency and Adaptive Anti-Alias	
			
		case TEX_METHOD_OVERLAY_CHROMA_AAA:

			switch (fData.fTexChannel) {

				default:
				case TEX_CHANNEL_INTENSITY:
				
					total = (colour.red + colour.green + colour.blue)/3.0;
			
					if (total <= total_cr_low || total >= total_cr_high) {
						
						new_red = total;
	
						new_green = total;
	
						new_blue = total;
					} else {

						delta = 2.0 * fabs(total_cr - total)/fData.fTextOverlay;

						new_red += (total - new_red) * delta;

						new_green += (total - new_green) * delta;

						new_blue += (total - new_blue) * delta;
					}	
	
					break;

				case TEX_CHANNEL_COLOUR:

					if (colour.red < cr_low.red || colour.red > cr_high.red) {
						new_red = colour.red;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.red - colour.red)/fData.fTextOverlay;
						new_red += (colour.red - new_red) * delta;
					}	
	
					if (colour.green < cr_low.green || colour.green > cr_high.green) {	
						new_green = colour.green;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.green - colour.green)/fData.fTextOverlay;
						new_green += (colour.green - new_green) * delta;
					}	
	
					if (colour.blue < cr_low.blue || colour.blue > cr_high.blue) {
						new_blue = colour.blue;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.blue - colour.blue)/fData.fTextOverlay;
						new_blue += (colour.blue - new_blue) * delta;
					}	
	
					break;

				case TEX_CHANNEL_RED:
					
					if (colour.red < cr_low.red || colour.red > cr_high.red) {
						new_red = colour.red;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.red - colour.red)/fData.fTextOverlay;
						new_red += (colour.red - new_red) * delta;
					}	
	
					break;

				case TEX_CHANNEL_GREEN:

					if (colour.green < cr_low.green || colour.green > cr_high.green) {	
						new_green = colour.green;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.green - colour.green)/fData.fTextOverlay;
						new_green += (colour.green - new_green) * delta;
					}	
	
					break;

				case TEX_CHANNEL_BLUE:

					if (colour.blue < cr_low.blue || colour.blue > cr_high.blue) {
						new_blue = colour.blue;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.blue - colour.blue)/fData.fTextOverlay;
						new_blue += (colour.blue - new_blue) * delta;
					}	
	
					break;

				case TEX_CHANNEL_YELLOW:

					if (colour.red < cr_low.red || colour.red > cr_high.red) {
						new_red = colour.red;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.red - colour.red)/fData.fTextOverlay;
						new_red += (colour.red - new_red) * delta;
					}	
	
					if (colour.green < cr_low.green || colour.green > cr_high.green) {	
						new_green = colour.green;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.green - colour.green)/fData.fTextOverlay;
						new_green += (colour.green - new_green) * delta;
					}	
	
					break;

				case TEX_CHANNEL_CYAN:

					if (colour.green < cr_low.green || colour.green > cr_high.green) {	
						new_green = colour.green;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.green - colour.green)/fData.fTextOverlay;
						new_green += (colour.green - new_green) * delta;
					}	
	
					if (colour.blue < cr_low.blue || colour.blue > cr_high.blue) {
						new_blue = colour.blue;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.blue - colour.blue)/fData.fTextOverlay;
						new_blue += (colour.blue - new_blue) * delta;
					}	
	
					break;

				case TEX_CHANNEL_MAGENTA:

					if (colour.red < cr_low.red || colour.red > cr_high.red) {
						new_red = colour.red;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.red - colour.red)/fData.fTextOverlay;
						new_red += (colour.red - new_red) * delta;
					}	
	
					if (colour.blue < cr_low.blue || colour.blue > cr_high.blue) {
						new_blue = colour.blue;
					} else {
						delta = 2.0 * fabs(fData.fTexChroma.blue - colour.blue)/fData.fTextOverlay;
						new_blue += (colour.blue - new_blue) * delta;
					}	
	
					break;

			}

			break;

	}		
	
	// Apply fade
	
	level *= pix.fade;

	// Apply

	if (new_red != pix.red) {
		pix.red += (new_red - pix.red) * level;
	}	
	
	if (new_green != pix.green) {
		pix.green += (new_green - pix.green) * level;
	}	
	
	if (new_blue != pix.blue) {
		pix.blue += (new_blue - pix.blue) * level;
	}	
	
	// Boundry check
	
	Bounds(pix);
	
	// All Done
	
	return;
}

// ***************************************************************
//				Parchment::DrawRect
//
// called for each tile of the image
//
// outputRect   : is the rectangle of the tile in the image
// red			: is a pointer to the output rasterLayer ( 3 channels of 16 bits for R,G,B )
//
// ***************************************************************

#if VERSIONNUMBER >= 0x080000
MCCOMErr Parchment::DrawRect(const TMCRect&	outputRect,
				 const TChannelDataBucket*	outputTile[],
				 const TChannelDataBucket* compositedOutputTile[],
				 const TBBox2D&				uvBox)



#else
MCCOMErr Parchment::DrawRect(const TMCRect& outputRect,
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
	TMCColorRGBA shColor;
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
#else
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
				// Do something
			
				Texture(pix, fData.fTextStrength, uvu, uvv);
				
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
//				Parchment::FinishDraw
//
// called for each frame after the postrenderering
//
//
// ***************************************************************
 
MCCOMErr Parchment::FinishDraw() {

	fColorOffscreen=NULL;
	fZBuffer=NULL;

	delete [] fDistanceBuffer;
	fDistanceBuffer=NULL;

	return MC_S_OK;
}

// Snap, crackle and Pop Events

SCP_Event::SCP_Event(long new_type, long new_when, long new_width) {

	type	= new_type;
	when	= new_when;
	width	= new_width;
	
	x	= 0.0;
	y	= 0.0;
	
	size	= 0.0;

	refCount = 0;

	left	= 0;
	right	= 0;
	
	return;
}

SCP_Event::SCP_Event(long new_type, long new_when, long new_width,
			double new_x, double new_y, double new_size) {

	type	= new_type;
	when	= new_when;
	width	= new_width;
	
	x	= new_x;
	y	= new_y;
	
	size	= new_size;

	refCount = 0;

	left	= 0;
	right	= 0;
	
	return;
}

SCP_Event::~SCP_Event() {

	if (left != 0) {
		left->delRef();
		left = 0;
	}

	if (right != 0) {
		right->delRef();
		right = 0;
	}	

	return;
}

void SCP_Event::addRef() {

	refCount++;

	return;
}	

void SCP_Event::delRef() {

	refCount--;

	if (refCount == 0) {
		delete this;
	}	

	return;
}	

void SCP_Event::index(SCP_Event *new_event) {

	// Sanity Check
	
	if (new_event == 0) {
		return;
	}

	if (new_event = this) {
		return;
	}	
	
	// After?
	
	if (new_event->when > when) {
		
		if ( right == 0 ) {
			right = new_event;
			new_event->addRef();
		} else {
			right->index(new_event);
		}

		// Crosses?
		
		if (new_event->when - new_event->width < when) {
			
			if (left == 0) {
				left = new_event;
				new_event->addRef();
			} else {
				left->index(new_event);
			}
		}	
	}

	// Before?
	
	if (new_event->when < when) {
		
		if (left == 0) {
			left = new_event;
			new_event->addRef();
		} else {
			left->index(new_event);
		}
		
		// Crosses?
		
		if (new_event->when + new_event->width > when) {
			
			if (right == 0) {
				right = new_event;
				new_event->addRef();
			} else {
				right->index(new_event);
			}
		}	
	}	
	
	return;
}

void SCP_Event::getEffect(double &intensity, TMCColorRGB& colour,
					double u, double v, double t) {

	// Apply my effect
	
	effect(intensity, colour, u, v, t);

	// Any extra effects?
	
	if (left != 0 && t < when) {
		left->getEffect(intensity, colour, u, v, t);
	}

	if (right != 0 && t > when) {
		right->getEffect(intensity, colour, u, v, t);
	}	
	
	// All done
	
	return;
}	
	
void SCP_Event::effect(double &intensity, TMCColorRGB& colour,
					double u, double v, double t) {

	double strength;
	
	if ( t < when - width ) {
		return;
	}

	if ( t > when + width ) {
		return;
	}	
	
	strength = 1.0 - fabs((t - when) / width);
	
	return;
}	
