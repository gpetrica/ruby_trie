/*
 *  levenshtein_distance.h
 *  otrie
 *
 *  Created by Petrica Ghiurca on 18.03.2011.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

#include "otrie2.h"

typedef void (*levenshtein_distance_callback)(Node* node, int length);
void levenshtein_distance(Node* trie, const char* word, int max_distance, levenshtein_distance_callback cb);