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
#include "MCBasicTypes.h"
#include "MCClassArray.h"
#include "dcgdebug.h"



template<class T> class DCGBinaryTree 
{
public:
	DCGBinaryTree<T>(int (*compare)(const void *, const void *)) 
	{	
		root = NULL;
		this->compare = compare;
	};

	~DCGBinaryTree<T>(void)
	{
	
	};

	void AddElem(T element)
	{
		try {
			if (!root)
			{
				root = new Node(element);
				return;
			}

			Node* currentNode = root;
			while (currentNode)
			{
				int compareValue = compare(element, currentNode->elements[0]);

				if (compareValue > 0)
				{
					if (currentNode->right == NULL)
					{
						currentNode->right = new Node(element);
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
						currentNode->left = new Node(element);
						return;
					}
					else
					{
						currentNode = currentNode->left;
					}
				}
				else
				{
					currentNode->elements.AddElem(element);
					return;
				}
			}
		}
		catch (TMCException exception)
		{
			Alert("AddElem");
			throw TMCException(exception);
		}
	};

	T GetTop() 
	{
		try {
			if (root == NULL)
			{
				return NULL;
			}
			Node* currentNode = root;
			while (currentNode->right != NULL)
			{
				currentNode = currentNode->right;
			}
			return currentNode->elements[currentNode->elements.GetElemCount() - 1];
		}
		catch (TMCException exception)
		{
			Alert("GetTop");
			throw TMCException(exception);
		}
	}

	void RemoveTop()
	{
		try {
			Node* currentNode = root;
			Node* previousNode = NULL;

			while (currentNode->right != NULL)
			{
				previousNode = currentNode;
				currentNode = currentNode->right;
			}
			currentNode->elements.RemoveElem(currentNode->elements.GetElemCount() - 1, 1);

			if (currentNode->elements.GetElemCount() == 0)
			{
				if (previousNode == NULL)
				{
					root = root->left;
				}
				else
				{
					previousNode->right = currentNode->left;
				}
				delete currentNode;

			}
		}
		catch (TMCException exception)
		{
			Alert("RemoveTop");
			throw TMCException(exception);
		}
	};

	void RemoveElem(T element)
	{
		root = RemoveElem(root, element);
	};

	void dump()
	{
		if (root)
			dumpInternal(root);

	};



protected:
	T emptyObject;
	int (*compare)(const void *, const void *);

	class Node
	{
	public:
		Node(T element) 
		{	
			left = NULL;
			right = NULL;
			elements.AddElem(element);
		};
		~Node(void)
		{
		};

		TMCArray<T> elements;
		Node *left, *right;
	};
	Node *root;


	void dumpInternal(Node *node)
	{
		#ifdef _DEBUG
			if (node->left)
			{
				dumpInternal(node->left);
			}
			char temp[120];
			uint32 elementCount = node->elements.GetElemCount();
			sprintf_s(temp, 120, "******Error  %.8f Elements %4d \n\0"
					, node->elements[0]->error
					, elementCount);
			
			for (uint32 elementIndex = 0; elementIndex < elementCount; elementIndex++)
			{
				sprintf_s(temp, 120, "Error  %.8f V  %4d, %4d  F  %4d, %4d \n\0"
					, node->elements[elementIndex]->error
					, node->elements[elementIndex]->vertices[0]
					, node->elements[elementIndex]->vertices[1]
					, node->elements[elementIndex]->facets[0]
					, node->elements[elementIndex]->facets[1]
					
					);
				OutputDebugStringA(temp);
			}
			if (node->right)
			{
				dumpInternal(node->right);
			}

		#endif
		}

private:
	Node* RemoveElem(Node* node, T element)
	{
		try {
			if (node == NULL)
			{
				return NULL;
			}

			Node* deleteNode;
			const T& topElement = node->elements[0];
			int compareValue = compare(element, topElement);

			if (compareValue > 0)
			{
				node->right = RemoveElem(node->right, element);
			}
			else if (compareValue < 0)
			{
				node->left = RemoveElem(node->left, element);
			}
			else
			{
				//find the element and remove it from the list
				uint32 elementCount = node->elements.GetElemCount();
				for (uint32 elementIndex = 0; elementIndex < elementCount; elementIndex++)
				{
					if (node->elements[elementIndex] == element)
					{
						node->elements.RemoveElem(elementIndex, 1);
						elementIndex = elementCount;
					}
				}

				if (node->elements.GetElemCount() == 0)
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
						Node* successor = node->right;
						Node* previousNode = node;

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
					delete deleteNode;
				}
			}	
			return node;
		}
		catch (TMCException exception)
		{
			Alert("RemoveElem");
			throw TMCException(exception);
		}
	}
};