
static const int BTREE_ORDER = 33;
static const int CAPACITY = 32;

// Order-33 B-Tree
struct btree_node {
	int n;		// Number of keys in node
	int key[32];	// Key values
	long child[33];	// Files offsets of child nodes
};	

struct btree_leaf {
	int n;
	int key[BTREE_ORDER];
	long child[(BTREE_ORDER+1)];
};

// Structure of Split Node
struct split_node{
	int key;	// Key
	int rightOff;	// Offset of right child
};

// create_btree_node
btree_node* create_btree_node();
btree_leaf* create_btree_leaf();
split_node* create_split_node();

void copyNodeToLeaf(btree_node *node, btree_leaf** leaf);

void addKeyToLeaf(int key, int childOff, btree_leaf** leaf);

int findCorrectInsertNode(int key, int nodeOff);

void insertKey(int key);
void insertAtThisNode(int key, int rightChildOff, int nodeOff);
void rearrangeNodeWithNewKey(int key, int childOff, btree_node **node);

void splitAndAdd(int key,int childOff, int splitNodeOff);

void writeNodeToEOF(btree_node** node);
void updateNodeOnFile(int offset, btree_node* node);


void addSplitToParent(split_node* splitNode);

// ************************ STACK ************************

struct snode {
	int data;	// Parent Offset
	snode* next;	// Next
};

void push(int data);
int pop();

// ************************ FIND ************************
void findKey(int key, int nodeOff);