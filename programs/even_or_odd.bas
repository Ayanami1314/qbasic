100 REM Check Even or Odd
110 INPUT n
120 LET rem = n - (n / 2) * 2
130 IF rem = 0 THEN 150
140 PRINT "Odd"
145 GOTO 160
150 PRINT "Even"
160 END