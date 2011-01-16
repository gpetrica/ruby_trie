#include <stdlib.h>  /* for malloc, free */
#include <string.h>  /* for memcmp, memmove */
#include "ruby.h"

// typdefs!
typedef enum { false = 0, true} bool;

typedef struct node {
	char character;
	VALUE value;
	struct node * first_child;
	struct node * next_sibling;
} trie_node;

static VALUE rb_cTrie;

// =========================
// = function declarations =
// =========================

//trie implementation
static trie_node * trie_node_for_key(trie_node * root, char * key, bool create_missing_nodes);
static trie_node * trie_sibling_for_char(trie_node * node, char ch);
static trie_node * trie_add_sibling_for_char(trie_node * node, char ch);
static trie_node * trie_new_node_with_char(char ch);
static trie_node * trie_new_node();
static VALUE rb_trie_find_children(VALUE self, VALUE key);
static VALUE rb_trie_find_children_with_block(VALUE self, VALUE key);
static VALUE rb_trie_levenshtein_search(VALUE self, VALUE word, VALUE max_distance);
static void trie_collect_values(void * t, VALUE prary);
static void trie_collect_values_with_yield(void * t);
static void trie_traverse(trie_node * trie, void (*lambda_func)(void *));
static void trie_traverse_with_context(trie_node * trie, VALUE context, void (*lambda_func)(void *, VALUE));
static void free_trie(trie_node * trie);
static void count_nodes_callback(void *n, VALUE accum);
static VALUE rb_trie_count_nodes(VALUE self);
static recursive_levenshtein_search(trie_node* trie, VALUE rary, int* prev_line, int max_dist, char* word, int word_length);
// int print_arr(char c, int* arr, int len);


// ========================
// = function definitions =
// ========================

// instance methods
static VALUE rb_trie_get_key(VALUE self, VALUE key) {
	trie_node * root;
	trie_node * node;
	char * key_cstring;
	
	//Check_Type(key, T_STRING);
	key_cstring = StringValuePtr(key);

	Data_Get_Struct(self, trie_node, root);
	
	node = trie_node_for_key(root, key_cstring, false);
	if (node == NULL) return Qnil;
	return node->value;
}

static VALUE rb_trie_set_key_to_value(VALUE self, VALUE key, VALUE value) {
	trie_node * root;
	trie_node * node;
	char * key_cstring;
	
	//Check_Type(key, T_STRING);
	key_cstring = StringValuePtr(key);

	Data_Get_Struct(self, trie_node, root);

	node = trie_node_for_key(root, key_cstring, true);	
	node->value = value;
	
	return Qnil;
}

static uint mem_count = 0;

static void count_nodes_callback(void *n, VALUE accum) {
  trie_node *node = (trie_node*)n;
  // rb_big_plus(accum, rb_uint2big(sizeof(*node)));
  mem_count+=sizeof(*node);
}

static VALUE rb_trie_count_nodes(VALUE self) {
  trie_node *root;
  Data_Get_Struct(self, trie_node, root);
  VALUE accum = rb_uint2big(0);
  mem_count = 0;
  trie_traverse_with_context(root, accum, count_nodes_callback);
  return rb_uint2big(mem_count);
}

static VALUE rb_trie_undef_key(VALUE self, VALUE key) {
	trie_node * root, * node, * prev, * next;
	VALUE return_value;	
	char * key_cstring;
	int steps;
	int i;
	
	//Check_Type(key, T_STRING);
	key_cstring = StringValuePtr(key);
	
	Data_Get_Struct(self, trie_node, root);
	next = root;
	node = NULL;
	prev = NULL;

	steps = strlen(key_cstring);
	
	for (i = 0; i < steps; i++) {
		if (next == NULL) return Qnil;
		
		while(next->character != key_cstring[i]) {
			if (next == NULL) return Qnil;
			next = next->next_sibling;
		}
		prev = node;
		node = next;
		next = node->first_child;
	}
	
	return_value = node->value;
	node->value = Qnil;
	
	if (node->first_child == NULL) { //node has no children. we can delete it.
		if (prev == NULL)  {
			//printf("should delete root");
		} else if (prev->first_child == node) {
			prev->first_child = node->next_sibling;
			free(node);
		} else if (prev->next_sibling == node) {
			prev->next_sibling = node->next_sibling;
			free(node);
		}
	}
	
	return return_value;
}

// garbage collection and allocation
static void trie_mark_value(void * t) {	
	rb_gc_mark( ((trie_node *)t)->value );
}

static void rb_trie_mark(trie_node * t) {
	trie_traverse(t, trie_mark_value);
}

static void rb_trie_free(trie_node * t) {
	free_trie(t);
}

static VALUE rb_trie_allocate (VALUE klass) {
	trie_node * t = trie_new_node();

	return Data_Wrap_Struct(klass, rb_trie_mark, rb_trie_free, t);
}

// extension init
void Init_trie() {
	rb_cTrie = rb_define_class("Trie", rb_cObject);

	rb_define_alloc_func (rb_cTrie, rb_trie_allocate);

	int arg_count = 0;
	//rb_define_method(rb_cTrie, "inspect", rb_trie_inspect, arg_count);
	rb_define_method(rb_cTrie, "memory", rb_trie_count_nodes, arg_count);
	
	arg_count = 1;
	rb_define_method(rb_cTrie, "[]", rb_trie_get_key, arg_count);
	rb_define_method(rb_cTrie, "delete", rb_trie_undef_key, arg_count);
	rb_define_method(rb_cTrie, "children", rb_trie_find_children, arg_count);
	rb_define_method(rb_cTrie, "each", rb_trie_find_children_with_block, arg_count);

	arg_count = 2;
	rb_define_method(rb_cTrie, "[]=", rb_trie_set_key_to_value, arg_count);
	// trie.levenshtein_search(word, max_distance)
	rb_define_method(rb_cTrie, "levenshtein_search", rb_trie_levenshtein_search, arg_count);
}


// =======================
// = trie implementation =
// =======================

static trie_node * trie_node_for_key(trie_node * root, char * key, bool create_missing_nodes) {		
	int steps, i;
	trie_node * next, * node;

	steps = strlen(key);
	next = root;

	for (i = 0; i < steps; i++) {
		if (next == NULL) {
			if (create_missing_nodes) {
				node->first_child = trie_new_node();
				next = node->first_child;
			}
			else return NULL;
		}

		node = trie_sibling_for_char(next, key[i]);

		if (node == NULL) {
			if (create_missing_nodes) {
				node = trie_add_sibling_for_char(next, key[i]);
			}	
			else return NULL;
		}

		next = node->first_child;
	}	
		
	return node;    
}

static void trie_collect_values(void * t, VALUE rary) {	
	trie_node *node = (trie_node*)t;
	if (node->value != Qnil) {
		rb_ary_push(rary, node->value);
	}
}

static void trie_collect_values_with_yield(void * t) {	
	trie_node *node = (trie_node*)t;
	if (node->value != Qnil) {
		// rb_ary_push(rary, node->value);
		rb_yield(node->value);
	}
}

static VALUE rb_trie_find_children(VALUE self, VALUE key) {
	trie_node * root;
	trie_node * node;
	char * key_cstring;
	VALUE rary = rb_ary_new();

	key_cstring = StringValuePtr(key);
	Data_Get_Struct(self, trie_node, root);

	node = trie_node_for_key(root, key_cstring, false);
	
	if (node != NULL && node->value != Qnil) {
		rb_ary_push(rary, node->value);
	}
	
	if (node == NULL || node->first_child == NULL) return rary;

	trie_traverse_with_context(node->first_child, rary, trie_collect_values);
	return rary;
}


static VALUE rb_trie_find_children_with_block(VALUE self, VALUE key) {
	trie_node * root;
	trie_node * node;
	char * key_cstring;
	VALUE rary = rb_ary_new();

	key_cstring = StringValuePtr(key);
	Data_Get_Struct(self, trie_node, root);

	node = trie_node_for_key(root, key_cstring, false);
	
	if (node != NULL && node->value != Qnil) {
		rb_yield(node->value);
	}
	
	if (node == NULL || node->first_child == NULL) return rary;

	trie_traverse(node->first_child, trie_collect_values_with_yield);
	return rary;
}

static VALUE rb_trie_levenshtein_search(VALUE self, VALUE word, VALUE max_distance) {
  trie_node *root;
  trie_node *node;
  char *word_cstring;
  VALUE rary = rb_ary_new();
  int i=0;
  
  Data_Get_Struct(self, trie_node, root);
  
  word_cstring = StringValuePtr(word);

  int first_line[strlen(word_cstring) + 1];
  for(; i < strlen(word_cstring) + 1; i++) {
    first_line[i] = i;
  }
  // print_arr('R', first_line, strlen(word_cstring)+1);
  recursive_levenshtein_search(root->next_sibling, rary, first_line, FIX2INT(max_distance), word_cstring, strlen(word_cstring));

  return rary;
}

int minimum(int* numbers, int len) {
  int minValue = numbers[0];
  int i;
  for(i=1; i<len; i++) {
    if (numbers[i] < minValue) minValue = numbers[i];
  }
  return minValue;
}

int min3(int a, int b, int c) {
  int min = a;
  if (b < min) min = b;
  if (c < min) min = c;
  return min;
}

static recursive_levenshtein_search(trie_node* trie, VALUE rary, int* prev_line, int max_dist, char* word, int word_length) {
  int cur_line[word_length + 1];
  int i,j, insert_cost, replace_cost, delete_cost;
  VALUE carr;

  cur_line[0] = prev_line[0] + 1;

  for(i=1; i < word_length + 1; i++) {
    insert_cost = cur_line[i-1] + 1;
    delete_cost = prev_line[i] + 1;
    if (trie->character != word[i-1]) {
      replace_cost = prev_line[i-1] + 1;
    } else {
      replace_cost = prev_line[i-1];
    }
    cur_line[i] = min3(insert_cost, delete_cost, replace_cost);
  }


  if (cur_line[word_length] <= max_dist && trie->value != Qnil) {
    carr = rb_ary_new();
    rb_ary_push(carr, trie->value);
    rb_ary_push(carr, INT2FIX(cur_line[word_length]));
    rb_ary_push(rary, carr);
  }

  if (minimum(cur_line, word_length + 1) <= max_dist) {
    if (trie->first_child != NULL)
      recursive_levenshtein_search(trie->first_child, rary, cur_line, max_dist, word, word_length);
    if (trie->next_sibling != NULL)
      recursive_levenshtein_search(trie->next_sibling, rary, prev_line, max_dist, word, word_length);
  }
}

static trie_node * trie_sibling_for_char(trie_node * node, char ch) {
	while(true) {
		if (node == NULL) return NULL;

		if (node->character == ch) return node;

		node = node->next_sibling;
	}
	return node;
}

static trie_node * trie_add_sibling_for_char(trie_node * node, char ch) {
	trie_node * current_next;

	current_next = node->next_sibling;
	node->next_sibling = trie_new_node_with_char(ch);
	node->next_sibling->next_sibling = current_next;

	return node->next_sibling;
}

static trie_node * trie_new_node_with_char(char ch) {
	trie_node * trie;
	trie = malloc(sizeof(trie_node));
	trie->character = ch;
	trie->value = Qnil;
	trie->first_child = NULL;
	trie->next_sibling = NULL;
	return trie;
}

static trie_node * trie_new_node() {
	return trie_new_node_with_char('s'); //insert most common starting letter here.
}

static void trie_traverse(trie_node * trie, void (* lambda_func)(void *)) {
	if (trie->next_sibling != NULL) {
		trie_traverse(trie->next_sibling, lambda_func);
	}

	if (trie->first_child != NULL) {
		trie_traverse(trie->first_child, lambda_func);
	}

	lambda_func(trie);
}

static void trie_traverse_with_context(trie_node * trie, VALUE context, void (*lambda_func)(void *, VALUE)) {
	if (trie->next_sibling != NULL) {
		trie_traverse_with_context(trie->next_sibling, context, lambda_func);
	}

	if (trie->first_child != NULL) {
		trie_traverse_with_context(trie->first_child, context, lambda_func);
	}

	lambda_func(trie, context);
}

static void free_trie(trie_node * trie) {
	trie_traverse(trie, free);
}