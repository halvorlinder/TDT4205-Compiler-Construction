%{
#include <vslc.h>
// The tokens defined in parser.y
#include "y.tab.h"
%}
%option noyywrap
%option array
%option yylineno

WHITESPACE [\ \t\v\r\n]

COMMENT \/\/[^\n]+

QUOTED \"([^\"\n]|\\\")*\"

FUNC func
BEGIN begin
END end
RETURN retrun
PRINT print
BREAK break
IF if
THEN then
ELSE else
FOR for
IN in
WHILE while
DO do
VAR var

ASSIGN :=
PLUS \+
MINUS -
TIMES \*
DIVIDE \/
EQUALS =
N_EQUALS !=
L_THAN \<
G_THAN >

NUMBER [0-9]+

IDENTIFIER [a-zA-Z_][a-zA-Z_0-9]*

%%
{WHITESPACE}+           { /* Eliminate whitespace */ }
{COMMENT}               { /* Eliminate comments */ }
{QUOTED}                { return STRING; }
{FUNC}                  { printf("func"); }

  /* 
   TODO:

   Add the rest of the translation rules here.
   See the lexical structure definition of the modified VSL in PS2.
   Also see the `%token` directives in parser.y for all symbolic names that can be returned - e.g. STRING, NUMBER, IDENTIFIER.

   Hint to get you started:
   The scanner returns STRING when matching the QUOTED regex above.
   When should the scanner return a NUMBER, IDENTIFIER, etc?
   In which specific scenarios should the scanner return a FUNC or PRINT?
  */

  /* Unknown chars get returned as single char tokens */
.                       { return yytext[0]; }
%%
