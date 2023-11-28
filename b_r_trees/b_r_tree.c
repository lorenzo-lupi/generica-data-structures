#include "b_r_tree.h"

static inline br_tree_node* alloc_br_tree_nil();
static inline br_tree_node* alloc_br_tree_node(void* key, void* value, br_tree_node* father, br_tree_node* nil);
static inline void     fix_up_insertion_br_tree(br_tree* brt, br_tree_node* p);
static inline void     left_rotate_br_tree(br_tree* brt, br_tree_node* p);
static inline void     right_rotate_br_tree(br_tree* brt, br_tree_node* p);


br_tree* alloc_br_tree(){
	br_tree* nbrt = malloc(sizeof(br_tree));
	if(nbrt == NULL)
		terminate_w_error("in alloc_br_tree", "malloc error");

	nbrt -> root = nbrt -> nil = alloc_br_tree_nil();
	nbrt -> nodes = 0;
	return nbrt;
}

boolean add_elem_to_br_tree(br_tree* brt, 
		int (*compare_key) (void*, void*),
	       	void* key, void* value){
	if(brt == NULL)
		terminate_w_error("in add_elem_to_br_tree", "null brt");

	if(key == NULL)
		terminate_w_error("in add_elem_to_br_tree", "null key");
	
	if(brt -> nodes == 0){
		brt -> root = alloc_br_tree_node(key, value, brt -> nil, brt -> nil);
	}
	else{
		br_tree_node* p = brt -> root;
		boolean has_reached_leaves = FALSE;
		while(!has_reached_leaves){ 
			switch(compare_key(key, p -> key)){
				case EQ:
					return FALSE;
				break;
				case LESS:
					if(p -> left_child == brt -> nil){
						p -> left_child = alloc_br_tree_node(key, value, p, brt -> nil);
						has_reached_leaves = TRUE;
					}
					p = p -> left_child;
				break;
				case MORE:
					if(p -> right_child == brt -> nil){
						p -> right_child = alloc_br_tree_node(key, value, p, brt -> nil);
						has_reached_leaves = TRUE;
					}
					p = p -> right_child;
				break;
			}
		}
		//FIXUP METHOD
		fix_up_insertion_br_tree(brt, p);
	}
	brt -> nodes ++;
	brt -> root -> color_value = BLACK;
	return TRUE;
}

void* get_from_br_tree(br_tree* brt, int (*compare_key) (void*, void*), void* key){
	if(brt == NULL)
		terminate_w_error("in add_elem_to_br_tree", "null brt");

	if(key == NULL)
		terminate_w_error("in add_elem_to_br_tree", "null key");
	
	br_tree_node* p = brt -> root;
	int i = 0;
	while(p != brt -> nil){
		i++;
		switch(compare_key(key, p -> key)){
			case EQ:
				printf("\t iterations: %d \n", i);
				return p -> value;
			break;
			case LESS:
				p = p -> left_child;
			break;
			case MORE:
				p = p -> right_child;
			break;
		}
	}
	return NULL;
}
/// ALLOCATION METHODS

static inline br_tree_node* alloc_br_tree_nil(){
	br_tree_node* nbrtn = malloc(sizeof(br_tree_node));
	if(nbrtn == NULL)
		terminate_w_error("in alloc_br_tree_nil", "malloc error");

	nbrtn -> color_value = BLACK;
	nbrtn -> father = nbrtn -> left_child = nbrtn -> right_child = nbrtn;
	return nbrtn;
}

static inline br_tree_node* alloc_br_tree_node(void* key, void* value, br_tree_node* father, br_tree_node* nil){
	if(father == NULL)
		terminate_w_error("in alloc_br_tree_node", "null father");

	br_tree_node* nbrtn = malloc(sizeof(br_tree_node));
	if(nbrtn == NULL)
		terminate_w_error("in alloc_br_tree_node", "malloc error");

	nbrtn -> key = key;
	nbrtn -> value = value;
	nbrtn -> color_value = RED;
	nbrtn -> father = father;
	nbrtn -> left_child = nbrtn -> right_child = nil;

	return nbrtn;
}

// TREE MANAGEMENT METHODS

static inline void     fix_up_insertion_br_tree(br_tree* brt, br_tree_node* p){
	br_tree_node* uncle;
	while(p -> father -> color_value == RED){
		if (p -> father == p -> father -> father -> left_child){
			uncle = p -> father -> father -> right_child;
			// case 1
			if(uncle -> color_value == RED){
				p -> father -> color_value = BLACK;
				uncle -> color_value = BLACK;
				p -> father -> father -> color_value = RED;
				p = p -> father -> father; // restart the loop with new p
			}
			else{
				if(p == p -> father -> right_child){
					p = p -> father;
					left_rotate_br_tree(brt, p);
				}
				p -> father -> color_value = BLACK;
				p -> father -> father -> color_value = RED; //violation of propriety 5 
				right_rotate_br_tree(brt, p -> father -> father); //fix up
			}
		}
		else{
			uncle = p -> father -> father -> left_child;
			
			if(uncle -> color_value == RED){
				p -> father -> color_value = BLACK;
				uncle -> color_value = BLACK;
				p -> father -> father -> color_value = RED;
				p = p -> father -> father; // restart the loop with new p
			}
			else{
				if(p == p -> father -> left_child){
					p = p -> father;
					right_rotate_br_tree(brt, p);
				}
				p -> father -> color_value = BLACK;
				p -> father -> father -> color_value = RED; //violation of propriety 5 
				left_rotate_br_tree(brt, p -> father -> father); //fix up
			}

		}
	}
}

static inline void     left_rotate_br_tree(br_tree* brt, br_tree_node* p){

		br_tree_node* pp = p -> right_child; 
		p -> right_child = pp -> left_child;
		if (pp -> left_child != brt -> nil){
			pp -> left_child -> father = p;
		}
		pp -> father = p -> father;
		if(p -> father == brt -> nil){
			brt -> root = pp;
		}
		else{
			if(p == p -> father -> left_child){
				p -> father -> left_child = pp;
			}
			else{
				p -> father -> right_child = pp;
			}
		}
		pp -> left_child = p;
		p -> father = p;
}

static inline void     right_rotate_br_tree(br_tree* brt, br_tree_node* p){
	br_tree_node* pp = p -> left_child;
	p -> left_child = p -> right_child;

	if(p -> left_child != brt -> nil){
		p -> left_child -> father = p;
	}
	pp -> father = p -> father;
	if(p -> father == brt -> nil){
		brt -> root = pp;
	}
	else{
		if(p == p -> father -> left_child){
			p -> father -> left_child = pp;
		}
		else{
			p -> father -> right_child = pp;
		}

	}
	pp -> right_child = p;
	p -> father = p;

}


