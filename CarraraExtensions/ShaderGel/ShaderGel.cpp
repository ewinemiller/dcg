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

#include "math.h"
#include "ShaderGel.h"
#include "MCCountedPtrHelper.h"
#include "I3dShTreeElement.h"
#include "I3DShShader.h"
#include "I3DShRenderFeature.h"
#include "InterfaceIDs.h"
#include "I3DShGel.h"
#include "I3DExGel.h"

#include "MFPartMessages.h"
#include "ComMessages.h"
#include "IMFResponder.h"
#include "interfaceids.h"
#include "Cognito.h"

#include "Copyright.h"


// My IIDs

#if (VERSIONNUMBER >= 0x050000)
const MCGUID CLSID_ShaderGel(R_CLSID_ShaderGel);

const MCGUID IID_ShaderGel(R_IID_ShaderGel);
#else
const MCGUID CLSID_ShaderGel={R_CLSID_ShaderGel};

const MCGUID IID_ShaderGel={R_IID_ShaderGel};
#endif


// QueryInterface so it can identify itself

MCCOMErr ShaderGel::QueryInterface(const MCIID& riid, void** ppvObj) {

	if (MCIsEqualIID(riid, IID_I3DShLightsourceGel)) {

		TMCCountedGetHelper<I3DShLightsourceGel> result(ppvObj);
		result = (I3DShLightsourceGel*)this;
		return MC_S_OK;
	}
	
	if (MCIsEqualIID(riid, IID_I3DExLightsourceGel)) {

		TMCCountedGetHelper<I3DExLightsourceGel> result(ppvObj);
		result = (I3DExLightsourceGel*)this;
		return MC_S_OK;
	}
	
	if (MCIsEqualIID(riid, IID_ShaderGel)) {

		TMCCountedGetHelper<ShaderGel> result(ppvObj);
		result = (ShaderGel*)this;
		return MC_S_OK;
	}
	
	return TBasicGel::QueryInterface(riid, ppvObj);
	//return TBasicDataExchanger::QueryInterface(riid, ppvObj);
}

// Constructor:

ShaderGel::ShaderGel() {
	
	// Default shader name
	
	fData.Shader		= "ShaderGel";

	// Start position is neutral
	
	fData.offsetX		= 0.0f;
	fData.offsetY		= 0.0f;
	fData.scaleX		= 1.0f;
	fData.scaleY		= 1.0f;
	fData.repeatX		= 0.0f;
	fData.repeatY		= 0.0f;

	fData.rotation		= 0.0f;

	fData.radialMapping	= false;
	
	fData.rotationFC	= false;
	fData.offsetXFC		= false;
	fData.offsetYFC		= false;
	fData.scaleXFC		= false;
	fData.scaleYFC		= false;
	fData.repeatXFC		= false;
	fData.repeatYFC		= false;

	offsetX		= 0.0;
	offsetY		= 0.0;
	scaleX		= 1.0;
	scaleY		= 1.0;
	repeatX		= 0.0;
	repeatY		= 0.0;
	
	// When did we last run?
	
	lastTime = -1.0;

	theta = 0.0;

	// No known scene
	
	scene = 0;

	// No known ancestor
	
	parent = 0;

	// No known popup either
	
	myPopup = 0;
	
	// No selected shader
	
	fData.popup = SHADER_POPUP_CURRENT;
	
	// Sync cached values
	
	ExtensionDataChanged();
	
	// All Done
	
	return;
}

// Get resource id

short ShaderGel::GetResID() {
	
	return 4000; // This is the view ID in the resource file
}

// Extension data

void* ShaderGel::GetExtensionDataBuffer() {
	
	return &fData; // The Shell uses this pointer to set the values of the Gel's parameters
}

// Create a clone...

void ShaderGel::Clone(IExDataExchanger** res, IMCUnknown* pUnkOuter) {

	TMCCountedCreateHelper<IExDataExchanger> result(res);

	ShaderGel *theClone = new ShaderGel();
	ThrowIfNil(theClone);
	
	theClone->SetControllingUnknown(pUnkOuter);
	
	CopyData(theClone);

	result = (IExDataExchanger*) theClone;

	return;
}

// Copy my data over...

void const ShaderGel::CopyData(ShaderGel* dest) {

	// Copy over the runtime data
	
	dest->fData = fData;

	// Copy over the scene data
	
	if (scene != 0) {
		dest->scene = scene;
	}

	// Tell it about its ancestors
	
	dest->parent = this;
	
	// All done
	
	return;
}

// Extension Data changed

MCCOMErr ShaderGel::ExtensionDataChanged() {

	TMCCountedPtr<I3DShTreeElementMember> 	myTreeElementMember;
	TMCCountedPtr<I3DShTreeElement>		myTreeElement;
	TMCCountedPtr<I3DShScene>		myScene;
	TMCCountedPtr<I3DShDataComponent> 	dataComp;
		
	TMCCountedPtr<CognitoData>		cognitoData;

	int count, nDataComp;

	double cogValue;

	TMCCountedPtr<ShaderGel>	theParent;
	
	// Ok, what's happened with the shader popup?
	
	switch (fData.popup) {

		// Clear Selected Shader?

		case SHADER_POPUP_CLEAR:
			
		  	fData.Shader = "";

			if (myPopup != 0) {
				TMCString255 name = "(None)";
				myPopup->SetItemText(0, name);
			}	

			break;
	
		// No selection
		 
		case SHADER_POPUP_IGNORE:
			
			fData.popup = SHADER_POPUP_CURRENT;
			
			break;
			
		// Already selected value

		case SHADER_POPUP_CURRENT:

			break;
		
		// Hmmmm...

		case -1:

			fData.popup = SHADER_POPUP_CURRENT;
			
			break;
		
		// Asked for a refresh

		case SHADER_POPUP_REFRESH:

			if (myPopup != 0) {
				FillShaderList2(myPopup);
			}	
			
			break;
		
		// Shader selected
	
		default:
			
			// Grab the scene
			
			if (scene != 0) {
			
				TMCString255 name;
				TMCCountedPtr<I3DShMasterShader> mastershader;
			
				// Retrieve master shader name
				
				scene->GetMasterShaderByIndex(&mastershader, fData.popup);
				mastershader->GetName(name);
				mastershader = 0;
			
				// Store in fData
			
				fData.Shader = name;

				// Update the popup?
				
				if (myPopup != 0) {
					myPopup->SetItemText(0, fData.Shader);
				}	
			}

			break;
	}

	// Select item 0 (although this doesn't always seem to work)...

	if (myPopup != 0) {
		myPopup->SetSelectedItem(0, true);
	}
	
	// We need to find Cognito
	
	cognitoData = 0;
	
	// Start off with the fData values...
	
	rotation = fData.rotation;
	
	offsetX	 = fData.offsetX;
	offsetY	 = fData.offsetY;
	
	scaleX	 = fData.scaleX;
	scaleY	 = fData.scaleY;
	
	repeatX	 = fData.repeatX;
	repeatY	 = fData.repeatY;
	
	// Go find the TreeElementMember API for this object
	
	myTreeElementMember = 0;
	myTreeElement = 0;
	
	this->QueryInterface(IID_I3DShTreeElementMember, (void**)&myTreeElementMember);

	if (myTreeElementMember != 0) {
	
		// Now find the Tree Element it points to...
	
		myTreeElementMember->GetTreeElement(&myTreeElement);

	}	

	// Do we gotta find the lights cognito modifier?
	
	if (myTreeElement != 0) {
		
		// Snaffle my scene value

		myTreeElement->GetScene(&myScene);

		if (myScene != 0) {

			// Stash it for the shader pop-up

			scene = myScene;

			// Tell my ancestors as well
			
			theParent = parent;
			
			while ( theParent != 0 ) {
				theParent->scene = myScene;
				theParent = theParent->parent;
			}	
			
			// Do we need to find the lights cognito data
		
			if ( fData.rotationFC
			  || fData.offsetXFC
			  || fData.offsetYFC
			  || fData.scaleXFC
			  || fData.scaleYFC 
			  || fData.repeatXFC
			  || fData.repeatYFC ) {
		
				// Ok, go see if there is a cognito data element attached
		
				nDataComp = myTreeElement->GetDataComponentsCount();
	
				for (count = nDataComp; count > 0 && !cognitoData; count--) {

					// loop on all the tree data components
		 
					myTreeElement->GetDataComponentByIndex(&dataComp, count-1);
		
					// Return it if it's the one we want...
	
					dataComp->QueryInterface(IID_CognitoDataExt, (void**)&cognitoData);
				}
	
				// Now can we use the data from Cognito?
	
				if (cognitoData != 0) {

					cogValue = 0;	
				
					cognitoData->GetRotation(myScene, cogValue);

					// Apply the value we got
			
					if (fData.rotationFC) {
						rotation += cogValue;
					}	

					if (fData.offsetXFC) {
						offsetX += cogValue;
					}	
	
					if (fData.offsetYFC) {
						offsetY += cogValue;
					}	

					if (fData.scaleXFC) {
						scaleX *= cogValue;
					}	

					if (fData.scaleYFC) {
						scaleY *= cogValue;
					}	

					if (fData.repeatXFC) {
						repeatX += cogValue;
					}	

					if (fData.repeatYFC) {
						repeatY += cogValue;
					}	
				}	
			}	
		}
	}
	
	// Convert and cache
	
	theta = rotation * TWO_PI;

	sin_theta = sin(theta);
	cos_theta = cos(theta);
	
	// All done
	
	return MC_S_OK;
}

// Put details into the master shader pop-up

void ShaderGel::FillShaderList(TMCCountedPtr<IMFPart> popuppart) {

	TMCCountedPtr<IMFTextPopupPart> popup;
	
	// We got a popup part?
	
	if (!popuppart) {
		return;
	}	
	
	// Find it's popup...
	
	popuppart->QueryInterface(IID_IMFTextPopupPart, (void**)&popup);

	// We got one?
	
	if (!popup) {
		return;
	}	
	
	// Remember it...
	
	myPopup = popup;

	// Go populate it
	
	FillShaderList2(popup);

	// All done...

	return;
}	
	
void ShaderGel::FillShaderList2(TMCCountedPtr<IMFTextPopupPart> popup) {

	TMCString255 name;
	
	uint32 nummenu = 0;
	
	TMCCountedPtr<I3DShScene>	theScene;
	TMCCountedPtr<ShaderGel>	theGel;
	
	// Wipe it...
	
	popup->RemoveAll();

	// Refill it...
	
	// First entry is always the current value...
	 
	if (fData.Shader.Length() != 0) {
		popup->AppendMenuItem(fData.Shader);
	} else {
		name = "(None)";
		popup->AppendMenuItem(name);
	}
	
	popup->SetItemActionNumber(nummenu, SHADER_POPUP_CURRENT);
	nummenu++;

	fData.popup = SHADER_POPUP_CURRENT;
	popup->SetSelectedItem(0, true);

	// Separator
	
	popup->AppendSeparator();
	popup->SetItemActionNumber(nummenu, SHADER_POPUP_IGNORE);
	nummenu++;
	
	// Second is always the Clear option...

	name = "(None)";
	popup->AppendMenuItem(name);
	popup->SetItemActionNumber(nummenu, SHADER_POPUP_CLEAR);
	nummenu++;

	// Confuscian programming: Ask parents if we don't know the scene
	
	theGel = this;
	theScene = theGel->scene;

	while ( !theScene
	     && theGel->parent != 0 ) {

		theGel = theGel->parent;
		theScene = theGel->scene;
	}

	// Cache results?
	
	if ( !(theGel.operator == (this)) 
	  && !(!theScene)) {
		scene = theScene;
	}	
	
//	// Third may be refresh...
//
//	if (scene != 0) {
//		name = "(Refresh)";
//		popup->AppendMenuItem(name);
//		popup->SetItemActionNumber(nummenu, SHADER_POPUP_REFRESH);
//		nummenu++;
//	}	
//
	// Separator
	
	popup->AppendSeparator();
	popup->SetItemActionNumber(nummenu, SHADER_POPUP_IGNORE);
	nummenu++;
	
	// Add the scenes master shader...
	
	if (scene != 0) {

		TMCCountedPtr<I3DShMasterShader> mastershader;
		
		uint32 numshaders = scene->GetMasterShadersCount();
		
		for (uint32 shaderindex = 0; shaderindex < numshaders; shaderindex++) {

			scene->GetMasterShaderByIndex(&mastershader, shaderindex);
			
			mastershader->GetName(name);
			mastershader = 0;
			
			popup->AppendMenuItem(name);
			popup->SetItemActionNumber(nummenu, shaderindex);
			nummenu++;
		}
	} else {
		name = "(Please Test Render)";
		popup->AppendMenuItem(name);
		popup->SetItemActionNumber(nummenu, SHADER_POPUP_IGNORE);
		nummenu++;
	}	

	// All done...
	
	return;
}


// Capture buttons on the dialog...

MCCOMErr ShaderGel::HandleEvent(MessageID message, IMFResponder* source, void* data){
	
 	IDType sourceID;
 	TMCCountedPtr<IMFPart> sourcePart;

	// Where'd it come from?
	
	if (source != 0) {
		source->QueryInterface(IID_IMFPart, (void**) &sourcePart);
	}	
	
	// What do I know it as?
	
	if (sourcePart != 0) {
		sourceID = sourcePart->GetIMFPartID();
		
		// Look for button pressed
	
		if ( message == kMsg_CUIP_ComponentAttached ) {

			// Refresh master shader list
		
			TMCCountedPtr<IMFPart> popuppart;
 
   			sourcePart->FindChildPartByID(&popuppart, IDTYPE('S','H','P','U'));

			if (popuppart != 0) {	
		  		FillShaderList(popuppart);
			}	
		}	
	}	
 
	// All Done
	
	return MC_S_OK;
}

// Work out the angle, given the opposite and adjacent

double findAngle(double opp, double adj) {

	double angle;
	
	if (adj != 0.0) {
		angle = atan(opp/adj);

		if (adj < 0.0) {
			if (opp < 0.0) {
				angle -= PI;
			} else {
				angle += PI;
			}
		}
	} else if (opp == 0.0) {
		angle = 0.0;
	} else if (opp > 0.0) {
		angle = PI/2.0;
	} else {
		angle = (3.0 * PI)/2.0;
	}	

	return angle;
}

// I3DExLightsourceGel methods :
boolean ShaderGel::GetGelValues(const TVector2& gelScreenPosition, TMCColorRGB &result) {
	
	TMCCountedPtr<I3DShTreeElementMember> 	myTreeElementMember;
	TMCCountedPtr<I3DShTreeElement>		myTreeElement;
	TMCCountedPtr<I3DShInstance>		myInstance;
	TMCCountedPtr<I3DShScene> 		myScene;
	TMCCountedPtr<I3DShMasterShader>	myShaderMaster;
	TMCCountedPtr<I3DShShader>		myShader;
	EShaderOutput				myShaderOut;
			
	TMCCountedPtr<ShaderGel>		theParent;
#if (VERSIONNUMBER >= 0x040000)
	TMCColorRGBA localresult;
#else
	TMCColorRGB localresult;
#endif

	ShadingIn 	shIn;
	real		mask;
	boolean 	shaderFull;

	double u,v;
	
	double new_u, new_v, work;
	
	MicroTick thisTime;
	
	real ioseconds = 0;

	boolean flip;
	
	// Set up some Error return values
	
	localresult.red	= 1.0f;
	localresult.green	= 0.0f;
	localresult.blue	= 0.0f;

	result = localresult;
	// Go find the TreeElementMember API for this object
	
	myTreeElementMember = 0;
	
	if (this->QueryInterface(IID_I3DShTreeElementMember, (void**)&myTreeElementMember) != MC_S_OK) {
		return true;
	}	

	if (!myTreeElementMember) {
		return true;
	}	
	
	// Now find the Tree Element it points to...
	
	myTreeElement = 0;
	
	myTreeElementMember->GetTreeElement(&myTreeElement);

	if (!myTreeElement) {
		return true;
	}	

	// Now find myInstance, if there is one...

	myInstance = 0;
	
	if (myTreeElement->QueryInterface(IID_I3DShInstance, (void**)&myInstance) != MC_S_OK) {
		return true;
	}	

	// Now find the scene...
	
	myScene = 0;
	
	myTreeElement->GetScene(&myScene);
			
	if (!myScene) {
		return true;
	}

	if (scene != myScene) {
		
		// Stash it for the shader pop-up

		scene = myScene;

		// Tell my ancestors as well
			
		theParent = parent;
			
		while ( theParent != 0 ) {

			if (theParent->scene != 0) {
				theParent->scene = myScene;
			}
			
			theParent = theParent->parent;
		}
	}	
			
	// Now find the shader?

	myShader = 0;
	myShaderMaster = 0;
	
	myScene->GetMasterShaderByName(&myShaderMaster, fData.Shader);

	if (!myShaderMaster) {
		ExtensionDataChanged();
		return true;
	}
	
	myShaderMaster->GetShader(&myShader);

	if (!myShader) {
		ExtensionDataChanged();
		return true;
	}

	myShaderMaster = 0;
	
	myShaderOut = myShader->GetImplementedOutput();

	if ( (myShaderOut & kUsesGetValue) != kUsesGetValue
	  && (myShaderOut & kUsesGetColor) != kUsesGetColor 
	  && (myShaderOut & kUsesDoShade) != kUsesDoShade ) {
			return true;
	}

	// New tick?
	
	thisTime = scene->GetTime(&ioseconds);

	if (thisTime != lastTime) {
		lastTime = thisTime;
		ExtensionDataChanged();
	}
	
	// Grab uv coord
	
	u = gelScreenPosition[0];
	v = gelScreenPosition[1];

	// Apply rotation
	
	if (theta != 0.0) {
		
		new_u = u * sin_theta - v * cos_theta;
		new_v = u * cos_theta + v * sin_theta;

		u = new_u;
		v = new_v;
	}	
	
	// Apply radial mapping
	
	if (fData.radialMapping) {

		new_u = findAngle(u,v)/TWO_PI;

		new_v = sqrt((u*u)+(v*v));

		u = new_u;

		v = new_v;
	}	

	// Apply scaling
	
	if (scaleX == 0.0) {
		u = 0.0;
	} else if (scaleX != 1.0) {
		u /= scaleX;
	}	
	
	if (scaleY == 0.0) {
		v = 0.0;
	} else if (scaleY != 1.0) {
		v /= scaleY;
	}	
	
	// Segment for repeat
	
	if (repeatX > 0.0) {

		// Preserve sign
		
		flip = false;
		
		if ( u < 0.0) {
			u *= -1.0;
		}
		
		// Scale back
	
		work = 1.0 / repeatX;
		
		while (u > work) {
			u -= work;
			flip ? flip = false : flip = true;
		}

		// Flipped block?
		
		if (flip) {
			u = work - u;
		}	
	}	
	
	if (repeatY > 0.0) {
		
		// Preserve sign
		
		flip = false;

		if ( v < 0.0) {
			v *= -1.0;
		}
		
		// Scale back
		
		work = 1.0 / repeatY;
		
		while (v > work) {
			v -= work;
			flip ? flip = false : flip = true;
		}

		// Flipped block?
		
		if (flip) {
			v = work - v;
		}	
	}	
	
	// Apply offsets
	
	u += offsetX;

	v += offsetY;


	// Prime shIn values...
	
	shIn.fPoint[0] = u;
	shIn.fPoint[1] = v;
	shIn.fPoint[2] = 0;
	
	shIn.fGNormal[0] = 0;
	shIn.fGNormal[1] = 0;
	shIn.fGNormal[2] = 1;
				
	shIn.fPointLoc[0] = u;
	shIn.fPointLoc[1] = v;
	shIn.fPointLoc[2] = 0;
				
	shIn.fNormalLoc[0] = 0;
	shIn.fNormalLoc[1] = 0;
	shIn.fNormalLoc[2] = 1;
				
	shIn.fUV[0] = u;
	shIn.fUV[1] = v;

	shIn.fInstance = myInstance;
	
	// Create shOut
	
#if (VERSIONNUMBER == 0x010000)
	ShadingOut shOut;
	shOut.SetDefaultValues(shIn);
#elif (VERSIONNUMBER == 0x020000)
	ShadingOut shOut(shIn);
#else
	ShadingOut shOut;
	shOut.SetDefaultValues();
#endif	
	
	// Go see what the shader does
	if ( (myShaderOut & kUsesGetColor) == kUsesGetColor)
	{
			myShader->GetColor(localresult, shaderFull, shIn);
	}
	else if ((myShaderOut & kUsesDoShade) == kUsesDoShade)
	{
			myShader->DoShade(shOut, shIn);
						
			localresult.red	= shOut.fColor.red;
			localresult.green	= shOut.fColor.green;
			localresult.blue	= shOut.fColor.blue;
	}
	else if ((myShaderOut & kUsesGetValue) == kUsesGetValue)
	{
			myShader->GetValue(mask, shaderFull, shIn);

			localresult.red	= mask;
			localresult.green	= mask;
			localresult.blue	= mask;
	}
	result = localresult;
	return true;
}
