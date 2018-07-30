/*  Anything Grows - plug-in for Carrara
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
#if CP_PRAGMA_ONCE
#pragma once 
#endif

#include "Basic3DCOMImplementations.h"
#include "LockMasterShaderDef.h"
#include "BitField.h"
#include "I3DShFacetMesh.h"
#include "I3DShScene.h"
#include "I3DExModifier.h"
#include "ISceneSelection.h"
#include "IChangeManagement.h"
#include "PublicUtilities.h"
#include "ISceneDocument.h"
#include "copyright.h"

// define the SceneOp CLSID

//MCDEFINE_GUID2( CLSID_LockMasterShader, R_CLSID_LockMasterShader );
extern const MCGUID CLSID_LockMasterShader;

#if (VERSIONNUMBER >= 0x030000)
class GrowsMenuPreparer : public TBasicMenuCallBack
{
public:
         virtual boolean MCCOMAPI SelfPrepareMenu(ISceneDocument* sceneDocument);
};
#endif

// SceneOp Object :
class LockMasterShader : public TBasicSceneCommand
{
	public :  
		LockMasterShader();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return 0; }

		// IExDataExchanger methods :
#if (VERSIONNUMBER >= 0x030000)
		virtual void	 MCCOMAPI GetMenuCallBack(ISelfPrepareMenuCallBack** callBack);
#else
		virtual boolean  MCCOMAPI SelfPrepareMenus			(ISceneDocument* sceneDocument);
#endif		
		virtual MCCOMErr MCCOMAPI Init						(ISceneDocument* sceneDocument);
		virtual MCCOMErr MCCOMAPI Prepare					();

		virtual boolean  MCCOMAPI CanUndo					();

		virtual boolean  MCCOMAPI Do						();
		virtual boolean  MCCOMAPI Undo						();
		virtual boolean  MCCOMAPI Redo						();

	private :
		TMCCountedPtr<ISceneDocument>	fSceneDocument;
		TMCCountedPtr<I3DShTreeElement>	fTree;
		TMCCountedPtr<I3DShScene>		fScene;

		TMCCountedPtr<ISceneSelection>	fSelection;
		TMCCountedPtr<ISceneSelection>	fCloneSelection;

		TMCCountedPtr<IChangeChannel>	fSelectionChannel;
		TMCCountedPtr<IChangeChannel>	fTreePropertyChannel;
		void DoTree(TMCCountedPtr<I3DShTreeElement> tree);
		void DoItem(TMCCountedPtr<I3DShTreeElement> tree, TMCCountedPtr<I3DShInstance> instance);
		void ClearTree(TMCCountedPtr<I3DShTreeElement> tree);
		void AddLock(TMCCountedPtr<I3DShTreeElement> tree, TMCCountedPtr<I3DShMasterShader> mastershader, uint32& AGrs );

};