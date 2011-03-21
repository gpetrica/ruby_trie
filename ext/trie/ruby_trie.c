/*
 *  ruby_trie.c
 *  otrie
 *
 *  Created by Petrica Ghiurca on 18.03.2011.
 *  Copyright 2011 Petrica Ghiurca. All rights reserved.
 *
 */

#include <ruby.h>
#include <stdlib.h>  /* for malloc, free */
#include <string.h>  /* for memcmp, memmove */
#include "otrie2.h"

static VALUE rb_cTrie;

static void count_nodes_callback(Node *trie, VALUE accum);
static VALUE rb_trie_count_nodes(VALUE self);
static VALUE rb_trie_allocate(VALUE klass);
static VALUE rb_trie_get_key(VALUE self, VALUE key);
static void trie_mark_value(Node*, VALUE);
static void rb_trie_mark(Node* t);
static void rb_trie_free(Node * t);
void tree_collect_values(Node *node, VALUE rary);
static VALUE rb_trie_find_children(VALUE self, VALUE key);
static void trie_collect_values_with_yield(Node * node, VALUE context);
static VALUE rb_trie_find_children_with_block(VALUE self, VALUE key);
static VALUE rb_trie_set_key_to_value(VALUE self, VALUE key, VALUE value);
static VALUE rb_trie_levenshtein_search(VALUE self, VALUE word, VALUE max_distance);

// extension init
void Init_trie() {
	rb_cTrie = rb_define_class("Trie", rb_cObject);
	
	rb_define_alloc_func(rb_cTrie, rb_trie_allocate);
	
	int arg_count = 0;
	//rb_define_method(rb_cTrie, "inspect", rb_trie_inspect, arg_count);
	rb_define_method(rb_cTrie, "memory", rb_trie_count_nodes, arg_count);
	
	arg_count = 1;
	rb_define_method(rb_cTrie, "[]", rb_trie_get_key, arg_count);
	// rb_define_method(rb_cTrie, "delete", rb_trie_undef_key, arg_count);
	rb_define_method(rb_cTrie, "children", rb_trie_find_children, arg_count);
	rb_define_method(rb_cTrie, "each", rb_trie_find_children_with_block, arg_count);
	
	arg_count = 2;
	rb_define_method(rb_cTrie, "[]=", rb_trie_set_key_to_value, arg_count);
	// trie.levenshtein_search(word, max_distance)
	rb_define_method(rb_cTrie, "levenshtein_search", rb_trie_levenshtein_search, arg_count);
}

static int total_memory; 
static void count_nodes_callback(Node *trie, VALUE accum) {
	int len = 0;
	if (trie->data) len = strlen(trie->data);	
	rb_big_plus(accum, rb_uint2big(len + sizeof(Node)));
	total_memory += len + sizeof(Node);
}

static VALUE rb_trie_count_nodes(VALUE self) {
	Node *root;
	Data_Get_Struct(self, Node, root);
	VALUE accum = rb_uint2big(0);
	total_memory = 0;
	node_visit(root, count_nodes_callback, accum);
	//return accum;
	rb_uint2big(total_memory);
}

static VALUE rb_trie_allocate(VALUE klass) {
	Node * t = new_node();
	return Data_Wrap_Struct(klass, rb_trie_mark, rb_trie_free, t);
}
				
static VALUE rb_trie_get_key(VALUE self, VALUE key) {
	Node * root;
	Node * node;
	char * key_cstring;
	
	Check_Type(key, T_STRING);
	key_cstring = StringValuePtr(key);
	
	Data_Get_Struct(self, Node, root);
	
	node = node_find(root, key_cstring);
	if (node == NULL) return Qnil;
	return node->value;
}
				

static void trie_mark_value(Node * t, VALUE context) {	
	rb_gc_mark( t->value );
}

static void rb_trie_mark(Node* t) {
	node_visit(t, trie_mark_value, Qnil);
}

static void rb_trie_free(Node * t) {
	free_node(t);
}

void tree_collect_values(Node *node, VALUE rary) {
	if (node->value != Qnil) {
		rb_ary_push(rary, node->value);
	}
}
				
static VALUE rb_trie_find_children(VALUE self, VALUE key) {
	Node * root;
	Node * node;
	char * key_cstring;
	VALUE rary = rb_ary_new();
	
	key_cstring = StringValuePtr(key);
	Data_Get_Struct(self, Node, root);
	
	node = node_find(root, key_cstring);
	
	if (node != NULL && node->value != Qnil) {
		rb_ary_push(rary, node->value);
	}
	
	if (node == NULL || node->first_child == NULL) return rary;
	
	node_visit(node->first_child, tree_collect_values, rary);
	return rary;
}

static void trie_collect_values_with_yield(Node * node, VALUE context) {
	if (node->value != Qnil) {
		rb_yield(node->value);
	}
}
				
static VALUE rb_trie_find_children_with_block(VALUE self, VALUE key) {
	Node * root;
	Node * node;
	char * key_cstring;
	VALUE rary = rb_ary_new();
	
	key_cstring = StringValuePtr(key);
	Data_Get_Struct(self, Node, root);
	
	node = node_find(root, key_cstring);
	
	if (node != NULL && node->value != Qnil) {
		rb_yield(node->value);
	}
	
	if (node == NULL || node->first_child == NULL) return rary;
	
	node_visit(node->first_child, trie_collect_values_with_yield, Qnil);
	return rary;
}


static VALUE rb_trie_set_key_to_value(VALUE self, VALUE key, VALUE value) {
	Node * root;
	Node * node;
	char * key_cstring;
	
	Check_Type(key, T_STRING);
	key_cstring = StringValuePtr(key);
	
	Data_Get_Struct(self, Node, root);
	
	node_insert(root, key_cstring, value);
	return Qnil;
}

void rb_levensthtein_cb(Node* node, int distance) {
	if (node->value != Qnil)
		rb_yield(node->value);
}

static VALUE rb_trie_levenshtein_search(VALUE self, VALUE word, VALUE max_distance) {
	Node *root;
	char *word_cstring;

	Data_Get_Struct(self, Node, root);
	
	word_cstring = StringValuePtr(word);
	
	levenshtein_distance(root, word_cstring, FIX2INT(max_distance), rb_levensthtein_cb);
		
	return Qnil;	
}