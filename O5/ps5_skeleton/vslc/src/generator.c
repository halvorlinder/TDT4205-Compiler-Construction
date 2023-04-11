#include <vslc.h>

// This header defines a bunch of macros we can use to emit assembly to stdout
#include "emit.h"

// In the System V calling convention, the first 6 integer parameters are passed in registers
#define NUM_REGISTER_PARAMS 6
static const char *REGISTER_PARAMS[6] = {RDI, RSI, RDX, RCX, R8, R9};

// Takes in a symbol of type SYMBOL_FUNCTION, and returns how many parameters the function takes
#define FUNC_PARAM_COUNT(func) ((func)->node->children[1]->n_children)

#define REG_SIZE 8

#define MIN(A, B) (((A) > (B)) ? (B) : (A))
#define MAX(A, B) (((A) < (B)) ? (B) : (A))

#define STROUT "strout"
#define INTOUT "intout"

static symbol_t *main_symbol;

extern char **string_list;
extern size_t string_list_len;
extern symbol_table_t *global_symbols;

static void generate_stringtable(void);
static void generate_global_variables(void);
static void generate_function(symbol_t *function);
static void generate_functions(void);
static void generate_statement(symbol_t *function, node_t *node);
static void generate_main(symbol_t *first);
static void generate_expression(symbol_t *function, node_t *expression);
static void generate_safe_print(void);

static int64_t bp_offset(symbol_t *function, symbol_t *identifier);
static void push_dummy(void);
static void generate_function_call(symbol_t *function, node_t *call);
static void push_identifier(symbol_t *function, symbol_t *identifier);
static void get_identifier(symbol_t *function, symbol_t *identifier, const char *reg);
static void get_constant(node_t *number_data, const char *reg);
static void push_constant(node_t *number_data);
static void push_identifier(symbol_t *function, symbol_t *identifier);
static void write_identifier(symbol_t *function, symbol_t *identifier, const char *reg);

/* Entry point for code generation */
void generate_program(void)
{
    generate_stringtable();
    generate_global_variables();

    DIRECTIVE(".section .text");

    generate_functions();

    generate_safe_print();
    generate_main(main_symbol);
}

/* Prints one .asciz entry for each string in the global string_list */
static void generate_stringtable(void)
{
    DIRECTIVE(".section .rodata");
    // These strings are used by printf
    DIRECTIVE("intout: .asciz \"%s\"", "%ld ");
    DIRECTIVE("strout: .asciz \"%s\"", "%s ");
    // This string is used by the entry point-wrapper
    DIRECTIVE("errout: .asciz \"%s\"", "Wrong number of arguments");

    for (int i = 0; i < string_list_len; i++)
    {
        DIRECTIVE("string%d: .asciz %s", (i), (string_list[i]));
    }
}

/* Prints .zero entries in the .bss section to allocate room for global variables and arrays */
static void generate_global_variables(void)
{
    DIRECTIVE(".section .bss");
    DIRECTIVE(".align 8");
    for (int i = 0; i < global_symbols->n_symbols; i++)
    {
        symbol_t *symbol = global_symbols->symbols[i];
        if (symbol->type == SYMBOL_GLOBAL_ARRAY || symbol->type == SYMBOL_GLOBAL_VAR)
        {
            uint64_t size = symbol->type == SYMBOL_GLOBAL_VAR ? REG_SIZE : REG_SIZE * *(uint64_t *)symbol->node->children[1]->data;
            void *name = symbol->type == SYMBOL_GLOBAL_VAR ? symbol->node->data : symbol->node->children[0]->data;
            DIRECTIVE(".%s: .zero %ld", ((char *)name), (size));
        }
    }
}

static void generate_functions(void)
{
    int main = 1;
    for (int i = 0; i < global_symbols->n_symbols; i++)
    {
        symbol_t *symbol = global_symbols->symbols[i];
        if (symbol->type == SYMBOL_FUNCTION)
        {
            // find the index of the main function and set it globally
            if (main)
            {
                main = 0;
                main_symbol = symbol;
            }
            generate_function(symbol);
        }
    }
}

/* Prints the entry point. preable, statements and epilouge of the given function */
static void generate_function(symbol_t *function)
{
    DIRECTIVE(".%s:", (function->name));
    PUSHQ(RBP);
    MOVQ(RSP, RBP);
    // Push the register args to the stack
    for (int i = 0; i < MIN(NUM_REGISTER_PARAMS, function->node->children[1]->n_children); i++)
    {
        PUSHQ(REGISTER_PARAMS[i]);
    }
    // Allocate space on stack for the local variables 
    for (int i = 0; i < function->function_symtable->n_symbols; i++)
    {
        symbol_t *symbol = function->function_symtable->symbols[i];
        if (symbol->type == SYMBOL_LOCAL_VAR)
        {
            push_dummy();
        }
    }
    generate_statement(function, function->node->children[2]);
    EMIT("leave");
    EMIT("ret");
}

//Calculates the offset of a local variable or parameter relative to rbp
static int64_t bp_offset(symbol_t *function, symbol_t *identifier)
{
    if (identifier->sequence_number >= NUM_REGISTER_PARAMS && identifier->type == SYMBOL_PARAMETER)
    {
        return REG_SIZE * (identifier->sequence_number - NUM_REGISTER_PARAMS + 2);
    }
    else
    {
        return identifier->type == SYMBOL_PARAMETER || FUNC_PARAM_COUNT(function) <= NUM_REGISTER_PARAMS
                   ? -REG_SIZE * (identifier->sequence_number + 1)
                   : -REG_SIZE * (identifier->sequence_number - (FUNC_PARAM_COUNT(function) - NUM_REGISTER_PARAMS) + 1);
    }
}

static void generate_function_call(symbol_t *function, node_t *call)
{
    symbol_t *called_func = call->children[0]->symbol;
    uint64_t num_args = FUNC_PARAM_COUNT(called_func);
    //Push the arguments that dont fit in registers to stack 
    for (int i = num_args - 1; i >= NUM_REGISTER_PARAMS; i--)
    {
        node_t *arg = call->children[1]->children[i];
        switch (arg->type)
        {
        case IDENTIFIER_DATA:
            push_identifier(function, arg->symbol);
            break;
        case NUMBER_DATA:
            push_constant(arg);
            break;
        case EXPRESSION:
            generate_expression(function, arg);
            break;
        default:
            break;
        }
    }
    // Place arguments in registers
    for (int i = 0; i < MIN(num_args, NUM_REGISTER_PARAMS); i++)
    {
        node_t *arg = call->children[1]->children[i];
        switch (arg->type)
        {
        case IDENTIFIER_DATA:
            get_identifier(function, arg->symbol, REGISTER_PARAMS[i]);
            break;
        case NUMBER_DATA:
            get_constant(arg, REGISTER_PARAMS[i]);
            break;
        case EXPRESSION:
            generate_expression(function, arg);
            POPQ(REGISTER_PARAMS[i]);
            break;
        default:
            break;
        }
    }
    EMIT("call .%s", called_func->name);
}

//Pushes the value of an identifier to stack
static void push_identifier(symbol_t *function, symbol_t *identifier)
{
    switch (identifier->type)
    {
    case SYMBOL_GLOBAL_VAR:
        EMIT("movq (.%s), %s", identifier->name, RAX);
        break;
    case SYMBOL_LOCAL_VAR:
    case SYMBOL_PARAMETER:
        int64_t offset = bp_offset(function, identifier);
        EMIT("movq %ld(%s), %s", offset, RBP, RAX);
        break;
    default:
        return;
    }
    PUSHQ(RAX);
}

//Pushes a constant to the stack
static void push_constant(node_t *number_data)
{
    EMIT("movq $%d, %s", *(int *)number_data->data, RAX);
    PUSHQ(RAX);
}

// Places the value of an identifier in a register
static void get_identifier(symbol_t *function, symbol_t *identifier, const char *reg)
{
    int64_t offset = bp_offset(function, identifier);
    EMIT("movq %ld(%s), %s", offset, RBP, reg);
}

// Places a constant in a register
static void get_constant(node_t *number_data, const char *reg)
{
    EMIT("movq $%ld, %s", *(int *)number_data->data, reg);
}

static void push_dummy()
{
    PUSHQ("$0");
}

// Overwrites the value of an identifier
static void write_identifier(symbol_t *function, symbol_t *identifier, const char *reg)
{
    int64_t offset = bp_offset(function, identifier);
    EMIT("movq %s,%ld(%s)", reg, offset, RBP);
}

//Recursively genereate expression evaluation
static void generate_expression(symbol_t *function, node_t *expression)
{
    node_type_t type = expression->type;
    switch (type)
    {
    case IDENTIFIER_DATA:
    {
        push_identifier(function, expression->symbol);
        return;
    }
    case NUMBER_DATA:
    {
        push_constant(expression);
        return;
    }
    case ARRAY_INDEXING:
    {
        generate_expression(function, expression->children[1]);
        POPQ(R11);
        EMIT("movq $.%s, %s", (char *)expression->children[0]->data, R10);
        EMIT("movq (%s, %s, %d), %s", R10, R11, REG_SIZE, RAX);
        PUSHQ(RAX);
        return;
    }
    case EXPRESSION:
    {
        char *operator= expression->data;
        if (strcmp(operator, "call") == 0)
        {
            generate_function_call(function, expression);
            PUSHQ(RAX);
            return;
        }
        // Unary minus
        if (strcmp(operator, "-") == 0 && expression->n_children == 1)
        {
            generate_expression(function, expression->children[0]);
            POPQ(R10);
            MOVQ("$0", RAX);
            SUBQ(R10, RAX);
            PUSHQ(RAX);
            return;
        }
        generate_expression(function, expression->children[0]);
        generate_expression(function, expression->children[1]);
        POPQ(R8);
        POPQ(RAX);
        if (strcmp(operator, "+") == 0)
        {
            ADDQ(R8, RAX);
        }
        else if (strcmp(operator, "-") == 0)
        {
            SUBQ(R8, RAX);
        }
        else if (strcmp(operator, "*") == 0)
        {
            IMULQ(R8, RAX);
        }
        else if (strcmp(operator, "/") == 0)
        {
            CQO;
            IDIVQ(R8);
        }
        PUSHQ(RAX);
        return;
    }
    default:
        break;
    }
}
static void generate_safe_print()
{
    LABEL("safe_printf");
    EMIT("pushq \%rbp");
    EMIT("movq \%rsp, \%rbp");
    EMIT("andq $-16, \%rsp");
    EMIT("call printf");
    EMIT("movq \%rbp, \%rsp");
    EMIT("popq \%rbp");
    EMIT("ret");
}

static void generate_assignment_statement(symbol_t *function, node_t *statement)
{
    node_t *left_side = statement->children[0];
    generate_expression(function, statement->children[1]);
    POPQ(RAX);
    switch (left_side->type)
    {
    case IDENTIFIER_DATA:
        switch (left_side->symbol->type)
        {
        case SYMBOL_LOCAL_VAR:
            write_identifier(function, left_side->symbol, RAX);
            return;
        case SYMBOL_GLOBAL_VAR:
            EMIT("movq %s, (.%s)", RAX, left_side->symbol->name);
            return;
        default:
            return;
        }
    case ARRAY_INDEXING:
        MOVQ(RAX, R12);
        generate_expression(function, left_side->children[1]);
        POPQ(R11);
        EMIT("movq $.%s, %s", (char *)left_side->children[0]->data, R10);
        EMIT("movq %s, (%s, %s, %d)", R12, R10, R11, REG_SIZE);
        return;
    default:
        break;
    }
}

static void generate_print_statement(symbol_t *function, node_t *statement)
{
    for (int i = 0; i < statement->n_children; i++)
    {
        node_t *item = statement->children[i];
        switch (item->type)
        {
        case STRING_DATA:
            EMIT("movq $string%ld,%s", *(size_t *)item->data, RSI);
            EMIT("movq $%s, %s", STROUT, RDI);
            break;
        case NUMBER_DATA:
            EMIT("movq $%ld, %s", (int64_t)item->data, RSI);
            EMIT("movq $%s, %s", INTOUT, RDI);
            break;
        case EXPRESSION:
        case ARRAY_INDEXING:
        case IDENTIFIER_DATA:
            generate_expression(function, item);
            MOVQ(RAX, RSI);
            EMIT("movq $%s, %s", INTOUT, RDI);
            break;
        default:
            break;
        }
        EMIT("movq $0, %s", RAX);
        EMIT("call safe_printf");
    }
    MOVQ("$10", RDI);
    EMIT("call putchar");
}

static void generate_return_statement(symbol_t *function, node_t *statement)
{
    //evaluating the expression already puts the value in RAX, so no need to do anything else
    generate_expression(function, statement->children[0]);
}

/* Recursively generate the given statement node, and all sub-statements. */
static void generate_statement(symbol_t *function, node_t *node)
{
    node_type_t type = node->type;
    switch (type)
    {
    case EXPRESSION:
        generate_expression(function, node);
        return;
    case ASSIGNMENT_STATEMENT:
        generate_assignment_statement(function, node);
        return;
    case PRINT_STATEMENT:
        generate_print_statement(function, node);
        return;
    case RETURN_STATEMENT:
        generate_return_statement(function, node);
        return;
    default:
        break;
    }
    for (int i = 0; i < node->n_children; i++)
    {
        generate_statement(function, node->children[i]);
    }
}

// Generates a wrapper, to be able to use a vsl function as our entrypoint
static void generate_main(symbol_t *first)
{
    // Make the globally available main function
    DIRECTIVE(".globl main");
    LABEL("main");

    // Save old base pointer, and set new base pointer
    PUSHQ(RBP);
    MOVQ(RSP, RBP);

    // Which registers argc and argv are passed in
    const char *argc = RDI;
    const char *argv = RSI;

    const size_t expected_args = FUNC_PARAM_COUNT(first);

    SUBQ("$1", argc); // argc counts the name of the binary, so subtract that
    EMIT("cmpq $%ld, %s", expected_args, argc);
    JNE("ABORT"); // If the provdied number of arguments is not equal, go to the abort label

    if (expected_args == 0)
        goto skip_args; // No need to parse argv

    // Now we emit a loop to parse all parameters, and push them to the stack,
    // in right-to-left order

    // First move the argv pointer to the vert rightmost parameter
    EMIT("addq $%ld, %s", expected_args * 8, argv);

    // We use rcx as a counter, starting at the number of arguments
    MOVQ(argc, RCX);
    LABEL("PARSE_ARGV"); // A loop to parse all parameters
    PUSHQ(argv);         // push registers to caller save them
    PUSHQ(RCX);

    // Now call strtol to parse the argument
    EMIT("movq (%s), %s", argv, RDI); // 1st argument, the char *
    MOVQ("$0", RSI);                  // 2nd argument, a null pointer
    MOVQ("$10", RDX);                 // 3rd argument, we want base 10
    EMIT("call strtol");

    // Restore caller saved registers
    POPQ(RCX);
    POPQ(argv);
    PUSHQ(RAX); // Store the parsed argument on the stack

    SUBQ("$8", argv);        // Point to the previous char*
    EMIT("loop PARSE_ARGV"); // Loop uses RCX as a counter automatically

    // Now, pop up to 6 arguments into registers instead of stack
    for (size_t i = 0; i < expected_args && i < NUM_REGISTER_PARAMS; i++)
        POPQ(REGISTER_PARAMS[i]);

skip_args:

    EMIT("call .%s", first->name);
    MOVQ(RAX, RDI);    // Move the return value of the function into RDI
    EMIT("call exit"); // Exit with the return value as exit code

    LABEL("ABORT"); // In case of incorrect number of arguments
    MOVQ("$errout", RDI);
    EMIT("call puts"); // print the errout string
    MOVQ("$1", RDI);
    EMIT("call exit"); // Exit with return code 1
}
