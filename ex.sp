(
_list 10 "examples.txt" slist
_spigot "./spigot.so" fl
_notes "60 60 62 64 67 69 71 72 74" gen_vals
_val var
_clk var
_env var

0.3 dmetro

_clk set

2 _clk get 

##############################################################################
##: ## Insert your string here

##: this line reads a line from the file "examples.txt"

# 1 _list sget



##: OR you could just provide a string here




"+[>><<+.]"





##############################################################################


_spigot fe _val set

_val get 0.001 0.01 0.1 tenvx _env set
_env get _val get 
dup 0 ne samphold 
_notes tget mtof 0.3 
1 7 _env get 1 *  fm *

dup
_spigot fc
)
