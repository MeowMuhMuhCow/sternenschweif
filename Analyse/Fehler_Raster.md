# Fehlerraster SCHWEIF.EXE – Originalfehler neu eingeordnet

Stand 19.10.2026. Grundlage: alle 243 nummerierten Einträge aus `ORIGINALFEHLER.md` (1–233, 300–307, 350–351), jeder an der Stelle im Quelltext geprüft. Die Einzelbewertung steht in `fehler_raster.tsv` (eine Zeile je Nummer). Kuriositäten ohne Nummer sind nicht bewertet. Code und `ORIGINALFEHLER.md` sind unverändert.

## Matrix

| | kritisch | mittel | egal | Summe |
|---|---:|---:|---:|---:|
| **sicher** | 9 | 86 | 129 | 224 |
| **unsicher** | 2 | 1 | 16 | 19 |
| **Summe** | 11 | 87 | 145 | 243 |

- **Patch a) (alles):** 243 Nummern. Davon brauchen 10 keinen Fix, weil sie sich als gewollt bzw. als kein Fehler erwiesen haben: 30, 40, 70, 96, 137, 156, 223, 228, 229 und 303 (toter Debug-Code). Nr. 6 und 212 sind dieselbe Stelle und brauchen nur einen Fix.
- **Patch b) (kritisch + mittel):** 98 Nummern (11 kritisch + 87 mittel).
- **Spielstand:** Keine Behebung ändert das Format von Spielständen oder Datenstrukturen (Spalte überall `nein`). Bei Nr. 25, 174 und 212 bleiben aber alte Spielstände inhaltlich verdorben (negatives Geld, verlorene Zähler, leere Ablagen). Der Fix verhindert nur neue Fälle.

## Wichtigste Fehler je Feld

### sicher · kritisch (9)

| Nr. | Stelle | Kern | So stellt man es nach |
|---:|---|---|---|
| 25 | O200 group::sub_money | `(unsigned long)(money -= n) <= 0` greift nur bei genau 0, Geld wird negativ. money_share() teilt unsigned, ein abgetrennter Held bekommt Hunderte Millionen Heller | mit < 100 Hellern die Stadt-Spende annehmen (oder im Schlafsaal bestohlen werden), dann Gruppe teilen |
| 123 | O078 Chest::take_items | Geld: gutgeschrieben wird `k * 100` (ganzer Stapel), abgezogen nur `take` | Truhe mit Geldstapel, Menge 1 nehmen, wiederholen |
| 212 / 6 | O263 group_confiscate_* | `n` wird nie erhöht, alle beschlagnahmten Sachen sind weg. Die Truhen in der Orkfestung bleiben leer | Gefangennahme im Orkgefängnis, fliehen, Truhen 0xa12/0x1b02 öffnen |
| 192 | O204 Bolzenfalle 0x160b | Schaden als laufende Summe, ab dem 3. Helden tödlich | Dungeon 5: 0x170b (scharf), dann 0x160b ohne Opfergabe |
| 199 | O264 Unrathaufen | ab 4 Funden ist die Bedingung immer falsch, der switch ohne Fall gibt einen undefinierten Gegenstand (und liest einen Müll-String), beliebig oft | Unrat an 0x1a15/0x171d durchsuchen, bis 4 Funde erreicht sind |
| 206 | O239 dwarf_test | `pa` bleibt NULL, sub_hero_le(NULL) schreibt nach 0000:0072 (Segment des INT-1Ch-Vektors) | ≥ 3 Helden, männlicher Zwerg + höchstens ein weiterer handlungsfähiger, Zwergenmutprobe verlieren |
| 93 | O118 tevent_097 | Schleife über alle 7 heroes[], auch NULL. move_item/add_equip_boni auf NULL lesen und schreiben die Interrupttabelle | Weg 97, dem Bauern helfen, mit < 7 handlungsfähigen Helden |
| 14 | O048 use_keyring | bis zu 27 Treffer in `names[17][50]`, Stapelüberlauf | 18 verschiedene Schlüssel an den Schlüsselbund, dann benutzen |

### unsicher · kritisch (2)

- **20, 84** (Händler/Laden): Wenn eine Ware nicht ins Gepäck passt, endet die Verteilung, bezahlt wird trotzdem alles. Es könnte Absicht sein, denn danach folgt eigens die Meldung „mehr gekauft, als Ihr tragen könnt“. Für den Spieler heißt das: Geld weg ohne Ware. Die Warnungen in der Kaufmaske sind selbst kaputt (94, 128).

### sicher · mittel (86) – Auswahl

- **12** zweiter Kampf bei Alarm (Feld 0x120f) · **8** 1/6 der Begegnungen in Gelände 0 fällt weg
- **45, 63** Kräuterfunde nutzen den Zähler eines fremden Weges (beliebig oft ernten)
- **114** Lorenfahrt: Absprung immer gefahrlos · **187** „Sichern“ wirkt in der gewählten Runde nie
- **126** hoher Dialog-AP-Bonus nie · **174** Unmutszähler der Gesprächspartner werden gelöscht
- **140** Pelzwand: `inv[-1]` senkt einen Zauberwert eines anderen Helden dauerhaft
- **138** Shurin-Gift: Präzedenzfehler, Erholung fast nie · **154** Blaue Keuche steckt mit Dumpfschädel an
- **111, 159, 166, 173** Zauber zu billig bzw. Gegnerheilung schadet · **215** Tollwut: Beruhigen umgekehrt
- **5, 153** uninitialisierte Rückgaben in der Reise (Ereignisse bzw. Abbruch zufällig)
- **37** Absturz-Tagebucheintrag bei jedem Durchlauf, mit Müllnamen

### unsicher · mittel (1)

- **13** Spinnenfeld 0x124: Der Kampf kommt immer wieder, bis das Nest an 0x225 zerstört ist. Das ist eher Absicht. Echt ist nur `ret = 0` trotz Kampf.

### sicher · egal (129) – typische Gründe

- **Unerreichbar:** 3, 7, 26, 33–35 (einziger Aufrufer übergibt 0), 54, 61, 105, 164, 167, 226
- **Toter Code:** 81, 211 (`new unsigned char(w*h)`, kein Aufrufer), 300–305
- **Heap ist genullt:** 27, 67, 73, 195. mem_alloc benutzt farcalloc, auch `new` läuft darüber. Viele „uninitialisiert“-Fälle laufen deshalb ins Leere.
- **Kosmetik:** 16, 50, 87, 176, 224

### unsicher · egal (16)

- **Kein Fehler / gewollt:** 30 (bewusstes Durchfallen), 40 (hero_mr_target liefert den Verteidiger), 70 (Diagnose ist gerechtfertigt), 137 (Zustand wird im Dialog gesetzt), 156 (−1 passt zum anderen Menü), 223, 228
- **Offen, ohne Wirkung:** 19, 28, 66, 96, 99, 100, 113, 221, 229

## Auffällige Umbewertungen gegenüber der alten Schwere

| Richtung | Nummern | Grund |
|---|---|---|
| hoch → egal | 3, 16, 17, 29, 31, 33, 34, 35, 40, 41, 50, 52, 56, 112, 198 | unerreichbar (3, 29, 33–35, 52, 56, 198: alle Auslagerungsblöcke < 48 KB), kein Leser (31: old_town/old_dungeon werden nie gelesen), kein Fehler (40), kein doppeltes Freigeben (112: travel_leave/travel_enter laden über g_tev_reload neu), Kosmetik bzw. kleines Leck (16, 17, 41, 50) |
| hoch → mittel | 1, 2, 8, 12, 13, 45, 51, 71, 111, 114, 129, 135, 151, 187, 203 | echte, aber begrenzte Folgen. 135 hängt nicht (man kommt per Gegenstandswahl heraus). 71: die Datei entsteht wegen des Modus 3 gar nicht, Folge ist nur eine 4-%-Sperre beim ersten Besuch. 151 betrifft Göttergunst, nicht Geld. 187 wirkt über fig_menu_repeat in der Folgerunde |
| mittel → kritisch | 93, 199, 206, 212 | Schreiben über NULL in die Interrupttabelle, undefinierte Gegenstände ohne Ende, verlorene Ausrüstung |
| niedrig → mittel/kritisch | 6, 122, 134, 149, 157, 177, 179, 186, 213, 220, 233 | 6 = Doppel von 212. Die übrigen: Text erscheint nie bzw. Werte sind spürbar falsch |

## Hinweise für die Patches

- **Nr. 96 nicht nach der alten Vermutung fixen:** +0x1a1 ist `old_dlg_mode`. `&f->f1a1` würde den Dialogmodus zerstören.
- **Nr. 6 und 212:** dieselbe Stelle. **218** ist der Text-Teil desselben switch wie **199**, beide also zusammen beheben.
- **Mit Nr. 25 zusammen denken:** 51, 118, 206. Sie liefern die Wege ins negative Geld.
- **Nr. 20/84 (unsicher, kritisch):** Für Patch b) wird hier eine Designentscheidung nötig. Entweder bezahlt man nur die gelieferten Stück, oder man lässt es (Absicht).
