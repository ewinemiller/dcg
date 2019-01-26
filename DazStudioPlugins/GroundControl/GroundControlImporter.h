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
#ifndef GROUND_CONTROL_IMPORTER
#define GROUND_CONTROL_IMPORTER

#include <dzgeometryimporter.h>

class GroundControlImporter : public 	DzGeometryImporter {
    Q_OBJECT
public:

    GroundControlImporter();
	virtual int getNumExtensions () const;
	virtual QString getDescription () const;
	virtual QString getExtension (int i) const;
	virtual bool recognize (const QString &filename) const;
	virtual void getDefaultOptions( DzFileIOSettings *options ) const;

protected:
	virtual DzError	read( const QString &filename, DzShape **shape,
						const DzFileIOSettings *options );
};

#endif 
