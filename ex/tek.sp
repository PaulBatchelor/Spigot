(
_prop var
_clk var
_kick var
_snare var
_rms var
_send var
_play var
_bpm 140 varset


_play get changed dup _bpm get 4 clock _clk set
dup _bpm get "+" tprop _kick set
_bpm get "-+" tprop _snare set

_clk get 1 "ex/tek.rnt" _spigot fe

_play get "play" print drop

_play get changed _bpm get "4(+???)+?-|+2(++)-4(++?+)" tprop _prop set
 
_prop get 0.001 0.001 0.01 tenvx 
# _prop get 100 500 trand 0.5 sine * 
0.4 noise * 

_prop get 200 800 trand 0.001 port  0.8 streson 
_prop get 200 800 trand 
0.1 1 sine 0.001 0.1 biscale port  0.9 streson 
_prop get 200 800 trand 0.1 port  0.9 streson 

dcblk

1000 500 butbp 

dup 0.3 _bpm get bpm2dur 0.75 * delay -10 ampdb * +


_kick get 0.001 0.001 0.05 tenvx
_kick get 0.001 0.001 0.01 tenvx 50 120 scale 
0.9 1 4.1514 
_kick get 0.001 0.001 0.01 tenv 0 3 scale fm * 
40 100 3 eqfil
_kick get 0.01 0.001 0.2 tenv 60 0.5 sine * + 
0.9 clip dup rms 1 swap - _rms set

+

0 _notes tget 0.01 port mtof 0.4 saw 0 _gates tget 0.1 port * 
1 _notes tget 0.01 port mtof 0.4 saw 1 _gates tget 0.1 port * 
2 _notes tget 0.01 port mtof 0.4 saw 2 _gates tget 0.1 port * 

+
+

300 butlp 500 buthp _rms get 0 1 scale * 
dup _send set
+ 

3 _notes tget 0.001 port mtof 0.1 1 8 1 fm _rms get *
3 _gates tget 0.001 port * 
dup -4 ampdb * _send get + _send set
+ 

4 _notes tget 0.001 port mtof 0.2 saw 
4 _notes tget 12 - 0.001 port mtof 0.9 tri +

4 _gates tget 0.001 port *

# remove this bass if CPU does not like
dup 20 inv 1 sine 1000 3000 biscale 0.8 diode bal 
_rms get *
+ 

_snare get 0.001 0.01 0.1 tenvx 0.3 noise * 
dup 
_snare get 0.2 maygate * 30 inv 1 sine 0.5 0.9 biscale 
_bpm get bpm2dur 0.75 * delay 2000 butlp +
dup
-5 ampdb * 
_send get + _send set
+
_send get dup 0.97 10000 revsc drop _rms get -30 -10 scale ampdb * +
_play get *
)
