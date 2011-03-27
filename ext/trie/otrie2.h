/*
 *  otrie2.h
 *  otrie
 *
 *  Created by Petrica Ghiurca on 18.03.2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */
#include <ruby.h>

#define bool int
#define true 1
#define false 0

#ifndef TRIE_NODE
#define TRIE_NODE

typedef struct trie_node {
	char *data;
	struct trie_node* first_child;
	struct trie_node* next_sibling;
	VALUE value;
} Node;

typedef struct pos_struct {
	Node *node;
	int offset;
} Pos;

typedef void (*node_iterator)(Node* node, VALUE context);

Node* new_node();
void free_node(Node*);
void node_insert(Node* node, const char* string, const VALUE value);
Node* node_find(Node* this, const char* string);
Node* partial_node_find(Node* this, const char* string);

Pos* new_pos(Node *node, int offset);
Node* pos_find_or_create_child(Pos* this, const char* string, bool down, bool insert);
void pos_next(Pos *this, const char* string, bool);
void node_visit(Node* this, node_iterator func, VALUE context);
#endif