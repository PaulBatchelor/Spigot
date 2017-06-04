(
_spigot "./spigot.so" fl
_clk var

4 metro dup _clk set

1
"ex/tracker.rnt"

_spigot fe 


0 _notes tget mtof 0.3 sine 0 _gates tget _play get * 0.01 port * 

_play get *
dup

_spigot fc
)
