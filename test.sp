(
_spigot "./spigot.so" fl
_notes "60 62 64 67 69 71 72 74" gen_vals
_val var
_clk var
_env var

0.3 dmetro _clk set

0 _clk get 
"seq.bf" 
_spigot fe _val set

_val get 0.001 0.01 0.01 tenvx _env set
_env get _val get "val" print _clk get samphold _notes tget mtof 0.3 
1 7 _env get 1 *  fm *

dup
_spigot fc
)
