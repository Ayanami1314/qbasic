100 REM Check Prime Number
110 INPUT n
120 IF n <= 1 THEN 190
130 LET is_prime = 1
140 LET i = 2
150 IF i * i > n THEN 180
160 IF n - (n / i) * i == 0 THEN 170
165 LET i = i + 1
166 GOTO 150
170 LET is_prime = 0
180 IF is_prime == 1 THEN 200
190 PRINT "Not Prime"
195 GOTO 210
200 PRINT "Prime"
210 END