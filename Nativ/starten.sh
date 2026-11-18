#!/bin/sh
# Startet die Windows-EXE. Arbeitsverzeichnis ist der Ordner Port.
cd "$(dirname "$0")/.." || exit 1
EXE="build/SCHWEIF_PORT.EXE"
if [ ! -f "$EXE" ]; then
    echo "build/SCHWEIF_PORT.EXE fehlt. Siehe release/BAUEN.md"
    exit 1
fi
read_pf() {
    [ -f "$1" ] || return 0
    while IFS= read -r line || [ -n "$line" ]; do
        case "$line" in
            ''|\#*) continue ;;
        esac
        DSA_GAMEDIR=$line
        export DSA_GAMEDIR
        return 0
    done < "$1"
}
if [ -z "$DSA_GAMEDIR" ]; then
    read_pf "release/SPIELPFAD.txt"
fi
if [ -z "$DSA_GAMEDIR" ]; then
    read_pf "SPIELPFAD.txt"
fi
exec "./$EXE" "$@"
