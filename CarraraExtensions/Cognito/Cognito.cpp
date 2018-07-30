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


#include "copyright.h"
#include "Cognito.h"
#include "MCCountedPtrHelper.h"
#include "COMUtilities.h"
#include "BasicModifiers.h"
#include "I3DShRenderFeature.h"
#include "I3DShRenderable.h"
#include "I3DShTreeElement.h"
#include "I3DShModifier.h"
#include "I3DShScene.h"
#include "MtxMath.h"

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_Cognito(R_CLSID_Cognito);
const MCGUID IID_Cognito(R_IID_Cognito);

const MCGUID CLSID_CognitoData(R_CLSID_CognitoData);
const MCGUID IID_CognitoData(R_IID_CognitoData);

const MCGUID IID_CognitoDataExt(R_IID_CognitoDataExt);
#else
const MCGUID CLSID_Cognito={R_CLSID_Cognito};
const MCGUID IID_Cognito={R_IID_Cognito};

const MCGUID CLSID_CognitoData={R_CLSID_CognitoData};
const MCGUID IID_CognitoData={R_IID_CognitoData};

const MCGUID IID_CognitoDataExt={R_IID_CognitoDataExt};
#endif

// MCX Methods

void Extension3DInit(IMCUnknown* utilities) {

}

void Extension3DCleanup() {

}


TBasicUnknown* MakeCOMObject(const MCCLSID& classId) {

	TBasicUnknown* res = NULL;

	//check serial if you haven't already when creating the key extensions in your project

	if (classId == CLSID_Cognito) {

		res = new Cognito; 

	}


	if (classId == CLSID_CognitoData) {
		res = new CognitoData;
	}	

	return res;
}

// Cognito Methods

void* Cognito::GetExtensionDataBuffer() {
	return &fData;
}

// Constructor / Destructor
Cognito::Cognito() {

	fData.vectorX	= 0.0f;
	fData.vectorY	= 1.0f;
	fData.vectorZ	= 0.0f;

	fData.around_x	= false;
	fData.around_y	= false;
	fData.around_z	= false;

	fData.radius	= 0.0f;

	fData.hint_vector = "Hint for Vector";
	fData.hint_parm = "Hint for Parm";
	fData.hint_hot = "Hint for HotPoint";

	depth = 0.0;

	checkTree = false;

	checkTime = -1;

	return;
}

// Find an objects Cognito Modifier

Cognito *getCognitoMod(I3DShTreeElement *treeElement) {

	TMCCountedPtr<I3DShModifier> modifier;
	TMCCountedPtr<Cognito> cgm;

	int count, nDataComp;

	cgm = NULL;

	nDataComp = treeElement->GetModifiersCount();

	for (count = nDataComp; count > 0; count--) {

		// loop on all the tree data components

		treeElement->GetModifierByIndex(&modifier, count-1);

		// Return it if it's the one we want...

		if (modifier->QueryInterface(IID_Cognito, (void**)&cgm) == MC_S_OK) {

			return cgm;
		}
	}

	// Actually returns NULL

	return cgm;
}	

// Find an objects CognitoDatabuffer

CognitoData *getCognitoData(I3DShTreeElement *treeElement) {

	TMCCountedPtr<I3DShDataComponent> component;
	TMCCountedPtr<CognitoData> cgd;

	int count, nDataComp;

	cgd = NULL;

	nDataComp = treeElement->GetDataComponentsCount();

	for (count = nDataComp; count > 0; count--) {

		// loop on all the tree data components

		treeElement->GetDataComponentByIndex(&component, count-1);

		// Return it if it's the one we want...

		if (component->QueryInterface(IID_CognitoData, (void**)&cgd) == MC_S_OK) {

			return cgd;
		}
	}

	// Actually returns NULL

	return cgd;
}	

// Get an trees Cognito buffer...

CognitoBuffer * Cognito::GetBuffer(I3DShTreeElement* tree) {

	TMCCountedPtr<Cognito> treeCogMod;

	CognitoBuffer *treeCogBuf;

	treeCogBuf = 0;

	// Get this objects cognito data

	treeCogMod = getCognitoMod(tree);

	if (treeCogMod) {

		treeCogBuf = (CognitoBuffer *) (treeCogMod->GetExtensionDataBuffer());

	}

	return treeCogBuf;
}

// Get an trees Cognito data buffer...

CognitoDataBuffer * Cognito::GetDataBuffer(I3DShTreeElement* tree) {

	TMCCountedPtr<CognitoData> treeCogData;

	CognitoDataBuffer *treeCogDataBuf;

	treeCogDataBuf = 0;

	// Get this objects cognito data

	treeCogData = getCognitoData(tree);

	if (treeCogData) {

		treeCogDataBuf = (CognitoDataBuffer *) (treeCogData->GetExtensionDataBuffer());

	}

	return treeCogDataBuf;
}

// Work out a trees rotation

double Cognito::GetRotation(I3DShTreeElement* tree) {

	TMCCountedPtr<I3DShScene> scene;

	CognitoDataBuffer *treeCogDataBuf;

	TMCCountedPtr<I3DShTreeElement> driver;

	CognitoDataBuffer *driverCogDataBuf;

	double rotation;

	// Get this objects cognito data

	treeCogDataBuf = GetDataBuffer(tree);

	if (!treeCogDataBuf) {
		return 0.0;
	}	

	// Simple case is the motor...

	if (treeCogDataBuf->motor) {
		return treeCogDataBuf->rotation;
	}	

	// If it's not a slave, then there's no rotation...

	if (!treeCogDataBuf->slave) {
		return 0.0;
	}	

	// Get the scene

	tree->GetScene(&scene);

	if (!scene) {
		return 0.0; 
	}	

	// Search for the driving tree

	scene->GetTreeElementByName(&driver, treeCogDataBuf->driver);

	if (!driver) {
		return 0.0;
	}

	// Find the drivers data block...

	driverCogDataBuf = GetDataBuffer(driver);

	if (!driverCogDataBuf) {
		return 0.0;
	}	

	// Ward for infinite loops

	depth += 1;

	if (depth > 250) {
		return 0.0;
	}	


	// Ok, get the drivers rotation...

	rotation = GetRotation(driver) + treeCogDataBuf->offset;

	// Ok, we can relax now...

	depth -= 1;

	// Now translate it into this objects rotation

	switch (treeCogDataBuf->linkage) {

		// Gears reverse direction and scale by teeth ratio
		// Cross linked chains are just like gears	

		case ACT_LINK_GEAR:
		case ACT_LINK_XCHAIN:

			rotation = -1.0 * (rotation * driverCogDataBuf->teeth) / treeCogDataBuf->teeth;

			break;

			// Axels just pass through the rotation	
			// Radial Offsets just pass through the rotation	

		case ACT_LINK_AXEL:
		case ACT_LINK_RAD_OFFSET:

			break;

			// Chains are like gears but do not reverse direction	

		case ACT_LINK_CHAIN:

			rotation = (rotation * driverCogDataBuf->teeth) / treeCogDataBuf->teeth;

			break;

			// Racks move left/right based upon their tooth pitch	

		case ACT_LINK_RACK:

			rotation *= driverCogDataBuf->teeth;

			if (treeCogDataBuf->pitch != 0.0) {
				rotation /= treeCogDataBuf->pitch;
			}	

			break;

			// Worms move in and out based upon the pitch of the worm

		case ACT_LINK_WORM:

			if (treeCogDataBuf->pitch != 0.0) {
				rotation /= treeCogDataBuf->pitch;
			}	

			break;

			// Cams move up and down, based on a radial offset
			// Rotation is translated to periodic linear motion
			// Item is assumed to be placed initially at the dwell point
			// Resulting motion is a positive offset from 0 to pushrod Length
			// Offset will have the effect of changing the phase of the cams

		case ACT_LINK_CAM:

			rotation -= floor(rotation);

			rotation = sin(rotation * PI) * fData.radius;

			break;

			// Pistons also move up and down
			// Rotation is returned as rotation

		case ACT_LINK_PISTON:

			break;

			// Pushrods moves up and down and side to side.
			// Rotation is returned as rotation

		case ACT_LINK_PUSHROD:

			break;

			// Pushrod Pistons moves up and down.
			// Rotation is returned as rotation

		case ACT_LINK_PUSHROD_PISTON:

			break;

			// Rockers and Trip Wire turn and turn back.
			// Rotation is returned modified by teeth

		case ACT_LINK_ROCKER:
		case ACT_LINK_TRIP_HAMMER:

			rotation = (rotation * driverCogDataBuf->teeth) / treeCogDataBuf->teeth;

			// Ok, what rotation do we actually see?

			rotation -= floor(rotation);

			if (treeCogDataBuf->linkage == ACT_LINK_TRIP_HAMMER) {

				if (rotation < 0.98) {
					rotation = sin((PI/2.0) * rotation);
				} else {
					rotation = sin((PI/2.0) * ((1.00 - rotation) * 49.0));
				}	

				rotation *= treeCogDataBuf->pitch;

			} else {

				// Rockers are simpler... 

				rotation = sin(rotation * TWO_PI) * treeCogDataBuf->pitch;
			}	

			rotation *= -1.0;

			break;

			// A pushrod cylinder should stop rotation, but we'll let it
			// through unmodified.

		case ACT_LINK_PUSHROD_CYLINDER:

			break;

		default:
			break;
	}		

	// Return the calculated rotation

	return rotation;
}

TMCCountedPtr<I3DShTreeElement> Cognito::GetDriver(I3DShTreeElement *tree, CognitoDataBuffer *treeCogDataBuf) {

	TMCCountedPtr<I3DShScene> scene;

	TMCCountedPtr<I3DShTreeElement> driver;

	driver = 0;

	// Motors don't have a linkage

	if (!treeCogDataBuf->slave) {
		return driver;
	}

	// Get the scene

	tree->GetScene(&scene);

	if (!scene) {
		return driver; 
	}	

	// Search for the driving tree

	scene->GetTreeElementByName(&driver, treeCogDataBuf->driver);

	return driver;
}	

int32 Cognito::GetLinkage(I3DShTreeElement *tree, CognitoDataBuffer *treeCogDataBuf) {

	TMCCountedPtr<I3DShTreeElement> driver;

	CognitoDataBuffer *driverCogDataBuf;

	int32 linkage, driver_linkage;

	// Motors don't have a linkage

	if (!treeCogDataBuf->slave) {
		return ACT_LINK_AXEL;
	}	

	// Normally the linkage is simple to work out

	linkage = treeCogDataBuf->linkage;

	// But Axels can be a bit complicated

	if (linkage == ACT_LINK_AXEL) { 

		// Search for the driving tree

		driver = GetDriver(tree, treeCogDataBuf);

		if (driver) {

			// Find the drivers data block...

			driverCogDataBuf = GetDataBuffer(driver);

			if (driverCogDataBuf) {

				// Ward for infinite loops

				depth += 1;

				if (depth > 250) {
					return 0.0;
				}	

				// Ok, get the drivers rotation...

				driver_linkage = GetLinkage(driver, driverCogDataBuf);

				// Ok, we can relax now...

				depth -= 1;

				// Adjust for Rack, PushRod and Worm linkages

				if ( driver_linkage == ACT_LINK_RACK
					|| driver_linkage == ACT_LINK_CAM
					|| driver_linkage == ACT_LINK_PUSHROD
					|| driver_linkage == ACT_LINK_PISTON
					|| driver_linkage == ACT_LINK_ROCKER
					|| driver_linkage == ACT_LINK_TRIP_HAMMER
					|| driver_linkage == ACT_LINK_WORM ) {
						linkage = driver_linkage;
				}	
			}	
		}
	}	

	return linkage;
}	

// Work out the angle, given the opposite and adjacent

double Cognito::findAngle(double opp, double adj) {

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

double	Cognito::AngleFromVertical(double x, double y, double z) {

	double angle;

	TVector3 work;

	// Take the vector of motion

	work[0] = x;
	work[1] = y;
	work[2] = z;

	// Normalize it

	work.Normalize();

	// Calculate rotation from X=0, y=0

	work[2] = 1 - work[2];

	angle = 2 * asin( sqrt( work[0] * work[0] + work[1] * work[1] + work[2] * work[2] ) /2 );

	return angle;
}	

// Apply the behaviour to an object

MCCOMErr Cognito::Apply(I3DShTreeElement* tree) {

	TMCCountedPtr<I3DShScene> scene;

	TMCCountedPtr<CognitoData> myCogData;

	CognitoDataBuffer *myCogDataBuf;

	TMCCountedPtr<I3DShTreeElement> driver;

	TMCCountedPtr<I3DShTreeElement> parent;

	CognitoBuffer *driverBuf;

	MicroTick thisTime;

	real ioseconds = 0;

	double rotation = 0.0;

	// Vars for position calculation

	TTreeTransform ttrf, pttrf, dttrf;

	TMatrix33 tr;

	TMatrix33 tr1;

	TMatrix33	ptr, ptri;

	TVector3 ofs;

	double theta, delta, mag, phi, tau;

	int32 linkage;

	// Get this objects cognito data

	myCogDataBuf = GetDataBuffer(tree);

	if (!myCogDataBuf) {
		return MC_S_OK;
	}	

	// Are we interested in this object?

	if ( !myCogDataBuf->motor
		&& !myCogDataBuf->slave ) {
			return MC_S_OK;
	}	

	// Update hints...
	fData.hint_parm2 = "Unused.";

	if (myCogDataBuf->motor) {
		fData.hint_vector = "Axis of Rotation.";
		fData.hint_parm = "Unused.";
	} else
		switch (myCogDataBuf->linkage) {

			case ACT_LINK_GEAR:
			case ACT_LINK_AXEL:
			case ACT_LINK_CHAIN:
			case ACT_LINK_XCHAIN:
				fData.hint_vector ="Axis of Rotation.";
				fData.hint_parm = "Unused.";
				fData.hint_hot = "Hot point at center of rotation.";
				break;

			case ACT_LINK_RACK:
			case ACT_LINK_WORM:
				fData.hint_vector ="Direction of Motion.";
				fData.hint_parm = "Unused.";
				fData.hint_hot = "Hot point at zero offset position.";
				break;

			case ACT_LINK_CAM:
				fData.hint_vector ="Direction of Motion.";
				fData.hint_parm = "Length of CAMs motion.";
				fData.hint_hot = "Hot point at zero offset position.";
				break;

			case ACT_LINK_RAD_OFFSET:
				fData.hint_vector ="Axis of Rotation.";
				fData.hint_parm = "Radius of Rotation.";
				fData.hint_hot = "Hot point at center of object and at center of rotation.";
				break;

			case ACT_LINK_PUSHROD:
				fData.hint_vector ="Direction of Pistons motion.";
				fData.hint_parm = "Distance from pushrod to piston.";
				fData.hint_hot = "Hot point and bearing end at the same place as the bearings.";
				break;

			case ACT_LINK_PISTON:
				fData.hint_vector ="Direction of Pistons motion.";
				fData.hint_parm = "Stroke length (= radius of driving wheel).";
				fData.hint_hot = "Hot point on motion vector in central position.";
				fData.hint_parm2 = "Pushrod length.";
				break;

			case ACT_LINK_PUSHROD_PISTON:
				fData.hint_vector ="Direction to Cylinders pivot.";
				fData.hint_parm = "Length from Pushrod to pivot.";
				fData.hint_hot = "Hot point and bearing end at the same place as the bearings.";
				break;

			case ACT_LINK_PUSHROD_CYLINDER:
				fData.hint_vector ="Direction to Cylinders pivot.";
				fData.hint_parm = "Length from Pushrod to pivot.";
				fData.hint_hot = "Hot point at cylinder pivot point along vector.";
				break;

			case ACT_LINK_ROCKER:
			case ACT_LINK_TRIP_HAMMER:
				fData.hint_vector ="Axis of Rotation.";
				fData.hint_parm = "Unused.";
				fData.hint_hot = "Hot point at center of rotation.";
				break;

			default:
				fData.hint_vector ="Unknown linkage!";
				fData.hint_parm = "Unknown linkage!";
				fData.hint_hot = "Unknown linkage!";
				break;
	}		

	// Get the scene

	tree->GetScene(&scene);

	if (!scene) {
		return MC_S_OK;
	}	

	// What's the time?

	thisTime = scene->GetTime(&ioseconds);

	if (thisTime == 0.0) {
		return MC_S_OK;
	}

	// Ok, get its rotation...

	rotation = GetRotation(tree);

	// Get the trees global transform

	tree->GetGlobalTreeTransform(ttrf);

	// Deja-vu?

	if (checkTime == thisTime) {

		if (checkTree) {
			if (ttrf == lastTree) {
				return MC_S_OK;
			}	
		}
	}	

	// What's the outer group?

	tree->GetTop(&parent);

	// Now, prime some transforms

	if (parent != 0) {
		parent->GetGlobalTreeTransform(pttrf);

#if (VERSIONNUMBER >= 0x060000 )
		ptr = pttrf.GetRotation();
#else
		ptr = pttrf.fRotation;
#endif
	}	

	TMtxMath::SolveInverseGeneralCR(ptr, ptri);

	// Work out the appropriate transform

	checkTree = false;

	checkTime = -1;

	linkage = GetLinkage(tree, myCogDataBuf);

	switch (linkage) {

		case ACT_LINK_GEAR:
		case ACT_LINK_AXEL:
		case ACT_LINK_CHAIN:
		case ACT_LINK_XCHAIN:
		case ACT_LINK_ROCKER:
		case ACT_LINK_TRIP_HAMMER:
			{

				// Start with current rotation

				tr = ttrf.GetRotation();

				// Back out parents rotation

				tr.Multiply(ptri, tr);

				// Translate the rotation into radians

				rotation *= TWO_PI;

				// Adjust rockers direction

				if (linkage == ACT_LINK_ROCKER) {
					rotation *= -1.0;
				}	

				// Calculate X/Y rotation angle
				theta = findAngle(fData.vectorY, fData.vectorX);

				// Rotation in X/Y to align to X

				tr1 = TMatrix33::kIdentity;

				tr1[0][0] =  cos(theta);
				tr1[0][1] = -sin(theta);
				tr1[1][0] =  sin(theta);
				tr1[1][1] =  cos(theta);
				tr.Multiply(tr1, tr);

				// Calculate X/Z rotation angle
				mag = sqrt(fData.vectorX * fData.vectorX + fData.vectorY * fData.vectorY);

				delta = findAngle(fData.vectorZ, mag);

				// Start by backing out the parents transform

				tr1 = TMatrix33::kIdentity;

				// Rotation in X/Z to align to X

				tr1[0][0] =  cos(delta);
				tr1[0][2] = -sin(delta);
				tr1[2][0] =  sin(delta);
				tr1[2][2] =  cos(delta);

				tr.Multiply(tr1, tr);

				// Apply the actual rotation
				// Rotation in Y-Z plane (around X)

				tr1 = TMatrix33::kIdentity;

				tr1[1][1] =  cos(rotation);
				tr1[1][2] = -sin(rotation);
				tr1[2][1] =  sin(rotation);
				tr1[2][2] =  cos(rotation);
				tr.Multiply(tr1, tr);

				// Apply back rotation in X/Z

				tr1 = TMatrix33::kIdentity;

				tr1[0][0] =  cos(-delta);
				tr1[0][2] = -sin(-delta);
				tr1[2][0] =  sin(-delta);
				tr1[2][2] =  cos(-delta);

				tr.Multiply(tr1, tr);

				// Apply back rotation in X/Y

				tr1 = TMatrix33::kIdentity;

				tr1[0][0] =  cos(-theta);
				tr1[0][1] = -sin(-theta);
				tr1[1][0] =  sin(-theta);
				tr1[1][1] =  cos(-theta);

				tr.Multiply(tr1, tr);

				// Reapply the parents rotation

				tr.Multiply(ptr, tr);

				// Apply

				ttrf.SetRotation(tr);

				// Apply update to Tree transform

				checkTree = true;
			}
			break;

		case ACT_LINK_RAD_OFFSET:

			{
				TVector3 tempOffset;
				tempOffset = ttrf.GetOffset();

				// Translate the rotation into radians

				rotation *= TWO_PI;

				// Set the linear motion into the translation
				// Offset calculated on Y/Z plane, othogoanl to X-axis

				ofs[0] = 0;
				ofs[1] = sin(rotation) * fData.radius;
				ofs[2] = cos(rotation) * fData.radius;

				// Calculate X/Z rotation angle

				mag = sqrt(fData.vectorX * fData.vectorX + fData.vectorY * fData.vectorY);

				delta = findAngle(fData.vectorZ, mag);

				// Apply back rotation in X/Z

				tr1 = TMatrix33::kIdentity;

				tr1[0][0] =  cos(-delta);
				tr1[0][2] = -sin(-delta);
				tr1[2][0] =  sin(-delta);
				tr1[2][2] =  cos(-delta);

				ofs = ofs * tr1;

				// Calculate X/Y rotation angle

				theta = findAngle(fData.vectorY, fData.vectorX);

				// Apply back rotation in X/Y

				tr1 = TMatrix33::kIdentity;

				tr1[0][0] =  cos(-theta);
				tr1[0][1] = -sin(-theta);
				tr1[1][0] =  sin(-theta);
				tr1[1][1] =  cos(-theta);

				ofs = ofs * tr1;

				// Apply the parents rotation

				ofs = ofs * ptri;

				// Apply the adjusted translation

				tempOffset += ofs;

				ttrf.SetOffset(tempOffset);

				// Apply update to Tree transform
				checkTree = true;

				// All done
			}	
			break;

			// Pushrods are implemented by transforming the rod along with its
			// end point.  This is done in the plane of its driver and uses the
			// radius specified for its driver.
			//
			// It is then rotated to bring its far end back to the axis specified
			// for its motion.  It's radius is used as it's length for this operation.

		case ACT_LINK_PUSHROD:

			{
				TVector3 tempOffset;
				tempOffset = ttrf.GetOffset();

				// Search for the driving tree

				driver = GetDriver(tree, myCogDataBuf);

				if (!driver) {
					return MC_S_OK;
				}	

				// Get this objects cognito data

				driverBuf = GetBuffer(driver);

				if (!driverBuf) {
					return MC_S_OK;
				}	

				// Rotation is in the plane of the driver

				rotation -= floor(rotation);

				// Translate the rotation into radians

				rotation *= TWO_PI;

				// Set the linear motion into the translation
				// Offset calculated on Y/Z plane, othogoanl to X-axis

				ofs[0] = 0;
				ofs[1] = sin(rotation) * driverBuf->radius;
				ofs[2] = cos(rotation) * driverBuf->radius;

				// Calculate X/Z rotation angle

				mag = sqrt(driverBuf->vectorX * driverBuf->vectorX + driverBuf->vectorY * driverBuf->vectorY);

				delta = findAngle(driverBuf->vectorZ, mag);

				// Apply back rotation in X/Z

				tr1 = TMatrix33::kIdentity;

				tr1[0][0] =  cos(-delta);
				tr1[0][2] = -sin(-delta);
				tr1[2][0] =  sin(-delta);
				tr1[2][2] =  cos(-delta);

				ofs = ofs * tr1;

				// Calculate X/Y rotation angle

				theta = findAngle(driverBuf->vectorY, driverBuf->vectorX);

				// Apply back rotation in X/Y

				tr1 = TMatrix33::kIdentity;

				tr1[0][0] =  cos(-theta);
				tr1[0][1] = -sin(-theta);
				tr1[1][0] =  sin(-theta);
				tr1[1][1] =  cos(-theta);

				ofs = ofs * tr1;

				// Apply the parents rotation

				ofs = ofs * ptri;

				// Apply the adjusted translation

				tempOffset += ofs;

				ttrf.SetOffset(tempOffset);

#if (VERSIONNUMBER >= 0x040000 )	
				tree->SetGlobalTreeTransform(ttrf, kXTreeBehaviorDefault);
#else
				tree->SetGlobalTreeTransform(ttrf, kXTreeBehaviorDefault, true);
#endif	

				// Now we need some rotation 

				if (fData.radius != 0.0) {

					// Fetch the trees updated global transform

					tree->GetGlobalTreeTransform(ttrf);

					// Now, what's the angle between (0,0)->(r,0) to (0,0), (z,y)

					// Calculate rotation from X=0, y=0

					tau = AngleFromVertical(fData.vectorX, fData.vectorY, fData.vectorZ);

					rotation = tau - rotation;

					// Calculate angle of rotation

					phi = findAngle( driverBuf->radius * sin(rotation), fData.radius );

					// Start with current rotation

					tr = ttrf.GetRotation();

					// Back out parents rotation

					tr.Multiply(ptri, tr);

					// Calculate X/Y rotation angle

					theta = findAngle(fData.vectorY, fData.vectorX);

					// Rotation in X/Y to align to X

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(theta);
					tr1[0][1] = -sin(theta);
					tr1[1][0] =  sin(theta);
					tr1[1][1] =  cos(theta);

					tr.Multiply(tr1, tr);

					// Calculate X/Z rotation angle

					mag = sqrt(fData.vectorX * fData.vectorX + fData.vectorY * fData.vectorY);

					delta = findAngle(fData.vectorZ, mag);

					// Rotation in X/Z to align to X

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(delta);
					tr1[0][2] = -sin(delta);
					tr1[2][0] =  sin(delta);
					tr1[2][2] =  cos(delta);

					tr.Multiply(tr1, tr);

					// Apply the actual rotation
					// Rotation in X-Z plane (around Y)

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(phi);
					tr1[0][2] = -sin(phi);
					tr1[2][0] =  sin(phi);
					tr1[2][2] =  cos(phi);

					tr.Multiply(tr1, tr);

					// Apply back rotation in X/Z

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(-delta);
					tr1[0][2] = -sin(-delta);
					tr1[2][0] =  sin(-delta);
					tr1[2][2] =  cos(-delta);

					tr.Multiply(tr1, tr);

					// Apply back rotation in X/Y

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(-theta);
					tr1[0][1] = -sin(-theta);
					tr1[1][0] =  sin(-theta);
					tr1[1][1] =  cos(-theta);

					tr.Multiply(tr1, tr);

					// Reapply the parents rotation

					tr.Multiply(ptr, tr);

					// Apply rotation

					ttrf.SetRotation(tr);

					// Apply update to Tree transform

					checkTree = true;

				}	


				// All done
			}	
			break;

			// Pushrod Pistonss are implemented by transforming the rod along with its
			// end point.  This is done in the plane of its driver and uses the
			// radius specified for its driver.
			//
			// It is then rotated to bring its far end back to point to a set point on
			// its specified axis.  It's radius is used as it's length for this operation.

		case ACT_LINK_PUSHROD_PISTON:
			{
				TVector3 tempOffset;
				tempOffset = ttrf.GetOffset();

				// Search for the driving tree

				driver = GetDriver(tree, myCogDataBuf);

				if (!driver) {
					return MC_S_OK;
				}	

				// Get this objects cognito data

				driverBuf = GetBuffer(driver);

				if (!driverBuf) {
					return MC_S_OK;
				}	

				// Rotation is in the plane of the driver

				rotation -= floor(rotation);

				// Translate the rotation into radians

				rotation *= TWO_PI;

				// Set the linear motion into the translation
				// Offset calculated on Y/Z plane, othogoanl to X-axis

				ofs[0] = 0;
				ofs[1] = sin(rotation) * driverBuf->radius;
				ofs[2] = cos(rotation) * driverBuf->radius;

				// Calculate X/Z rotation angle

				mag = sqrt(driverBuf->vectorX * driverBuf->vectorX + driverBuf->vectorY * driverBuf->vectorY);

				delta = findAngle(driverBuf->vectorZ, mag);

				// Apply back rotation in X/Z

				tr1 = TMatrix33::kIdentity;

				tr1[0][0] =  cos(-delta);
				tr1[0][2] = -sin(-delta);
				tr1[2][0] =  sin(-delta);
				tr1[2][2] =  cos(-delta);

				ofs = ofs * tr1;

				// Calculate X/Y rotation angle

				theta = findAngle(driverBuf->vectorY, driverBuf->vectorX);

				// Apply back rotation in X/Y

				tr1 = TMatrix33::kIdentity;

				tr1[0][0] =  cos(-theta);
				tr1[0][1] = -sin(-theta);
				tr1[1][0] =  sin(-theta);
				tr1[1][1] =  cos(-theta);

				ofs = ofs * tr1;

				// Apply the parents rotation

				ofs = ofs * ptri;

				// Apply the adjusted translation

				tempOffset += ofs;

				ttrf.SetOffset(tempOffset);

#if (VERSIONNUMBER >= 0x040000 )	
				tree->SetGlobalTreeTransform(ttrf, kXTreeBehaviorDefault);
#else
				tree->SetGlobalTreeTransform(ttrf, kXTreeBehaviorDefault, true);
#endif	

				// Now we need some rotation 

				if (fData.radius != 0.0) {

					// Fetch the trees updated global transform

					tree->GetGlobalTreeTransform(ttrf);

					// Now, what's the angle between (0,0)->(r,0) to (0,0), (z,y)

					// Calculate rotation from X=0, y=0

					tau = AngleFromVertical(fData.vectorX, fData.vectorY, fData.vectorZ);

					rotation = tau - rotation;

					// Calculate angle of rotation

					phi = findAngle( driverBuf->radius * sin(rotation), 
						fData.radius - (driverBuf->radius * cos(rotation)) );

					// phi += 0.25 * cos(tau) * TWO_PI;

					// Start with current rotation

					tr = ttrf.GetRotation();

					// Back out parents rotation

					tr.Multiply(ptri, tr);

					// Calculate X/Y rotation angle

					theta = findAngle(fData.vectorY, fData.vectorX);

					// Rotation in X/Y to align to X

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(theta);
					tr1[0][1] = -sin(theta);
					tr1[1][0] =  sin(theta);
					tr1[1][1] =  cos(theta);

					tr.Multiply(tr1, tr);

					// Calculate X/Z rotation angle

					mag = sqrt(fData.vectorX * fData.vectorX + fData.vectorY * fData.vectorY);

					delta = findAngle(fData.vectorZ, mag);

					// Rotation in X/Z to align to X

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(delta);
					tr1[0][2] = -sin(delta);
					tr1[2][0] =  sin(delta);
					tr1[2][2] =  cos(delta);

					tr.Multiply(tr1, tr);

					// Apply the actual rotation
					// Rotation in x-Z plane (around X)

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(phi);
					tr1[0][2] = -sin(phi);
					tr1[2][0] =  sin(phi);
					tr1[2][2] =  cos(phi);

					tr.Multiply(tr1, tr);

					// Apply back rotation in X/Z

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(-delta);
					tr1[0][2] = -sin(-delta);
					tr1[2][0] =  sin(-delta);
					tr1[2][2] =  cos(-delta);

					tr.Multiply(tr1, tr);

					// Apply back rotation in X/Y

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(-theta);
					tr1[0][1] = -sin(-theta);
					tr1[1][0] =  sin(-theta);
					tr1[1][1] =  cos(-theta);

					tr.Multiply(tr1, tr);

					// Reapply the parents rotation

					tr.Multiply(ptr, tr);

					// Apply rotation

					ttrf.SetRotation(tr);

					// Apply update to Tree transform

					checkTree = true;

				}	

				// All done
			}
			break;

			// Pushrod cylinders are implemented by rotating the piston around its
			// hot point, so it lines up with the point of the rad offset.

		case ACT_LINK_PUSHROD_CYLINDER:
			{

				// Search for the driving tree

				driver = GetDriver(tree, myCogDataBuf);

				if (!driver) {
					return MC_S_OK;
				}	

				// Get this objects cognito data

				driverBuf = GetBuffer(driver);

				if (!driverBuf) {
					return MC_S_OK;
				}	

				// Rotation is in the plane of the driver

				rotation -= floor(rotation);

				// Translate the rotation into radians

				rotation *= TWO_PI;

				// Now we need some rotation 

				if (fData.radius != 0.0) {

					// Now, what's the angle between (0,0)->(r,0) to (0,0), (z,y)

					// What the angle from vertical?

					tau = AngleFromVertical(fData.vectorX, fData.vectorY, fData.vectorZ);

					rotation = tau - rotation;

					// Adjust to maintain origin

					phi = findAngle( driverBuf->radius * sin(rotation), 
						fData.radius - (driverBuf->radius * cos(rotation)) );

					// phi += 0.25 * cos(tau) * TWO_PI;

					// Start with current rotation

					tr = ttrf.GetRotation();

					// Back out parents rotation

					tr.Multiply(ptri, tr);

					// Calculate X/Y rotation angle

					theta = findAngle(fData.vectorY, fData.vectorX);

					// Rotation in X/Y to align to X

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(theta);
					tr1[0][1] = -sin(theta);
					tr1[1][0] =  sin(theta);
					tr1[1][1] =  cos(theta);

					tr.Multiply(tr1, tr);

					// Calculate X/Z rotation angle

					mag = sqrt(fData.vectorX * fData.vectorX + fData.vectorY * fData.vectorY);

					delta = findAngle(fData.vectorZ, mag);

					// Rotation in X/Z to align to X

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(delta);
					tr1[0][2] = -sin(delta);
					tr1[2][0] =  sin(delta);
					tr1[2][2] =  cos(delta);

					tr.Multiply(tr1, tr);

					// Apply the actual rotation
					// Rotation in Y-Z plane (around X)

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(phi);
					tr1[0][2] = -sin(phi);
					tr1[2][0] =  sin(phi);
					tr1[2][2] =  cos(phi);

					tr.Multiply(tr1, tr);

					// Apply back rotation in X/Z

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(-delta);
					tr1[0][2] = -sin(-delta);
					tr1[2][0] =  sin(-delta);
					tr1[2][2] =  cos(-delta);

					tr.Multiply(tr1, tr);

					// Apply back rotation in X/Y

					tr1 = TMatrix33::kIdentity;

					tr1[0][0] =  cos(-theta);
					tr1[0][1] = -sin(-theta);
					tr1[1][0] =  sin(-theta);
					tr1[1][1] =  cos(-theta);

					tr.Multiply(tr1, tr);

					// Reapply the parents rotation

					tr.Multiply(ptr, tr);

					// Apply rotation

					ttrf.SetRotation(tr);

					// Apply update to Tree transform

					checkTree = true;

				}	

				// All done
			}
			break;

		case ACT_LINK_PISTON:
			{
				TVector3 myOffset;
				myOffset = ttrf.GetOffset();

				// Calculate rotation from X=0, y=0

				tau = AngleFromVertical(fData.vectorX, fData.vectorY, fData.vectorZ);

				// What's the rotation

				rotation *= TWO_PI;

				rotation -= tau;

				real32 verticalOffset = fData.radius2 //full length of push rod
					- sqrt(pow(fData.radius2, 2) - pow(sin(rotation) * fData.radius, 2)) 
					- cos(rotation) * fData.radius; //

				// Calculate offset

				ofs[0] = fData.vectorX * verticalOffset;
				ofs[1] = fData.vectorY * verticalOffset;
				ofs[2] = fData.vectorZ * verticalOffset;

				// Apply the parents rotation

				ofs = ofs * ptri;

				// Set the linear motion into the translation

				myOffset -= ofs;

				ttrf.SetOffset(myOffset);

				// Apply update to Tree transform
				checkTree = true;

				// All done
			}

			break;

		case ACT_LINK_RACK:

		case ACT_LINK_CAM:

		case ACT_LINK_WORM:

		default:

			// Set the linear motion into the translation
			{
				TVector3 tempOffset;
				tempOffset = ttrf.GetOffset();

				tempOffset[0] -= rotation * fData.vectorX;
				tempOffset[1] -= rotation * fData.vectorY;
				tempOffset[2] -= rotation * fData.vectorZ;

				ttrf.SetOffset(tempOffset);

				// Apply update to Tree transform
				checkTree = true;

				// All done
			}


			break;

	}		

	// Update the trees global transform

	if (checkTree) {

#if (VERSIONNUMBER >= 0x040000 )	
		tree->SetGlobalTreeTransform(ttrf, kXTreeBehaviorDefault);
#else
		tree->SetGlobalTreeTransform(ttrf, kXTreeBehaviorDefault, true);
#endif

		// Fetch it back as Carrara may have messed with it

		tree->GetGlobalTreeTransform(lastTree);
	}	

	// Remember which frame this applies for

	checkTime = thisTime;

	// All done

	return MC_S_OK;
}

// Helper for the data panel

MCCOMErr Cognito::ExtensionDataChanged() {

	// Rotate around X

	if (fData.around_x) {
		fData.around_x = false;

		fData.vectorX = 1.0f;
		fData.vectorY = 0.0f;
		fData.vectorZ = 0.0f;
	}	

	// Rotate around Y

	if (fData.around_y) {
		fData.around_y = false;

		fData.vectorX = 0.0f;
		fData.vectorY = 1.0f;
		fData.vectorZ = 0.0f;
	}	

	// Rotate around Z

	if (fData.around_z) {
		fData.around_z = false;

		fData.vectorX = 0.0f;
		fData.vectorY = 0.0f;
		fData.vectorZ = 1.0f;
	}	

	// Reset apply flags

	checkTree = false;
	checkTime = -1.0;

	// All done...

	return MC_S_OK;
}

// QueryInterface so it can identify itself

MCCOMErr Cognito::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_I3DExModifier))
	{
		TMCCountedGetHelper<I3DExModifier> result(ppvObj);
		result = (I3DExModifier*)this;
		return MC_S_OK;
	}

	if (MCIsEqualIID(riid, IID_Cognito))
	{
		TMCCountedGetHelper<Cognito> result(ppvObj);
		result = (Cognito*)this;
		return MC_S_OK;
	}

	return TBasicDataExchanger::QueryInterface(riid, ppvObj);
}


// CognitoData Methods

void* CognitoData::GetExtensionDataBuffer() {

	return &fData;
}

// Constructor / Destructor of the C++ Object :
CognitoData::CognitoData() {

	fData.rotation	= 0.0f;
	fData.pitch	= 0.0f;
	fData.offset 	= 0.0f;
	fData.linkage	= ACT_LINK_GEAR;
	fData.teeth	= 32;
	fData.motor	= false;
	fData.slave	= false;
	fData.driver;

	SaveData();

	depth = 0;

	return;
}

void CognitoData::SaveData() {

	fSave.rotation = fData.rotation;
	fSave.pitch    = fData.pitch;
	fSave.offset   = fData.offset;
	fSave.linkage  = fData.linkage;
	fSave.teeth    = fData.teeth;
	fSave.motor    = fData.motor;
	fSave.slave    = fData.slave;
	fSave.driver   = fData.driver;

	return;
}	


// React to changes

MCCOMErr CognitoData::ExtensionDataChanged() {

	if (fData.motor & !fSave.motor) {
		fData.slave= false;
	}

	if (fData.slave & !fSave.slave) {
		fData.motor = false;
	}

	SaveData();

	return MC_S_OK;
}


// QueryInterface so it can identify itself

MCCOMErr CognitoData::QueryInterface(const MCIID& riid, void** ppvObj)
{
	if (MCIsEqualIID(riid, IID_I3DExDataComponent))
	{
		TMCCountedGetHelper<I3DExDataComponent> result(ppvObj);
		result = (I3DExDataComponent*)this;
		return MC_S_OK;
	}

	if (MCIsEqualIID(riid, IID_CognitoData))
	{
		TMCCountedGetHelper<CognitoData> result(ppvObj);
		result = (CognitoData*)this;
		return MC_S_OK;
	}

	if (MCIsEqualIID(riid, IID_CognitoDataExt))
	{
		TMCCountedGetHelper<CognitoData> result(ppvObj);
		result = (CognitoData*)this;
		return MC_S_OK;
	}

	return TBasicDataExchanger::QueryInterface(riid, ppvObj);
}

// External GetRotation

MCCOMErr CognitoData::GetRotation(I3DShScene* scene, double &rotation) {

	TMCCountedPtr<I3DShTreeElement> driver;

	TMCCountedPtr<CognitoData> driverCogData;

	CognitoDataBuffer *driverCogDataBuf;

	// Simple case is the motor...

	if (fData.motor) {
		rotation = fData.rotation;
		return MC_S_OK;
	}	

	// If it's not a slave, then there's no rotation...

	if (!fData.slave) {
		rotation = 0.0;
		return MC_S_OK;
	}	

	// Check the scene

	if (!scene) {
		rotation = 0.0;
		return MC_S_OK;
	}	

	// Search for the driving tree

	scene->GetTreeElementByName(&driver, fData.driver);

	if (!driver) {
		rotation = 0.0;
		return MC_S_OK;
	}

	// Find the drivers data block...

	driverCogData = 0;

	driverCogData = getCognitoData(driver);

	if (!driverCogData) {
		rotation = 0.0;
		return MC_S_OK;
	}	

	// Then it's data...

	driverCogDataBuf = (CognitoDataBuffer *) (driverCogData->GetExtensionDataBuffer());

	if (!driverCogDataBuf) {
		rotation = 0.0;
		return MC_S_OK;
	}	

	// Ward for infinite loops

	depth += 1;

	if (depth > 250) {
		rotation = 0.0;
		return MC_S_OK;
	}	

	// Ok, get the drivers rotation...

	driverCogData->GetRotation(scene, rotation);

	rotation += fData.offset;

	// Ok, we can relax now...

	depth -= 1;

	// Now translate it into this objects rotation

	switch (fData.linkage) {

		// Gears reverse direction and scale by teeth ratio
		// Cross linked chains are just like gears	

		case ACT_LINK_GEAR:
		case ACT_LINK_XCHAIN:

			rotation = -1.0 * (rotation * driverCogDataBuf->teeth) / fData.teeth;

			break;

			// Axels just pass through the rotation	
			// Radial Offsets just pass through the rotation	

		case ACT_LINK_AXEL:
		case ACT_LINK_RAD_OFFSET:

			break;

			// Chains are like gears but do not reverse direction	

		case ACT_LINK_CHAIN:

			rotation = (rotation * driverCogDataBuf->teeth) / fData.teeth;

			break;

			// Racks move left/right based upon their tooth pitch	

		case ACT_LINK_RACK:

			rotation *= driverCogDataBuf->teeth;

			if (fData.pitch != 0.0) {
				rotation /= fData.pitch;
			}	

			break;

			// Worms move in and out based upon the pitch of the worm

		case ACT_LINK_WORM:

			if (fData.pitch != 0.0) {
				rotation /= fData.pitch;
			}	

			break;

			// Cams move up and down, based on a radial offset
			// Rotation is translated to periodic linear motion
			// Item is assumed to be placed initially at the dwell point
			// Resulting motion is a positive offset from 0 to pushrod Length
			// Offset will have the effect of changing the phase of the cams

		case ACT_LINK_CAM:

			rotation -= floor(rotation);

			rotation = sin(rotation * PI);

			break;

			// Pistons also move up and down
			// Rotation is returned as rotation

		case ACT_LINK_PISTON:

			break;

			// Pushrods moves up and down and side to side.
			// Rotation is returned as rotation

		case ACT_LINK_PUSHROD:

			break;

			// Pushrod Pistons moves up and down.
			// Rotation is returned as rotation

		case ACT_LINK_PUSHROD_PISTON:

			break;

			// Rockers and Trip Wire turn and turn back.
			// Rotation is returned modified by teeth

		case ACT_LINK_ROCKER:
		case ACT_LINK_TRIP_HAMMER:

			rotation = (rotation * driverCogDataBuf->teeth) / fData.teeth;

			// Ok, what rotation do we actually see?

			rotation -= floor(rotation);

			if (fData.linkage == ACT_LINK_TRIP_HAMMER) {

				if (rotation < 0.98) {
					rotation = sin((PI/2.0) * rotation);
				} else {
					rotation = sin((PI/2.0) * ((1.00 - rotation) * 49.0));
				}	

				rotation *= fData.pitch;

			} else {

				// Rockers are simpler... 

				rotation = sin(rotation * TWO_PI) * fData.pitch;
			}	

			rotation *= -1.0;

			break;

			// A pushrod cylinder should stop rotation, but we'll let it
			// through unmodified.

		case ACT_LINK_PUSHROD_CYLINDER:

			break;

		default:
			break;
	}		

	// Return 

	return MC_S_OK;
}

