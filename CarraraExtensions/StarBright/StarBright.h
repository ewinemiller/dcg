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
#ifndef __StarBright__
#define __StarBright__  

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include	"BasicEnvironment.h"
#include	"APITypes.h"
#include	"MCColorRGBA.h"
#include	"StarBrightDef.h"

// Define the StarBright CLSID ( see the BackDef.h file to get the R_CLSID_Back value )

extern const MCGUID CLSID_StarBright;
                  
class Star {
	
public:
	// Constructors
	
	Star();
	Star(double x, double y, double z);
	Star(double x, double y, double z, double range, double red, double green, double blue);

	// Destructor
	
	~Star();

	// Add a new star
	
	void add(Star *new_star);
	
	void getColour(TMCColorRGB& colour, double px, double py, double pz);

	void addRef();

	void deRef();
	
private:
	void initOctaves();
	
	void addToOctave(Star *new_star, int octave);

	void getMyContribution(TMCColorRGB& colour, double px, double py, double pz);

	Star	*octaves[8];

#define	OCT_XPYPZP	0	
#define	OCT_XPYPZN	1	
#define	OCT_XPYNZP	2	
#define	OCT_XPYNZN	3	
#define	OCT_XNYPZP	4	
#define	OCT_XNYPZN	5	
#define	OCT_XNYNZP	6	
#define	OCT_XNYNZN	7	
	
	double my_x;
	double my_y;
	double my_z;

	double my_range;

	double my_red, my_green, my_blue;

	int refCount;
};	
 
#define NUM_COLOURS	12

struct StarBrightData {
	
	int32		seed;
	int32		numStars;
	
	TMCColorRGBA	firmament;
	TMCColorRGBA	horizon;
	
	TMCColorRGBA	colourMap[NUM_COLOURS];
	
	real32		magnification;
	real32		intensity;
	
	real32		rotation;
	real32		elevation;
	
	boolean		xPositive;
	boolean		yPositive;
	boolean		zNegative;
};

class StarBright : public TBasicBackground
{
	public :  
		StarBright();
		~StarBright();
		
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(StarBrightData); }
    
		// IExDataExchanger methods :
		virtual void*		MCCOMAPI     GetExtensionDataBuffer();
		virtual MCCOMErr	MCCOMAPI     ExtensionDataChanged();
		// virtual int16		MCCOMAPI     GetResID();
  
		// I3DExBackgroundShader methods
		virtual MCCOMErr	MCCOMAPI     GetBackgroundColor(	TMCColorRGB& color,
										boolean& fullAreaDone,
										const TVector3& direction);
  
	private :
		StarBrightData	fData;			// StarBright Data

		StarBrightData	fSave;			// StarBright Save

		Star	*first_star;

#define	NUM_STYLES	6

		double style_range[NUM_STYLES];

#define STYLE_DARK	0
#define STYLE_XSMALL	1
#define STYLE_SMALL	2
#define STYLE_MEDIUM	3
#define STYLE_LARGE	4
#define STYLE_XLARGE	5

		double sinRot, cosRot, sinElv, cosElv;

		void saveData();

		bool dataChanged();

		void generateStars();
};
                          

#endif

