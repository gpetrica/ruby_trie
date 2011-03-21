/*
 *  otrie2.c
 *  otrie
 *
 *  Created by Petrica Ghiurca on 18.03.2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>  /* for malloc, free */
#include <string.h>  /* for memcmp, memmove */
#include <stdio.h>

#include "otrie2.h"

Node* new_node() {
	Node *node = malloc(sizeof(Node));
	memset(node, 0, sizeof(Node));
	return node;
}

Node* new_node_string_len(const char* string, const int len) {
	Node *node = new_node();
	node->data = malloc(len+1);
	strncpy(node->data, string, len);
	node->data[len] = 0;
	return node;
}

Node* new_node_string(const char* string) {
	Node *node = new_node();
	int len = strlen(string);
	node->data = malloc(len+1);
	strcpy(node->data, string);
	return node;
}

void node_update_data(Node* node, const char* string, int len) {
	char* new_data = malloc(len+1);
	strncpy(new_data, string, len);
	new_data[len] = 0;
	free(node->data);
	node->data = new_data;
}

void free_node(Node *node) {
	if (node->first_child) free_node(node->first_child);
	if (node->next_sibling) free_node(node->next_sibling);
	free(node->data);
	free(node);
}

void node_insert(Node* node, const char* string, const VALUE value) {
	int len = strlen(string);
	Pos *cur = new_pos(node, 0);
	int i=0;
	for (; i<len; i++) {
		pos_next(cur, string + i, true);
	}
	cur->node->value = value;
}

Node* node_find(Node* this, const char* string) {
	int len = strlen(string);
	Pos *cur = new_pos(this, 0);
	int i=0;
	for(; i<len; i++) {
		pos_next(cur, string + i, false);
		if (cur->node == NULL) { return NULL; }
	}
	if (strlen(cur->node->data) == cur->offset + 1)
		return cur->node;
	return NULL;
}

Pos* new_pos(Node *node, int offset) {
	Pos *pos = malloc(sizeof(Pos));
	pos->node = node;
	pos->offset = offset;
	return pos;
}

Node* pos_find_or_create_child(Pos* this, const char* string, bool insert) {
	Node *child = this->node->first_child;
	Node *last_child = NULL;
	
	while(child != NULL && *child->data != *string) { 
		last_child = child;
		child = child -> next_sibling; 
	}
	if (child == NULL && insert) {
		child = new_node_string(string);
		if (this->node -> first_child != NULL)  {
			last_child -> next_sibling = child;
		} else {
			this -> node -> first_child = child;
		}
	}
	return child;	
}

void pos_next(Pos *this, const char* string, bool insert) {
	if (this -> node -> data == NULL) {
		this->node = pos_find_or_create_child(this, string, insert);
		this->offset = 0;
		return;
	}
	
	int len = strlen(this->node->data);
	if (this -> offset + 1 < len) {
		if (this -> node->data[this -> offset + 1] == string[0]) {
			this -> offset++;
			return;
		} else {
			// split paths 
			// - new child node with old partial content
			// - new child node with new content			
			if (insert) {
				Node *splitChild = new_node_string(this->node->data + this->offset + 1);
				Node *newChild = new_node_string(string); 
				node_update_data(this->node, this->node->data, this->offset + 1);
				splitChild -> next_sibling = newChild;
				this->node -> first_child = splitChild;
				
				this->node = newChild;
				this->offset = 0;
			} else {
				this -> node = NULL;
				this -> offset = 0;
			}
		}
	} else {
		// reached end of data... find a child
		this->node = pos_find_or_create_child(this, string, insert);
		this->offset = 0;
		return;
	}
}

void node_visit(Node* this, node_iterator func, VALUE context) {
	if (this->next_sibling != NULL) {
		 node_visit(this->next_sibling, func, context);
	}
	if (this->first_child != NULL) {
		node_visit(this->first_child, func, context);
	}
	
	func(this, context);
}