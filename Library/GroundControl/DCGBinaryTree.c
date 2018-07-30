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
#include "DCGBinaryTree.h"
#include "GroundControlLibInternal.h"
#include <stdio.h>
#include <stdlib.h>

struct dcgBinaryTreeNode_struct
{
	void** elements;
	unsigned int elementCount;
	dcgBinaryTreeNode *left, *right;

};

//nodes

void* DCGBinaryTreeNodeGetElement(dcgBinaryTreeNode *node, unsigned int elementIndex) {
	//grabs a specific element
	return node->elements[elementIndex];
}

void* DCGBinaryTreeNodeGetTopElement(dcgBinaryTreeNode *node) {
	//pops the last one off the stack
	return node->elements[node->elementCount - 1];
}


void DCGBinaryTreeNodeAddElement(dcgBinaryTreeNode *node, void* element) {
	unsigned long ptrSize = sizeof(char*);
	void** newElements = calloc(node->elementCount + 1, ptrSize);
	unsigned long i;

	newElements[node->elementCount] = element;

	if (node->elements != NULL) {
		for (i = 0; i < node->elementCount; i++) {
			newElements[i] = node->elements[i];
		}
		free(node->elements);
	}

	node->elementCount++;
	node->elements = newElements;
}

void DCGBinaryTreeNodeRemoveTopElement(dcgBinaryTreeNode *node) {
	//lazy, it will be deallocated later
	node->elementCount--;
}

void DCGBinaryTreeNodeRemoveElementByIndex(dcgBinaryTreeNode *node, unsigned int elementIndex) {
	//lazy, it will be deallocated later
	unsigned long i;
	for (i = elementIndex; i < node->elementCount - 1; i++) {
		node->elements[i] = node->elements[i + 1];
	}
	node->elementCount--;

}

void DCGBinaryTreeNodeCleanUpElements(dcgBinaryTreeNode *node) {
	if (node->elements != NULL) {
		free(node->elements);
		node->elements = NULL;
	}
}

void DCGBinaryTreeNodeCleanUp(dcgBinaryTreeNode *node) {	
	if (node->left != NULL) {
		DCGBinaryTreeNodeCleanUp(node->left);
		free(node->left);
		node->left = NULL;
	}
	if (node->right != NULL) {
		DCGBinaryTreeNodeCleanUp(node->right);
		free(node->right);
		node->right = NULL;
	}
	if (node->elements != NULL) {
		free(node->elements);
		node->elements = NULL;
	}
}

dcgBinaryTreeNode* DCGBinaryTreeNodeRemoveElement(dcgBinaryTree *tree, dcgBinaryTreeNode* node, void* element)
{
	dcgBinaryTreeNode* deleteNode = NULL;
	int compareValue;
	unsigned int elementIndex;

	if (node == NULL)
	{
		return NULL;
	}

	compareValue = tree->compare(element, DCGBinaryTreeNodeGetTopElement(node));

	if (compareValue > 0)
	{
		node->right = DCGBinaryTreeNodeRemoveElement(tree, node->right, element);
	}
	else if (compareValue < 0)
	{
		node->left = DCGBinaryTreeNodeRemoveElement(tree, node->left, element);
	}
	else
	{
		//find the element and remove it from the list
		for (elementIndex = 0; elementIndex < node->elementCount; elementIndex++)
		{
			if (DCGBinaryTreeNodeGetElement(node, elementIndex) == element)
			{
				DCGBinaryTreeNodeRemoveElementByIndex(node, elementIndex);
				elementIndex = node->elementCount;
			}
		}

		if (node->elementCount == 0)
		{
			deleteNode = node;
			if (node->left == NULL)
			{
				node = node->right;
			}
			else if (node->right == NULL)
			{
				node = node->left;
			}
			else
			{
				dcgBinaryTreeNode* successor = node->right;
				dcgBinaryTreeNode* previousNode = node;

				while (successor->left != NULL)
				{
					previousNode = successor;
					successor = successor->left;
				}

				successor->left = node->left;
				previousNode->left = successor->right;
				if (previousNode != node)
				{
					successor->right = node->right;
				}
				node = successor;

			}
			DCGBinaryTreeNodeCleanUpElements(deleteNode);
			free(deleteNode);
		}
	}	
	return node;
}	

dcgBinaryTreeNode* DCGBinaryTreeNodeNew(void *element)
{
	dcgBinaryTreeNode *node = calloc(1, sizeof(dcgBinaryTreeNode));
	unsigned long ptrSize = sizeof(char*);
	node->elementCount = 1;
	node->elements = calloc(node->elementCount, ptrSize);
	node->elements[0] = element;
	node->left = NULL;
	node->right = NULL;
	return node;
}



//tree

void DCGBinaryTreeInit(dcgBinaryTree *tree, int (*compare)(const void *, const void *)) 
{	
	tree->root = NULL;
	tree->compare = compare;
}


void DCGBinaryTreeCleanUp(dcgBinaryTree *tree) 
{	
	if (tree->root != NULL) {
		DCGBinaryTreeNodeCleanUp(tree->root);
		free(tree->root);
		tree->root = NULL;
	}
}

void DCGBinaryTreeAddElem(dcgBinaryTree *tree, void *element)
{
	dcgBinaryTreeNode *currentNode = tree->root;

	if (tree->root == NULL)
	{
		tree->root = DCGBinaryTreeNodeNew(element);
		return;
	}

	while (currentNode)
	{
		int compareValue = tree->compare(element, DCGBinaryTreeNodeGetTopElement(currentNode));

		if (compareValue > 0)
		{
			if (currentNode->right == NULL)
			{
				currentNode->right = DCGBinaryTreeNodeNew(element);
				return;
			}
			else
			{
				currentNode = currentNode->right;
			}
		}
		else if (compareValue < 0)
		{
			if (currentNode->left == NULL)
			{
				currentNode->left = DCGBinaryTreeNodeNew(element);
				return;
			}
			else
			{
				currentNode = currentNode->left;
			}
		}
		else
		{
			DCGBinaryTreeNodeAddElement(currentNode, element);
			return;
		}
	}
}

void* DCGBinaryTreeGetTop(dcgBinaryTree *tree) {
	dcgBinaryTreeNode *currentNode = tree->root;
	if (currentNode == NULL)
	{
		return NULL;
	}
	while (currentNode->right != NULL)
	{
		currentNode = currentNode->right;
	}
	return DCGBinaryTreeNodeGetTopElement(currentNode);
}

void DCGBinaryTreeRemoveTop(dcgBinaryTree *tree) {
	dcgBinaryTreeNode* currentNode = tree->root;
	dcgBinaryTreeNode* previousNode = NULL;

	while (currentNode->right != NULL)
	{
		previousNode = currentNode;
		currentNode = currentNode->right;
	}
	DCGBinaryTreeNodeRemoveTopElement(currentNode);

	if (currentNode->elementCount == 0)
	{
		if (previousNode == NULL)
		{
			tree->root = tree->root->left;
		}
		else
		{
			previousNode->right = currentNode->left;
		}
		DCGBinaryTreeNodeCleanUpElements(currentNode);
		free(currentNode);
	}
}

void DCGBinaryTreeRemoveElement(dcgBinaryTree *tree, void *element) {
	tree->root = DCGBinaryTreeNodeRemoveElement(tree, tree->root, element);
}
	
