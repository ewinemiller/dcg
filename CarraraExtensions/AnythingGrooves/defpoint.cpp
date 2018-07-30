/*  Anything Grooves - plug-in for Carrara
    Copyright (C) 2000 Eric Winemiller

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
#include "defpoint.h"
#include "MCException.h"
#include "copyright.h"
#if (VERSIONNUMBER >= 0x050000)
#include "COMSafeUtilities.h"
#endif

DefPointArray::DefPointArray() {
	data = NULL;
	leftpointsU = NULL;
	rightpointsU = NULL;
	toppointsV = NULL;
	bottompointsV = NULL;
	moldU = 0;
	moldV = 0;
	}

DefPointArray::~DefPointArray() {
	Deallocate();
	}

void DefPointArray::Allocate (const int32 lowU, const int32 highU, const int32 lowV, const int32 highV){
	mlowU = lowU;
	mhighU = highU;
	mlowV = lowV;
	mhighV = highV;
	mU = highU - lowU + 1;
	mV = highV - lowV + 1;
	mUmV = mU * mV;
	if ((mU > moldU)||(mV > moldV)) {
		moldU = mU;
		moldV = mV;
		Deallocate();
		data = (DefPoint*)MCcalloc(mUmV,sizeof(DefPoint));
		leftpointsU = (DefPoint*)MCcalloc(mV,sizeof(DefPoint));
		rightpointsU = (DefPoint*)MCcalloc(mV,sizeof(DefPoint));
		toppointsV = (DefPoint*)MCcalloc(mU,sizeof(DefPoint));
		bottompointsV = (DefPoint*)MCcalloc(mU,sizeof(DefPoint));
		}
	Clear();

	}

void DefPointArray::Clear() {
	for (int32 U = 0; U < moldU; U++) {
		for (int32 V = 0; V < moldV; V++) {
			int32 index = moldV * U + V;
			data[index].bHasData = false;;
			}
		}
	}

void DefPointArray::Deallocate() {
	if (data != NULL) {
		MCfree(data);
		data = NULL;
		}
	if (leftpointsU != NULL) {
		MCfree(leftpointsU);
		leftpointsU = NULL;
		}
	if (rightpointsU != NULL) {
		MCfree(rightpointsU);
		rightpointsU = NULL;
		}
	if (toppointsV != NULL) {
		MCfree(toppointsV);
		toppointsV = NULL;
		}
	if (bottompointsV != NULL) {
		MCfree(bottompointsV);
		bottompointsV = NULL;
		}
		moldU = 0;
		moldV = 0;

	}

DefPoint* DefPointArray::Point(int32 U, int32 V){
	int32 index = mV * (U - mlowU) + V - mlowV;
	if ((index >= mUmV)|| (index < 0)){
		return NULL;
		}
	else {
		return &data[index];
		}
	}

DefPoint* DefPointArray::GetRightMost(int32 U, int32 V){
	int32 index = mV * (U + 1 - mlowU) + V - mlowV;

	if ((index >= mUmV) || (index < 0) || (data[index].bHasData == false)){
		int32 index = V - mlowV;
		if (index >= mV) {
			return NULL;
			}
		else {
			return &rightpointsU[index];
			}
		}
	else {
		return &data[index];
		}
	}

DefPoint* DefPointArray::GetLeftMost(int32 U, int32 V){
	int32 index = mV * (U - mlowU) + V - mlowV;
	if ((index >= mUmV) || (index < 0) || (data[index].bHasData == false)){
		int32 index = V - mlowV;
		if (index >= mV) {
			return NULL;
			}
		else {
			return &leftpointsU[index];
			}
		}

	else {
		return &data[index];
		}
	}


DefPoint*  DefPointArray::GetTop(int32 U) {
	int32 index = U - mlowU;
	if ((index >= mU)||(index < 0)) {
		return NULL;
		}
	else {
		return &toppointsV[index];
		}
	}

DefPoint*  DefPointArray::GetBottom(int32 U) {
	int32 index = U - mlowU;
	if ((index >= mU)||(index < 0)) {
		return NULL;
		}
	else {
		return &bottompointsV[index];
		}
	}

DefPoint*  DefPointArray::GetLeft(int32 V) {
	int32 index = V - mlowV;
	if ((index >= mV)||(index < 0)) {
		return NULL;
		}
	else {
		return &leftpointsU[index];
		}
	}

DefPoint*  DefPointArray::GetRight(int32 V) {
	int32 index = V - mlowV;
	if ((index >= mV)||(index < 0)) {
		return NULL;
		}
	else {
		return &rightpointsU[index];
		}
	}

