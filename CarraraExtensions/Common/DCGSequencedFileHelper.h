/*  Carrara plug-in utilities
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
#ifndef __DCGSequencedFileHelper__
#define __DCGSequencedFileHelper__

#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "MCClassArray.h"
#include "MCFiletype.h"
#include "limits.h"

class DCGSequencedFileHelper
{
private:
	DCGSequencedFileHelper();


	static const void fillFileParts(const TMCDynamicString& fileName, TMCDynamicString& nonNumericName, TMCDynamicString& extension, TMCDynamicString& numeric)
	{
		TMCDynamicString tempFileName;
		TMCString15 sep;
		gFileUtilities->GetSeparator(sep);

		uint32 slashPosition = fileName.FindCharPosFromRight(sep[0]);
		uint32 extensionPosition = fileName.FindCharPosFromRight('.');

		nonNumericName = fileName;

		//set the extension and file name portion
		if (extensionPosition != UINT_MAX)
		{
			extension = fileName;
			extension.SubString(extensionPosition + 1, extension.Length() - extensionPosition + 1);
			nonNumericName.SubString(slashPosition + 1, extensionPosition - (slashPosition + 1));
		}
		else
		{
			extension.SetLength(0);
			nonNumericName.SubString(slashPosition + 1, nonNumericName.Length() - (slashPosition + 1));
		}

		//crop the numeric stuff off the file name
		//look for the first non numeric from the right
		//crop off the rest
		if (nonNumericName.IsNumericString()) {
			//the whole thing is numeric
			numeric = nonNumericName;
			nonNumericName = "";
		}
		else {
			for(int index = nonNumericName.Length() - 1; index >= 0; index--)
			{
				tempFileName = nonNumericName;
				tempFileName.SubString(index, nonNumericName.Length() - index);
				if (!tempFileName.IsNumericString())
				{
					numeric = nonNumericName;
					nonNumericName.SubString(0, index + 1);
					numeric.SubString(index + 1, numeric.Length() - (index + 1));
					index  = -1;
				}
			}
		}

	}

	static const void fillFilePath(const TMCDynamicString& fileName, TMCDynamicString& path)
	{
		TMCString15 sep;
		gFileUtilities->GetSeparator(sep);

		uint32 slashPosition = fileName.FindCharPosFromRight(sep[0]);
		path = fileName;
		path.SubString(0, slashPosition + 1);
	}

public:
	//looks for sequenced files in a folder that match the file name and extension
	//returns an array of the file names sorted by the trailing numeric portion of the file
	static void fillFileList(const TMCDynamicString& firstFile, const TMCFileType* fileType, TMCClassArray<TMCDynamicString>& foundFiles)
	{
		foundFiles.SetElemCount(0);

		TMCClassArray<TMCDynamicString> numericParts;
		TMCCountedPtr<IMCFileIterator> iter;
		TMCDynamicString path;
		TMCDynamicString sequencedFileName;
		TMCDynamicString sequencedNonNumericName;
		TMCDynamicString sequencedNumericName;
		TMCDynamicString sequencedExtension;

		TMCCountedPtr<IMCFile> sequencedFile;
		TMCDynamicString firstFileNonNumericName;
		TMCDynamicString firstFileNumericName;
		TMCDynamicString firstFileExtension;

		fillFileParts(firstFile, firstFileNonNumericName, firstFileExtension, firstFileNumericName);
		fillFilePath(firstFile, path);

		//look in the same folder as our object for stuff that matches 
		//the non-numeric portion of its name.
		gFileUtilities->CreateIMCFileIterator(&iter,path,fileType);
		gFileUtilities->CreateIMCFile(&sequencedFile);

		for (iter->First(sequencedFile);iter->More();iter->Next(sequencedFile))
		{
			//loop through grab the file name
			sequencedFile->GetFileFullPathName(sequencedFileName);

			//File iter doesn't throw out folders if they have the extension
			//we're looking for in the name (eg. looking for DEM file type will include a folder named 1651848.DEM.SDTS)
			if (sequencedFile->IsDirectory())
			{
				continue;
			}

			fillFileParts(sequencedFileName, sequencedNonNumericName, sequencedExtension, sequencedNumericName);

			if (firstFileNonNumericName != sequencedNonNumericName || firstFileExtension != sequencedExtension)
			{
				continue;
			}

			//insert into the list in order of numeric part
			//would just append or prepend, but OSX and Windows
			//are populating the iter in different orders
			//TODO: could use smarter search
			uint32 insertLocation = foundFiles.GetElemCount();

			for (uint32 fileIndex = 0; fileIndex < foundFiles.GetElemCount(); fileIndex++)
			{
				if (sequencedNumericName.CompareQuick(numericParts[fileIndex]) > 0)
				{
					continue;
				}
				else
				{
					insertLocation = fileIndex;
					fileIndex = foundFiles.GetElemCount();
				}
			}
			foundFiles.InsertElem(insertLocation, sequencedFileName);
			numericParts.InsertElem(insertLocation, sequencedNumericName);

		}
	};

};



#endif