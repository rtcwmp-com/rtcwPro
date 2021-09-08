# Cvar / Command list

<br>

> #### The following conventions are used to describe command arguments:
> `string` - normal text characters  
> `integer` - whole numbers variable, e.g. 1, 2, 3, 4  
> `float` - floating point value variable, e.g. 1.0, 0.9, 0.8, 0.7  
> `bitflag` - binary digit whole value, e.g. 1, 2, 4, 8, 16 (or add 1+2 = 3, 1+2+4 = 7, etc)  


# Client  
#### Engine/renderer (cl/r)
| Command | Argument type | Value range | Default | Description |
|---------|---------------|-------------|---------|-------------|
| /in_mouse | `integer` | `0/1/2`| `1` | Switch mouse input type between default and raw `(2)` |
| /con_height | `float` | `0.1/1.0` | `0.5` | Change console height |
| /cl_activateLean | `integer` | `0/1` | `1` | Toggle leaning when holding move keys and +activate |
| /r_bloom | `integer` | `0/1` | `0` | Enable/disable bloom effect |

