struct bst_node {
	int key;	// Key value
	long l;		// File offset of left child node
	long r;		// File offset of right child node
};

bst_node* create_bst_node(int key);

//bst_node* find_node(int offset, int key);
void find_node(int offset, int key);
void printfindresult(bst_node *node, int key);

bst_node* find_parent(int offset, int childKey);

struct qnode
{
	long data;
	qnode *next;
};

qnode* newNode(int d);
void insertNode(int d);
qnode* removeNode();

void insertNode(int d, qnode **head, qnode **tail);

qnode* removeNode(qnode **head, qnode **tail);

void print_data();
void printTime();