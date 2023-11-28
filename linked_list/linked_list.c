#include "linked_list.h" 

struct fe_parallel_arg{
	llist_node* start_node;	
	int counter;
	void (*consumer) (void*);
	boolean last_sl;
};

static inline llist_node* alloc_node(void* value);
static inline void resize_sublists(llist* l);
static void* foreach_thread_function(void* arg);
static inline void* destroy_llist_node(llist_node* n);

//PUBLIC FUNCTIONS

llist* alloc_llist(){
	llist* n_l = malloc(sizeof(llist));
	if(n_l == NULL)
		terminate_w_error("in linked_list.c", "malloc malfunction");
	n_l -> head = NULL;
	n_l -> tail = NULL;
	n_l -> length = 0;

	return n_l;
}



void add_elem_to_llist(llist* l, void* value){
	if(l == NULL)
		terminate_w_error("in linked_list.c : add_elem_to_llist", "null argument");
	if(l -> length == 0)
		l -> head = l -> tail = alloc_node(value);
	else
		l -> tail = (l -> tail -> next) = (alloc_node(value));

	l -> length ++; 
}

void* remove_elem_from_llist(llist* l, 
		void* value, 
		boolean (*eq) (void*, void*)){
	if(l == NULL)	
		terminate_w_error("in linked_list.c : remove_elem_from_llist", "null arguments");

	if(value == NULL || l -> length == 0)
		return FALSE;
	
	llist_node* prec_ptr = NULL;
	llist_node* ptr = l -> head;
	boolean flag = FALSE;	
	while(ptr -> next != NULL && !(flag = (*eq) (ptr -> value, value))){
		prec_ptr = ptr;
		ptr = ptr -> next;	
	}
	if(!flag) return NULL;
	void* ret_val = NULL;

	if(ptr == l -> tail && ptr == l -> head){
		ret_val = ptr -> value;
		free(l -> head);
		l -> head = l -> tail = NULL;
	}
	else{
		if(ptr == l -> head){
			llist_node* buf = l -> head;
			ret_val = buf -> value;
			l -> head = ptr -> next;
			free(buf);
		}
		else{
			if(ptr == l -> tail){
				llist_node* buf = l -> tail;
				ret_val = buf -> value;
				l -> tail = prec_ptr;
				l -> tail -> next = NULL;
				free(buf);
			}
			else{
				llist_node* buf = ptr;
				ret_val = buf -> value;
				prec_ptr -> next = ptr -> next;
				free(buf);
			}
		}
	}
	l -> length --;
	return ret_val;
}


boolean contains(llist* l, void* value, boolean (*eq) (void*, void*)){
	if(l == NULL)
		terminate_w_error("in linked_list.c : contains", "null arguments");

	if(value == NULL || l -> length == 0)
		return FALSE;

	llist_node* ptr = l -> head;

	while(ptr != NULL)
		if((*eq) (value, ptr -> value))
			return TRUE;
		else ptr = ptr -> next;

	return FALSE;
}


void foreach_llist(llist* list, void (*consumer) (void*)){
	if(list == NULL)
		terminate_w_error("NULL ARGUMENTS:", "foreach function");

	llist_node* node = list -> head;
	while(node != NULL){
		(*consumer) (node -> value);
		node = node -> next;
	}
}



// 	PARALLEL FOREACH !



void foreach_parallel_llist(llist* list, void (*consumer) (void*)){

	if(list == NULL)
		terminate_w_error("in linked_list.c : foreach_parallel_llist", "null arguments");

	//	
	if(list -> length < HACK_PTHREADS){
		foreach_llist(list, consumer);
		return;
	}
	
	void* thread_res;
	
	struct fe_parallel_arg args[HACK_PTHREADS]; 
	
	pthread_t threads_[HACK_PTHREADS];

	int status[HACK_PTHREADS];

	llist_node* ptr = list -> head;
	int counter = 0;
	int sub_lens = list -> length / HACK_PTHREADS;

	for(int i = 0; counter < HACK_PTHREADS; i++){
		if(!(i % (sub_lens))){
			args[counter].start_node = ptr;
	        	args[counter].counter = sub_lens;
	        	args[counter].consumer = consumer;
			args[counter].last_sl = (counter == HACK_PTHREADS-1);

			status[counter] = pthread_create(&threads_[counter], NULL, foreach_thread_function, &args[counter]);
			if(status[counter])
				terminate_w_error("In foreach_parallel: ", "can't create pthread");
			counter++;
		}
		ptr = ptr -> next;
	}

	for(int i = 0; i < HACK_PTHREADS; i++)
		status[i] =  pthread_join(threads_[i], &thread_res);
	
	for(int i = 0; i < HACK_PTHREADS; i++)
		if(status[i])
			terminate_w_error("In foreach_parallel:", "error in joining threads");

}

// 	END OF PARALLEL FOREACH
//
//
// DESTROY FUNCTION
//
//1//1//1//1//1//1/ /1//1//1//1//1//1/ /1//1//1//1//1//1//1/ /1//1//1//1//1//1/  /1//1
static inline void* destroy_llist_node(llist_node* n){
	void* buf_val;
	if(n != NULL){
		buf_val = n -> value;
		free(n);
	}
	return buf_val;
}

void destroy_llist(llist* l, void (*value_destroyer) (void*)){
	if(l != NULL){
		llist_node* ptr = l -> head;
		llist_node* ptr_buff;
		while(ptr != NULL){
			value_destroyer(ptr -> value);
			ptr_buff = ptr;
			ptr = ptr -> next;
			free(ptr_buff);
		}
		free(l);
	}
}

llist* l_concat_llists(llist* l1, llist* l2){
	llist* n_l = alloc_llist();
	llist_node* ptr;
	if(l1 != NULL){
		ptr = l1 -> head;
		while(ptr != NULL){
			add_elem_to_llist(n_l, ptr -> value);
			ptr = ptr -> next;
		}
	}

	if(l2 != NULL){
		ptr = l2 -> head;
		while(ptr != NULL){
			add_elem_to_llist(n_l, ptr -> value);
			ptr = ptr -> next;
		}
	}
	return n_l;
}

llist* concat_llists(llist* l1, llist* l2){
	llist* n_l = alloc_llist();
	if(l1 == NULL && l2 == NULL) return NULL;
	
	if(l1 == NULL || l1 -> head == NULL){
		n_l -> head = l2 -> head;	
		n_l -> tail = l2 -> tail;
		n_l -> length = l2 -> length;



		free(l2);
		if(l1 != NULL) free(l1);
		return n_l;
	}

	if(l2 == NULL || l2 -> head == NULL){
		n_l -> head = l1 -> head;	
		n_l -> tail = l1 -> tail;
		n_l -> length = l1 -> length;


		free(l1);
		if(l2 != NULL) free(l2);
		return n_l;
	}

	
	n_l -> head = l1 -> head;
	n_l -> tail = l2 -> tail;
	l1 -> tail -> next = l2 -> head;
	n_l -> length = l1 -> length + l2 -> length;

	free(l1);
	free(l2);

	return n_l;
}

//PRIVATE FUNCTIONS
static inline llist_node* alloc_node(void* value){
	llist_node* n_n = malloc(sizeof(llist_node));
	if(n_n == NULL)
		terminate_w_error("in linked_list.c", "malloc malfunction");
	n_n -> value = value;
	n_n -> next = NULL;
	return n_n;
}

static void* foreach_thread_function(void* arg){

        struct fe_parallel_arg* conv_arg = (struct fe_parallel_arg*) arg;
	int counter = conv_arg -> counter;
	llist_node* ptr = conv_arg -> start_node;
	boolean last_sl = conv_arg -> last_sl;

	//last sub-list shall continue until the end of the list
	while((counter-- > 0 || last_sl)
		       	&& ptr != NULL){
		(*conv_arg -> consumer) (ptr -> value);
		ptr = ptr -> next;
	}
	return NULL;
}

