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

DIGIT_SEQUENCE [0-9]+

NAME [a-zA-Z_][a-zA-Z_0-9]*

%%
{WHITESPACE}+               { /* Eliminate whitespace */ }
{COMMENT}                   { /* Eliminate comments */ }
{QUOTED}                    { return STRING; }
{DIGIT_SEQUENCE}            { return NUMBER; }

func                        { return FUNC; }
begin                       { return OPENBLOCK; }
end                         { return CLOSEBLOCK; }
return                      { return RETURN; }
print                       { return PRINT; }
break                       { return BREAK; }
if                          { return IF; }
then                        { return THEN; }
else                        { return ELSE; }
for                         { return FOR; }
in                          { return IN; }
while                       { return WHILE; }
do                          { return DO; }
var                         { return VAR; }

{NAME}                      { return IDENTIFIER; }

  /* Unknown chars get returned as single char tokens */
.                       { return yytext[0]; }
%%
