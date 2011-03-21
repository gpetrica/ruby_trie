/*
 *  levenshtein_distance.c
 *  otrie
 *
 *  Created by Petrica Ghiurca on 18.03.2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdlib.h>
#include <string.h>  /* for memcmp, memmove */
#include <stdio.h>

#include "otrie2.h"
#include "levenshtein_distance.h"


void recursive_levenshtein_search(Node* trie, int node_offset, levenshtein_distance_callback cb, int* prev_line, int max_dist, const char* word, int word_length);

void levenshtein_distance(Node* trie, const char* word, int max_distance, levenshtein_distance_callback cb) {
	int first_line[strlen(word) + 1];
	int i=0;
	for(; i < strlen(word) + 1; i++) {
		first_line[i] = i;
	}
	recursive_levenshtein_search(trie->first_child, 0, cb, first_line, max_distance, word, strlen(word));
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

void recursive_levenshtein_search(Node* trie, int node_offset, levenshtein_distance_callback cb, int* prev_line, int max_dist, const char* word, int word_length) {
	int cur_line[word_length + 1];
	int i, insert_cost, replace_cost, delete_cost;
	
	cur_line[0] = prev_line[0] + 1;
	
	for(i=1; i < word_length + 1; i++) {
		insert_cost = cur_line[i-1] + 1;
		delete_cost = prev_line[i] + 1;
		if (trie->data[node_offset] != word[i-1]) {
			replace_cost = prev_line[i-1] + 1;
		} else {
			replace_cost = prev_line[i-1];
		}
		cur_line[i] = min3(insert_cost, delete_cost, replace_cost);
	}
	
	
	if (cur_line[word_length] <= max_dist && (strlen(trie->data) == (node_offset + 1)) && trie->value != Qnil) {
		cb(trie, cur_line[word_length]);
	}
	
	if (minimum(cur_line, word_length + 1) <= max_dist) {
		if (strlen(trie->data) > (node_offset + 1)) 
			recursive_levenshtein_search(trie, node_offset + 1, cb, cur_line, max_dist, word, word_length);
		if (trie->first_child != NULL)
			recursive_levenshtein_search(trie->first_child, 0, cb, cur_line, max_dist, word, word_length);
		if (trie->next_sibling != NULL)
			recursive_levenshtein_search(trie->next_sibling, 0, cb, prev_line, max_dist, word, word_length);
	}
	
}