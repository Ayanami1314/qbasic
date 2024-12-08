100 REM Sum of First N Numbers
110 INPUT n
120 LET sum = 0
130 LET i = 1
140 IF i > n THEN 170
150 LET sum = sum + i
160 LET i = i + 1
165 GOTO 140
170 PRINT sum
180 END