/*  Ground Control Library - Elevation map tools
    Copyright (C) 2003 Eric Winemiller

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
void gcNormalize(TVector2d *vec) {
	double magnitude = sqrt(vec->east * vec->east + vec->north * vec->north);
	if (magnitude > 0) {
		magnitude = 1.0 / magnitude;
		vec->east *= magnitude;
		vec->north *= magnitude;
	}
	else { 
		vec->east = 0;
		vec->north = 1;
	}
}

TVector2d gcLineLineIntersection(TVector2d *originD, TVector2d *directionD, TVector2d *originE, TVector2d *directionE) {
	double z, t;
	TVector2d SR;

	gcNormalize(directionD);
	gcNormalize(directionE);

	z = directionD->east * directionE->north - directionD->north * directionE->east;

	SR.east = originD->east - originE->east;
	SR.north = originD->north - originE->north;

	t = (SR.east * directionE->north - SR.north * directionE->east) / z;

	SR.east = originD->east - t * directionD->east;
	SR.north = originD->north - t * directionD->north;

	return SR;
}

TVector2d getCircumcircleCenter(const gcPoint *pt0, const gcPoint *pt1, const gcPoint *pt2)
{

	TVector2d AB, AC, halfAB, halfAC, perpAB, perpAC;

	AB.east = pt1->east - pt0->east;
	AB.north = pt1->north - pt0->north;

	AC.east = pt2->east - pt0->east;
	AC.north = pt2->north - pt0->north;

	halfAB.east = pt0->east + AB.east * 0.5;
	halfAB.north = pt0->north + AB.north * 0.5;

	halfAC.east = pt0->east + AC.east * 0.5;
	halfAC.north = pt0->north + AC.north * 0.5;

	perpAB.east = AB.north;
	perpAB.north = -AB.east;

	perpAC.east = AC.north;
	perpAC.north = -AC.east;

	return gcLineLineIntersection(&halfAB, &perpAB, &halfAC, &perpAC);
}

double gcGetValueFromFarAlong(const long minRange, const long maxRange, const long curValue, const long minCalc, const long maxCalc)
{
	double farAlong = (double)(curValue - minRange)
			/  (double)(maxRange - minRange);

	return  (double)(minCalc) + farAlong * (double)(maxCalc - minCalc);
}

void gcFillPlaneEquation(const gcPoint* pt1, const gcPoint* pt2, const gcPoint* pt3, TVector4d *planeEquation)
{
	TVector3d v1 , v2;
	double magnitude;
	
	v1.x = pt2->east - pt1->east;
	v1.y = pt2->north - pt1->north;
	v1.z = pt2->elevation - pt1->elevation;
	v2.x = pt3->east - pt1->east;
	v2.y = pt3->north - pt1->north; 
	v2.z = pt3->elevation - pt1->elevation;

	planeEquation->x = (double)(v1.y) * (double)(v2.z) - (double)(v1.z) * (double)(v2.y);
    planeEquation->y = (double)(v1.z) * (double)(v2.x) - (double)(v1.x) * (double)(v2.z);
    planeEquation->z = (double)(v1.x) * (double)(v2.y) - (double)(v1.y) * (double)(v2.x);

	magnitude = sqrt(planeEquation->x * planeEquation->x + planeEquation->y * planeEquation->y + planeEquation->z * planeEquation->z);
	if (magnitude > 0) {
		magnitude = 1.0 / magnitude;
		planeEquation->x *= magnitude;
		planeEquation->y *= magnitude;
		planeEquation->z *= magnitude;
	}
	else { 
		planeEquation->x = planeEquation->y = 0;
		planeEquation->z = 1;
	}

	planeEquation->w = -((double)(pt1->east) * planeEquation->x + (double)(pt1->north) * planeEquation->y + (double)(pt1->elevation) * planeEquation->z); 
}

double gcGetZFromPlaneEquation(const TVector4d *planeEquation, const long east, const long north)
{
	return - (planeEquation->w + planeEquation->x * (double)(east) + planeEquation->y * (double)(north)) / planeEquation->z;
}

double sqrd(const double val) {
	return val * val;
}

long sqr(long val) {
	return val * val;
}

double sqrTVector2d(const TVector2d *vec1, const TVector2d *vec2) {
	return sqrd(vec1->east - vec2->east) + sqrd(vec1->north - vec2->north);
}

double getDistanceFromPointToLine(const gcPoint *p1, const gcPoint *p2, unsigned long east, unsigned long north)
{
	double distance = fabs((double)((p2->east - p1->east) * (p1->north - north) - (p1->east - east) * (p2->north - p1->north)));

	distance /= sqrt((double)(sqr(p2->east - p1->east) + sqr(p2->north - p1->north)));

	return distance;
}