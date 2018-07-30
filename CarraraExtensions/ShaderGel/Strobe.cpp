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
#include	"MCCountedObject.h"
#include	"I3DShScene.h"
#include	"I3DShTreeElement.h"
#include	"I3DShRenderFeature.h"
#include	"I3DShAnimation.h"
#include	"copyright.h"
#include	"math.h"
#include	"strobe.h"
#include "Cognito.h"


// My class id

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_Strobe(R_CLSID_Strobe);
const MCGUID CLSID_MCStrobe(R_CLSID_MCStrobe);
#else
const MCGUID CLSID_Strobe={R_CLSID_Strobe};
const MCGUID CLSID_MCStrobe={R_CLSID_MCStrobe};
#endif

// Single channel varient

Strobe::Strobe() {	// We just initialize the values

	int i;

	fData.steps = 24;

	fData.seed = 1;

	fData.rotation = 0.0f;
	fData.cycle_length = 1.0f;
	fData.cycle_phase = 0.0f;

	fData.fromCognito = false;
	fData.invertCog = false;

	for(i = 0; i < NUM_STEPS; i++ ) {

		fData.step[i] = 0;
		
		fData.strength1[i] = 0.0f;
		fData.strength2[i] = 0.0f;
		fData.strength3[i] = 0.0f;
		
		fData.effect[i] = EFFECT_FADE;
	}

	fData.strobe = "01";

	last_time = 0;

	strobe_on = false;
	flicker_on = false;

	for( i = 0; i < NUM_SHADERS; i++) {
		fShaders[i]	= 0;
		fData.fShader[i]	= 0;
	}		

	myRand = new mtRand();
	
	myRand->init_genrand(fData.seed);
	
	return;
}

Strobe::~Strobe() {

	return;
}	

// This one's a bit sneaky as it's returning a pointer into the classes
// automatic data rather than to a sepater data structure object.

void* Strobe::GetExtensionDataBuffer() {

	return &fData;
}

boolean Strobe::IsEqualTo(I3DExShader* aShader) {

	int i;

	for( i = 0; i < NUM_SHADERS; i++) {
		if (fShaders[i] != ((Strobe*)aShader)->fShaders[i]) {
			return false;
		}
	}	
	
	if (fData.seed != ((Strobe*)aShader)->fData.seed) {
			return false;
	}
	
	if (fData.steps != ((Strobe*)aShader)->fData.steps) {
			return false;
	}
	
	if (fData.rotation != ((Strobe*)aShader)->fData.rotation) {
			return false;
	}
	
	if (fData.cycle_length != ((Strobe*)aShader)->fData.cycle_length) {
			return false;
	}
	
	if (fData.cycle_phase != ((Strobe*)aShader)->fData.cycle_phase) {
			return false;
	}
	
	if (fData.strobe != ((Strobe*)aShader)->fData.strobe) {
			return false;
	}
	
	for( i = 0; i < NUM_STEPS; i++) {
		
		if (fData.step[i] != ((Strobe*)aShader)->fData.step[i]) {
			return false;
		}
		
		if (fData.strength1[i] != ((Strobe*)aShader)->fData.strength1[i]) {
			return false;
		}
		
		if (fData.strength2[i] != ((Strobe*)aShader)->fData.strength2[i]) {
			return false;
		}
		
		if (fData.strength3[i] != ((Strobe*)aShader)->fData.strength3[i]) {
			return false;
		}
		
		if (fData.effect[i] != ((Strobe*)aShader)->fData.effect[i]) {
			return false;
		}
	}	
	
	return true;
}

MCCOMErr Strobe::GetShadingFlags(ShadingFlags& theFlags) {

	theFlags.fConstantChannelsMask = kColorChannel;
	
	// All done
	
	return MC_S_OK;
}

EShaderOutput Strobe::GetImplementedOutput() {

         uint32 implemented = kUsesGetColor;
         return (EShaderOutput)implemented;
}

boolean Strobe::AssignsChannels() {

	return true;
}	

MCCOMErr Strobe::ExtensionDataChanged() {

	int i;
	
	bool changed, unused, this_unused;
	float temp;
	
	// Go find the shaders and see what they want

	for( i = 0; i < NUM_SHADERS; i++) {
		
		fShaders[i] = 0;
		
		if (fData.fShader[i] != 0) {
			fData.fShader[i]->QueryInterface(IID_I3DShShader, (void**)&fShaders[i]);
		}
	}
	
	// Reset PRN
	
	myRand->init_genrand(fData.seed);

	// A little range checking
	
	for(i = 0; i < NUM_STEPS; i++) {
	
		if (fData.step[i] < 0) {
			fData.step[i] = 0;
		}

		if (fData.step[i] >= fData.steps) {
			fData.step[i] = fData.steps - 1;
		}	
	}	
	
	// Resort step definitions (Yicky Bubble sort, but never mind)

	changed = true;

	while (changed) {

		changed = false;

		for(i = 0; i < (NUM_STEPS - 1); i++) {

			// Is this entry in use?
			
			this_unused = false;

			if ( fData.step[i] == 0
			  && fData.strength1[i] == 0.0f
			  && fData.strength2[i] == 0.0f
			  && fData.strength3[i] == 0.0f
			  && fData.effect[i] == EFFECT_FADE) {
				this_unused = true;
			}	
			
			// Is the next entry in use?
			
			unused = false;

			if ( fData.step[i+1] == 0
			  && fData.strength1[i+1] == 0.0f
			  && fData.strength2[i+1] == 0.0f
			  && fData.strength3[i+1] == 0.0f
			  && fData.effect[i+1] == EFFECT_FADE) {
				unused = true;
			}	
			
			// Merge if a merge is needed
			
			if (fData.step[i+1] == fData.step[i] && !unused) {

				changed = true;
				
				fData.step[i] = fData.step[i+1];
				fData.step[i+1] = 0;

				fData.strength1[i] = fData.strength1[i+1];
				fData.strength1[i+1] = 0.0f;

				fData.strength2[i] = fData.strength2[i+1];
				fData.strength2[i+1] = 0.0f;

				fData.strength3[i] = fData.strength3[i+1];
				fData.strength3[i+1] = 0.0f;

				fData.effect[i] = fData.effect[i+1];
				fData.effect[i+1] = EFFECT_FADE;
			}	
		
			// Swap if a swap is needed
			
			if ( ( fData.step[i+1] < fData.step[i] && !unused )
			  || ( this_unused && fData.step[i+1] != 0 && i != 0 ) ) {
				
				changed = true;
				
				temp = fData.step[i+1];
				fData.step[i+1] = fData.step[i];
				fData.step[i] = temp;

				temp = fData.strength1[i+1];
				fData.strength1[i+1] = fData.strength1[i];
				fData.strength1[i] = temp;

				temp = fData.strength2[i+1];
				fData.strength2[i+1] = fData.strength2[i];
				fData.strength2[i] = temp;

				temp = fData.strength3[i+1];
				fData.strength3[i+1] = fData.strength3[i];
				fData.strength3[i] = temp;
				
				temp = fData.effect[i+1];
				fData.effect[i+1] = fData.effect[i];
				fData.effect[i] = temp;
			}
		}
	}	
	
	// All done
	
	return MC_S_OK;
}

#if (VERSIONNUMBER >= 0x030000)
	real
#else
	MCCOMErr
#endif

		Strobe::GetColor(TMCColorRGB& result,boolean& fullArea,ShadingIn& shIn) {

	double rot, this_step;
	
	double last_step, next_step, frac;

	double str[NUM_SHADERS], max_str;
	
	int i, next_i, last_i;

	float ioseconds;

	real this_time;

	boolean shaderFull = false;
	
	TMCCountedPtr<I3DShScene> scene;
	
	TMCCountedPtr<I3DShTreeElement> tree;
	
	TMCCountedPtr<I3DShDataComponent> comp;

#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA colour;
#else
	TMCColorRGB colour;
#endif
			
	TMCCountedPtr<CognitoData> cognitoData;

	int count, nDataComp;

	double cogValue;

	float mask;

	uint32 impOut;
				
	// Validate
	
	// Work out the strengths of the three shaders 
	
	// Start with rotation
	
	rot = fData.rotation;

	// Find ShIn->Instance->TreeElement->Scene

	scene = 0;
	
	if ( shIn.fInstance != 0 ) {
		
		shIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);

		if (tree != 0) {
			
			// Find the scene
	
			tree->GetScene(&scene);
		}
	}	
			
	// Cognito stuff in here
	
	if (fData.fromCognito && scene) {

		// We need to find Cognito
	
		cognitoData = 0;
	
		// Ok, go see if there is a cognito data element attached
		
		nDataComp = tree->GetDataComponentsCount();
	
		for (count = nDataComp; count > 0 && !cognitoData; count--) {

			// loop on all the tree data components
		 
			tree->GetDataComponentByIndex(&comp, count-1);
		
			// Return it if it's the one we want...
	
			comp->QueryInterface(IID_CognitoDataExt, (void**)&cognitoData);
		}
	
		// Use it if we've got it...
	
		if (cognitoData != 0) {

			cogValue = 0;	
			
			cognitoData->GetRotation(scene, cogValue);

			if (fData.invertCog) {
				rot -= cogValue;
			} else {	
				rot += cogValue;
			}
		}
	}	
	
	// Adjust for relative cycle length 
	
	if (fData.cycle_length != 0.0) {
		rot /= fData.cycle_length;
	}	
	
	// Adjust for cycle phase
	
	rot += fData.cycle_phase;
	
	// Wrap to interval 0..1
	
	rot -= floor(rot);
	
	if (rot < 0.0) {
		rot += 1.0f;
	}	
	
	// Work out the step
	
	this_step = (1.0 * fData.steps) * rot + 0.000001;
	
	// Previous data point...
	
	last_step = floor(this_step);
	last_i = 0;

	if (last_step > 0) {
		
		for (i = 1; i < NUM_STEPS; i++) {
			
			if ( fData.step[i] > last_step
			  || ( fData.step[i] == 0
			    && i > 0 ) ) {
				
				last_i = i - 1;
				last_step = fData.step[last_i];
				break;
			}
		}
	}	
	
	// Next data point...
	
	next_i = last_i + 1;
	next_step = fData.step[next_i];

	// Check for boundry...
	
	if (fData.step[next_i] == 0) {
		next_step = fData.steps;
		next_i = 0;
	}	
	
	// Work out if we've changed frames...
	
	if ( scene != 0 ) {
		
		this_time = scene->GetTime(&ioseconds);
	
		if (this_time != last_time) {

			// Strobe
		
			unsigned int chi,l;

			int f;
			
			uint16 schar;
			
			chi = 0;
			l = fData.strobe.Length();
			f = floor(this_step);
			
			for (i = 0; i < f; i++) {
			
				chi++;

				if (chi >= l) {
					chi = 0;
				}
			}	
		
			schar = fData.strobe.GetCharAt(chi);
			
			if (schar == '0') {
				strobe_on = false;
			} else {
				strobe_on = true;
			}	
		
			// Flicker
		
			if (myRand->genrand_int32() > (GENRAND_MAX/2.0)) {
				flicker_on = true;
			} else {
				flicker_on = false;
			}

			// Remember we've been here...

			last_time = this_time;
		}	
	}	
	
	// Calculate strengths

	switch (fData.effect[last_i]) {
		
		default:
		case EFFECT_HOLD:

			str[0] = fData.strength1[last_i];
			str[1] = fData.strength2[last_i];
			str[2] = fData.strength3[last_i];
			
			break;
			
		case EFFECT_FADE:
			
			if (next_step == 0) {
				next_step = fData.steps;
			}	
	
			frac = (this_step - last_step) / (next_step - last_step);

			str[0] = fData.strength1[last_i]
				     + (fData.strength1[next_i] - fData.strength1[last_i]) * frac;	
		
			str[1] = fData.strength2[last_i]
				     + (fData.strength2[next_i] - fData.strength2[last_i]) * frac;	
		
			str[2] = fData.strength3[last_i]
				     + (fData.strength3[next_i] - fData.strength3[last_i]) * frac;	
			break;

		case EFFECT_STROBE:

			if (!strobe_on) {
				str[0] = fData.strength1[last_i];
				str[1] = fData.strength2[last_i];
				str[2] = fData.strength3[last_i];
			} else {
				str[0] = fData.strength1[next_i];
				str[1] = fData.strength2[next_i];
				str[2] = fData.strength3[next_i];
			}	
			
			
			break;
			
		case EFFECT_FLICKER:

			if (!flicker_on) {
				str[0] = fData.strength1[last_i];
				str[1] = fData.strength2[last_i];
				str[2] = fData.strength3[last_i];
			} else {
				str[0] = fData.strength1[next_i];
				str[1] = fData.strength2[next_i];
				str[2] = fData.strength3[next_i];
			}	
			
			break;
			
		case EFFECT_SPECKLE:

			if (myRand->genrand_int32() > (GENRAND_MAX/2.0)) {
				str[0] = fData.strength1[last_i];
				str[1] = fData.strength2[last_i];
				str[2] = fData.strength3[last_i];
			} else {
				str[0] = fData.strength1[next_i];
				str[1] = fData.strength2[next_i];
				str[2] = fData.strength3[next_i];
			}	
			
			break;
	}
	
	// Cap the strengths
	
	max_str = 0.0;

	for( i = 0; i < NUM_SHADERS; i++) {
		max_str += str[i];
	}	

	if (max_str > 1.0) {
		for( i = 0; i < NUM_SHADERS; i++) {
			str[i] /= max_str;
		}	
	}	
	
	// Extract values for the three shaders
	
	// Start with black

	result.Set(0.0, 0.0, 0.0);
	
#if (VERSIONNUMBER == 0x010000)
	ShadingOut shOut;
	shOut.SetDefaultValues(shIn);
#elif (VERSIONNUMBER == 0x020000)
	ShadingOut shOut(shIn);
#else
		ShadingOut shOut;
		shOut.SetDefaultValues();
#endif	
	
	for( i = 0; i < NUM_SHADERS; i++ ) {
		
		// Go see what the shader does
	
		if (fShaders[i] != 0 && str[i] > 0.01) {
			
			impOut = fShaders[i]->GetImplementedOutput();

			if ((impOut & kUsesDoShade) > 0) {

					shOut.Clear();
					
					fShaders[i]->DoShade(shOut, shIn);
						
					shOut *= str[i];
					result += shOut.fColor;
					 
					
			} else if ((impOut & kUsesGetColor) > 0) {

					fShaders[i]->GetColor(colour, shaderFull, shIn);
				
					result.red += colour.red * str[i];
					result.green += colour.green * str[i];
					result.blue += colour.blue * str[i];
					
			} else if ((impOut & kUsesGetValue) > 0) {

					fShaders[i]->GetValue(mask, shaderFull, shIn);

					result.red	+= mask * str[i];
					result.green	+= mask * str[i];
					result.blue	+= mask * str[i];
			}		
		}	
	}		

	// All done
	
	return MC_S_OK;
}

// Multi-Channel Varient

MCStrobe::MCStrobe() {	// We just initialize the values

	int i;

	fData.steps = 24;

	fData.seed = 1;

	fData.rotation = 0.0f;
	fData.cycle_length = 1.0f;
	fData.cycle_phase = 0.0f;

	fData.fromCognito = false;
	fData.invertCog = false;

	for(i = 0; i < NUM_STEPS; i++ ) {

		fData.step[i] = 0;
		
		fData.strength1[i] = 0.0f;
		fData.strength2[i] = 0.0f;
		fData.strength3[i] = 0.0f;
		
		fData.effect[i] = EFFECT_FADE;
	}

	fData.strobe = "01";

	last_time = 0;

	strobe_on = false;
	flicker_on = false;

	for( i = 0; i < NUM_SHADERS; i++) {
		fData.fShader[i]	= 0;
		fShaders[i]	= 0;
	}		

	myRand = new mtRand();
	
	myRand->init_genrand(fData.seed);
	
	return;
}

MCStrobe::~MCStrobe() {

	return;
}	

// This one's a bit sneaky as it's returning a pointer into the classes
// automatic data rather than to a sepater data structure object.

void* MCStrobe::GetExtensionDataBuffer() {

	return &fData;
}

boolean MCStrobe::IsEqualTo(I3DExShader* aShader) {

	int i;

	for( i = 0; i < NUM_SHADERS; i++) {
		if (fShaders[i] != ((MCStrobe*)aShader)->fShaders[i]) {
			return false;
		}
	}	
	
	if (fData.seed != ((MCStrobe*)aShader)->fData.seed) {
			return false;
	}
	
	if (fData.steps != ((MCStrobe*)aShader)->fData.steps) {
			return false;
	}
	
	if (fData.rotation != ((MCStrobe*)aShader)->fData.rotation) {
			return false;
	}
	
	if (fData.cycle_length != ((MCStrobe*)aShader)->fData.cycle_length) {
			return false;
	}
	
	if (fData.cycle_phase != ((MCStrobe*)aShader)->fData.cycle_phase) {
			return false;
	}
	
	if (fData.strobe != ((MCStrobe*)aShader)->fData.strobe) {
			return false;
	}
	
	for( i = 0; i < NUM_STEPS; i++) {
		
		if (fData.step[i] != ((MCStrobe*)aShader)->fData.step[i]) {
			return false;
		}
		
		if (fData.strength1[i] != ((MCStrobe*)aShader)->fData.strength1[i]) {
			return false;
		}
		
		if (fData.strength2[i] != ((MCStrobe*)aShader)->fData.strength2[i]) {
			return false;
		}
		
		if (fData.strength3[i] != ((MCStrobe*)aShader)->fData.strength3[i]) {
			return false;
		}
		
		if (fData.effect[i] != ((MCStrobe*)aShader)->fData.effect[i]) {
			return false;
		}
	}	
	
	return true;
}

MCCOMErr MCStrobe::GetShadingFlags(ShadingFlags& theFlags) {

	// For now, ask for everything...
	
	theFlags.fNeedsColor		= true;
	theFlags.fNeedsPoint		= true;
	theFlags.fNeedsNormal		= true;
	theFlags.fNeedsIsoUV		= true;
	theFlags.fNeedsUV		= true;
	theFlags.fNeedsPointLoc		= true;
	theFlags.fNeedsNormalLoc	= true;
	theFlags.fNeedsPixelRatio	= true;
	theFlags.fChangesNormal		= true;
	theFlags.fUVSpaceShaders	= true;

	// All channels subject to variation
	
	theFlags.fConstantChannelsMask = kNoChannel;
	
	// All done
	
	return MC_S_OK;
}

EShaderOutput MCStrobe::GetImplementedOutput() {

         uint32 implemented = kUsesDoShade;
         return (EShaderOutput)implemented;
}

boolean MCStrobe::AssignsChannels() {

	return true;
}	

MCCOMErr MCStrobe::ExtensionDataChanged() {

	int i;
	
	bool changed, unused, this_unused;
	float temp;
	
	
	// Go find the shaders and see what they want

	for( i = 0; i < NUM_SHADERS; i++) {
		
		fShaders[i] = 0;
		
		if (fData.fShader[i] != 0) {
			fData.fShader[i]->QueryInterface(IID_I3DShShader, (void**)&fShaders[i]);
		}
	}
	
	// Reset PRN
	
	myRand->init_genrand(fData.seed);

	// A little range checking
	
	for(i = 0; i < NUM_STEPS; i++) {
	
		if (fData.step[i] < 0) {
			fData.step[i] = 0;
		}

		if (fData.step[i] >= fData.steps) {
			fData.step[i] = fData.steps - 1;
		}	
	}	
	
	// Resort step definitions (Yicky Bubble sort, but never mind)

	changed = true;

	while (changed) {

		changed = false;

		for(i = 0; i < (NUM_STEPS - 1); i++) {

			// Is this entry in use?
			
			this_unused = false;

			if ( fData.step[i] == 0
			  && fData.strength1[i] == 0.0f
			  && fData.strength2[i] == 0.0f
			  && fData.strength3[i] == 0.0f
			  && fData.effect[i] == EFFECT_FADE) {
				this_unused = true;
			}	
			
			// Is the next entry in use?
			
			unused = false;

			if ( fData.step[i+1] == 0
			  && fData.strength1[i+1] == 0.0f
			  && fData.strength2[i+1] == 0.0f
			  && fData.strength3[i+1] == 0.0f
			  && fData.effect[i+1] == EFFECT_FADE) {
				unused = true;
			}	
			
			// Merge if a merge is needed
			
			if (fData.step[i+1] == fData.step[i] && !unused) {

				changed = true;
				
				fData.step[i] = fData.step[i+1];
				fData.step[i+1] = 0;

				fData.strength1[i] = fData.strength1[i+1];
				fData.strength1[i+1] = 0.0f;

				fData.strength2[i] = fData.strength2[i+1];
				fData.strength2[i+1] = 0.0f;

				fData.strength3[i] = fData.strength3[i+1];
				fData.strength3[i+1] = 0.0f;

				fData.effect[i] = fData.effect[i+1];
				fData.effect[i+1] = EFFECT_FADE;
			}	
		
			// Swap if a swap is needed
			
			if ( ( fData.step[i+1] < fData.step[i] && !unused) 
			  || ( this_unused && fData.step[i+1] != 0 && i != 0 )) {
				
				changed = true;
				
				temp = fData.step[i+1];
				fData.step[i+1] = fData.step[i];
				fData.step[i] = temp;

				temp = fData.strength1[i+1];
				fData.strength1[i+1] = fData.strength1[i];
				fData.strength1[i] = temp;

				temp = fData.strength2[i+1];
				fData.strength2[i+1] = fData.strength2[i];
				fData.strength2[i] = temp;

				temp = fData.strength3[i+1];
				fData.strength3[i+1] = fData.strength3[i];
				fData.strength3[i] = temp;
				
				temp = fData.effect[i+1];
				fData.effect[i+1] = fData.effect[i];
				fData.effect[i] = temp;
			}
		}
	}	
	
	// All done
	
	return MC_S_OK;
}

MCCOMErr MCStrobe::DoShade(ShadingOut &result, ShadingIn &shIn) {

	double rot, this_step;
	
	double last_step, next_step, frac;

	double str[NUM_SHADERS], max_str;
	
	int i, next_i, last_i;

	float ioseconds;

	real this_time;

	boolean shaderFull = false;
	
	TMCCountedPtr<I3DShScene> scene;
	
	TMCCountedPtr<I3DShTreeElement> tree;
	
	TMCCountedPtr<I3DShDataComponent> comp;

#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA colour;
#else
	TMCColorRGB colour;
#endif
			
	TMCCountedPtr<CognitoData> cognitoData;

	int count, nDataComp;

	double cogValue;

	float mask;

	uint32 impOut;
				
	// Validate
	
	// Work out the strengths of the three shaders 
	
	// Start with rotation
	
	rot = fData.rotation;

	// Find ShIn->Instance->TreeElement->Scene

	scene = 0;
	
	if ( shIn.fInstance != 0 ) {
		
		shIn.fInstance->QueryInterface(IID_I3DShTreeElement, (void**)&tree);

		if (tree != 0) {
			
			// Find the scene
	
			tree->GetScene(&scene);
		}
	}	
			
	// Cognito stuff in here
	
	if (fData.fromCognito && scene != 0) {

		// We need to find Cognito
	
		cognitoData = 0;
	
		// Ok, go see if there is a cognito data element attached
		
		nDataComp = tree->GetDataComponentsCount();
	
		for (count = nDataComp; count > 0 && !cognitoData; count--) {

			// loop on all the tree data components
		 
			tree->GetDataComponentByIndex(&comp, count-1);
		
			// Return it if it's the one we want...
	
			comp->QueryInterface(IID_CognitoDataExt, (void**)&cognitoData);
		}
	
		// Use it if we've got it...
	
		if (cognitoData != 0) {

			cogValue = 0;	
			
			cognitoData->GetRotation(scene, cogValue);

			if (fData.invertCog) {
				rot -= cogValue;
			} else {	
				rot += cogValue;
			}
		}
	}	

	// Adjust for relative cycle length 
	
	if (fData.cycle_length != 0.0) {
		rot /=fData. cycle_length;
	}	
	
	// Adjust for cycle phase
	
	rot += fData.cycle_phase;
	
	// Wrap to interval 0..1
	
	rot -= floor(rot);
	
	if (rot < 0.0) {
		rot += 1.0f;
	}	
	
	// Work out the step
	
	this_step = (1.0 * fData.steps) * rot + 0.000001;
	
	// Previous data point...
	
	last_step = floor(this_step);
	last_i = 0;

	if (last_step > 0) {
		
		for (i = 1; i < NUM_STEPS; i++) {
			
			if ( fData.step[i] > last_step
			  || ( fData.step[i] == 0
			    && i > 0 ) ) {
				
				last_i = i - 1;
				last_step = fData.step[last_i];
				break;
			}
		}
	}	
	
	// Next data point...
	
	next_i = last_i + 1;
	next_step = fData.step[next_i];

	// Check for boundry...
	
	if (fData.step[next_i] == 0) {
		next_step = fData.steps;
		next_i = 0;
	}	
	
	// Work out if we've changed frames...
	
	if ( scene != 0 ) {
		
		this_time = scene->GetTime(&ioseconds);
		
		if (this_time != last_time) {

			// Strobe
		
			unsigned int chi,l;

			int f;
			
			uint16 schar;
			
			chi = 0;
			l = fData.strobe.Length();
			f = floor(this_step);
			
			for (i = 0; i < f; i++) {
			
				chi++;

				if (chi >= l) {
					chi = 0;
				}
			}	
		
			schar = fData.strobe.GetCharAt(chi);
			
			if (schar == '0') {
				strobe_on = false;
			} else {
				strobe_on = true;
			}	
		
			// Flicker
		
			if (myRand->genrand_int32() > (GENRAND_MAX/2.0)) {
				flicker_on = true;
			} else {
				flicker_on = false;
			}

			// Remember we've been here...

			last_time = this_time;
		}	
	}	
	
	// Calculate strengths

	switch (fData.effect[last_i]) {
		
		default:
		case EFFECT_HOLD:

			str[0] = fData.strength1[last_i];
			str[1] = fData.strength2[last_i];
			str[2] = fData.strength3[last_i];
			
			break;
			
		case EFFECT_FADE:
			
			if (next_step == 0) {
				next_step = fData.steps;
			}	
	
			frac = (this_step - last_step) / (next_step - last_step);

			str[0] = fData.strength1[last_i]
				     + (fData.strength1[next_i] - fData.strength1[last_i]) * frac;	
		
			str[1] = fData.strength2[last_i]
				     + (fData.strength2[next_i] - fData.strength2[last_i]) * frac;	
		
			str[2] = fData.strength3[last_i]
				     + (fData.strength3[next_i] - fData.strength3[last_i]) * frac;	
			break;

		case EFFECT_STROBE:

			if (!strobe_on) {
				str[0] = fData.strength1[last_i];
				str[1] = fData.strength2[last_i];
				str[2] = fData.strength3[last_i];
			} else {
				str[0] = fData.strength1[next_i];
				str[1] = fData.strength2[next_i];
				str[2] = fData.strength3[next_i];
			}	
			
			
			break;
			
		case EFFECT_FLICKER:

			if (!flicker_on) {
				str[0] = fData.strength1[last_i];
				str[1] = fData.strength2[last_i];
				str[2] = fData.strength3[last_i];
			} else {
				str[0] = fData.strength1[next_i];
				str[1] = fData.strength2[next_i];
				str[2] = fData.strength3[next_i];
			}	
			
			break;
			
		case EFFECT_SPECKLE:

			if (myRand->genrand_int32() > (GENRAND_MAX/2.0)) {
				str[0] = fData.strength1[last_i];
				str[1] = fData.strength2[last_i];
				str[2] = fData.strength3[last_i];
			} else {
				str[0] = fData.strength1[next_i];
				str[1] = fData.strength2[next_i];
				str[2] = fData.strength3[next_i];
			}	
			
			break;
	}
	
	// Cap the strengths
	
	max_str = 0.0;

	for( i = 0; i < NUM_SHADERS; i++) {
		max_str += str[i];
	}	

	if (max_str > 1.0) {
		for( i = 0; i < NUM_SHADERS; i++) {
			str[i] /= max_str;
		}	
	}	
	
	// Extract values for the three shaders
	
	// Start with black

	result.Clear();
	
	for( i = 0; i < NUM_SHADERS; i++ ) {
		
		// Go see what the shader does
	
		if (fShaders[i] != 0 && str[i] > 0.01) {
			
			impOut = fShaders[i]->GetImplementedOutput();

			if ((impOut & kUsesDoShade) > 0) {

					// shOut.Clear();
					
					// shOut.fOpacity	= 1.0f;	
					// shOut.fAmbient 	= 1.0f;
					// shOut.fLambert 	= 1.0f;
					
#if (VERSIONNUMBER == 0x010000)
					ShadingOut shOut;
					shOut.SetDefaultValues(shIn);
#elif (VERSIONNUMBER == 0x020000)
					ShadingOut shOut(shIn);
#else
					ShadingOut shOut;
					shOut.SetDefaultValues();
#endif
					
					shIn.fCurrentCompletionMask = kNoChannel;
					
					fShaders[i]->DoShade(shOut, shIn);
#if (VERSIONNUMBER >= 0x040000)
					shOut.fLayerMask = 1.0f;
#else
					shOut.fOpacity	= 1.0f;	
#endif
					shOut.fAmbient 	= 1.0f;
					shOut.fLambert 	= 1.0f;
					
					shOut *= str[i];
					result += shOut;
					 
					
			} else if ((impOut & kUsesGetColor) > 0) {

					fShaders[i]->GetColor(colour, shaderFull, shIn);
				
					result.fColor.red += colour.red * str[i];
					result.fColor.green += colour.green * str[i];
					result.fColor.blue += colour.blue * str[i];
					
			} else if ((impOut & kUsesGetValue) > 0) {

					fShaders[i]->GetValue(mask, shaderFull, shIn);

					result.fColor.red	+= mask * str[i];
					result.fColor.green	+= mask * str[i];
					result.fColor.blue	+= mask * str[i];
			}		
		}	
	}		

	// Hmmm....
	
	shIn.fCurrentCompletionMask = kAllChannels;
	
#if (VERSIONNUMBER >= 0x040000)
	result.fLayerMask = 1.0f;
#else
	result.fOpacity	= 1.0f;	
#endif
	result.fAmbient 	= 1.0f;
	result.fLambert 	= 1.0f;
	
	// All done
	
	return MC_S_OK;
}

