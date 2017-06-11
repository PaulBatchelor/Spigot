(
_spigot "./spigot.so" fl
_notes "60 60 62 64 67 69 71 72 74" gen_vals
_val var
_clk var
_env var

0.3 dmetro _clk set

_clk get 1


"ex/ex.rnt"

_spigot fe 

_clk get _output get * 0.001 0.01 0.1 tenvx _env set
_env get _output get 
dup 0 ne samphold 
_notes tget mtof 0.3 
1 7 _env get 1 *  fm *

_spigot fc
)
