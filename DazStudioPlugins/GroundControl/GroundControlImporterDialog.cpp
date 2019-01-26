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
/*****************************
   Include files
*****************************/
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QGroupBox>
#include <QtGui/QPushButton>
#include <QtGui/QToolTip>
#include <QtGui/QWhatsThis>
#include <QtGui/QComboBox>
#include <QtGui/QLineEdit>
#include <QtGui/QGridLayout>
#include <cfloat>

#include "dzapp.h"
#include "dzstyle.h"
#include "dzmainwindow.h"
#include "dzactionmgr.h"
#include "dzaction.h"
#include "DzFloatSlider.h"

#include "GroundControlImporterDialog.h"

#define GROUND_CONTROL_NAME     "Ground Control"
#define MESH_TYPE_NAME "%1MeshType"
#define ADAPTIVE_ERROR_NAME "%1AdaptiveError"
#define MESH_DENSITY_NAME "%1MeshDensity"
#define SCALE_NAME "%1Scale"
#define SIZE_X_NAME "%1SizeX"
#define SIZE_Z_NAME "%1SizeZ"
#define MIN_ELEVATION_NAME "%1MaxElevation"
#define MAX_ELEVATION_NAME "%1MinElevation"

/*****************************
   Local definitions
*****************************/

GroundControlImporterDialog::GroundControlImporterDialog( QWidget *parent, const gcMapInfo *mapInfo, int fullPolyCount
	, float sizeX, float sizeZ, float meshDensity, float adaptiveError, int meshType
	, float minElevation, float maxElevation) :
    DzBasicDialog( parent, GROUND_CONTROL_NAME )
{
    // Declarations
    int margin = style()->pixelMetric( DZ_PM_GeneralMargin );
    int wgtHeight = style()->pixelMetric( DZ_PM_ButtonHeight );
    int btnMinWidth = style()->pixelMetric( DZ_PM_ButtonMinWidth );

    // Set the dialog title
    setWindowTitle( tr("Ground Control Import Options") );

	QLabel *staticLabel;

	if (mapInfo->hasRealSize) {
		staticLabel = new QLabel( this );
		staticLabel->setText(QString("This elevation map has real world sizes."));
		addWidget( staticLabel );
		staticLabel = new QLabel( this );
		staticLabel->setText(QString("Scale will be applied to all sizes."));
		addWidget( staticLabel );
	}
	else if (mapInfo->hasRealElevation) {
		staticLabel = new QLabel( this );
		staticLabel->setText(QString("This elevation map has real world elevation."));
		addWidget( staticLabel );
		staticLabel = new QLabel( this );
		staticLabel->setText(QString("Vertical scale will be applied to the elevation."));
		addWidget( staticLabel );
	}
	else {
		staticLabel = new QLabel( this );
		staticLabel->setText(QString("This elevation map has no real world sizes."));
		addWidget( staticLabel );
	}
	QGridLayout *sizeGrid = new QGridLayout( this );

	staticLabel = new QLabel( this );
	staticLabel->setText(QString("Size X: "));
	sizeGrid->addWidget(staticLabel, 1, 1);

	QLineEdit *sizeXControl = new QLineEdit( this );
	sizeXControl->setObjectName(QString(SIZE_X_NAME).arg( GROUND_CONTROL_NAME ) );
	sizeXControl->setText(QString::number(sizeX, 'f', 2));
	sizeXControl->setValidator(new QDoubleValidator(0, FLT_MAX, 2, this) );
	sizeGrid->addWidget( sizeXControl, 1, 2);		
			
	staticLabel = new QLabel( this );
	staticLabel->setText(QString("Meters"));
	sizeGrid->addWidget(staticLabel, 1, 3);

	staticLabel = new QLabel( this );
	staticLabel->setText(QString("Size Z: "));
	sizeGrid->addWidget(staticLabel, 2, 1);

	QLineEdit *sizeZControl = new QLineEdit( this );
	sizeZControl->setObjectName(QString(SIZE_Z_NAME).arg( GROUND_CONTROL_NAME ) );
	sizeZControl->setText(QString::number(sizeZ, 'f', 2));
	sizeZControl->setValidator(new QDoubleValidator(0, FLT_MAX, 2, this) );
	sizeGrid->addWidget( sizeZControl, 2, 2);		

	staticLabel = new QLabel( this );
	staticLabel->setText(QString("Minimum Elevation: "));
	sizeGrid->addWidget(staticLabel, 3, 1);

	QLineEdit *minElevationControl = new QLineEdit( this );
	minElevationControl->setObjectName(QString(MIN_ELEVATION_NAME).arg( GROUND_CONTROL_NAME ) );
	minElevationControl->setText(QString::number(minElevation, 'f', 2));
	minElevationControl->setValidator(new QDoubleValidator(0, FLT_MAX, 2, this) );
	sizeGrid->addWidget( minElevationControl, 3, 2);		

	staticLabel = new QLabel( this );
	staticLabel->setText(QString("Maximum Elevation: "));
	sizeGrid->addWidget(staticLabel, 4, 1);

	QLineEdit *maxElevationControl = new QLineEdit( this );
	maxElevationControl->setObjectName(QString(MAX_ELEVATION_NAME).arg( GROUND_CONTROL_NAME ) );
	maxElevationControl->setText(QString::number(maxElevation, 'f', 2));
	maxElevationControl->setValidator(new QDoubleValidator(0, FLT_MAX, 2, this) );
	sizeGrid->addWidget( maxElevationControl, 4, 2);		

	addLayout( sizeGrid );

	if (mapInfo->hasRealSize || mapInfo->hasRealElevation) {
	
		staticLabel = new QLabel( tr("Scale"), this );
		staticLabel->setFixedHeight( wgtHeight );
		staticLabel->setMinimumWidth( btnMinWidth );
		addWidget( staticLabel );

		QComboBox *scaleControl = new QComboBox(this);
		scaleControl->setObjectName(QString(SCALE_NAME).arg( GROUND_CONTROL_NAME ) );
 		scaleControl->addItem("Fixed", 0);
		scaleControl->addItem("1/10th", 1);
		scaleControl->addItem("1/100th", 2);
		scaleControl->addItem("1/1000th", 3);
		scaleControl->addItem("1/10000th", 4);
		scaleControl->setCurrentIndex(0);
		addWidget(scaleControl);
	
	}

	staticLabel = new QLabel( tr("Mesh type"), this );
    staticLabel->setFixedHeight( wgtHeight );
    staticLabel->setMinimumWidth( btnMinWidth );
    addWidget( staticLabel );

	QComboBox *meshTypeControl = new QComboBox(this);
	meshTypeControl->setObjectName(QString(MESH_TYPE_NAME).arg( GROUND_CONTROL_NAME ) );
 	meshTypeControl->addItem("Grid - point sampling", 0);
	meshTypeControl->addItem("Grid - bilinear sampling", 1);
	meshTypeControl->addItem("Grid - BS and smart split", 2);
	meshTypeControl->addItem("Adaptive", 3);
	meshTypeControl->setCurrentIndex(meshType);
	addWidget(meshTypeControl);

	DzFloatSlider *adaptiveErrorControl = new DzFloatSlider(this);
	adaptiveErrorControl->setObjectName(QString(ADAPTIVE_ERROR_NAME).arg( GROUND_CONTROL_NAME ) );
	adaptiveErrorControl->setValue(adaptiveError);
	adaptiveErrorControl->setMinMax(.001f, 1.0f);
	adaptiveErrorControl->setSensitivity(.01f);
	adaptiveErrorControl->setDisplayAsPercent(true);
	adaptiveErrorControl->setClamped(true);
	adaptiveErrorControl->setTextEditable(true);
	adaptiveErrorControl->setLabel("Max error for Adaptive");
    adaptiveErrorControl->setFixedHeight( wgtHeight );
	adaptiveErrorControl->setShowLabel(true);
	addWidget( adaptiveErrorControl);

	DzFloatSlider *meshDensityControl = new DzFloatSlider(this);
	meshDensityControl->setObjectName(QString(MESH_DENSITY_NAME).arg( GROUND_CONTROL_NAME ) );
	meshDensityControl->setValue(meshDensity);
	meshDensityControl->setMinMax(.001f, 1.0f);
	meshDensityControl->setSensitivity(.01f);
	meshDensityControl->setDisplayAsPercent(true);
	meshDensityControl->setClamped(true);
	meshDensityControl->setTextEditable(true);
	meshDensityControl->setLabel("Mesh density");
    meshDensityControl->setFixedHeight( wgtHeight );
	meshDensityControl->setShowLabel(true);
	addWidget( meshDensityControl);
	
    staticLabel = new QLabel( this );
	staticLabel->setText(QString("Polygons in a full size grid mesh: %1").arg(fullPolyCount));
    staticLabel->setFixedHeight( wgtHeight );
    staticLabel->setMinimumWidth( btnMinWidth );
    addWidget( staticLabel );

	// Make the dialog fit its contents, with a minimum width, and lock it down
    resize( QSize( 300, 0 ).expandedTo(minimumSizeHint()) );
    setFixedWidth( width() );
    setFixedHeight( height() );
}

const float GroundControlImporterDialog::getAdaptiveError() {
	DzFloatSlider *adaptiveErrorControl  = findChild<DzFloatSlider*>(QString(ADAPTIVE_ERROR_NAME).arg( GROUND_CONTROL_NAME ));
	return adaptiveErrorControl->getValue();
}

const float GroundControlImporterDialog::getMeshDensity() {
	DzFloatSlider *meshDensityControl  = findChild<DzFloatSlider*>(QString(MESH_DENSITY_NAME).arg( GROUND_CONTROL_NAME ));
	return meshDensityControl->getValue();
}
const int GroundControlImporterDialog::getMeshType() {
	QComboBox *meshTypeControl = findChild<QComboBox*>(QString(MESH_TYPE_NAME).arg( GROUND_CONTROL_NAME ) );
	return meshTypeControl->currentIndex();
}
const int GroundControlImporterDialog::getScale() {
	QComboBox *scaleControl = findChild<QComboBox*>(QString(SCALE_NAME).arg( GROUND_CONTROL_NAME ) );
	if (scaleControl == NULL) {
		return 0;
	}
	else {
		return scaleControl->currentIndex();
	}
}
const float GroundControlImporterDialog::getSizeX() {
	QLineEdit *sizeXControl  = findChild<QLineEdit*>(QString(SIZE_X_NAME).arg( GROUND_CONTROL_NAME ));
	return sizeXControl->text().toFloat();
}

const float GroundControlImporterDialog::getSizeZ() {
	QLineEdit *sizeZControl  = findChild<QLineEdit*>(QString(SIZE_Z_NAME).arg( GROUND_CONTROL_NAME ));
	return sizeZControl->text().toFloat();
}

const float GroundControlImporterDialog::getMinElevation() {
	QLineEdit *minElevationControl  = findChild<QLineEdit*>(QString(MIN_ELEVATION_NAME).arg( GROUND_CONTROL_NAME ));
	return minElevationControl->text().toFloat();
}

const float GroundControlImporterDialog::getMaxElevation() {
	QLineEdit *maxElevationControl  = findChild<QLineEdit*>(QString(MAX_ELEVATION_NAME).arg( GROUND_CONTROL_NAME ));
	return maxElevationControl->text().toFloat();
}
