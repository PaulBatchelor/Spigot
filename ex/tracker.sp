(
_spigot "./spigot.so" fl
_clk var

4 metro dup _clk set

1
"ex/tracker.rnt"

_spigot fe 


0 _notes tget 0.001 port mtof 0.3  1 1 1 fm 0 _gates tget 0.01 port * 

1 _notes tget 0.001 port mtof 0.3  1 1 1 fm 1 _gates tget 0.01 port * 

2 _notes tget 0.001 port mtof 0.3  1 1 1 fm 2 _gates tget 0.01 port * 

3 _notes tget 0.001 port mtof 0.3  1 1 1 fm 3 _gates tget 0.01 port * 

+ + +

-3 ampdb *

_play get *
dup

_spigot fc
)
