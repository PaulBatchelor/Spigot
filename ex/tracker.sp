(
_spigot "./spigot.so" fl
_clk var
_expr var
0 58 4 clock dup _clk set

1
"ex/tracker.rnt"

_spigot fe 

tick _play get 0.5 0 thresh + 0 10 1 line _expr set 

0 _notes tget 0 _gates tget 0.5 0 thresh 0.05 tport 
0.1 1 3 jitter + dup 0 pset
mtof 0.3 saw
0 p 11.9 - mtof 0.3 saw + 
0 p 0.12 + mtof 0.3 saw +  -3 ampdb * 
30 mtof 20 4 eqfil


dup _expr get 100 700 scale 0.8 moogladder bal -4 ampdb * 

0 _gates tget 0.01 port * 

1 _notes tget 1 _gates tget 0.5 0 thresh 0.02 tport 
6 0.2 _expr get * sine + mtof 0.3  1 1.001
_expr get 0.3 1.6 scale fm 1 _gates tget 0.01 0.1 1 0.2 adsr *


2 _notes tget 2 _gates tget 0.5 0 thresh 0.02 tport 
6.6 0.2 _expr get * sine + 
0.1 1 3 jitter + mtof  
0.3  1 1 _expr get 0.3 1.7 scale  fm 2 _gates tget 0.01 0.1 1 0.2 adsr *

3 _notes tget 
3 _gates tget 0.5 0 thresh 0.02 tport 6.3 0.3 sine + mtof 0.2 1 1 _expr get 0 1 scale 
fm 3 _gates tget 0.01 0.1 1 0.2 adsr *

+ + 100 buthp

# the bass
+

_expr get -8 -4 scale ampdb *

_expr get 400 8000 scale butlp 

_play get *

dup 1000 buthp dup 0.94 10000 revsc drop -20 ampdb * + dcblk

dup

_spigot fc
)
