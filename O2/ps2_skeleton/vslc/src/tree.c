#define NODETYPES_IMPLEMENTATION
#include <vslc.h>

/* Global root for parse tree and abstract syntax tree */
node_t *root;

// Tasks
static void node_print ( node_t *node, int nesting );
static void node_finalize ( node_t *discard );
static void destroy_subtree ( node_t *discard );

/* External interface */
void print_syntax_tree ( void )
{
    node_print ( root, 0 );
}

void destroy_syntax_tree ( void )
{
    // This will destroy the enitre tree by deleting root and all its children
    destroy_subtree ( root );
}

/* Inner workings */
/* Prints out the given node and all its children recursively */
static void node_print ( node_t *node, int nesting )
{
    if ( node != NULL )
    {
        printf ( "%*s%s", nesting, "", node_strings[node->type] );
        if ( node->type == IDENTIFIER_DATA ||
             node->type == STRING_DATA ||
             node->type == EXPRESSION )
            printf ( "(%s)", (char *) node->data );
        else if ( node->type == NUMBER_DATA )
            printf ( "(%ld)", *((int64_t *)node->data) );
        putchar ( '\n' );
        for ( int64_t i=0; i<node->n_children; i++ )
            node_print ( node->children[i], nesting+1 );
    }
    else
        printf ( "%*s%p\n", nesting, "", node );
}

/* Initialize a node with type, data, and children */ 
void node_init ( node_t *nd, node_type_t type, void *data, uint64_t n_children, ... )
{

   nd->type = type;
   nd->data = data;
   nd->n_children = n_children;
   nd->symbol = NULL;
   nd->children = malloc(n_children*sizeof(node_t*));

   va_list valist;
   va_start(valist, n_children);
   for (uint64_t i = 0; i < n_children; i++){
    node_t* child = va_arg(valist, node_t*);
    nd->children[i] = child;
   }
   va_end(valist);
   
}

/* Frees the memory owned by the given node, but does not touch its children */
static void node_finalize ( node_t *discard )
{
   free(discard->symbol);
   free(discard->data);
   free(discard->children);
   free(discard);
}

/* Frees the memory owned by the given node, and all its children */
static void destroy_subtree ( node_t *discard )
{
    for (int c = 0; c< discard->n_children; c++){
        destroy_subtree(discard->children[c]);
    }
    node_finalize(discard);
}
