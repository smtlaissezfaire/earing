#ifndef naming_h
#define naming_h

typedef struct tnode { 
    char splitchar; 
    struct tnode *low;
    struct tnode *equal;
    struct tnode *high; 
    void *value;
} tnode; 

typedef struct NamingCollect {
    size_t length;
    void **values;
} NamingCollect;

typedef void (*Naming_traverse_cb)(void *value, void *data);
typedef void (*Naming_destroy_cb)(void *value);

void *Naming_search(tnode *root, const char *s, int len);
tnode *Naming_insert(tnode *p, const char *s, int len, void *value);
void Naming_traverse(tnode *p, Naming_traverse_cb cb, void *data);
void Naming_destroy(tnode *p, Naming_destroy_cb cb);

NamingCollect Naming_collect(tnode *root, const char *s, int len);

#endif
