#!/usr/bin/env bash
set -euo pipefail

# Zaawansowany filtr zapisów partii z katalogu recent_games.
# Wymagania: GNU date (Linux), bash 4+, podstawowe narzędzia coreutils/grep/awk.

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd -- "${SCRIPT_DIR}"/.. && pwd)"
GAMES_DIR="${ROOT_DIR}/recent_games"

if [[ ! -d "${GAMES_DIR}" ]]; then
  echo "Nie znaleziono katalogu recent_games w ${ROOT_DIR}" >&2
  exit 1
fi

# Wybór odmiany (wariantu) - lista podkatalogów w recent_games
mapfile -t __variants < <(find "${GAMES_DIR}" -mindepth 1 -maxdepth 1 -type d -printf '%f\n' 2>/dev/null | sort)
SEARCH_DIR="${GAMES_DIR}"
if (( ${#__variants[@]} > 0 )); then
  echo "Dostępne odmiany w ${GAMES_DIR}:"
  echo "  0) Wszystkie odmiany"
  for i in "${!__variants[@]}"; do
    idx=$((i+1))
    printf '  %d) %s\n' "${idx}" "${__variants[i]}"
  done
  read -r -p $'Wybierz odmianę (numer lub nazwa, Enter = wszystkie): ' __choice || true
  if [[ -n "${__choice}" ]]; then
    # Rozpoznaj numer
    if [[ "${__choice}" =~ ^[0-9]+$ ]]; then
      if (( __choice == 0 )); then
        SEARCH_DIR="${GAMES_DIR}"
      else
        sel_index=$((__choice-1))
        if (( sel_index >= 0 && sel_index < ${#__variants[@]} )); then
          SEARCH_DIR="${GAMES_DIR}/${__variants[sel_index]}"
        else
          echo "Niepoprawny numer odmiany, przeszukam wszystkie odmiany." >&2
          SEARCH_DIR="${GAMES_DIR}"
        fi
      fi
    else
      # Nazwa odmiany
      if [[ -d "${GAMES_DIR}/${__choice}" ]]; then
        SEARCH_DIR="${GAMES_DIR}/${__choice}"
      else
        echo "Nie odnaleziono odmiany '${__choice}', przeszukam wszystkie odmiany." >&2
        SEARCH_DIR="${GAMES_DIR}"
      fi
    fi
    # Informacja o szachach cylindrycznych
    if [[ "${SEARCH_DIR}" == "${GAMES_DIR}/cylinder" || "${SEARCH_DIR}" == "${GAMES_DIR}/cylindrical" ]]; then
      echo "[uwaga] Szachy cylindryczne (cylinder) nie są jeszcze obsługiwane przez program — tylko przeszukiwanie zapisów." >&2
    fi
  fi
fi

normalize_day() {
  # Zamienia polskie/angielskie nazwy/skrót na numer dnia ISO (1=pon). Zwraca pusty gdy nieznany.
  local input="${1,,}"  # lower
  case "${input}" in
    pon|poniedzialek|poniedziałek|mon|monday) echo 1 ;;
    wt|wtorek|tue|tuesday) echo 2 ;;
    sr|sroda|środa|wed|wednesday) echo 3 ;;
    czw|czwartek|thu|thursday) echo 4 ;;
    pt|piatek|piątek|fri|friday) echo 5 ;;
    sob|sobota|sat|saturday) echo 6 ;;
    nd|nie|niedz|niedziela|sun|sunday) echo 7 ;;
    *) echo "" ;;
  esac
}

to_minutes() {
  # Konwersja HH:MM -> minuty od północy.
  local hh=${1%%:*}
  local mm=${1##*:}
  [[ -z ${hh} || -z ${mm} ]] && { echo ""; return; }
  echo $((10#${hh}*60 + 10#${mm}))
}

prompt() {
  local msg=$1 default=${2-}
  if [[ -n "${default}" ]]; then
    read -r -p "${msg} [${default}]: " ans || true
    echo "${ans:-$default}"
  else
    read -r -p "${msg}: " ans || true
    echo "${ans}"
  fi
}

# Interaktywne pytania
echo "=== Filtrowanie zapisów partii (recent_games) ==="
result_hint=$(prompt "Pamiętasz wynik? (white/black/draw/stalemate/timeout)" "")
weekday_hint=$(prompt "Pamiętasz, w jaki dzień tygodnia rozegrano partię? (np. środa/Wednesday)" "")
start_time=$(prompt "Filtr godzinowy - start (HH:MM, Enter aby pominąć)" "")
end_time=$(prompt "Filtr godzinowy - koniec (HH:MM, Enter aby pominąć)" "")
reason_hint=$(prompt "Interesuje Cię powód zakończenia? (checkmate/stalemate/timeout/resign/etc)" "")
opening_hint=$(prompt "Czego szukać w pierwszych ruchach? (np. 'e4 c5', 'Sicilian', Enter aby pominąć)" "")

# Normalizacja kryteriów
weekday_num=""
if [[ -n "${weekday_hint}" ]]; then
  weekday_num=$(normalize_day "${weekday_hint}")
  if [[ -z "${weekday_num}" ]]; then
    echo "[uwaga] Nie rozpoznano dnia tygodnia, filtr zostanie pominięty." >&2
  fi
fi

start_min=""; end_min=""
if [[ -n "${start_time}" || -n "${end_time}" ]]; then
  if [[ -n "${start_time}" && -n "${end_time}" ]]; then
    start_min=$(to_minutes "${start_time}")
    end_min=$(to_minutes "${end_time}")
    if [[ -z "${start_min}" || -z "${end_min}" ]]; then
      echo "[uwaga] Niepoprawny format godzin, pomijam filtr czasu." >&2
      start_min=""; end_min=""
    fi
  else
    echo "[uwaga] Podaj oba końce przedziału czasu; inaczej filtr zostanie pominięty." >&2
    start_min=""; end_min=""
  fi
fi

matches=0
while IFS= read -r -d '' file; do
  stem=$(basename "${file}" .txt)
  IFS=_ read -r date_raw time_raw p3 p4 p5 <<<"${stem}"

  # Data/godzina z nazwy pliku
  if ! game_date_fmt=$(date -d "${date_raw}" +%Y-%m-%d 2>/dev/null); then
    echo "[uwaga] Pomijam plik z nieparsowalną datą: ${file}" >&2
    continue
  fi
  game_time_hhmm="${time_raw:0:2}:${time_raw:2:2}"
  game_minutes=$(to_minutes "${game_time_hhmm}")
  game_weekday=$(date -d "${game_date_fmt}" +%u)

  # Treść pliku
  content=$(cat -- "${file}")
  result_line=$(printf '%s\n' "${content}" | grep -E "^(1-0|0-1|1/2-1/2)" | head -n1 || true)
  reason_line=$(printf '%s\n' "${content}" | grep -i "^Reason:" | head -n1 | sed 's/^Reason:[[:space:]]*//I' || true)
  moves_preview=$(printf '%s\n' "${content}" | head -n 12 | tr '\n' ' ')

  # Normalizacja wyniku
  normalized_result=""
  case "${result_line}" in
    1-0*) normalized_result="white" ;;
    0-1*) normalized_result="black" ;;
    1/2-1/2*) normalized_result="draw" ;;
  esac
  # Fallback z nazwy
  if [[ -z "${normalized_result}" ]]; then
    if [[ ${p3} == "white" && ${p4} == "win" ]]; then
      normalized_result="white"
    elif [[ ${p3} == "black" && ${p4} == "win" ]]; then
      normalized_result="black"
    elif [[ ${p3} == "stalemate" || ${p4} == "stalemate" ]]; then
      normalized_result="draw"
    elif [[ ${p4} == "timeout" ]]; then
      normalized_result="timeout"
    fi
  fi

  # Filtry
  [[ -n "${weekday_num}" && "${game_weekday}" != "${weekday_num}" ]] && continue

  if [[ -n "${start_min}" && -n "${end_min}" ]]; then
    if [[ -z "${game_minutes}" ]]; then
      continue
    fi
    if (( game_minutes < start_min || game_minutes > end_min )); then
      continue
    fi
  fi

  if [[ -n "${result_hint}" ]]; then
    if [[ "${normalized_result}" != "${result_hint,,}" ]]; then
      continue
    fi
  fi

  if [[ -n "${reason_hint}" ]]; then
    if ! printf '%s' "${reason_line}" | grep -iq "${reason_hint}"; then
      continue
    fi
  fi

  if [[ -n "${opening_hint}" ]]; then
    if ! printf '%s' "${moves_preview}" | grep -iq "${opening_hint}"; then
      continue
    fi
  fi

  matches=$((matches + 1))
  printf '\n[%02d] %s\n' "${matches}" "${file#${ROOT_DIR}/}"
  printf '  Data/godz: %s %s (dzień ISO %s)\n' "${game_date_fmt}" "${game_time_hhmm}" "${game_weekday}"
  printf '  Wynik: %s\n' "${normalized_result:-nieznany}"
  printf '  Powód: %s\n' "${reason_line:-brak/niezapisany}"
  printf '  Pierwsze ruchy: %s\n' "${moves_preview}"

done < <(find "${SEARCH_DIR}" -maxdepth 1 -type f -name '*.txt' -print0 | sort -z)

if (( matches == 0 )); then
  echo "\nBrak plików spełniających zadane kryteria."
else
  echo "\nZnaleziono ${matches} pasujących plików."
fi
