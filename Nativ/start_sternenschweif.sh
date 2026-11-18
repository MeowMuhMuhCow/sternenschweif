#!/usr/bin/env bash
# ===========================================================================
# Startskript fuer Das Schwarze Auge: Sternenschweif (Nativer Port, Linux/Wine/WSL)
# ===========================================================================
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PORT_EXE_NAME="${PORT_EXE_NAME:-SCHWEIF_PORT.EXE}"
PROBENLOG="${PROBENLOG:-0}"

if [[ "${1:-}" == "--probenlog" || "${1:-}" == "log" ]]; then
    PROBENLOG=1
    shift
fi

SPIELPFAD="${SPIELPFAD:-}"
if [[ -n "${1:-}" && ( -f "$1/DATA/STAR.DAT" || -f "$1/STAR.DAT" || -f "$1/SCHWEIF.EXE" ) ]]; then
    SPIELPFAD="$1"
    shift
fi

if [[ -z "$SPIELPFAD" && -f "$SCRIPT_DIR/SPIELPFAD.txt" ]]; then
    SPIELPFAD="$(tr -d '\r\n' < "$SCRIPT_DIR/SPIELPFAD.txt")"
fi

if [[ -z "$SPIELPFAD" ]]; then
    for cand in \
        "$SCRIPT_DIR" \
        "$SCRIPT_DIR/.." \
        "$HOME/GOG Games/Realms of Arkania 2" \
        "$HOME/.wine/drive_c/GOG Games/Realms of Arkania 2"; do
        if [[ -f "$cand/DATA/STAR.DAT" || -f "$cand/STAR.DAT" || -f "$cand/SCHWEIF.EXE" ]]; then
            SPIELPFAD="$(cd "$cand" && pwd)"
            break
        fi
    done
fi

if [[ -z "$SPIELPFAD" ]]; then
    echo "[FEHLER] Spielordner mit DATA/STAR.DAT bzw. SCHWEIF.EXE nicht gefunden." >&2
    echo "Bitte Pfad in $SCRIPT_DIR/SPIELPFAD.txt eintragen oder als ersten Parameter uebergeben." >&2
    exit 1
fi

printf '%s\n' "$SPIELPFAD" > "$SCRIPT_DIR/SPIELPFAD.txt"

EXE="$SCRIPT_DIR/$PORT_EXE_NAME"
[[ ! -f "$EXE" ]] && EXE="$SCRIPT_DIR/build/$PORT_EXE_NAME"
[[ ! -f "$EXE" ]] && EXE="$SCRIPT_DIR/SCHWEIF_PORT.EXE"
[[ ! -f "$EXE" ]] && EXE="$SCRIPT_DIR/build/SCHWEIF_PORT.EXE"

if [[ ! -f "$EXE" ]]; then
    echo "[FEHLER] $PORT_EXE_NAME nicht gefunden. Bitte zuerst bauen: python3 Port/bauen_port.py" >&2
    exit 1
fi

LOG_DIR="$SPIELPFAD"
if ! touch "$SPIELPFAD/.schweif_wtest.tmp" 2>/dev/null; then
    LOG_DIR="$SCRIPT_DIR/Spielstaende"
    mkdir -p "$LOG_DIR"
else
    rm -f "$SPIELPFAD/.schweif_wtest.tmp"
fi

CSV_DATEI="$LOG_DIR/PROBEN.CSV"
if [[ -s "$CSV_DATEI" ]]; then
    mkdir -p "$LOG_DIR/Probenlogs"
    TS="$(date -r "$CSV_DATEI" +%Y-%m-%d_%H-%M-%S 2>/dev/null || date +%Y-%m-%d_%H-%M-%S)"
    mv -f "$CSV_DATEI" "$LOG_DIR/Probenlogs/PROBEN_${TS}.CSV"
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
    PY_SKRIPT="$SCRIPT_DIR/Probenlog/probenlog.py"
    [[ ! -f "$PY_SKRIPT" && -f "$SCRIPT_DIR/../Probenlog/probenlog.py" ]] && PY_SKRIPT="$SCRIPT_DIR/../Probenlog/probenlog.py"
    if [[ -f "$PY_SKRIPT" ]]; then
        tail -n +1 -f "$CSV_DATEI" | python3 "$PY_SKRIPT" &
        TAIL_PID=$!
    fi
fi

if command -v wine >/dev/null 2>&1 && [[ "$(uname -r)" != *microsoft* && "$(uname -r)" != *WSL* ]]; then
    wine "$EXE" --gamedir "$SPIELPFAD" "$@"
else
    "$EXE" --gamedir "$SPIELPFAD" "$@"
fi
