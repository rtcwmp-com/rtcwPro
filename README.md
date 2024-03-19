# RtcwPro

![Alt tag](Assets/rtcwpro.png?raw=true "Title")

Return to Castle Wolfenstein Mod/Engine - based on OSPx/xMod (created by Nate) with additions from RtcwPub, ioRtcw, Enemy Territory, ET Legacy, ETPub, and other Q3 mods

Main objective: create a competition mod similar to OSP with updates for tournaments and pugs

To run a RtcwPro server (Linux only) you may use Msh Docker available here: https://github.com/msh100/rtcw

To install RtcwPro client follow these instructions: https://rtcwpro.com/install-instructions.php

If you have any questions/comments/concerns then feel free to reach out to us on Discord: https://discord.gg/fn9JVWnbTx

RtcwPro Dev Team
-nihilist, KrazyKaze, Tarator, Snappas

Contributions from Nate, Dutchmeat, Nobo, S4NDM4NN, crumbs, and Spaztik

Additional credits for public code: Nobo, Nico, suburb, jinx, rhea, OpenJK, ETLegacy, ETpub, Jaymod, ioquake, iortcw.
Installer powered by Advanced Installer: https://www.advancedinstaller.com/

**Change Log**
 - 1.0/1.0.1 [changelog](changelog/1.0-changelog.txt)
   - corrected head hitboxes (RtcwPub port)
   - faster PK3 downloads (ioRtcw port)
   - unlag antilag
   - antiwarp
   - player HUD names during spectating
   - updated statistics
   - global level server configuration (ET port)
   - server cvar restrictions (ET port)
   - sv_fps fix for flamethrower
   - allow teams feature for ET map porting (i.e. specific team can open doors with lock)
   - minimizer using "minimize" console command
 
 - 1.1 [changelog](changelog/1.1-changelog.txt)
   - dead bodies cannot grab spawn flags
   - specatator freecam can select a player by aiming at them and pressing +attack
   - JSON stat files created for web stats (in progress)
 
 - 1.1.1 [changelog](changelog/1.1.1-changelog.txt)
   - UPS meter (cg_drawSpeed)
   - Added headshot damage cvar to help with antilag/hitbox changes (g_hsDamage)
   
 - 1.1.2 [changelog](changelog/1.1.2-changelog.txt)
   - added nopicmip to tree/ivy/truss shaders so high values of picmip will not remove them

 - 1.2 [changelog](changelog/1.2-changelog.txt)
   - Release 1.2 includes custom client (wolfMP.exe) to allow cvar restrictions, http downloads, and many other features
   - various renderer and overflow fixes
   - add http downloads: cl_wwwdownload, sv_wwwdownload, sv_wwwbaseurl
   - add new resolutions (r_mode): /modelist
   - skybox fixes, clamping etc
   - add r_bloom (demo only)
   - add r_textureanisotropy
   - new flaring, r_flareCoeff etc
   - reduce only non-radius damage knockback
   - fix knockback issues during revives
   - restore revive boosting
   - fix clients hanging on level change
   - fix vote exploits: suicide etc
   - userinfo exploit fixes
   - nuke string fixes
   - add ipv6 support
   - reworked respawn server: safer transitions
   - add new menu script handling
   - add mouse wheel scrollable server list
   - default primitives to 2
   - equalize default values for most cvars with pb cvar list
   - fix download exploit
   - add custom spawn of entities: fs/maps/mapname.spawns
   - add raw mouse input: /in_mouse 2 /in_restart
   - add json stats generation
   - post json stats to remote url: g_stats_curl_submit
   - fix pause limits
   - add additional pause info on pause
   - fix crash on callvote map during pause
   - add new colors and add support for them in console
   - fix ref's status during pause
   - fix obj disappearing into map geometry
   - fix players collision into world: related to the above (very old vanilla bug)
   - add comprehensive cvar restriction: sv_gameconfig <filename> or /config <filename>
   - thread stats submission and other curl calls to avoid lag
   - fix dynos moving during pause
   - remove /r_wolffog from cheat protected cvars
   - add cg_hitsounds and g_hitsounds: latter controls it
   - add new country flags
   - add new logo
   - remove SP asset stuff
   - add challenge validation on the server side
   - add UDP filtering on the server side to prevent common flood attacks against servers
   - fix team overlay with teams greater than teammaxoverlay
   - fix crash with stats related to dyno splash damage
   - fix incorrect dyno defused print for defending team dynos
   - fix player muting
   - fix pause-game clock related issues
   - fix /handicap bugs: removed the command altogether
   - fix random player class changes
   - fix demo view options, third person etc
   - add TAB <key> value expansion in console for current cvar values
   - fix infinite load screens during map change
   - fix num maps limit on the server side: tested with 100
   - fix taking damage from world during pause
   - unlatch com_maxfps so it doesn't need vid_restart
   - add autoexec_mapname on the client side: e.g. main/autoexec_mp_ice.cfg
   - add enemy timer: /timerset <seconds>
   - fix panzer/instant gib damage issues
   - add g_spawnOffset for spawn time offset between teams: random between 1 and cvar integer - 1
   - refactor antilag to rewind more accurately
   - fix revive anim bug
   - auto s_stop for K_SPACE while watching demos
   - default /com_hunkmegs to 256
   - increase cvar buffer in engine to avoid overflows
   - check if file exists for callvote map in mod
   - add an unload mod button to options > mods
   - fix warmup damage
   - display obj icon for team mates in team overlay
   - new ref command: /ref rename <id>
   - add shoutcaster role /scs <pw>
   - add /ref logout and /scs logout
   - shoutcasters can see dynamite timers above dynos
   - shoutcasters can use /noclip
   - shoutcasters can see obj triggers: cg_drawTriggers
   - shoutcasters can change spectator freecam speed: /specspeed <value>
   - shoutcasters can see both teams' reinforcement time
   - add adjustable console height: con_height 0.1-1 and shortcuts for it e.g. shift+console key and alt+console key
   - toggle bodies grabbing flags: g_bodiesGrabFlags
   - allow join during pause
   - add custom screen shake: g_screenShake
   - default r_mode to 6
   - add cg_tracers

 - 1.2.1 [changelog](changelog/1.2.1-changelog.txt)  
   - Note for linux servers: always run the image with root permissions!
   - client: fix /map and /devmap on the client side
   - client: display current round time in warmup between rounds
   - client: add new style for RT: cg_drawReinforcementTime 1 = default, 2 = new, 3 = default and new
   - client: add new style for ERT: cg_drawEnemyTimer 1 = default, 2 = new, 3 = default and new
   - client: change RT color: cg_reinforcementTimeColor green = default
   - client: change ERT color: cg_enemyTimerColor red = default
   - client: change default RT position: cg_reinforcementTimeX 95 = default, cg_reinforcementTimeY 50 = default
   - client: change new RT position: cg_reinforcementTimeProX 145 = default, cg_reinforcementTimeProY 445 = default
   - client: change default ERT position: cg_enemyTimerX 98 = default, cg_enemyTimerY 60 = default
   - client: change new ERT position: cg_enemyTimerProX 185 = default, cg_enemyTimerProY 445 = default
   - client: add autoexec_team: _axis, _allies, and _spectator
   - client: add autoexec_class: _s _e _m _l (soldier, engineer, medic, lieutenant)
   - client: fix a keyboard bind key issue on azerty keyboards
   - client: split (bitflag) cg_hitsounds: 1 = hs only, 2 = body only, 4 = team only, 7 = all (1+2+4)
   - client: add cg_findMedic 1 = default: toggle camera lock at medics when waiting for a revive
   - client: free up the ^ char so it can be used in con notify and names
   - client: minor draw fixes to accommodate new stuff
   - client: add cg_drawGun 2: hide only weapons (not holdables)
   - client: change console color: con_color -1 = default, int from 0 to 31 based on vanilla color codes/keys
   - server: shoutcasters always noclip
   - server: fix bug that causes abnormal cpu usage and ping
   - server: fix a bug missed on 1.2 with obj getting lost in solids
   - server: fix bug where medics will drop weapon on death
   - server: include server IP in stats
   - server: fix specinvites not being transferred between sessions
   - server: fix sv_gameConfig not loading on startup: cvar is now read only (set from command line on startup)
   - server: add g_mapScriptDirectory "" = default: folder needs to be in fs
   - server: fix validaiton for cvar rest causing incorrect violation kicks
   - server: fix end of round announcer sounds
   - server: fix stats offset bug with map_restart on SW2
   - server: fix swap after map_restart on SW2
   - server: add objective captures to json events and player stats
 
 - 1.2.2 [changelog](changelog/1.2.2-changelog.txt)  
   - server: lowercase expected file name for map configs (linux)
   - server: lowercase expected file name for .spawns files (linux)
   - server: only exec map configs on map load (not map restart)
   - server: fix ready status being reset on client death
   - server: shoutcasters can follow other shoutcasters by /follow id
   - server: shoutcasters can /followobj to follow active obj carrier (if any)
   - server: shoutcasters can /noclip
   - server: add map name search on /callvote map
   - server: add /maps to list maps on server
 
 - 1.2.3 [changelog](changelog/1.2.3-changelog.txt)  
   - server: fix early airstrikes bug
   - server: add vote_allow_cointoss  
 
 - 1.2.4 [changelog](changelog/1.2.4-changelog.txt)  
    - server: fix callvote map matching of maps that differ by number  
    - server: add g_damageRadiusKnockback to change explosions knockback, default 1000
    - server: do not submit stats for early exit rounds
    - server: do not submit stats if no more than 2 players are active
    - server: add server country to json output
    - server: fix bug where clients spawn with varying health instead of respecting number of team medics
    - server: fix hitsound sequence issues
    - client: change default for cg_reinforcementTimeColor to red
    - client: change default for cg_enemyTimerColor to green
    - client: add cl_activateLean to toggle leaning when holding move keys and +activate, default 1
    - client: add cg_hitsoundBodyStyle 1-5 to change body hitsound, default 1
    - client: add cg_hitsoundHeadStyle 1-8 to change head hitsound, default 1
    - client: add cg_notifyTextX and cg_notifyTextY to change kill feed position
    - client: add cg_notifyTextWidth to change kill feed char width, default 8
    - client: add cg_notifyTextHeight to change kill feed char height, default 8
    - client: add cg_notifyTextShadow to toggle shadowing of kill feed
    - client: add cg_chatX and cg_chatY to change chat position
    - client: add cg_teamOverlayX and cg_teamOverlayY to change team overlay position
    - client: add cg_compassX and cg_compassY to change compass position
    - client: add cg_lagometerX and cg_lagometerY to change lagometer position
    - client: add cg_drawFrags to toggle "you killed" frag center prints
    - client: add cg_fragsY to change frag center print position
    - client: add cg_fragsWidth to change frag center print char width size, default 16
    - client: add cg_zoomedSensLock to toggle zoom sens lock when zooming in
    - client: add cg_pauseMusic
    - client: un-hardcode cg_zoomedSens
    - client: disable http due to overflows causing crashes
    - client: deprecate con_color due to a possible crash
    - client: only draw triggers in freecam
    - client: don't draw dynamite timers when scoreboard is up
    - client: remove unneeded delay in the default body hitsound
    - client: fix bug where raw input will keep initializing for no reason
    - client: fix draw obj icon not updating when it should
    - client: fix weapon switch to pistol at dropweapon when holding ammo packs
    - client: fix spawn shield icons not showing up
    - client: fix player bounding box collision/sticking
    - client: optimize events handling  
 
  - 1.2.5 [changelog](changelog/1.2.5-changelog.txt)  
    - client/server: revert #323 fix that 'broke' hitreg  
    - server: clean out SP entities on the server side to prevent crashes when loading SP maps  
    - server: make sv_checkversion read only  
    - client: revert events to 1.2.3 state  
    - client: deprecate cg_pauseMusic due to a possible crash  
    
  - 1.2.7 [changelog](changelog/1.2.7-changelog.txt)  
    - asset: Added quake head hitsound - headStyle 9
    - asset: Added new country flags
    - asset: Added new medpack image with more red color
    - config: Added cg_errordecay, r_showtris, and r_shownormals to server template configs 
    - client: Added custom console colors and alpha
    - client: Adjusted raw mouse input
    - client: Fixed cdkey generation  
    - client: Added cg_muzzleFlash 2 to show flash on client and enemy
    - client: Added mouse buttons 4 and 5
    - client/server: Print who issued /readyteam in console 
    - client/server: Center print when opponent loses objective
    - client/server: Improved end of round sound duplication
    - client/server: Added download message for invalid client version 
    - client/server: Fixed footstep bobbing for high FPS  
    - client/server: Fixed cg gun frame crash on team switch
    - server: Kick players with shared guids that are messing up stats
    - server: Fixed kick voting so it only looks at player count on the team that called the vote
    - server: Fixed team locking during warmup/disconnects
    - server: Prevent document revive bug
    - server: Fixed artillery instant kill bug
    - server: Added cvar to control forcetapout
    - server: Fixed map voting when loading mapindex 0
    - server: Fixed knockback for fps  
    - server: Fixed maxlives so players do not respawn 
    - server: Fixed objective capture stats
    - server: Fixed vote percent for non startmatch votes 
    - server: Record filename when FS_FileForHandle is NULL
    - server: #387 Fixed grenade splash damage 
    - server: Added a few logging prints for json stats submit to API
    - server: Fixed stats sync issue
    - server: #345 change to next best weapon when killed with primed grenade
    - server: Fixed spawn flag stats
    - server: #382 reset objective and dyno stats on end of AB round
    - server: Issue #379 Fixed warmup stats
    - server: Issue #372 Fixed objective destroyed stats 
    - server: Added 'true' ping from rtcwPub
    - server: Fixed physics for FPS (1.2.61 already released to a few servers)
    - server: Added /maps command

- 1.2.8 [changelog](changelog/1.2.8-changelog.txt)
  
- 1.2.9 [changelog](changelog/1.2.9-changelog.txt)
  
- 1.3 [changelog](changelog/1.3-changelog.txt)
  
- 1.3.1 [changelog](changelog/1.3.1-changelog.txt)  
