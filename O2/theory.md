---
title: "Exercise 1"
author: Halvor Linder Henriksen
# date: March 22, 2005
geometry: margin=2cm
output: pdf_document
---
## 1 Regular languages, NFAs and DFAs

### 1.1
The issue here is the left recursion in the statement L -> L;I | I  
It represents a list of one or more ;-separated I-s, so it can easily be made right recursive instead.  
L -> I | I;L  
The new statement has a common prefix in the two cases, so it must be left factored. This produces  
L -> II'  
I' -> ε | ;L


### 1.2
| NT  | First  | Follow  | Null|
| ---  | --- | --- | --- |
| S  | s | $ | No
| C  | c | x | No
| L  | i | d y | No
| I  | i | ; d y | No 
| I' | ; | d y | Yes
| D  | d | y | Yes

### 1.3 

| NT \ T  | s  | x  | y | c | ; | i | d |
| --- | --- | --- | --- | --- | --- | --- | --- |
| S  | S->sCxLDy | 
| C  | | | | C -> c
| L  | | | | | L -> II'
| I  | | | | | I -> i
| I' | | | I' -> ε | | | I' -> ;L | I' -> ε
| D  | | | D -> ε | | | | D -> d
