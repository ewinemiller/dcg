/*  Anything Goos - plug-in for LightWave
Copyright (C) 2009 Eric Winemiller

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
#include <lwtypes.h>
#include <math.h>

float dot( LWFVector a, LWFVector b )
{
	return a[ 0 ] * b[ 0 ] + a[ 1 ] * b[ 1 ] + a[ 2 ] * b[ 2 ];
}


void cross( LWFVector a, LWFVector b, LWFVector c )
{
	c[ 0 ] = a[ 1 ] * b[ 2 ] - a[ 2 ] * b[ 1 ];
	c[ 1 ] = a[ 2 ] * b[ 0 ] - a[ 0 ] * b[ 2 ];
	c[ 2 ] = a[ 0 ] * b[ 1 ] - a[ 1 ] * b[ 0 ];
}

float fsqr(float val) {
	return val * val;
}

double dsqr(double val) {
	return val * val;
}

float vsqr(LWFVector a, LWFVector b) {

	return fsqr(a[0] - b[0]) + fsqr(a[1] - b[1]) + fsqr(a[2] - b[2]);
}

double vdfsqr(LWDVector a, LWFVector b) {

	return dsqr(a[0] - b[0]) + dsqr(a[1] - b[1]) + dsqr(a[2] - b[2]);
}

double vddsqr(LWDVector a, LWDVector b) {

	return dsqr(a[0] - b[0]) + dsqr(a[1] - b[1]) + dsqr(a[2] - b[2]);
}

void normalize( LWFVector v )
{
	float r;

	r = ( float ) sqrt( dot( v, v ));
	if ( r > 0 ) {
		v[ 0 ] /= r;
		v[ 1 ] /= r;
		v[ 2 ] /= r;
	}
}
