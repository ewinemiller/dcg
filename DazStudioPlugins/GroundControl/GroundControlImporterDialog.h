/*  Ground Control - plug-in for DAZStudio
 Copyright (C) 2015 Eric Winemiller
 
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
#ifndef GROUND_CONTROL_IMPORTER_DLG_H
#define GROUND_CONTROL_IMPORTER_DLG_H


/*****************************
   Include files
*****************************/

#include "dzbasicdialog.h"
#include "GroundControlLib.h"


/****************************
   Forward declarations
****************************/
class QPushButton;

/*****************************
   Class definitions
*****************************/
class GroundControlImporterDialog : public DzBasicDialog {
    Q_OBJECT
public:
    GroundControlImporterDialog( QWidget *parent, const gcMapInfo *mapInfo, int fullPolyCount
		, float sizeX, float sizeZ, float meshDensity, float adaptiveError, int meshType
		, float minElevation, float maxElevation);

    virtual ~GroundControlImporterDialog(){};

	const float getAdaptiveError();
	const float getMeshDensity();
	const int getMeshType();
	const int getScale();
	const float getSizeX();
	const float getSizeZ();
	const float getMinElevation();
	const float getMaxElevation();

};

#endif // GROUND_CONTROL_IMPORTER_DLG_H
