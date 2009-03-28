#include <stdlib.h>
#include "naming.h"
#include <stdio.h>
#include <assert.h>

static void Naming_collect_build(void *value, NamingCollect *results)
{
    if(results->length < 100) {
        results->values[results->length++] = value;
    }
}

NamingCollect Naming_collect(tnode *root, const char *s, int len)
{
    NamingCollect results = {.length = 0, .values = NULL};
    tnode *p = root;
    int i = 0;
    results.values = calloc(100, sizeof(void *));

    // first we get to where we match the prefix
    while(i < len && p) {
        if (s[i] < p->splitchar) {
            p = p->low; 
        } else if (s[i] == p->splitchar) {
            i++;
            p = p->equal; 
        } else {
            p = p->high; 
        }
    }

    if(p) {
        // we found node matching this prefix, so traverse and collect
        Naming_traverse(p, (Naming_traverse_cb)Naming_collect_build, &results);
    }

    return results;
}

void *Naming_search(tnode *root, const char *s, int len)
{
    tnode *p = root;
    int i = 0;

    while(i < len && p) {
        if (s[i] < p->splitchar) {
            p = p->low; 
        } else if (s[i] == p->splitchar) {
            i++;
            if(i < len) p = p->equal; 
        } else {
            p = p->high; 
        }
    }

    if(p) {
        return p->value;
    } else {
        return NULL; 
    }
}

tnode *Naming_insert(tnode *p, const char *s, int len, void *value)
{
    if (p == NULL) { 
        p = (tnode *) calloc(1,sizeof(tnode)); 
        p->splitchar = *s; 
    }

    if (*s < p->splitchar) {
        p->low = Naming_insert(p->low, s, len, value); 
    } else if (*s == p->splitchar) { 
        if (len > 1) {
            // not done yet, keep going but one less
            p->equal = Naming_insert(p->equal, s+1, len - 1, value);
        } else {
            p->value = value;
        }
    } else {
        p->high = Naming_insert(p->high, s, len, value);
    }

    return p; 
}

void Naming_traverse(tnode *p, Naming_traverse_cb cb, void *data)
{
    if (!p) return; 
    if(p->low) Naming_traverse(p->low, cb, data);

    if (p->equal) {
        Naming_traverse(p->equal, cb, data); 
    }

    if(p->high) Naming_traverse(p->high, cb, data); 

    if(p->value) cb(p->value, data);
}

void Naming_destroy(tnode *p, Naming_destroy_cb cb)
{
    if (!p) return; 
    if(p->low) Naming_destroy(p->low, cb); 

    if (p->equal) {
        Naming_destroy(p->equal, cb); 
    }

    if(p->high) Naming_destroy(p->high, cb); 
   
    if(p->value) cb(p->value);
    if(p) free(p);
}
