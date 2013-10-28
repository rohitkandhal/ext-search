#include <windows.h>
#include <stdio.h>
#include <str.h>
#include <time.h>
#include <datastructure.h>
#include <filereader.h>
#include <stdlib.h>
#include <iostream>

filereader fp;
string fname;

int fileOff;
int rootOff;
int recordLen;

snode* head;	// Stack

qnode* head1;	// Queue1: Current Level
qnode* tail1;

qnode* head2;	// Queue2: Next Level
qnode* tail2;

int main( int argc, char *argv[] )
{
	string inp_str;	// input string : Command + Key
	fileOff = 0;
	rootOff = 0;
	
	recordLen = sizeof(btree_node);

	head = NULL;	// Parent Stack

	if(argc == 2)
	{	// Get FileName
		fname = argv[1];
	}

	// Create B-Tree index File
	fp.open(fname, 'x', 1, 1);

	// Any Node exist flag
	bool isRootCreated = 0;

	while(std::cin >> inp_str)
	{
		string cmd[2];	// cmd[0] -> Command, cmd[1] -> Key

		// Split inp_str to cmd[0] and cmd[1]
		inp_str.token(cmd, strlen(inp_str), " ");	

		// 1. Add Command
		if(strcmp(cmd[0], "add") == 0)
		{
			int keyValue;
			keyValue = cmd[1]; 

			if(isRootCreated == 0)
			{
				btree_node* n1 = create_btree_node();
				
				rootOff = 0;

				isRootCreated = 1;	// Update Root created flag

				n1->key[0] = keyValue;
				n1->n++;

				// Write root to file
				fp.seek(rootOff, BEGIN);
				fp.write_raw((char*)n1, sizeof(btree_node));

				fileOff += recordLen;
			}
			else
			{
				insertKey(keyValue);
			}

			// After each add reset the parent stack
			head = NULL;
		}

		// 2. Find Command
		else if(strcmp(cmd[0], "find") == 0)
		{
			int key = cmd[1];

			findKey(key, rootOff);
		}

		// 3. Print data
		else if(strcmp(cmd[0], "print") == 0)
		{
			print_data();
		}

		// 4. End Command
		else if(strcmp(cmd[0],"end") == 0)
		{
			//fp.close();
			break;
		}
	}
}

btree_node* create_btree_node()
{
	btree_node* node = new btree_node();
	node->n = 0;

	int temp = 0;
	for(temp = 0; temp < BTREE_ORDER; temp++)
	{
		node->key[temp] = -1;
		node->child[temp] = -1;
	}

	node->child[temp] = -1;

	return node;
}

btree_leaf* create_btree_leaf()
{
	btree_leaf* leaf = new btree_leaf();

	leaf->n = 0;

	for(int i = 0; i<= BTREE_ORDER; i++)
	{
		leaf->key[i] = -1;
		leaf->child[i] = -1;
	}
	return leaf;
}

split_node* create_split_node()
{
	split_node* node = new split_node();

	node->key = -1;
	node->rightOff = -1;

	return node;
}

void copyNodeToLeaf(btree_node *node, btree_leaf** leaf)
{
	int i;

	for(i = 0; i < node->n; i++)
	{
		(*leaf)->key[i] = node->key[i];
		(*leaf)->child[i] = node->child[i];
	}
	(*leaf)->child[i] = node->child[i];

	(*leaf)->n = node->n;
}

void addKeyToLeaf(int key, int childOff, btree_leaf** leaf)
{
	int i;
	for(i = 0; i < (*leaf)->n; i++)
	{
		if((*leaf)->key[i] > key)
		{
			// Insert Before
			break;
		}
	}

	int j;
	for(j = CAPACITY; j > i; j--)	// In leaf we have 1 extra key node
	{
		(*leaf)->key[j] = (*leaf)->key[j-1];
		(*leaf)->child[j+1] = (*leaf)->child[j];
	}

	(*leaf)->key[j] = key;
	(*leaf)->child[j+1] = childOff;

	(*leaf)->n++;
}

void writeNodeToEOF(btree_node** node)
{
	// Seek to end of file
	fp.seek(fileOff, BEGIN);
	fp.write_raw((char*)(*node), sizeof(btree_node));

	// Update fileOffset
	fileOff += recordLen;
}

void updateNodeOnFile(int offset, btree_node* node)
{
	// Seek to end of file
	fp.seek(offset, BEGIN);
	fp.write_raw((char*)node, sizeof(btree_node));

}

// **************************** INSERT ****************************

void insertKey(int key)
{
	// Find correct child
	int childOff = findCorrectInsertNode(key, rootOff);

	// Insert at this child node
	insertAtThisNode(key, -1 , childOff);
}

int findCorrectInsertNode(int key, int nodeOff)
{
	btree_node* n1 = create_btree_node();

	fp.seek(nodeOff, BEGIN);
	fp.read_raw((char*)n1, sizeof(btree_node));

	int i;

	for(i = 0; i < CAPACITY; i++)
	{
		if((n1->key[i] == -1) || (n1->key[i] > key))
		{
			break;
		}
	}

	// Check if there is child for this node
	if(n1->child[i] != -1)
	{
		// push this node offset to stack (for parent reference)
		push(nodeOff);

		// search in child
		return findCorrectInsertNode(key, n1->child[i]);
	}
	else
	{
		return nodeOff;
	}
}

void insertAtThisNode(int key, int rightChildOff, int nodeOff)
{
	// Read this node
	btree_node* n1 = create_btree_node();

	fp.seek(nodeOff, BEGIN);
	fp.read_raw((char*)n1, sizeof(btree_node));

	// check for it's capacity
	if((n1->n) < CAPACITY)
	{
		// This node has space. Add and Rearrange
		rearrangeNodeWithNewKey(key, rightChildOff, &n1);

		// Write updated node to disk
		updateNodeOnFile(nodeOff, n1);
	}
	else
	{
		// We need to split and add
		splitAndAdd(key, rightChildOff, nodeOff);
	}
}


// Adds a key to a node (Assumes that node has capacity)
// Does not updates to file.
void rearrangeNodeWithNewKey(int key, int childOff, btree_node **node)
{
	if(((*node)->n) < CAPACITY)
	{
		int i;
		for(i = 0; i < (*node)->n; i++)
		{
			if((*node)->key[i] > key)
			{
				// Insert Before
				break;
			}
		}

		int j;
		for(j = CAPACITY -1; j > i; j--)
		{
			(*node)->key[j] = (*node)->key[j-1];
			(*node)->child[j+1] = (*node)->child[j];
		}

		(*node)->key[j] = key;
		(*node)->child[j+1] = childOff;

		(*node)->n++;
	}
	else
	{
		printf("\n CAN'T INSERT WITHIN NODE, NO SPACE");
	}
}


void splitAndAdd(int key,int childOff, int splitNodeOff)
{
	// Create Btree node
	btree_node* oldNode = create_btree_node();
	
	// Populate it from file
	fp.seek(splitNodeOff, BEGIN);
	fp.read_raw((char*)oldNode, sizeof(btree_node));

	// Create Btree leaf
	btree_leaf* leaf = create_btree_leaf();

	// Copy Node to Leaf
	copyNodeToLeaf(oldNode, &leaf);

	// Add Key to Leaf
	addKeyToLeaf(key, childOff, &leaf);

	// Split leaf into 0- (n/2) | (n/2) | (n/2) 
	btree_node* newRightNode = create_btree_node();

	int i;
	int leafKeyCount = leaf->n;

	for(i = ((leafKeyCount)/2 + 1); i < leafKeyCount; i++)
	{
		newRightNode->key[i - (leafKeyCount/2) - 1] = leaf->key[i];
		newRightNode->child[(i) - (leafKeyCount/2)] = leaf->child[i+1];
		newRightNode->n++;
	}

	// Update Old Node
	for(int j = 0; j < CAPACITY; j++)
	{
		if(j < (leafKeyCount/2))
		{
			oldNode->key[j] = leaf->key[j];
			oldNode->child[j+1] = leaf->child[j+1];
		}
		else if( j > (leafKeyCount/2) )
		{
			oldNode->key[j] = -1;
			oldNode->child[j+1] = -1;
			oldNode->n--;
		}
	}

	// As Right split node is added in the end
	// keep fileoffset in temp variable
	int newRightChildOffset;
	newRightChildOffset = fileOff;

	newRightNode->child[0] = leaf->child[(leafKeyCount/2)+1];
	oldNode->child[(leafKeyCount/2)+1] = -1;

	writeNodeToEOF(&newRightNode);

	// Create Split Node which we need to add to parent
	split_node* splitNode = create_split_node();

	splitNode->key = leaf->key[leafKeyCount/2];
	splitNode->rightOff = newRightChildOffset;

	// Remove the split node from the original node
	oldNode->key[leafKeyCount/2] = -1;
	oldNode->n--;
	
	// Update original node (from which we have deleted elements)
	updateNodeOnFile(splitNodeOff, oldNode);

	// Add the split node to the parent
	addSplitToParent(splitNode);

}

void addSplitToParent(split_node* splitNode)
{
	int parentOff = pop();

	// If no parent exist in stack, means we need to create new root
	if(parentOff == NULL)
	{
		btree_node* newRoot = create_btree_node();
		newRoot->key[0] = splitNode->key;
		newRoot->child[0] = rootOff;	// New Root will have current Root as left child
		newRoot->child[1] = splitNode->rightOff;
		newRoot->n++;

		// Write new root to file
		rootOff = fileOff;
		writeNodeToEOF(&newRoot);
	}
	else
	{
		insertAtThisNode(splitNode->key, splitNode->rightOff, parentOff);

	}
}



void push(int data)
{
	// Insert in front of linked list
	snode* newNode = new snode();
	newNode->data = data;
	newNode->next = head;

	head = newNode;
}

int pop()
{
	int data = NULL;
	// Remove from front
	if(head)
	{
		data = head->data;
		head = head->next;
	}
	return data;
}

// **************************** FIND ****************************
void findKey(int key, int nodeOff)
{
	// Read this node
	btree_node* n1 = create_btree_node();

	fp.seek(nodeOff, BEGIN);
	fp.read_raw((char*)n1, sizeof(btree_node));

	int i;
	for(i = 0; i < n1->n; i++)
	{
		if((n1->key[i] == -1) || (n1->key[i] > key))
		{
			break;
		}
		else if(n1->key[i] == key)
		{
			printf("Record %d exists.\n", key);
			return;
		}
	}

	if(n1->child[i] != -1)
	{
		// Search it's child
		findKey(key, n1->child[i]);

	}
	else
	{
		// Key does not exist
		printf("Record %d does not exist.\n", key);
	}

}

// **************************** FIND ****************************

void print_data()
{
	// add root	
	insertNode(rootOff, &head1, &tail1);
	
	int currNodeOff = 0;
	int level = 1;
	
	//fp.clear();
	
	qnode* currLevelNode;	// Current Level Node
	qnode* nextLevelNode;	// Next Level Node
	
	btree_node* currBTreeNode = create_btree_node();

	currLevelNode = removeNode(&head1, &tail1);

	//printf("\n");
	
	while(currLevelNode)
	{
		printf(" \n%d: ", level);

		while(currLevelNode)
		{
			currNodeOff = currLevelNode->data;

			fp.seek(currNodeOff, BEGIN);
			fp.read_raw((char*)currBTreeNode, sizeof(btree_node));

			for(int i = 0; i <= currBTreeNode->n; i++)
			{
				if(i < (currBTreeNode->n))
				{
					if(i!= 0)
					{
						printf(",");
					}
					printf("%d", currBTreeNode->key[i]); 
				}
				if(currBTreeNode->child[i] != -1)
				{
					insertNode(currBTreeNode->child[i], &head2, &tail2);
				}
			}

			printf("/%d ", currNodeOff); 

			currLevelNode = removeNode(&head1, &tail1);
		}
		head1 = head2;
		tail1 = tail2;
		
		currLevelNode = removeNode(&head1, &tail1);
		
		level++;

		head2 = NULL;
		tail2 = NULL;
	}
}

// **************************** QUEUE IMPLEMENTATION ****************************

qnode* newNode(int d)
{
	qnode* temp;
	temp = (qnode*)malloc(sizeof(qnode));

	temp->data = d;
	temp->next = NULL;

	return temp;
}

void insertNode(int d, qnode **head, qnode **tail)
{
	qnode* tmp;
	qnode* headtmp;
	qnode* tailtmp;

	tmp = newNode(d);
	headtmp = *head;
	tailtmp = *tail;

	if(tailtmp == NULL)
	{
		// No element exists
		if(headtmp != NULL)
		{
			printf("\nERROR: TAIL NOT SET PROPERLY");
		}

		*tail = tmp;
		*head = *tail;
	}
	else
	{
		// Insert in the end
		tailtmp->next = tmp;
		*tail = tmp;
	}
}

qnode* removeNode(qnode **head, qnode **tail)
{
	qnode* tmp;
	qnode* headtmp;
	qnode* tailtmp;

	if(*head == NULL)
	{
		if(*tail != NULL)
		{
			printf("\n Tail pointer not updated");
		}
		return NULL;
	}
	else
	{
		tmp = *head;
		*head= (*head)->next;

		if(!(*head))
		{
			*tail = *head;
		}
	}
	return tmp;
}