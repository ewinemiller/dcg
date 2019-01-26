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
#include "GroundControlImporter.h"
#include "GroundControlImporterDialog.h"

#include <dzfacetshape.h>
#include <dzfacetmesh.h>
#include <GroundControlLib.h>
#include <dzdefaultmaterial.h>
#include <dzimagemgr.h>
#include <dzapp.h>
#include <dzvertexmap.h>
#include <dzcontentmgr.h>
#include <DzContentFolder.h>

using namespace Qt;

const int NUMBER_OF_EXTENSIONS = 9;
char *extensions[NUMBER_OF_EXTENSIONS] = {"dem", "r32", "ter", "bil", "hgt", "pgm", "asc", "flt", "img"};

GroundControlImporter::GroundControlImporter() {

};

int GroundControlImporter::getNumExtensions () const{
	return NUMBER_OF_EXTENSIONS;
};

QString GroundControlImporter::getDescription () const{
	return "Ground Control elevation map";
};

QString GroundControlImporter::getExtension (int i) const{
	return extensions[i];
};

bool GroundControlImporter::recognize (const QString &filename) const{
	bool returnValue = false;

	gcMapInfo fullMapInfo;

	gcInitialize(&fullMapInfo);

	if (gcCanOpenFile(filename.toAscii(), &fullMapInfo) == GC_OK)
	{
		returnValue = true;
	}

	gcCleanUp(&fullMapInfo);

	return returnValue;
};

void GroundControlImporter::getDefaultOptions( DzFileIOSettings *options ) const {
};

DzError	GroundControlImporter::read( const QString &filename, DzShape **shape,
									const DzFileIOSettings *options ) {
	float* XLines = NULL;
	float* ZLines = NULL;
	DzError returnValue = DZ_NO_ERROR;
	DzContentMgr*  contentMgr = dzApp->getContentMgr();

	DzContentFolder* folder = contentMgr->getContentDirectory(0);
	folder->addSubFolder("GroundControl");
	QString generateShadersPath = contentMgr->getContentDirectoryPath(0);  
	QString justFilename = getBaseFileName(filename);
	QString shaderPath = generateShadersPath + "/GroundControl/" + justFilename + ".png";
	float sizeZ = 20.0f, sizeX = 20.0f, sizeY = 1.0f, meshDensity = .1f, adaptiveError = .01f;
    int meshType = GC_MESH_GRID_SMART;
    int fullPolyCount, targetPolyCount;
    DzMainWindow *mw = dzApp->getInterface();
    GroundControlImporterDialog *dlg;
    int scale;
    float scaleFactor;
    unsigned long east, north, point, facet;
    DzFacetMesh *vMesh = new DzFacetMesh();
    DzMap *uvList = new DzMap(DzMap::FLOAT2_MAP);
    DzFacetShape *facetShape = NULL;
    DzDefaultMaterial* material = new DzDefaultMaterial();
    DzImageMgr* imageManager = dzApp->getImageMgr();
    QImage* image = NULL;
    DzTexture* texture = NULL;

    

	gcMapInfo fullMapInfo;

	gcInitialize(&fullMapInfo);

	if (gcCanOpenFile(filename.toAscii(), &fullMapInfo)== GC_ERROR) {
		returnValue = DZ_UNABLE_TO_OPEN_FILE_ERROR;
		goto Finish;
	}

	if (gcOpenFile(filename.toAscii(), &fullMapInfo)== GC_ERROR) {
		returnValue = DZ_UNABLE_TO_OPEN_FILE_ERROR;
		goto Finish;
	}	
	
	gcNormalizeAndClipElevation(&fullMapInfo);

	gcFillNoData(&fullMapInfo, GC_NO_DATA_PERIMETER_AVERAGE, 0);

	////////////////////////////////////////////////////////////////////////
	//get user options

	fullPolyCount = gcGetFullPolyCount(&fullMapInfo);
	if (fullPolyCount > 100000000) {
		meshDensity = .001f;
	}
	else if (fullPolyCount > 10000000) {
		meshDensity = .01f;
	}

	if (fullMapInfo.hasRealSize == GC_TRUE) {
		sizeZ = fullMapInfo.size[GC_NORTH];
		sizeX = fullMapInfo.size[GC_EAST];
	}
	else {
		if (fullMapInfo.hasRealElevation == GC_FALSE) {
			fullMapInfo.minElevation = 0;
			fullMapInfo.maxElevation = 1.0f;
		}
		if (fullMapInfo.samples[GC_EAST] > fullMapInfo.samples[GC_NORTH]) {
			sizeZ *= (float)fullMapInfo.samples[GC_NORTH] / (float)fullMapInfo.samples[GC_EAST];
		}
		else if (fullMapInfo.samples[GC_EAST] < fullMapInfo.samples[GC_NORTH]) {
			sizeX *= (float)fullMapInfo.samples[GC_EAST] / (float)fullMapInfo.samples[GC_NORTH];
		}
	}

 
	dlg = new GroundControlImporterDialog( reinterpret_cast<QWidget*>(mw), &fullMapInfo, fullPolyCount, sizeX, sizeZ, meshDensity, adaptiveError, meshType,
		fullMapInfo.minElevation, fullMapInfo.maxElevation);

    if( dlg->exec() != QDialog::Accepted )
    {
		returnValue = DZ_UNABLE_TO_OPEN_FILE_ERROR;
		goto Finish;
	}
	adaptiveError = dlg->getAdaptiveError();
	meshDensity = dlg->getMeshDensity();
	meshType = dlg->getMeshType();

	sizeX = dlg->getSizeX();
	sizeZ = dlg->getSizeZ();

	fullMapInfo.minElevation = dlg->getMinElevation();
	fullMapInfo.maxElevation = dlg->getMaxElevation();

	scale = dlg->getScale();
	scaleFactor = pow(10.0f, scale);

	if (fullMapInfo.hasRealSize) {
		fullMapInfo.minElevation /= scaleFactor;
		fullMapInfo.maxElevation /= scaleFactor;
		sizeX /= scaleFactor;
		sizeZ /= scaleFactor;
	}
	else if (fullMapInfo.hasRealElevation) {
		fullMapInfo.minElevation /= scaleFactor;
		fullMapInfo.maxElevation /= scaleFactor;
	}

	////////////////////////////////////////////////////////////////////////
	//build the mesh
	targetPolyCount = (float)fullPolyCount * meshDensity;

	if (gcCreateMesh(&fullMapInfo, targetPolyCount, meshType, adaptiveError) == GC_ERROR) {
		returnValue = DZ_UNABLE_TO_OPEN_FILE_ERROR;
		goto Finish;
	}

	//DAZ X=West to East, Z=North to South, Y = elevation

	XLines = reinterpret_cast<float*>(calloc( fullMapInfo.mesh->samples[GC_EAST], sizeof( float )));
	if ( !XLines ) goto Finish;

	ZLines = reinterpret_cast<float*>(calloc( fullMapInfo.mesh->samples[GC_NORTH], sizeof( float )));
	if ( !ZLines ) goto Finish;

	for (east = 0; east < fullMapInfo.mesh->samples[GC_EAST]; east++){
		XLines[east] = sizeX * fullMapInfo.mesh->EPercent[east] - sizeX * 0.5f;
	}

	for (north = 0; north < fullMapInfo.mesh->samples[GC_NORTH]; north++){
		ZLines[north] = - sizeZ * fullMapInfo.mesh->NPercent[north] + sizeZ * 0.5f;
	}


	sizeY = fullMapInfo.maxElevation - fullMapInfo.minElevation;
    vMesh->beginEdit(false);
	 
	vMesh->preSizeVertexArray(fullMapInfo.mesh->pointCount);
	uvList->preSize(fullMapInfo.mesh->pointCount);

	for (point = 0; point < fullMapInfo.mesh->pointCount; point++) {
		const DzVec3 vertex(XLines[fullMapInfo.mesh->points[point].east], 
			fullMapInfo.minElevation + fullMapInfo.mesh->points[point].elevation * sizeY,  
			ZLines[fullMapInfo.mesh->points[point].north]);
		const DzPnt2 uv = {fullMapInfo.mesh->EPercent[fullMapInfo.mesh->points[point].east], 
			fullMapInfo.mesh->NPercent[fullMapInfo.mesh->points[point].north]};
		vMesh->addVertex(vertex);
		uvList->appendPnt2Value(uv);
	}
	vMesh->setUVList(uvList);

	vMesh->preSizeFacets(fullMapInfo.mesh->facetCount);
	for (facet = 0; facet < fullMapInfo.mesh->facetCount; facet++) {

		const int vertIdx[4] = {fullMapInfo.mesh->facets[facet].pointIndex[0],
			fullMapInfo.mesh->facets[facet].pointIndex[1],
			fullMapInfo.mesh->facets[facet].pointIndex[2],
			-1};
		vMesh->addFacet(vertIdx, vertIdx);
	}

	vMesh->finishEdit();  

	//build our shape and assign the mesh
	(*shape)= new DzFacetShape();
	facetShape = reinterpret_cast<DzFacetShape*>(*shape);
	facetShape->setFacetMesh(vMesh);
	(*shape)->setName(fullMapInfo.description);

	//generate a texture out of the elevation map and make that our material
	material->setLabel(fullMapInfo.description);

	image = new QImage(fullMapInfo.samples[GC_EAST], fullMapInfo.samples[GC_NORTH],QImage::Format_RGB888);

	for (east = 0; east < fullMapInfo.samples[GC_EAST]; east++){
		for (north = 0; north < fullMapInfo.samples[GC_NORTH]; north++){
			float value = gcGetElevationByCoordinates(&fullMapInfo, east, north);
			value *= 255.0f;
			image->setPixel(east, fullMapInfo.samples[GC_NORTH] - north - 1, qRgb(value, value, value));
		}
	}


	imageManager->saveImage(shaderPath , *image);
	delete image;
	
	texture = imageManager->getImage(shaderPath);

	material->setColorMap(texture);
	material->setBumpMap(texture);
	material->setBumpStrength(1.0f);
	material->setBumpMax(1.0f);
	material->setBumpMin(0);

	(*shape)->addMaterial(material);

Finish:
	gcCleanUp(&fullMapInfo);
	if (XLines) free (XLines);
	if (ZLines) free (ZLines);
	return returnValue;
};
