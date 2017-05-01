(
# "util/slider8.sp" load
_spigot "./spigot.so" fl
_val var
_clk var
# delay feedback 
_s0 0.9 varset

# clock speed
_s1 var 0.1 0.7 1 randi _s1 set

# mincer level
_s2 0.5 varset

# pitch 
_s3 0.0 varset

# noise/pluck balance
_s4 var 0 0.9 0.3 4 rspline _s4 set

# loop traversal in mincer
_s5 0.5 varset

# clock probability 
_s6 var 0.5 1.0  0.5 4 rspline _s6 set

# reverb level
_s7 0.5 varset


_buf sr 2 * zeros

0 10 100 randh floor 
_s1 get 1 50 scale metro _s6 get maytrig
1 
",[..-]+[.-],[><>.><<->,[.-.]<] ...  +[.,] +[>><<,] +[.] +[><.] >.+-<>+[.-,]" 
_spigot fe _val set

4 _out "ex/clicks.scm" ps

0 _out tget

0 _out tget 
_s0 get 0.99 * 0.003 0.1 10 randh 1.0 128 smoothdelay 
1000 butlp
+ 
3 dmetro 0.5 maytrig _buf tblrec

_s5 get 3 20 scale inv 1 sine 0 2 biscale 1 _s3 get 1 4 scale 512 _buf mincer _s2 get * +

dcblk

dup _s7 get * dup 0.94 10000 revsc drop 0.3 * +
dup
_spigot fc
)
