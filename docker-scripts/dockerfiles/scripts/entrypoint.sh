#!/bin/bash

# Set config defaults
CONF_REDIR=${REDIRECTURL:-"http://rtcw.life/files/mapdb"}
CONF_PORT=${MAP_PORT:-27960}
CONF_STARTMAP=${STARTMAP:-mp_ice}
CONF_HOSTNAME=${HOSTNAME:-RTCW}
CONF_MAXCLIENTS=${MAXCLIENTS:-32}
CONF_PASSWORD=${PASSWORD:-""}
CONF_RCONPASSWORD=${RCONPASSWORD:-""}
CONF_REFPASSWORD=${REFEREEPASSWORD:-""}
CONF_SCPASSWORD=${SCPASSWORD:-""}
CONF_TIMEOUTLIMIT=${TIMEOUTLIMIT:-1}
CONF_SERVERCONF=${SERVERCONF:-"defaultcomp"}
CONF_SETTINGSGIT=${SETTINGSURL:-"https://github.com/Oksii/rtcw-config.git"}
CONF_SETTINGSBRANCH=${SETTINGSBRANCH:-"master"}
CONF_CHECKVERSION=${CONF_CHECKVERSION:-"17"}
CONF_STATS_SUBMIT=${STATS_SUBMIT:-"0"}
CONF_STATS_URL=${STATS_URL:-"https://rtcwproapi.donkanator.com/submit"}

AUTO_UPDATE=${AUTO_UPDATE:-"true"}

GAME_BASE="/home/game"
SETTINGS_BASE="${GAME_BASE}/settings"

# Update the configs git directory
if [ "${AUTO_UPDATE}" == "true" ]; then
    echo "Checking if any configuration updates exist to pull"
    if git clone --depth 1 --single-branch --branch "${CONF_SETTINGSBRANCH}" "${CONF_SETTINGSGIT}" "${SETTINGS_BASE}.new"; then
        rm -rf "${SETTINGS_BASE}"
        mv "${SETTINGS_BASE}.new" "${SETTINGS_BASE}"
    else
        echo "Configuration repo could not be pulled," \
            "using latest pulled version"
    fi
fi

declare -A default_maps=(
    [mp_assault]="mp_pak0"
    [mp_base]="mp_pak0"
    [mp_beach]="mp_pak0"
    [mp_castle]="mp_pak0"
    [mp_depot]="mp_pak0"
    [mp_destruction]="mp_pak0"
    [mp_sub]="mp_pak0"
    [mp_village]="mp_pak0"
    [mp_trenchtoast]="mp_pakmaps0"
    [mp_ice]="mp_pakmaps1"
    [mp_keep]="mp_pakmaps2"
    [mp_chateau]="mp_pakmaps3"
    [mp_tram]="mp_pakmaps4"
    [mp_dam]="mp_pakmaps5"
    [mp_rocket]="mp_pakmaps6"
)

run_mutations() {
    map="${1}"; shift
    map_mutated=0

    # Run global mutations on this map
    if [ -f "${SETTINGS_BASE}/map-mutations/global.sh" ]; then
        bash "${SETTINGS_BASE}/map-mutations/global.sh" \
            "${GAME_BASE}/tmp/maps/${map}.bsp" \
            "${GAME_BASE}/tmp/maps/${map}.bsp.tmp"
        if [ -f "${GAME_BASE}/tmp/maps/${map}.bsp.tmp" ]; then
            map_mutated=1
            echo "Global mutation happened on ${map}"
            mv "${GAME_BASE}/tmp/maps/${map}.bsp.tmp" \
                "${GAME_BASE}/tmp/maps/${map}.bsp"
        fi
    fi

    # Check if a map mutations script exist for this map and execute it
    if [ -f "${SETTINGS_BASE}/map-mutations/${map}.sh" ]; then
        echo "Running mutations script on ${map}"
        bash "${SETTINGS_BASE}/map-mutations/${map}.sh" \
            "${GAME_BASE}/tmp/maps/${map}.bsp"
        map_mutated=1
    fi

    if [ "${map_mutated}" == "1" ]; then
        mkdir -p "${GAME_BASE}/rtcwpro/maps"
        mv "${GAME_BASE}/tmp/maps/${map}.bsp" \
            "${GAME_BASE}/rtcwpro/maps/${map}.bsp"
    else
        echo "No mutations were made to ${map}"
    fi
}

# Iterate over all maps and download them if necessary
export IFS=":"
for map in $MAPS; do
    if [ -n "${default_maps[$map]}" ]; then
        echo "${map} is a default map so we will not attempt to download"
        continue
    fi

    if [ ! -f "${GAME_BASE}/main/${map}.pk3" ]; then
        echo "Attempting to download ${map}"
        if [ -f "/maps/${map}.pk3" ]; then
            echo "Map ${map} is sourcable locally, copying into place"
            cp "/maps/${map}.pk3" "${GAME_BASE}/main/${map}.pk3.tmp"
        else
            # TODO: We make no effort to ensure this was successful, maybe we
            # should attempt to retry or at the very least try and skip the
            # mutations that happen further on in the loop.
            wget -O "${GAME_BASE}/main/${map}.pk3.tmp" "${CONF_REDIR}/$map.pk3"
        fi

        mv "${GAME_BASE}/main/${map}.pk3.tmp" "${GAME_BASE}/main/${map}.pk3"
    fi

    # This is the place we run mutations on the BSPs contained within maps.
    rm -rf "${GAME_BASE}/rtcwpro/maps/${map}.bsp"
    mkdir -p "${GAME_BASE}/tmp/"
    unzip "${GAME_BASE}/main/${map}.pk3" -d "${GAME_BASE}/tmp/"

    run_mutations "${map}"

    rm -rf "${GAME_BASE}/tmp/"
done

# We need to still run mutations on default maps if they exist.
for map in "${!default_maps[@]}"; do
    rm -rf "${GAME_BASE}/rtcwpro/maps/${map}.bsp"

    echo "Running mutations on default map ${map}"
    mkdir -p "${GAME_BASE}/tmp/maps/"
    unzip \
        -j "main/${default_maps[$map]}.pk3" \
        -d "${GAME_BASE}/tmp/maps/" \
        "maps/${map}.bsp"

    run_mutations "${map}"

    rm -rf "${GAME_BASE}/tmp/"
done

# We need to cleanup mapscripts on every invokation as we don't know what is
# going to exist in the settings directory.
for mapscript in "${GAME_BASE}/rtcwpro/maps/"*.script; do
    [ -f "${mapscript}" ] || break
    rm -rf "${mapscript}"
done

for mapscript in "${SETTINGS_BASE}/mapscripts/"*.script; do
    [ -f "${mapscript}" ] || break
    cp "${mapscript}" "${GAME_BASE}/rtcwpro/maps/"
done

# Only configs live within the config directory so we don't need to be careful
# about just recreating this directory.
rm -rf "${GAME_BASE}/rtcwpro/configs/"
mkdir -p "${GAME_BASE}/rtcwpro/configs/"
cp "${SETTINGS_BASE}/configs/"*.config "${GAME_BASE}/rtcwpro/configs/"

# We need to set g_needpass if a password is set
if [ "${CONF_PASSWORD}" != "" ]; then
    CONF_NEEDPASS='set g_needpass "1"'
fi

# Iterate over all config variables and write them in place
cp "${SETTINGS_BASE}/server.cfg" "${GAME_BASE}/main/server.cfg"
for var in "${!CONF_@}"; do
    value=$(echo "${!var}" | sed 's/\//\\\//g')
    sed -i "s/%${var}%/${value}/g" "${GAME_BASE}/main/server.cfg"
done
sed -i "s/%CONF_[A-Z]*%//g" "${GAME_BASE}/main/server.cfg"

# Append extra.cfg if it exists
if [ -f "${GAME_BASE}/extra.cfg" ]; then
    cat "${GAME_BASE}/extra.cfg" >> "${GAME_BASE}/main/server.cfg"
fi

# Preload libnoquery if we want to block status queries
if [ "${NOQUERY}" == "true" ]; then
    export LD_PRELOAD="${GAME_BASE}/libnoquery.so"
fi

# Rtcwpro uses a different binary which is provided in their package
binary="${GAME_BASE}/wolfded.x86"

# Exec into the game
exec "${binary}" \
    +set dedicated 2 \
    +set fs_game "rtcwpro" \
    +set com_hunkmegs 512 \
    +set vm_game 0 \
    +set ttycon 0 \
    +set net_ip 0.0.0.0 \
    +set net_port "${CONF_PORT}" \
    +set sv_maxclients "${CONF_MAXCLIENTS}" \
    +set fs_basepath "${GAME_BASE}" \
    +set fs_homepath "${GAME_BASE}" \
    +set sv_GameConfig "${CONF_SERVERCONF}" \
    +set sv_authenabled 0 \
    +set sv_AuthStrictMode 0 \
    +set sv_checkversion "${CONF_CHECKVERSION}" \
    +exec "server.cfg" \
    +map "${CONF_STARTMAP}" \
    "${@}"
