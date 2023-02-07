%{
#include <vslc.h>

/* State variables from the flex generated scanner */
extern int yylineno; // The line currently being read
extern char yytext[]; // The text of the last consumed lexeme
/* The main flex driver function used by the parser */
int yylex ( void );
/* The function called by the parser when errors occur */
int yyerror ( const char *error )
{
    fprintf ( stderr, "%s on line %d\n", error, yylineno );
    exit ( EXIT_FAILURE );
}

%}

%left '+' '-'
%left '*' '/'
%right UMINUS

%nonassoc IF THEN
%nonassoc ELSE

%token FUNC OPENBLOCK CLOSEBLOCK PRINT RETURN BREAK WHILE FOR IN DO
%token VAR NUMBER IDENTIFIER STRING

%start program

%%
program :
      global_list {                                     // program -> global_list
        root = malloc (sizeof(node_t));                 // root = PROGRAM
        node_init (root, PROGRAM, NULL, 1, $1);         // $1 = GLOBAL_LIST
      }
    ;

global_list :
      global {                                          // global_list -> global
        $$ = malloc (sizeof(node_t));                   // $$ = GLOBAL_LIST
        node_init ($$, GLOBAL_LIST, NULL, 1, $1);       // $1 = GLOBAL
      }
    | global_list global {                              // global_list -> global_list global
        $$ = malloc (sizeof(node_t));                   // $$ = GLOBAL_LIST
        node_init ($$, GLOBAL_LIST, NULL, 2, $1, $2);   // $1 = GLOBAL_LIST, $2 = GLOBAL
      }
    ;

global :
      function {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, GLOBAL, NULL, 1, $1);       
      }
    | declaration {                              
        $$ = malloc (sizeof(node_t));                  
        node_init ($$, GLOBAL, NULL, 1, $1); 
      }
    | array_declaration {                              
        $$ = malloc (sizeof(node_t));                  
        node_init ($$, GLOBAL, NULL, 1, $1); 
      }
    ;

declaration :
      VAR variable_list {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, DECLARATION, NULL, 1, $2);       
      }
    ;

variable_list :
      identifier {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, VARIABLE_LIST, NULL, 1, $1);       
      }
    | variable_list ',' identifier {                              
        $$ = malloc (sizeof(node_t));                  
        node_init ($$, VARIABLE_LIST, NULL, 2, $1, $3); 
      }
    ;

array_declaration :
      VAR array_indexing {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, ARRAY_DECLARATION, NULL, 1, $2);       
      }
    ;

array_indexing :
      identifier '[' expression ']'{                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, ARRAY_INDEXING, NULL, 2, $1, $3);       
      }
    ;

function :
      FUNC identifier '(' parameter_list ')' statement{                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, FUNCTION, NULL, 3, $2, $4, $6);       
      }
    ;

parameter_list :
      variable_list {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, PARAMETER_LIST, NULL, 1, $1);       
      }
    | {                              
        $$ = malloc (sizeof(node_t));                  
        node_init ($$, PARAMETER_LIST, NULL, 0); 
      }
    ;

statement :
      assignment_statement {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT, NULL, 1, $1);       
      }
    | return_statement {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT, NULL, 1, $1);       
      }
    | print_statement {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT, NULL, 1, $1);       
      }
    | if_statement {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT, NULL, 1, $1);       
      }
    | while_statement {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT, NULL, 1, $1);       
      }
    | for_statement {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT, NULL, 1, $1);       
      }
    | break_statement {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT, NULL, 1, $1);       
      }
    | block {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT, NULL, 1, $1);       
      }
    ;

block :
      OPENBLOCK declaration_list statement_list CLOSEBLOCK {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, BLOCK, NULL, 2, $2, $3);       
      }
    | OPENBLOCK statement_list CLOSEBLOCK {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, BLOCK, NULL, 1, $2);       
      }
    ;

declaration_list :
      declaration {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, DECLARATION_LIST, NULL, 1, $1);       
      }
    | declaration_list declaration {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, DECLARATION_LIST, NULL, 2, $1, $2);       
      }
    ;

statement_list :
      statement {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT_LIST, NULL, 1, $1);       
      }
    | statement_list statement {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, STATEMENT_LIST, NULL, 2, $1, $2);       
      }
    ;

assignment_statement :
      identifier ':' '=' expression {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, ASSIGNMENT_STATEMENT, NULL, 2, $1, $4);       
      }
    | array_indexing ':' '=' expression {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, ASSIGNMENT_STATEMENT, NULL, 2, $1, $4);       
      }
    ;

return_statement :
      RETURN expression {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, RETURN_STATEMENT, NULL, 1, $2);       
      }
    ;

print_statement :
      PRINT print_list {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, PRINT_STATEMENT, NULL, 1, $2);       
      }
    ;

print_list :
      print_item {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, PRINT_LIST, NULL, 1, $1);       
      }
    | print_list ',' print_item {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, PRINT_LIST, NULL, 2, $1, $3);       
      }
    ;

print_item :
      expression {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, PRINT_ITEM, NULL, 1, $1);       
      }
    | string {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, PRINT_ITEM, NULL, 1, $1);       
      }
    ;

break_statement :
      BREAK {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, BREAK_STATEMENT, NULL, 0);       
      }
    ;

if_statement :
      IF relation THEN statement {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, IF_STATEMENT, NULL, 2, $2, $4);       
      }
    | IF relation THEN statement ELSE statement {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, IF_STATEMENT, NULL, 3, $2, $4, $6);       
      }
    ;

while_statement :
      WHILE relation DO statement {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, WHILE_STATEMENT, NULL, 2, $2, $4);       
      }
    ;

relation :
      expression '=' expression {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, RELATION, NULL, 2, $1, $3);       
      }
    | expression '!' '=' expression {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, RELATION, NULL, 2, $1, $4);       
      }
    | expression '<' expression {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, RELATION, NULL, 2, $1, $3);       
      }
    | expression '>' expression {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, RELATION, NULL, 2, $1, $3);       
      }
    ;

for_statement :
      FOR identifier IN expression '.' '.' expression DO statement {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, FOR_STATEMENT, NULL, 4, $2, $4, $7, $9);       
      }
    ;

expression :
      expression '+' expression {                                          
        $$ = malloc (sizeof(node_t));                   
        char* data = malloc(2*sizeof(char));
        strcpy(data, "+\0");
        node_init ($$, EXPRESSION, data, 2, $1, $3);       
      }
    | expression '-' expression {                              
        $$ = malloc (sizeof(node_t));                   
        char* data = malloc(2*sizeof(char));
        strcpy(data, "-\0");
        node_init ($$, EXPRESSION, data, 2, $1, $3);       
      }
    | expression '*' expression {                              
        $$ = malloc (sizeof(node_t));                   
        char* data = malloc(2*sizeof(char));
        strcpy(data, "*\0");
        node_init ($$, EXPRESSION, data, 2, $1, $3);       
      }
    | expression '/' expression {                              
        $$ = malloc (sizeof(node_t));                   
        char* data = malloc(2*sizeof(char));
        strcpy(data, "/\0");
        node_init ($$, EXPRESSION, data, 2, $1, $3);       
      }
    | '-' expression %prec UMINUS {                              
        $$ = malloc (sizeof(node_t));                   
        char* data = malloc(2*sizeof(char));
        strcpy(data, "-\0");
        node_init ($$, EXPRESSION, data, 1, $2);       
      }
    | '(' expression ')' {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, EXPRESSION, NULL, 1, $2);       
      }
    | number {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, EXPRESSION, NULL, 1, $1);       
      }
    | identifier {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, EXPRESSION, NULL, 1, $1);       
      }
    | array_indexing {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, EXPRESSION, NULL, 1, $1);       
      }
    | identifier '(' argument_list ')' {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, EXPRESSION, NULL, 2, $1, $3);       
      }
    ;

expression_list :
      expression  {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, EXPRESSION_LIST, NULL, 1, $1);       
      }
    | expression_list ',' expression {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, EXPRESSION_LIST, NULL, 2, $1, $3);       
      }
    ;

argument_list :
      expression_list  {                                          
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, ARGUMENT_LIST, NULL, 1, $1);       
      }
    | {                              
        $$ = malloc (sizeof(node_t));                   
        node_init ($$, ARGUMENT_LIST, NULL, 0);       
      }
    ;

identifier :
      IDENTIFIER {                                          
        $$ = malloc (sizeof(node_t));                   
        char *data = strdup(yytext);
        node_init ($$, IDENTIFIER_DATA, data, 0);       
      }
    ;

number :
      NUMBER {                                          
        $$ = malloc (sizeof(node_t));                   
        int64_t value = strtol(yytext, NULL, 10);
        int64_t* data = malloc(sizeof(int64_t));
        *data = value;
        node_init ($$, NUMBER_DATA, data, 0);       
      }
    ;

string :
      STRING {                                          
        $$ = malloc (sizeof(node_t));                   
        char *data = strdup(yytext);
        node_init ($$, STRING_DATA, data, 0);       
      }
    ;

%%
