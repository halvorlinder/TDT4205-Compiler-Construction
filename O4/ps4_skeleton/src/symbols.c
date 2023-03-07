#include <vslc.h>
#include "assert.h"

/* Global symbol table and string list */
symbol_table_t *global_symbols;
char **string_list;
size_t string_list_len;
size_t string_list_capacity;

static void find_globals(void);
static void bind_names(symbol_table_t *local_symbols, node_t *root);
static void print_symbol_table(symbol_table_t *table, int nesting);
static void destroy_symbol_tables(void);
static void destroy_symbol_tables_internal(symbol_table_t * table);

static size_t add_string(char *string);
static void print_string_list(void);
static void destroy_string_list(void);

/* External interface */

/* Creates a global symbol table, and local symbol tables for each function.
 * While building the symbol tables:
 *  - All usages of symbols are bound to their symbol table entries.
 *  - All strings are entered into the string_list
 */
void create_tables(void)
{
    find_globals();
    for (int i = 0; i < global_symbols->n_symbols; i++)
    {
        symbol_t *global = global_symbols->symbols[i];
        if (global->type == SYMBOL_FUNCTION)
        {
            bind_names(global->function_symtable, global->node->children[2]);
        }
    }
}

/* Prints the global symbol table, and the local symbol tables for each function.
 * Also prints the global string list.
 * Finally prints out the AST again, with bound symbols.
 */
void print_tables(void)
{
    print_symbol_table(global_symbols, 0);
    printf("\n == STRING LIST == \n");
    print_string_list();
    printf("\n == BOUND SYNTAX TREE == \n");
    print_syntax_tree();
}

/* Destroys all symbol tables and the global string list */
void destroy_tables(void)
{
    destroy_symbol_tables();
    destroy_string_list();
}

static symtype_t get_symtype(node_type_t node_type)
{
    switch (node_type)
    {
    case DECLARATION:
        return SYMBOL_GLOBAL_VAR;
    case ARRAY_DECLARATION:
        return SYMBOL_GLOBAL_ARRAY;
    case FUNCTION:
        return SYMBOL_FUNCTION;
    default:
        return -1;
    }
}

static symbol_table_t *init_function_symtable(node_t *param_list_node)
{
    symbol_table_t *table = symbol_table_init();
    for (int i = 0; i < param_list_node->n_children; i++)
    {
        symbol_t *sym = malloc(sizeof(symbol_t));
        *sym = (symbol_t){
            param_list_node->children[i]->data,
            SYMBOL_PARAMETER,
            param_list_node->children[i],
            0,
            table};
        symbol_table_insert(table, sym);
    }
    return table;
}

/* Internal matters */

/* Goes through all global declarations in the syntax tree, adding them to the global symbol table.
 * When adding functions, local symbol tables are created, and symbols for the functions parameters are added.
 */
static void find_globals(void)
{
    global_symbols = symbol_table_init();
    for (int i = 0; i < root->n_children; i++)
    {
        for (int j = 0; j < root->children[i]->n_children; j++)
        {
            if (root->children[i]->children[j]->type == IDENTIFIER_DATA)
            {
                symtype_t symtype = get_symtype(root->children[i]->type);
                symbol_table_t *function_symtable = symtype == SYMBOL_FUNCTION ? init_function_symtable(root->children[i]->children[1]) : NULL;
                symbol_t *sym = malloc(sizeof(symbol_t));
                *sym = (symbol_t){
                    root->children[i]->children[j]->data,
                    symtype,
                    root->children[i],
                    0,
                    function_symtable};
                symbol_table_insert(global_symbols, sym);
            }
        }
    }
}

static void bind_children(symbol_table_t *local_symbols, node_t *node)
{
    for (int i = 0; i < node->n_children; i++)
        bind_names(local_symbols, node->children[i]);
}

/* A recursive function that traverses the body of a function, and:
 *  - Adds variable declarations to the function's local symbol table.
 *  - Pushes and pops local variable scopes when entering blocks.
 *  - Binds identifiers to the symbol it references.
 *  - Inserts STRING_DATA nodes' data into the global string list, and replaces it with its list position.
 */
static void bind_names(symbol_table_t *local_symbols, node_t *node)
{
    switch (node->type)
    {
    case BLOCK:
    {
        symbol_hashmap_t *new_hm = symbol_hashmap_init();
        new_hm->backup = local_symbols->hashmap;
        local_symbols->hashmap = new_hm;
        bind_children(local_symbols, node);
        local_symbols->hashmap = new_hm->backup;
        symbol_hashmap_destroy(new_hm);
        return;
    }
    case DECLARATION:
    {
        for(int j = 0; j<node->n_children; j++)
        {
            node_t* child = node->children[j];
            symbol_t* sym = malloc(sizeof(symbol_t));
            *sym = (symbol_t){
                child->data,
                SYMBOL_LOCAL_VAR,
                child,
                0,
                local_symbols};
            symbol_table_insert(local_symbols, sym);
        }
        return;
    }
    case IDENTIFIER_DATA:
    {
        symbol_t *sym = symbol_hashmap_lookup(local_symbols->hashmap, node->data);
        if(sym==NULL)
            sym = symbol_hashmap_lookup(global_symbols->hashmap, node->data);
        node->symbol = sym;
        return;
    }
    case STRING_DATA:
    {
        uint64_t *index = malloc(sizeof(uint64_t));
        *index = add_string(node->data);
        node->data = index;
        return;
    }
    }
    bind_children(local_symbols, node);
}

static inline void indent(int n)
{
    for (int i = 0; i < n; i++)
        putchar('\t');
}
/* Prints the given symbol table, with sequence number, symbol names and types.
 * When printing function symbols, its local symbol table is recursively printed, with indentation.
 */
static void print_symbol_table(symbol_table_t *table, int nesting)
{
    if (table == NULL)
        return;
    for (int i = 0; i < table->n_symbols; i++)
    {
        indent(nesting);
        printf("%d: %s(%s)\n", i, SYMBOL_TYPE_NAMES[table->symbols[i]->type], table->symbols[i]->name);
        if (nesting == 0)
            print_symbol_table(table->symbols[i]->function_symtable, 1);
    }
}

/* Frees up the memory used by the global symbol table, all local symbol tables, and their symbols */
static void destroy_symbol_tables(void)
{
    destroy_symbol_tables_internal(global_symbols);
}

static void destroy_symbol_tables_internal(symbol_table_t * table)
{ 
    for (int i = 0; i < table->n_symbols; i++){
        symbol_t* symbol = table->symbols[i];
        if (symbol->type==SYMBOL_FUNCTION){
            destroy_symbol_tables_internal(symbol->function_symtable);
        }
        free(symbol);
    }
    symbol_hashmap_destroy(table->hashmap);
    free(table->symbols);
    free(table);
 }

/* Adds the given string to the global string list, resizing if needed.
 * Takes ownership of the string, and returns its position in the string list.
 */
static size_t add_string(char *string)
{
    if (string_list_capacity == string_list_len)
    {
        uint64_t new_capacity = string_list_capacity == 0 ? 1 : string_list_capacity * 2;
        string_list = realloc(string_list, new_capacity * sizeof(char *));
        string_list_capacity = new_capacity;
    }
    string_list[string_list_len] = string;
    string_list_len++;
    return string_list_len - 1;
}

/* Prints all strings added to the global string list */
static void print_string_list(void)
{
    for (int i = 0; i < string_list_len; i++)
    {
        printf("%d: %s\n", i, string_list[i]);
    }
}

/* Frees all strings in the global string list, and the string list itself */
static void destroy_string_list(void)
{
    for (int i = 0; i<string_list_len; i++){
        free(string_list[i]);
    }
    free(string_list);
}
