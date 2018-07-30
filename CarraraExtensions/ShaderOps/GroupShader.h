/*  Shader Ops - plug-in for Carrara
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
#ifndef __GROUPSHADER__
#define __GROUPSHADER__
 
#if CP_PRAGMA_ONCE
#pragma once
#endif

#include "Basic3DCOMImplementations.h"
#include "GroupShaderDef.h"
#include "BitField.h"
#include "I3DShFacetMesh.h"
#include "I3DShScene.h"
#include "I3DExModifier.h"
#include "ISceneSelection.h"
#include	"IMFDialogPart.h"
#include	"IMFPart.h"
#include	"IMFResponder.h"
#include "IChangeManagement.h"
#include "PublicUtilities.h"
#include "ISceneDocument.h"
#include "copyright.h"

// define the SceneOp CLSID

//MCDEFINE_GUID2( CLSID_GroupShader, R_CLSID_GroupShader );
extern const MCGUID CLSID_GroupShader;

struct GroupShaderData
{
	ActionNumber acShaderIndex;
	
};
 
#if (VERSIONNUMBER >= 0x030000)
class GroupShaderMenuPreparer : public TBasicMenuCallBack
{
public:
         virtual boolean MCCOMAPI SelfPrepareMenu(ISceneDocument* sceneDocument);
};
#endif

// SceneOp Object :
class GroupShader : public TBasicSceneCommand
{
	public :  
		GroupShader();
		STANDARD_RELEASE;
		virtual int32   MCCOMAPI GetParamsBufferSize    ()
			const        { return sizeof(GroupShaderData); }

		// IExDataExchanger methods :
		virtual void*	 MCCOMAPI GetExtensionDataBuffer	();
		virtual MCCOMErr MCCOMAPI ExtensionDataChanged		();
		virtual int16	 MCCOMAPI GetResID					();
		virtual MCCOMErr 			MCCOMAPI HandleEvent(MessageID message, IMFResponder* source, void* data);

		
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
		GroupShaderData					fData;					// Scene Operation Data
		TMCCountedPtr<ISceneDocument>	fSceneDocument;
		TMCCountedPtr<I3DShTreeElement>	fTree;
		TMCCountedPtr<I3DShScene>		fScene;

		TMCCountedPtr<ISceneSelection>	fSelection;
		TMCCountedPtr<ISceneSelection>	fCloneSelection;
		int32 lShadersApplied;

		TMCCountedPtr<IChangeChannel>	fSelectionChannel;
		TMCCountedPtr<IChangeChannel>	fTreePropertyChannel;
		void DoTree(TMCCountedPtr<I3DShTreeElement> tree);
		void DoItem(TMCCountedPtr<I3DShTreeElement> tree, TMCCountedPtr<I3DShInstance> instance);
		void FillShaderList (TMCCountedPtr<IMFPart> popuppart);
};



#endif