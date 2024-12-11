100 REM Check Even or Odd
130 IF rem < 1 THEN 150
145 GOTO 160
110 INPUT n
150 PRINT "Even"
140 PRINT "Odd"
120 LET rem = n - (n / 2) * 2
160 END