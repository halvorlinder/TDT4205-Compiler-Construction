---
title: "Exercise 3"
author: Halvor Linder Henriksen
geometry: margin=2cm
output: pdf_document
---
## 1 Bottom/up parsing tables

### 1.1
![](1_1.svg)

### 1.2
The states highlighted in red have shift reduce conflicts, as both actions are possible.

### 1.3 

We can use the follow sets of the left hand side of the productions to remove shift/reduce conflicts. 

The resulting table:k

|   | +  | *  | x | $ | E | T | F |
| --- | --- | --- | --- | --- | --- | --- | --- |
| 0  |  |  | s5 |  | g1 | g8 | g3 |
| 1  | s2 |  |  | r0 |  |  |  |
| 2  |  |  | s5 |  |  | g4 | g3 |
| 3  | r4 | r4 | r4 | r4 |  |  |  |
| 4  | r1 | s6 |  | r1 |  |  |  |
| 5  | r5 | r5 | r5 | r5 |  |  |  |
| 6  |  |  | s5 |  |  |  | g7 |
| 7  | r3 | r3 | r3 | r3 |  |  |  |
| 8  |  | s6/r2 |  | r2 |  |  |  |

There is still one shift/reduce conflict in the SLR parsing table. The grammar is not SLR-parseable.
