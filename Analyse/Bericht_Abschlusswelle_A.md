# Abschlusswelle A: Namen, Byte-Wüsten, Konstanten (C-Seite)

Stand 19.10.2026. Zusammenfassung des Auftragsberichts. Die Originaldatei ging beim Aufräumen der Arbeitsbäume verloren,
der Inhalt ist hier vollständig nachgetragen. Endbuild nach dem Zusammenführen mit B und C: **BYTEGLEICH mit dem Original**.

## Zahlen (namen.tsv)

| | vorher | nachher |
|---|---|---|
| Funktionen ohne Namen (C-Seite) | 13 | 0 |
| Daten ohne Namen | 132 | **0** |
| Daten mit Adresszusatz | 134 | 17 (nur noch doppelte Texte) |
| Namen entfallen, Text steht als Literal im Code | 0 | 116 |

## Was gemacht wurde

1. **Datennamen.** Alle offenen Daten sind benannt, die meisten über die Spieltexte.
   - Die 14 Merker in O257 sind Gesprächsthemen (XDIALOG.DAT, STDTALKS.LTX, TALKS_nn.LTX), z. B. `g_topic_kvirasim`, `g_topic_ingramosch`, `g_topic_artherion`.
   - Den Merker aus O183 (`g_topic_ingramosch`) schaltet das Treffen mit Elsurion in Kvirasim frei.
   - `g_trial_lost`: Gerichtsverhandlung in Gashok.
   - `g_low_stone_days`: Frist nach der Abgabe des Salamandersteins.
   - Unsichere Deutungen sind als „vermutet“ markiert.
2. **Halbnamen.** 90 Dungeon-Kampfmerker heißen nach dem Gegner, z. B. `g_dng03_skeleton_warriors_171d`. Die Position bleibt im Namen.
3. **Byte-Wüsten** sind als Texte oder lesbare Tabellen geschrieben:
   - Artikel und Pronomen (O056),
   - Tempellisten (O044),
   - `"t.$$$"` (O029),
   - Versionskennung (O032),
   - `DIARY.TXT` (O105),
   - die Themen-Dateitabelle `g_conv_topic_tab` (O254, gelesen von O257).
   94 echt unbenutzte Bereiche heißen `g_unused_<adr>`. Neun Stellen, die als „unbenutzt“ markiert waren, werden in Wahrheit gelesen und haben jetzt Namen.
4. **Konstanten** in `s_const.h`: `MONTH_*`, `HOLIDAY_*`, `TOPIC_*`, `FINALE_OP_*`.
   - `g_finale_script` gibt Zeiten in Sekunden an.
   - Die Kommentare zu den Jahreszeiten (SEASON) nannten falsche Monate, das ist korrigiert.
5. **Funktionen.** 13 Funktionen sind umbenannt, z. B. `loc_return_redraw`, `loc_fade_out`, `inn_days_tick` und `talk_check_quest_anim`. Leere Funktionen heißen `unused_…`.
6. **Kampf-Variablen.** Das Reststück DRO131 entfällt.
   - O128 (Kampfrunde) liefert jetzt die 57 Kampf-Globalen (ds:c948..cab8), Zuordnung **vermutet**, bytegleich.
   - `g_dng03_last_pos` liefert O131.
7. **Originalfehler #233**: In O113 erscheint der Zusatztext SECTION.LTX[13] nie, weil sein Merker nie gesetzt wird.

Alte Namen bleiben über 235 `#define`s im Block `/* Abschlusswelle A */` am Ende von `s_proto.h` kompilierbar.

## Beim Zusammenführen entschieden

Zwei Variablen der 3D-Engine hatten A und C verschieden benannt. Übernommen sind Cs Namen, weil C die Engine gelesen hat:

| Adresse | A | C, übernommen |
|---|---|---|
| ds:8c7e | `g_nr_proj_extra` (vermutet) | `g_nr_clip_faces` |
| ds:94a6 | `g_face_list2` | `g_face_depth` |

## Offen

- OVL030 ist absichtlich unverändert, weil seine Übersetzung vom Compiler-Heap abhängt. Es benutzt weiter Rohnamen wie `g_2808`. Die Daten selbst sind in ihren Besitzermodulen benannt.
- Einige Namen haben niedrige Sicherheit, z. B. `g_opt_fight_keep_view`.
- Die Finale-Texte sind nicht zitiert, weil OUTRO.LTX nicht lesbar extrahiert ist.
- Die Kampfrichtungen haben nur Kommentare, keine Konstanten.
