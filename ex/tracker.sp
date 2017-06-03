(
_spigot "./spigot.so" fl
_clk var

1 metro dup _clk set

1
"ex/tracker.rnt"

_spigot fe 


0 _notes tget mtof 0.3 sine 

dup

_spigot fc
)
