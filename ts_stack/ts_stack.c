#include "ts_stack.h"

static inline ts_stack_node* alloc_ts_stack_node(void* value);
static inline void destroy_ts_stack_node(ts_stack_node* ts_s,void (*destroy_value) (void*));


//public functions
ts_stack* alloc_ts_stack(){
	ts_stack* n_ts_s = malloc(sizeof(ts_stack));
	if(n_ts_s == NULL)
		terminate_w_error("in alloc_ts_stack", "can't allocate memory");
	n_ts_s -> head = NULL;

	int status;
	status = pthread_mutex_init(&n_ts_s -> mutex, NULL);
	if(status != 0)
		terminate_w_error("in allo_ts_stack", "can't can't init mutex");
	return n_ts_s;
}


void* ts_pop(ts_stack* ts_s){
	if(ts_s == NULL)
		terminate_w_error("in pop", "null arguments");

	int status =  pthread_mutex_lock(&ts_s -> mutex);
	if(status != 0)
		terminate_w_error("in pop", "can't lock the mutex");

	if(ts_s -> head == NULL){
		status = pthread_mutex_unlock(&ts_s -> mutex);
		if(status != 0)
			terminate_w_error("in pop", "can't unlock the mutex");
		return NULL;
	}

	void* ret_val = ts_s -> head -> value;
	//this way I don't need to write a fake value destroyer
	ts_s -> head -> value = NULL;
	ts_stack_node* f_h = ts_s -> head;
	ts_s -> head = ts_s -> head -> next;

	destroy_ts_stack_node(f_h, NULL);

	status = pthread_mutex_unlock(&ts_s -> mutex);
	if(status != 0)
		terminate_w_error("in pop", "can't unlock the mutex");
	
	return ret_val;
}


void ts_push(ts_stack* ts_s, void* val){
	if(ts_s == NULL)
		terminate_w_error("in push", "null arguments");

	int status =  pthread_mutex_lock(&ts_s -> mutex);
	if(status != 0)
		terminate_w_error("in push", "can't lock the mutex");
	
	ts_stack_node* n_n = alloc_ts_stack_node(val);
	n_n -> next = ts_s -> head;
	ts_s -> head = n_n;

	status = pthread_mutex_unlock(&ts_s -> mutex);
	if(status != 0)
		terminate_w_error("in push", "can't unlock the mutex");
	

}


void destroy_ts_stack(ts_stack* ts_s, void (*destroy_value) (void*)){
	if(ts_s != NULL){
		while(ts_s -> head != NULL)
			destroy_value(ts_pop(ts_s));
		
		if(pthread_mutex_destroy(&ts_s -> mutex) != 0)
			terminate_w_error("can't destroy ts_stack' mutex", "");
		free(ts_s);
	}
}

// private functions

static inline ts_stack_node*  alloc_ts_stack_node(void* value){
	ts_stack_node* n_n = malloc(sizeof(ts_stack_node));
	if(n_n == NULL)
		terminate_w_error("in push", "can't alloc st_stack_node");

	n_n -> value = value;
	n_n -> next  = NULL;

	return n_n;
}

static inline void destroy_ts_stack_node(ts_stack_node* ts_n, void (*destroy_value) (void*)){
	if(ts_n != NULL){
		if(ts_n -> value != NULL) 
			destroy_value(ts_n -> value);
		free(ts_n);
	}
}
