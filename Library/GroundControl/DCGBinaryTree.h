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
#ifndef _DCG_BINARYTREE
#define _DCG_BINARYTREE 1

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dcgBinaryTreeNode_struct dcgBinaryTreeNode;

typedef struct dcgBinaryTree_struct  
{
	dcgBinaryTreeNode *root;
	int (*compare)(const void *, const void *);
} dcgBinaryTree;


extern void DCGBinaryTreeInit(dcgBinaryTree *tree, int (*compare)(const void *, const void *));
extern void DCGBinaryTreeCleanUp(dcgBinaryTree *tree);
extern void DCGBinaryTreeAddElem(dcgBinaryTree *tree, void *element);
extern void* DCGBinaryTreeGetTop(dcgBinaryTree *tree);
extern void DCGBinaryTreeRemoveTop(dcgBinaryTree *tree);
extern void DCGBinaryTreeRemoveElement(dcgBinaryTree *tree, void *element);

#ifdef __cplusplus
}
#endif

#endif