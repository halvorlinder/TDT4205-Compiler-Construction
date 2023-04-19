---
  title: "Exercise 6"
  author: Halvor Linder Henriksen
  geometry: margin=2cm
  output: pdf_document
---

## 1.1 Control flow graph  
![](graph.svg)  

## 1.2 Reaching definitions  

Let  

d1: num = 1  
d2: sum = num  
d3: stop = 0  
d4: tmp = sum  
d5: rem = 0  
d6: sum = 0  
d7: rem = tmp/10  
d8: rem = tmp - rem * 10  
d9: sum = sum + rem * rem  
d10: tmp = tmp/10  
d11: stop = 1  
d12: num = num + 1  

Then the data flow equations are given by:  

L0 = {}  
L1 = L0 - d12 + d1  
L1.5 = L1 U L32  
L2 = L1.5   
L3 = L2  
L4 = L3 - (d6, d9) + d2  
L5 = L4 - d11 + d3  
L5.5 = L5 U L26  
L6 = L5.5  
L7 = L6  
L8 = L7 - d10 + d4  
L9 = L8 - (d7, d8) + d5  
L10 = L9 - (d2, d9) + d6  
L10.5 = L10 U L16  
L11 = L10.5  
L12 = L11  
L13 = L12 - (d5, d8) + d7  
L14 = L13 - (d5, d7) + d8  
L15 = L14 - (d2, d6) + d9  
L16 = l15 - d4 + d10  
L17 = L11  
L18 = L17  
L19 = L18  
L20 = L19 - d3 + d11  
L21 = L18 U L20  
L22 = L21  
L23 = L22  
L24 = L23 - d3 + d11  
L25 = L22 U L24  
L26 = L25  
L27 = L6  
L28 = L27  
L29 = L28  
L30 = L29  
L31 = L28 U L30  
L32 = L31 - d1 + d12  
L33 = L2  
L34 = L33  

The solutions to the equations are:

L0    = {}  
L1    = {d1}  
L1.5  = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L2    = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L3    = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L4    = {d1,d2,d3,d4,d5,d8,d10,d11,d12}  
L5    = {d1,d2,d3,d4,d5,d8,d10,d12}  
L5.5  = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L6    = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L7    = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L8    = {d1,d2,d3,d4,d5,d6,d8,d9,d11,d12}  
L9    = {d1,d2,d3,d4,d5,d6,d9,d11,d12}  
L10   = {d1,d3,d4,d5,d6,d11,d12}  
L10.5 = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L11   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L12   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L13   = {d1,d3,d4,d6,d7,d9,d10,d11,d12}  
L14   = {d1,d3,d4,d6,d8,d9,d10,d11,d12}  
L15   = {d1,d3,d4,d8,d9,d10,d11,d12}  
L16   = {d1,d3,d8,d9,d10,d11,d12}  
L17   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L18   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L19   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L20   = {d1,d4,d5,d6,d8,d9,d10,d11,d12}  
L21   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L22   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L23   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L24   = {d1,d4,d5,d6,d8,d9,d10,d11,d12}  
L25   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L26   = {d1,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L27   = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L28   = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L29   = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L30   = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L31   = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L32   = {d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L33   = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  
L34   = {d1,d2,d3,d4,d5,d6,d8,d9,d10,d11,d12}  

(Maybe a smaller graph would've been sufficient for this exercise)