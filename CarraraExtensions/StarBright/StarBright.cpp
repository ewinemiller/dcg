/*  StarBright - plug-in for Carrara
    Copyright (C) 2000 Michael Clarke

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
#include "stdlib.h"
#include "math.h"
#include "StarBright.h"
#include "genrand.h"

// DLL Methods

#if VERSIONNUMBER >= 0x050000
const MCGUID CLSID_StarBright(R_CLSID_StarBright);
#else
const MCGUID CLSID_StarBright = {R_CLSID_StarBright};
#endif

void Extension3DInit(IMCUnknown* utilities) {
	
}

void Extension3DCleanup() {
	
}

TBasicUnknown* MakeCOMObject(const MCCLSID& classId) {	// This method instanciate

	TBasicUnknown* res = NULL;
	
	//check serial if you haven't already when creating the key extensions in your project

	if (classId == CLSID_StarBright) {
		
		res = new StarBright; 
		
	}

	return res;
}

// Class Star

void Star::initOctaves() {

	octaves[0] = 0;
	octaves[1] = 0;
	octaves[2] = 0;
	octaves[3] = 0;
	octaves[4] = 0;
	octaves[5] = 0;
	octaves[6] = 0;
	octaves[7] = 0;

	refCount = 0;
	
	return;
}	

Star::Star() {

	my_x = 0.0;
	my_y = 0.0;
	my_z = 0.0;

	my_range = 0.0;

	my_red	 = 0.0;
	my_green = 0.0;
	my_blue	 = 0.0;

	initOctaves();
	
	return;
}	

Star::Star(double x, double y, double z) {

	my_x = x;
	my_y = y;
	my_z = z;

	my_range = 0.0;

	my_red	 = 0.0;
	my_green = 0.0;
	my_blue	 = 0.0;

	initOctaves();
	
	return;
}	

Star::Star(double x, double y, double z, double range, double red, double green, double blue) {

	my_x = x;
	my_y = y;
	my_z = z;

	my_range = range;

	my_red	 = red;
	my_green = green;
	my_blue	 = blue;

	initOctaves();
	
	return;
}	

Star::~Star() {

	int i, j;

	// Lose any subindexed stars...
	
	for(i = 0; i < 8; i++) {

		if (octaves[i] != 0) {
			for (j = i+1; j < 8; j++) {
				if (octaves[j] == octaves[i]) {
					octaves[j]->deRef();
					octaves[j] = 0;
				}
			}	
			octaves[i]->deRef();
			octaves[i] = 0;
		}
	}	
	
	return;
}

void Star::addRef() {

	refCount++;

	return;
}	

void Star::deRef() {

	refCount--;

	if (refCount == 0) {
		delete this;
	}	

	return;
}	

// Index another star

void Star::addToOctave(Star *new_star, int octave) {

	if (octaves[octave] == 0) {
		octaves[octave] = new_star;
		new_star->addRef();
	} else {
		octaves[octave]->add(new_star);
	}	
	
	return;
}	

void Star::add(Star *new_star) {

	double nxp, nyp, nzp, nxm, nym, nzm;
	
	// Sanity check
	
	if (new_star == 0) {
		return;
	}	

	if (new_star == this) {
		return;
	}	
	
	// Left upper front
	
	nxp = new_star->my_x + new_star->my_range;
	nyp = new_star->my_y + new_star->my_range;
	nzp = new_star->my_z + new_star->my_range;
	
	// Right lower back
	
	nxm = new_star->my_x - new_star->my_range;
	nym = new_star->my_y - new_star->my_range;
	nzm = new_star->my_z - new_star->my_range;
	
	// Note that else is not used in the following as it is possible for the star to be
	// indexed into multiple octaves.  The stars 'volume' is from (nxp, nyp, nzp) to
	// (nxm, nym, nzm).  The star should end up indexed in all octaves that it has at
	// least one corner in.
	//
	// We ignore corners on boundries as the contribution would be negligible.
	//
	
	if ( nxp > my_x ) {
		
		if ( nyp > my_y ) {
			if ( nzp > my_z ) {
				addToOctave(new_star, OCT_XPYPZP);
			}	
	
			if ( nzm < my_z ) {
				addToOctave(new_star, OCT_XPYPZN);
			}
		}	
	
		if ( nym < my_y ) {
			
			if ( nzp > my_z ) {
				addToOctave(new_star, OCT_XPYNZP);
			}	
		
			if ( nzm < my_z ) {
				addToOctave(new_star, OCT_XPYNZN);
			}
		}	
	}

	if (nxm < my_x) {
		
		if (nyp > my_y) {
			
			if (nzp > my_z ) {
				addToOctave(new_star, OCT_XNYPZP);
			}	
	
			if (nzm < my_z ) {
				addToOctave(new_star, OCT_XNYPZN);
			}
		}	
	
		if (nym < my_y) {
			
			if (nzp > my_z ) {
				addToOctave(new_star, OCT_XNYNZP);
			}	
	
			if (nzm < my_z ) {
				addToOctave(new_star, OCT_XNYNZN);
			}
		}	
	}	
	
	return;
}	

// Work out which stars a ray hits...

void Star::getColour(TMCColorRGB& colour, double px, double py, double pz) {

	int octave;
	
	// What do I do to its colour?
	
	if (my_range > 0) {
		getMyContribution(colour, px, py, pz);
	}	
	
	// Ok, check the other stars

	octave = -1;
	
	if (px > my_x) {
		if (py > my_y) {
			if (pz > my_z) {
				octave = OCT_XPYPZP;
			} else {
				octave = OCT_XPYPZN;
			}
		} else {
			if (pz > my_z) {
				octave = OCT_XPYNZP;
			} else {
				octave = OCT_XPYNZN;
			}
		}	
	} else {
		if (py > my_y) {
			if (pz > my_z) {
				octave = OCT_XNYPZP;
			} else {
				octave = OCT_XNYPZN;
			}
		} else {
			if (pz > my_z) {
				octave = OCT_XNYNZP;
			} else {
				octave = OCT_XNYNZN;
			}
		}	
	}	
	
	// Go a level deeper if there is one... */
	
	if (octave > -1) {
		if (octaves[octave] != 0) {
			octaves[octave]->getColour(colour, px, py, pz);
		}	
	}	
	
	// All done...
	
	return;
}

// What do we get from this star?

void Star::getMyContribution(TMCColorRGB& colour, double px, double py, double pz) {

	double dx, dy, dz;
	
	double dist;

	double cont;
	
	if (my_range <= 0.0) {
		return;
	}	
	
	// Close on X?
	
	dx = fabs(px - my_x);

	if (dx > my_range) {
		return;
	}	
	
	// Close on Y?
	
	dy = fabs(py - my_y);

	if (dy > my_range) {
		return;
	}	
	
	// Close on Z?
	
	dz = fabs(pz - my_z);

	if (dz > my_range) {
		return;
	}	

	// Calculate actual distance...
	
	dist = sqrt((dx * dx) + (dy * dy) + (dz * dz));

	// Is it within range?
	
	if (dist > my_range) {
		return;
	}	
	
	// Calculate base contribution.
	
	cont = 1.0 - (dist/my_range);

	// Add colour in proportion...
	
	colour.R += (float) (my_red * cont);
	colour.G += (float) (my_green * cont);
	colour.B += (float) (my_blue * cont);
	
	// All done
	
	return;
}

// Class StarBright

// Constructor 
StarBright::StarBright() {

	// Data initialisation
	
	fData.seed		= 1;
	fData.numStars		= 1000;

	fData.firmament.R	= 0.0f;
	fData.firmament.G	= 0.0f;
	fData.firmament.B	= 0.1f;
	
	fData.horizon.R		= 0.0f;
	fData.horizon.G		= 0.0f;
	fData.horizon.B		= 0.0f;
	
	fData.magnification	= 1.0f;
	fData.intensity		= 1.0f;
	
	fData.rotation		= 0.0f;
	fData.elevation		= 0.0f;
	
	fData.xPositive		= true;
	fData.yPositive		= true;
	fData.zNegative		= true;
	
	// Prime the cache
	
	saveData();

	// But force an initial significant difference...
	
	fSave.seed = 0;
	
	// Start without any stars...
	
	first_star = 0;
	
	// Initialize style data...
	
	style_range[0] = 0.000;	// Dark
	style_range[1] = 0.002;	// X Small	
	style_range[2] = 0.003;	// Small
	style_range[3] = 0.004;	// Medium
	style_range[4] = 0.005;	// Large
	style_range[5] = 0.006;	// X Large
	
#define MAX_RANGE 0.006	
	
	// Initialize colour map...
	
	fData.colourMap[ 0].red   = 1.00f;		// White
	fData.colourMap[ 0].green = 1.00f;		// White
	fData.colourMap[ 0].blue  = 1.00f;		// White
	
	fData.colourMap[ 1].red   = 1.00f;		// White
	fData.colourMap[ 1].green = 1.00f;		// White
	fData.colourMap[ 1].blue  = 1.00f;		// White
	
	fData.colourMap[ 2].red   = 1.00f;		// White
	fData.colourMap[ 2].green = 1.00f;		// White
	fData.colourMap[ 2].blue  = 1.00f;		// White

	fData.colourMap[ 3].red   = 1.00f;		// White
	fData.colourMap[ 3].green = 1.00f;		// White
	fData.colourMap[ 3].blue  = 1.00f;		// White
	
	fData.colourMap[ 4].red   = 1.00f;		// Yellow
	fData.colourMap[ 4].green = 1.00f;		// Yellow
	fData.colourMap[ 4].blue  = 0.90f;		// Yellow

	fData.colourMap[ 5].red   = 1.00f;		// Yellow
	fData.colourMap[ 5].green = 1.00f;		// Yellow
	fData.colourMap[ 5].blue  = 0.90f;		// Yellow

	fData.colourMap[ 6].red   = 1.00f;		// Yellow
	fData.colourMap[ 6].green = 1.00f;		// Yellow
	fData.colourMap[ 6].blue  = 0.90f;		// Yellow

	fData.colourMap[ 7].red   = 0.90f;		// Blue
	fData.colourMap[ 7].green = 0.90f;		// Blue
	fData.colourMap[ 7].blue  = 1.00f;		// Blue
	
	fData.colourMap[ 8].red   = 0.90f;		// Blue
	fData.colourMap[ 8].green = 0.90f;		// Blue
	fData.colourMap[ 8].blue  = 1.00f;		// Blue
	
	fData.colourMap[ 9].red   = 0.90f;		// Blue
	fData.colourMap[ 9].green = 0.90f;		// Blue
	fData.colourMap[ 9].blue  = 1.00f;		// Blue
	
	fData.colourMap[10].red   = 1.00f;		// Red
	fData.colourMap[10].green = 0.90f;		// Red
	fData.colourMap[10].blue  = 0.90f;		// Red
	
	fData.colourMap[11].red   = 0.90f;		// Green
	fData.colourMap[11].green = 1.00f;		// Green
	fData.colourMap[11].blue  = 0.90f;		// Green
	
	// Calculate initial stars
	
	ExtensionDataChanged();
	
	return;
}

StarBright::~StarBright() {

	if (first_star != 0) {
		delete first_star;
		first_star = 0;
	}	
	
	return;
}	

// IExDataExchanger methods :
  
void* StarBright::GetExtensionDataBuffer() {
	
	return &fData; // used by the shell to set the new parameters
}

// Keep a copy of our data so we can tell what's changed

void StarBright::saveData() {

	fSave.seed		= fData.seed;
	fSave.numStars		= fData.numStars;

	fSave.firmament.R	= fData.firmament.R;
	fSave.firmament.G	= fData.firmament.G;
	fSave.firmament.B	= fData.firmament.B;
	
	fSave.horizon.R		= fData.horizon.R;
	fSave.horizon.G		= fData.horizon.G;
	fSave.horizon.B		= fData.horizon.B;
	
	fSave.magnification	= fData.magnification;
	fSave.intensity		= fData.intensity;
	
	fSave.rotation		= fData.rotation;
	fSave.elevation		= fData.elevation;
	
	fSave.xPositive		= fData.xPositive;
	fSave.yPositive		= fData.yPositive;
	fSave.zNegative		= fData.zNegative;

	fSave.colourMap[0]	= fData.colourMap[0];
	fSave.colourMap[1]	= fData.colourMap[1];
	fSave.colourMap[2]	= fData.colourMap[2];
	fSave.colourMap[3]	= fData.colourMap[3];
	fSave.colourMap[4]	= fData.colourMap[4];
	fSave.colourMap[5]	= fData.colourMap[5];
	fSave.colourMap[6]	= fData.colourMap[6];
	fSave.colourMap[7]	= fData.colourMap[7];
	fSave.colourMap[8]	= fData.colourMap[8];
	fSave.colourMap[9]	= fData.colourMap[9];
	fSave.colourMap[10]	= fData.colourMap[10];
	fSave.colourMap[11]	= fData.colourMap[11];
	
	return;
}

// Work out if there has been a change which means we need to
// regenerate the star structure.

bool StarBright::dataChanged() {

	if (fSave.seed != fData.seed) {
		return true;
	}	
	
	if (fSave.numStars != fData.numStars) {
		return true;
	}	
	
	if (fSave.magnification != fData.magnification) {
		return true;
	}	
	
	if (fSave.intensity != fData.intensity) {
		return true;
	}	
	
	if (fSave.xPositive != fData.xPositive) {
		return true;
	}	
	
	if (fSave.yPositive != fData.yPositive) {
		return true;
	}	
	
	if (fSave.zNegative != fData.zNegative) {
		return true;
	}	
	
	if (fSave.colourMap[0]  != fData.colourMap[0]) {
		return true;
	}
	
	if (fSave.colourMap[1]  != fData.colourMap[1]) {
		return true;
	}
	
	if (fSave.colourMap[2]  != fData.colourMap[2]) {
		return true;
	}
	
	if (fSave.colourMap[3]  != fData.colourMap[3]) {
		return true;
	}
	
	if (fSave.colourMap[4]  != fData.colourMap[4]) {
		return true;
	}
	
	if (fSave.colourMap[5]  != fData.colourMap[5]) {
		return true;
	}
	
	if (fSave.colourMap[6]  != fData.colourMap[6]) {
		return true;
	}
	
	if (fSave.colourMap[7]  != fData.colourMap[7]) {
		return true;
	}
	
	if (fSave.colourMap[8]  != fData.colourMap[8]) {
		return true;
	}
	
	if (fSave.colourMap[9]  != fData.colourMap[9]) {
		return true;
	}
	
	if (fSave.colourMap[10]  != fData.colourMap[10]) {
		return true;
	}
	
	if (fSave.colourMap[11]  != fData.colourMap[11]) {
		return true;
	}
	
	return false;
}	

void StarBright::generateStars() {
	
	double dx, dy, dz;

	double mag;

	double range;

	int colour;
	
	int i;

	mtRand *myRand;
	
	// Initialize the PRN

	myRand = new mtRand();
	
	myRand->init_genrand(fData.seed);
	
	// Lose all the old stars... 

	if (first_star != 0) {
		delete first_star;
		first_star = 0;
	}	

	// Build up a primary divisioning with dark stars.
	// 
	// These provide a guarenteed good first two levels of subdivision
	// in the search space.  These 8 stars reduce the potential
	// search space for each other star by 1/64 (98.4%)
	
	// Add the core star first
	
	first_star = new Star(0.0,0.0,0.0);

	// All +ve
	
	first_star->add(new Star(-0.5, -0.5, 0.5));
	
	// -ve X, -ve X, Y, -ve X, Y, Z, -ve X, Z
	
	if (fData.xPositive) {
		first_star->add(new Star(0.5, -0.5, 0.5));

		if (fData.yPositive) {
			first_star->add(new Star(0.5, 0.5, 0.5));

			if (fData.zNegative) {
				first_star->add(new Star(0.5, 0.5, -0.5));
			}	
		}
		
		if (fData.zNegative) {
			first_star->add(new Star(0.5, -0.5, -0.5));
		}	
	}	

	// -ve Y, -ve Y,Z

	if (fData.yPositive) {
		first_star->add(new Star(-0.5, 0.5, 0.5));
		
		if (fData.zNegative) {
			first_star->add(new Star(-0.5, 0.5, -0.5));
		}	
	}
	
	// -ve Z
	
	if (fData.zNegative) {
		first_star->add(new Star(-0.5, -0.5, -0.5));
	}	

	// Generate the requested stars
	
#define rand1	(((double) myRand->genrand_int32())/((double) GENRAND_MAX))

#define STAR_STYLE	(myRand->genrand_int32() % (NUM_STYLES - 1)) + 1
	
#define STAR_COLOUR	(myRand->genrand_int32() % NUM_COLOURS)  

	for(i = 0; i < fData.numStars; i++) {

		// Generate X, Y, Z coords
		
		dx = -1.0 * rand1;

		if (fData.xPositive) {
			dx = (dx + 0.5) * 2.0;
		}
	
		dy = -1.0 * rand1;

		if (fData.yPositive) {
			dy = (dy + 0.5) * 2.0;
		}
	
		dz = rand1;
		
		if (fData.zNegative) {
			dz = (dz - 0.5) * 2.0;
		}
	
		// Generate adjusted radius
		
		range = style_range[STAR_STYLE] * fData.magnification;

		if (fData.intensity < 1.0) {
			range -= MAX_RANGE * fData.magnification * (1.0 - fData.intensity);
		}		
		
		// Generate colour
		
		colour = STAR_COLOUR;
		
		// Calculate magnitude and normalize X, Y and Z
		
		mag = sqrt((dx * dx) + (dy * dy) + (dz * dz));

		if (mag > 0.0 && range > 0.0) {
			
			dx /= mag;
			dy /= mag;
			dz /= mag;
			
			first_star->add(new Star(	dx, dy, dz,
							range,
							fData.colourMap[colour].red,
							fData.colourMap[colour].green,
							fData.colourMap[colour].blue	));
		}	
	
	}	

#undef rand1
#undef STAR_STYLE
#undef STAR_COLOUR

	delete myRand;
	
	return;
}

// Called when data is changed through the API
// Also called at the begining of each frame when there
// is interpolated or keyframed data changes

MCCOMErr StarBright::ExtensionDataChanged() {

	double maxMag;
	
	// Precalculate for roatation and elevation
	
	sinRot = sin((fData.rotation * TWO_PI)/360);
	cosRot = cos((fData.rotation * TWO_PI)/360);
	
	sinElv = sin((fData.elevation * TWO_PI)/360);
	cosElv = cos((fData.elevation * TWO_PI)/360);

	// Enforce maximum magnification limit
	
	maxMag = 12000.0/fData.numStars;
	
	if (fData.magnification > maxMag) {
		fData.magnification = maxMag;
	}	
	
	// Only regnerate stars if significant data has changed
	
	// if (dataChanged()) {
	//	generateStars();
	// }

	// Remember current settings for stars
	
	// saveData();

	// All done
	
	return MC_S_OK;
}

//Environment background function :
MCCOMErr StarBright::GetBackgroundColor(	TMCColorRGB& color,
						boolean& fullAreaDone,
						const TVector3& direction) {
	fullAreaDone = true;

	double px, py, pz, zd, zd2, zr;
	
	float modz;

	modz = fabs(direction[2]);
		
	color.R   = fData.firmament.R * modz + fData.horizon.R * (1 - modz);
	color.G   = fData.firmament.G * modz + fData.horizon.G * (1 - modz);
	color.B   = fData.firmament.B * modz + fData.horizon.B * (1 - modz);
	
	// Do we have any stars yet?
	
	if (dataChanged()) {

		// Remember current data
		
		saveData();
		
		// Go make some stars...
		
		generateStars();

		// We got a problem?
		
		if (first_star == 0) {
			return MC_S_OK;
		}	
	}
	
	if (fData.rotation != 0.0) {
		px = cosRot*direction[0] - sinRot*direction[1];
		py = sinRot*direction[0] + cosRot*direction[1];
	} else {
		px = direction[0];
		py = direction[1];
	}	
	
	if (fData.elevation != 0.0) {
		zd = sqrt(px * px + py * py);

		pz = cosElv*direction[2] - sinElv*zd;

		// Calc new diagonal length for pro-rata adjustment
		
		zd2 = sinElv*direction[2] + cosElv*zd;

		if (zd != 0) {
			zr = zd2/zd;

			px *= zr;
			py *= zr;
		}	
		
	} else {
		pz = direction[2];
	}	
	
	first_star->getColour( color, px, py, pz);

	return MC_S_OK;
}
 

