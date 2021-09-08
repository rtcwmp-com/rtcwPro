# Cvar / Command list

<br>

> #### The following conventions are used to describe argument types:
> `string` - normal text characters  
> `integer` - whole numbers variable, e.g. 1, 2, 3, 4  
> `float` - floating point value variable, e.g. 1.0, 0.9, 0.8, 0.7  
> `bitflag` - binary digit whole value, e.g. 1, 2, 4, 8, 16 (or add 1+2 = 3, 1+2+4 = 7, etc)  

> #### Cvar color table  
`white` `red` `green` `blue` `yellow` `magenta` `cyan` `orange` `mdred` `mdgreen` `dkgreen` `mdcyan` `mdyellow` `mdorange` `mdblue` `ltgrey` `mdgrey` `dkgrey` `black`  

# Client  
#### Engine/renderer (cl/r)
| Cvar | Argument type | Value range | Default | Description |
|---------|---------------|-------------|---------|-------------|
| in_mouse | `integer` | `0-2`| `1` | Switch mouse input type between default and raw `(2)` |
| con_height | `float` | `0.1-1.0` | `0.5` | Change console height |
| cl_activateLean | `integer` | `0-1` | `1` | Toggle leaning when holding move keys and +activate |
| r_bloom | `integer` | `0-1` | `0` | Enable/disable bloom effect |

#### Mod (cg)
| Cvar | Argument type | Value range | Default | Description |
|---------|---------------|-------------|---------|-------------|
| cg_crosshairPulse | `integer` | `0-1` | `1` | Enable/disable pulsing of the crosshair. |
| cg_showFlags | `integer` | `0-1` | `1` | Enable/disable scoreboard coutnry flags |
| cg_bloodDamageBlend | `float` | `0.0-1.0` | `1.0` | Control blood flends when getting shot |
| cg_bloodFlash | `float` | `0.0-1.0` | `1.0` | Control blood flash when getting shot |
| cg_crosshairAlpha | `float` | `0.0-1.0` | `1.0` | Control crosshair opacity |
| cg_crosshairAlphaAlt | `float` | `0.0-1.0` | `1.0` | Control crosshair alt opacity |
| cg_crosshairColor | `string` | Color table | `white` | Change crosshair color |
| cg_crosshairColorAlt | `string` | Color table | `white` | Change crosshair color alt |
| ch_font | `integer` | `0-2` | `0` | Enable/disable OSP fonts |
| cg_drawWeaponIconFlash | `integer` | `0-1` | `0` |  Enable/disable weapon icon flashing when empty |
| cg_printObjectiveInfo | `integer` | `0-1` | `1` | Enable/disable printing of OBJ notifications in kill feed |
| cg_htisounds | `bitflag` | `1-2-4` | `0` | Control hitsounds |
| cg_drawSpeed | `integer` | `0-4` | `0` | Draw speed meter. `1` - current only, `2` - current and top, `3` - current with accel color, `4` - current and top with accel color |
| cg_speedX | `integer` | `-999-999` | `315` | Change speed meter position in the horizontal axis |
| cg_speedY | `integer` | `-999-999` | `340` | Change speed meter position in the vertical axis |
| cg_tracers | `integer` | `0-3` | `1` | Draw bullet tracers. `1` - all, `2` - own only, `3` - other's only |
| cg_drawTriggers | `integer` | `0-1` | `1` | Draw objeective triggers (for Shoutcasters only) |

