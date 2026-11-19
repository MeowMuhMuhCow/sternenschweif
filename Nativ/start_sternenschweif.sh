#!/usr/bin/env bash
# ===========================================================================
# Startskript fuer Das Schwarze Auge: Sternenschweif (Nativer Linux/macOS-Port)
# ===========================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_EXE_NAME="${PORT_EXE_NAME:-SCHWEIF_PORT}"
PROBENLOG="${PROBENLOG:-0}"

if [[ -d "$SCRIPT_DIR/lib" ]]; then
    export LD_LIBRARY_PATH="$SCRIPT_DIR/lib:${LD_LIBRARY_PATH:-}"
fi

if [[ "${1:-}" == "--probenlog" || "${1:-}" == "log" ]]; then
    PROBENLOG=1
    shift
fi

has_star_dat() {
    local d="$1"
    [[ -d "$d" ]] && find "$d" -maxdepth 2 \( -iname "STAR.DAT" -o -iname "SCHWEIF.EXE" \) -print -quit 2>/dev/null | grep -q .
}

SPIELPFAD="${SPIELPFAD:-}"
if [[ -n "${1:-}" ]] && has_star_dat "$1"; then
    SPIELPFAD="$(cd "$1" && pwd)"
    shift
fi

for sp_file in "$SCRIPT_DIR/SPIELPFAD.txt" "$SCRIPT_DIR/../SPIELPFAD.txt" "$PWD/SPIELPFAD.txt"; do
    if [[ -z "$SPIELPFAD" && -f "$sp_file" ]]; then
        CAND_FILE="$(tr -d '\r\n' < "$sp_file")"
        if [[ "$CAND_FILE" =~ ^([A-Za-z]):[\\/](.*)$ ]]; then
            DRV="$(echo "${BASH_REMATCH[1]}" | tr 'A-Z' 'a-z')"
            REL="${BASH_REMATCH[2]//\\//}"
            CAND_FILE="/mnt/$DRV/$REL"
        fi
        if [[ -n "$CAND_FILE" ]] && has_star_dat "$CAND_FILE"; then
            SPIELPFAD="$CAND_FILE"
            break
        fi
    fi
done

if [[ -z "$SPIELPFAD" ]] || ! has_star_dat "$SPIELPFAD"; then
    for cand in \
        "$SCRIPT_DIR" \
        "$SCRIPT_DIR/.." \
        "$HOME/GOG Games/Realms of Arkania 2" \
        "$HOME/GOG Games/Das Schwarze Auge - Sternenschweif" \
        "$HOME/GOG Games/Sternenschweif" \
        "$HOME/.local/share/Steam/steamapps/common/Realms of Arkania 2" \
        "$HOME/.local/share/Steam/steamapps/common/Realms of Arkania - Star Trail Classic" \
        "$HOME/Games/gog/realms-of-arkania-2" \
        "$HOME/Games/Heroic/Realms of Arkania 2" \
        "$HOME/.wine/drive_c/GOG Games/Realms of Arkania 2"; do
        if has_star_dat "$cand"; then
            SPIELPFAD="$(cd "$cand" && pwd)"
            break
        fi
    done
fi

if [[ -z "$SPIELPFAD" ]] || ! has_star_dat "$SPIELPFAD"; then
    echo "[FEHLER] Spielordner mit DATA/STAR.DAT bzw. SCHWEIF.EXE nicht gefunden." >&2
    echo "Bitte Pfad in $SCRIPT_DIR/SPIELPFAD.txt eintragen oder als ersten Parameter uebergeben:" >&2
    echo "  ./start_sternenschweif.sh \"/pfad/zu/Sternenschweif\"" >&2
    exit 1
fi

BASE_NAME="${PORT_EXE_NAME%.EXE}"
PORT_BIN=""
for bin in \
    "$SCRIPT_DIR/$BASE_NAME" \
    "$SCRIPT_DIR/build/$BASE_NAME" \
    "$SCRIPT_DIR/../build/$BASE_NAME" \
    "$SCRIPT_DIR/SCHWEIF_PORT" \
    "$SCRIPT_DIR/build/SCHWEIF_PORT" \
    "$SCRIPT_DIR/../build/SCHWEIF_PORT"; do
    if [[ -f "$bin" ]]; then
        PORT_BIN="$bin"
        break
    fi
done

if [[ -z "$PORT_BIN" ]]; then
    echo "[FEHLER] Port-Binary '$BASE_NAME' nicht gefunden." >&2
    exit 1
fi

LOG_DIR="$SPIELPFAD"
if ! touch "$SPIELPFAD/.schweif_wtest_$$" 2>/dev/null; then
    LOG_DIR="$(dirname "$PORT_BIN")/Spielstaende"
    mkdir -p "$LOG_DIR"
else
    rm -f "$SPIELPFAD/.schweif_wtest_$$"
fi

CSV_DATEI="$LOG_DIR/PROBEN.CSV"
if [[ -s "$CSV_DATEI" ]]; then
    mkdir -p "$SCRIPT_DIR/Probenlogs"
    TS="$(date -r "$CSV_DATEI" +%Y-%m-%d_%H-%M-%S 2>/dev/null || date +%Y-%m-%d_%H-%M-%S)"
    mv -f "$CSV_DATEI" "$SCRIPT_DIR/Probenlogs/PROBEN_${BASE_NAME}_${TS}.CSV"
fi
rm -f "$CSV_DATEI"

TAIL_PID=""
cleanup() {
    if [[ -n "$TAIL_PID" ]]; then
        kill "$TAIL_PID" 2>/dev/null || true
    fi
}
trap cleanup EXIT

if [[ "$PROBENLOG" == "1" ]]; then
    : > "$CSV_DATEI"
    if [[ "${DSA_NO_PROBENLOG_WINDOW:-0}" != "1" ]]; then
        if [[ -f "$SCRIPT_DIR/Probenlog/probenlog_fenster.py" ]]; then
            python3 "$SCRIPT_DIR/Probenlog/probenlog_fenster.py" "$CSV_DATEI" &
            TAIL_PID=$!
        elif [[ -f "$SCRIPT_DIR/Probenlog/probenlog.py" ]]; then
            python3 "$SCRIPT_DIR/Probenlog/probenlog.py" --live "$CSV_DATEI" &
            TAIL_PID=$!
        fi
    fi
fi

echo "Starte Sternenschweif ($BASE_NAME) mit Spieldaten aus: $SPIELPFAD"
"$PORT_BIN" --gamedir "$SPIELPFAD" "$@"
