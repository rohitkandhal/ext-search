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
int recordLen = 12;
int parentOff;


// Queue Implementation

qnode* head1;	// Current Level
qnode* tail1;

qnode* head2;	// Next Level
qnode* tail2;

int main( int argc, char *argv[] )
{
	string inp_str;	// input string : Command + Key
	fileOff = 0;

	if(argc == 2)
	{
		fname = argv[1];
	}

	// Create index file
	// fp.open(fname, ios::out | ios::in | ios::trunc);
	fp.open(fname, 'x', 1,1);


	/*if(!fp.is_open())
	{
		printf("Unable to open %s", fname);
	}*/

	int isRootAdded = 0;

	while(std::cin >> inp_str)
	{
		string cmd[2];	// cmd[0] -> Command, cmd[1] -> Key

		// Split inp_str to cmd[0] and cmd[1]
		inp_str.token(cmd, strlen(inp_str), " ");	

		// 1. Add Command
		if(strcmp(cmd[0], "add") == 0)
		{
			bst_node* n1 = create_bst_node(cmd[1]);

			bst_node* parent = create_bst_node(-1);

			if(isRootAdded == 1)
			{
				parent= find_parent(0, n1->key);
			}
			
			fp.seek(fileOff, BEGIN);
			fp.write_raw((char*)n1, sizeof(bst_node));

			isRootAdded= 1;

			// fp.seekg(fileOff, fp.beg);
			// fp.write((char*)n1, sizeof(bst_node));

			if(parent != NULL && parent->key >= 0)
			{
				if(parent->key > n1->key)
				{
					parent->l = fileOff;
				}
				else if(parent->key < n1->key)
				{
					parent->r = fileOff;
				}
				
				fp.seek(parentOff, BEGIN);
				fp.write_raw((char*)parent, sizeof(bst_node));

				// fp.seekg(parentOff, fp.beg);
				// fp.write((char*)parent, sizeof(bst_node));
			}

			fileOff += recordLen;

		}
		
		// 2. Find Command
		else if(strcmp(cmd[0], "find") == 0)
		{
			int key = cmd[1];

			find_node(0, key);			
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

bst_node* create_bst_node(int key)
{
	bst_node* n1 = new bst_node();
	n1->key = key;
	n1->l = -1;
	n1->r = -1;

	return n1;
}

void print_data()
{
	// add root
	insertNode(0, &head1, &tail1);
	int currNodeOff = 0;
	int level = 1;
	
	fp.clear();
	
	qnode* currLevelN;	// Current Level Node
	qnode* nextLevelN;	// Next Level Node
	bst_node* currBST = create_bst_node(-1);

	currLevelN = removeNode(&head1, &tail1);

	printf("\n");
	
	while(currLevelN)
	{
		printf(" \n");
		printf("%d:", level);

		while(currLevelN)
		{
			currNodeOff = currLevelN->data;

			fp.seek(currNodeOff, BEGIN);
			fp.read_raw((char*)currBST, sizeof(bst_node));

			//fp.seekg(currNodeOff, fp.beg);
			//fp.read((char*)currBST, sizeof(bst_node));

			if(currBST->l >= 0)
			{
				insertNode(currBST->l, &head2, &tail2);
			}
			if(currBST->r >= 0)
			{
				insertNode(currBST->r, &head2, &tail2);
			}
			printf(" %d/%d", currBST->key, currNodeOff); 
			currLevelN = removeNode(&head1, &tail1);
		}
		head1 = head2;
		tail1 = tail2;
		currLevelN = removeNode(&head1, &tail1);
		
		level++;

		head2 = NULL;
		tail2 = NULL;
	}
}

void find_node(int offset, int key)
{
	bst_node *parent = create_bst_node(-1);
	int newOffset = 0;
	
	fp.seek(offset, BEGIN);
	// fp.seekg(offset, fp.beg);
	
	fp.read_raw((char*)parent, sizeof(bst_node));
	// fp.read((char*) parent, sizeof(bst_node));

	if(parent->key == key)
	{
		printfindresult(parent,key);
	}
	else if(parent->key > key)
	{
		newOffset = parent->l;
		free(parent);
		if(newOffset >=0)
		{
			return find_node(newOffset, key);
		}
		else
		{
			printfindresult(NULL, key);
		}
	}
	else if(parent->key < key)
	{
		newOffset = parent->r;
		free(parent);
		if(newOffset >=0)
		{
			return find_node(newOffset, key);
		}
		else
		{
			printfindresult(NULL, key);
		}

	}
}

void printfindresult(bst_node *node, int key)
{
	if(node && node->key >= 0)
	{
		printf("\nRecord %d exists.", key);
	}
	else
	{
		printf("\nRecord %d does not exist.", key);
	}

	free(node);
}


bst_node* find_parent(int offset, int childKey)
{
	bst_node* parent = create_bst_node(-1);

	fp.seek(offset, BEGIN);
	//fp.seekg(offset, fp.beg);
	
	//if(fp.peek()!= EOF)
	{
		fp.read_raw((char*)parent, sizeof(bst_node));
		//fp.read((char*) parent, sizeof(bst_node));
	
		parentOff = offset;
		if(parent->key >= 0)
		{
			if(parent->key > childKey)
			{
				if(parent->l != -1)
				{
					return find_parent(parent->l, childKey);
				}
				return parent;
			}
			else if(parent->key < childKey)
			{
				if(parent->r != -1)
				{
					return find_parent(parent->r, childKey);
				}
				return parent;
			}
		}
	}
	/*else
	{
		fp.clear();
	}*/

	return NULL;
}


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