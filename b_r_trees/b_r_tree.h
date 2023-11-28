#ifndef GDS_BR_TREES
#define GDS_BR_TREES

#include "../utils/errors.h"
#include "../utils/types.h"

typedef struct br_tree_node_struct{
	void* key;
	void* value;
	color color_value;
	struct br_tree_node_struct* father;
	struct br_tree_node_struct* left_child;
	struct br_tree_node_struct* right_child;
} br_tree_node;

typedef struct br_tree_struct{
	br_tree_node* nil;
	br_tree_node* root;
	unsigned int nodes;
#ifdef 	KEEP_NODES_IN_THE_LIST
//this means that deletion costs O(n)
	llist* list_nodes;	
#endif
} br_tree;

br_tree* alloc_br_tree();

boolean add_elem_to_br_tree(br_tree* brt, int (*compare_key) (void*, void*), void* key, void* value);

void* get_from_br_tree(br_tree* brt, int (*compare_key) (void*, void*), void* key);

#endif


