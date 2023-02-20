#define NODETYPES_IMPLEMENTATION
#include <stdlib.h>
#include <assert.h>
#include <vslc.h>

/* Global root for parse tree and abstract syntax tree */
node_t *root;

// Tasks
static void node_print(node_t *node, int nesting);
static void node_finalize(node_t *discard);
static void destroy_subtree(node_t *discard);
static node_t *simplify_tree(node_t *node);
static node_t *constant_fold_expression(node_t *node);
static node_t *replace_for_statement(node_t *for_node);

typedef enum
{
    PLUS,
    MINUS,
    MULT,
    DIV
} operation;

/* External interface */
void print_syntax_tree()
{
    if (getenv("GRAPHVIZ_OUTPUT") != NULL)
        graphviz_node_print(root);
    else
        node_print(root, 0);
}

void simplify_syntax_tree(void)
{
    root = simplify_tree(root);
}

void destroy_syntax_tree(void)
{
    destroy_subtree(root);
    root = NULL;
}

/* Initialize a node with type, data, and children */
void node_init(node_t *nd, node_type_t type, void *data, uint64_t n_children, ...)
{
    va_list child_list;
    *nd = (node_t){
        .type = type,
        .data = data,
        .symbol = NULL,
        .n_children = n_children,
        .children = (node_t **)malloc(n_children * sizeof(node_t *))};
    va_start(child_list, n_children);
    for (uint64_t i = 0; i < n_children; i++)
        nd->children[i] = va_arg(child_list, node_t *);
    va_end(child_list);
}

/* Inner workings */
/* Prints out the given node and all its children recursively */
static void node_print(node_t *node, int nesting)
{
    if (node != NULL)
    {
        printf("%*s%s", nesting, "", node_strings[node->type]);
        if (node->type == IDENTIFIER_DATA ||
            node->type == STRING_DATA ||
            node->type == EXPRESSION ||
            node->type == RELATION)
            printf("(%s)", (char *)node->data);
        else if (node->type == NUMBER_DATA)
            printf("(%ld)", *((int64_t *)node->data));
        putchar('\n');
        for (int64_t i = 0; i < node->n_children; i++)
            node_print(node->children[i], nesting + 1);
    }
    else
        printf("%*s%p\n", nesting, "", node);
}

/* Frees the memory owned by the given node, but does not touch its children */
static void node_finalize(node_t *discard)
{
    if (discard != NULL)
    {
        free(discard->data);
        free(discard->children);
        free(discard);
    }
}

/* Recursively frees the memory owned by the given node, and all its children */
static void destroy_subtree(node_t *discard)
{
    if (discard != NULL)
    {
        for (uint64_t i = 0; i < discard->n_children; i++)
            destroy_subtree(discard->children[i]);
        node_finalize(discard);
    }
}

static node_t *simplify_unary(node_t *node)
{
    node_t *child = node->children[0];
    node_finalize(node);
    return child;
}

static node_t *simplify_list(node_t *node)
{
    if (node->n_children == 1)
        return node;
    uint64_t num_grandchildren = node->children[0]->n_children;
    node_t *discard_child = node->children[0];
    node_t *keep_child = node->children[1];
    node->children = realloc(node->children, (num_grandchildren + 1) * sizeof(node_t *));
    node->n_children = num_grandchildren + 1;
    memcpy(node->children, discard_child->children, num_grandchildren * sizeof(node_t *));
    node->children[num_grandchildren] = keep_child;
    node_finalize(discard_child);
    return node;
}

static node_t *simplify_optional(node_t *node)
{
    if (node->n_children == 0)
        return node;
    node->children = realloc(node->children, (node->children[0]->n_children) * sizeof(node_t *));
    node->n_children = node->children[0]->n_children;
    node_t *discard = node->children[0];
    memcpy(node->children, node->children[0]->children, (node->children[0]->n_children) * sizeof(node_t *));
    node_finalize(discard);
    return node;
}

static node_t *simplify_binary_expression(node_t *node, operation op)
{
    if (node->children[0]->type != NUMBER_DATA || node->children[1]->type != NUMBER_DATA)
        return node;
    node_t *number_data_node = malloc(sizeof(node_t));
    int64_t *n = malloc(sizeof(int64_t));

    int64_t data1, data2;
    data1 = (int64_t) * (int64_t *)node->children[0]->data;
    data2 = (int64_t) * (int64_t *)node->children[1]->data;
    switch (op)
    {
    case PLUS:
        *n = data1 + data2;
        break;
    case MINUS:
        *n = data1 - data2;
        break;
    case MULT:
        *n = data1 * data2;
        break;
    case DIV:
        *n = data1 / data2;
        break;
    default:
        break;
    }
    node_init(number_data_node, NUMBER_DATA, n, 0);
    destroy_subtree(node);
    return number_data_node;
}
static node_t *simplify_unary_expression(node_t *node, operation op)
{
    if (node->children[0]->type != NUMBER_DATA)
        return node;
    int64_t data = (int64_t) * (int64_t *)node->children[0]->data;
    node_t *number_data_node = malloc(sizeof(node_t));
    int64_t *n = malloc(sizeof(int64_t));

    switch (op)
    {
    case MINUS:
        *n = -data;
    default:
        break;
    }
    node_init(number_data_node, NUMBER_DATA, n, 0);
    destroy_subtree(node);
    return number_data_node;
}

/* Recursive function to convert a parse tree into an abstract syntax tree */
static node_t *simplify_tree(node_t *node)
{
    if (node == NULL)
        return NULL;

    // Simplify everything is the node's subtree before proceeding
    for (uint64_t i = 0; i < node->n_children; i++)
        node->children[i] = simplify_tree(node->children[i]);

    switch (node->type)
    {
    // These nodes only have one child, and carry no semantic value.
    case PROGRAM:
        return simplify_unary(node);
    case GLOBAL:
        return simplify_unary(node);
    case STATEMENT:
        return simplify_unary(node);
    case PRINT_ITEM:
        return simplify_unary(node);

    // Non empty
    case GLOBAL_LIST:
        return simplify_list(node);
    case VARIABLE_LIST:
        return simplify_list(node);
    case DECLARATION_LIST:
        return simplify_list(node);
    case STATEMENT_LIST:
        return simplify_list(node);
    case PRINT_LIST:
        return simplify_list(node);
    case EXPRESSION_LIST:
        return simplify_list(node);

    // // Possibly empty
    case PARAMETER_LIST:
        return simplify_optional(node);
    case ARGUMENT_LIST:
        return simplify_optional(node);

    // Do contstant folding, if possible
    // Also prunes expressions that are just wrapping atomic expressions
    case EXPRESSION:
        return constant_fold_expression(node);

    case FOR_STATEMENT:
        return replace_for_statement(node);

    default:
        break;
    }

    return node;
}

// Helper macros for manually building an AST
#define NODE(variable_name, ...)                    \
    node_t *variable_name = malloc(sizeof(node_t)); \
    node_init(variable_name, __VA_ARGS__)
// After an IDENTIFIER_NODE has been added to the tree, it can't be added again
// This macro replaces the given variable with a new node, containting a copy of the data
#define DUPLICATE_VARIABLE(variable)                         \
    do                                                       \
    {                                                        \
        char *identifier = strdup(variable->data);           \
        variable = malloc(sizeof(node_t));                   \
        node_init(variable, IDENTIFIER_DATA, identifier, 0); \
    } while (false)
#define FOR_END_VARIABLE "__FOR_END__"

static node_t *constant_fold_expression(node_t *node)
{
    assert(node->type == EXPRESSION);

    if (node->data == NULL)
        return simplify_unary(node);
    if (node->n_children == 1 && !strcmp(node->data, "-"))
        return simplify_unary_expression(node, MINUS);
    if (!strcmp(node->data, "+"))
        return simplify_binary_expression(node, PLUS);
    if (!strcmp(node->data, "-"))
        return simplify_binary_expression(node, MINUS);
    if (!strcmp(node->data, "*"))
        return simplify_binary_expression(node, MULT);
    if (!strcmp(node->data, "/"))
        return simplify_binary_expression(node, DIV);

    return node;
}

// Replaces the FOR_STATEMENT with a BLOCK.
static node_t *replace_for_statement(node_t *for_node)
{
    assert(for_node->type == FOR_STATEMENT);

    // extract child nodes from the FOR_STATEMENT
    node_t *variable = for_node->children[0];
    node_t *start_value = for_node->children[1];
    node_t *end_value = for_node->children[2];
    node_t *body = for_node->children[3];

    NODE(end_variable, IDENTIFIER_DATA, strdup(FOR_END_VARIABLE), 0);
    NODE(variable_list, VARIABLE_LIST, NULL, 2, variable, end_variable);
    NODE(declaration, DECLARATION, NULL, 1, variable_list);
    NODE(declaration_list, DECLARATION_LIST, NULL, 1, declaration);

    DUPLICATE_VARIABLE(variable);
    NODE(init_assignment, ASSIGNMENT_STATEMENT, NULL, 2, variable, start_value);
    DUPLICATE_VARIABLE(end_variable);
    NODE(end_assignment, ASSIGNMENT_STATEMENT, NULL, 2, end_variable, end_value);

    uint64_t *one = malloc(sizeof(uint64_t));
    *one = 1;
    NODE(one_data, NUMBER_DATA, one, 0);

    DUPLICATE_VARIABLE(variable);
    NODE(increment_expression, EXPRESSION, strdup("+"), 2, variable, one_data);
    DUPLICATE_VARIABLE(variable);
    NODE(increment, ASSIGNMENT_STATEMENT, NULL, 2, variable, increment_expression);

    NODE(body_list, STATEMENT_LIST, NULL, 2, body, increment);
    NODE(while_block, BLOCK, NULL, 1, body_list);

    DUPLICATE_VARIABLE(variable);
    DUPLICATE_VARIABLE(end_variable);
    NODE(relation, RELATION, strdup("<"), 2, variable, end_variable);
    NODE(while_loop, WHILE_STATEMENT, NULL, 2, relation, while_block);

    NODE(statement_list, STATEMENT_LIST, NULL, 3, init_assignment, end_assignment, while_loop);
    NODE(block, BLOCK, NULL, 2, declaration_list, statement_list);

    node_finalize(for_node);
    return block;
}
