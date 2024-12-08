100 REM Factorial Calculation
110 INPUT n
120 LET fact = 1
130 LET i = 1
140 IF i > n THEN 170
150 LET fact = fact * i
160 LET i = i + 1
165 GOTO 140
170 PRINT fact
180 END