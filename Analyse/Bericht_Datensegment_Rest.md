# Bericht Phase B, Daten zurück in die Module (Arbeitsbaum /root/ds2_baum, 19.10.2026)

## Ergebnis
- Voller Build `bash tools/build.sh` (build_final.log): **BYTEGLEICH mit dem Original** (MD5 da3b59ce86a82e4b8bb14d36ed37d4d0).
  145 Module definieren ihre Daten selbst (dscheck GLEICH), 301 Module aus C, 0 Linker-Meldungen.
- **19 von 19** Modulen erledigt: O014, O025, O026, O032 (zuletzt), O034, O041, O046, O048, O053, O078, O084, O086,
  O103, O108, O140, O224, O232, O258 und R12.
- Zusätzlich erledigt:
  - Das O008-Reststück: `_INIT_` liegt jetzt im Modul (Spalte `DI`), DRO008 entfällt.
  - Erweiterung: R01, R03 und R23 haben Spalte D und bestehen dscheck. Die Prüfungen sind unverändert, also bytegleich.
- Grundlage sind die KS-Patches aus /root/ks_baum/abgabe (eingespielt, Sicherung ausgang_ks.tgz). Alle Patches sind
  gegen diesen Stand erzeugt, mit -p1. Auf einer frischen Kopie laufen `patch --dry-run` sauber durch, und das Ergebnis
  ist gleich dem Arbeitsstand, mit zurückgesetzten Pfaden /root/sternenschweif.

## Patches (Reihenfolge egal)
| Datei | Inhalt |
|---|---|
| csrc.patch | 34 Quelltexte: Datenblöcke der Module, Zugriffe auf die neuen 1-basierten Felder (O034, O097, O143, O183, O235 …) |
| names.patch | s_data.h, namen.tsv, daten/manuell_phaseB.tsv (neue Namen und Beschreibungen), s_struct.h, s_class.h, s_const.h, s_proto.h, proto_fest.tsv |
| tools_c_module.patch | dsconv, cobjfix_d, dscheck, dsinfo (INIT_FN/EXIT_FN, Spalte DI), dmgen, build.sh, dsb.sh, dsall.sh, neu: pubdup.py, heapbild.py, fill/FILL.ASM; frisch.txt; c_module.tsv |
| dsana.patch | korrekturen.tsv (4 Zeilen), PLAN.md Abschnitt 8 |
| leitfaden_originalfehler.patch | LEITFADEN_C.md, neuer Abschnitt „Phase B … schwere Fälle“ (ORIGINALFEHLER.md unverändert, s. u.) |

## Wichtige Befunde
- Die Lösungsmuster stehen im Leitfaden:
  - Auto-Aggregat-Vorlagen
  - 1-basierte Tabellen `tab[i - 1]`
  - Methoden- und Funktionszeigertabellen
  - `_INIT_` im Modul (DI)
  - Objekte von Konstruktor-Klassen in O032 als reiner Speicher
- Doppelte Publics lassen BC.EXE stumm abbrechen. Deshalb gibt es tools/pubdup.py vor jedem Link.
- **O140:** Was als Lesen über das Tabellenende aussah (g_fight_defs[0xab..0xb1]), ist dasselbe 1-basierte Muster mit
  Versatz 0xab. Der Code lautet jetzt `g_fight_defs_ab[f->id - 0xab]` und ist bytegleich. Das ist kein Originalfehler,
  deshalb vergebe ich **#233 nicht**. Neue Originalfehler sind bei der Datenarbeit nicht aufgetaucht.
- **Kopierschutz-Daten:** g_copyprot_sum (ds:35c2, DATA), g_tamper_count (ds:c6c6) und g_copyprot mit ok/ok2 (ds:c6ca)
  liegen mitten im zusammenhängenden Bereich von O032. Module liefern ihre Daten am Stück, deshalb können diese Daten
  nicht zu SEG003 bzw. SEG023 gehören. Sie bleiben in O032. SEG003 besitzt nur g_copyprot_passed (ds:3e6e). Die Prüfungen
  selbst sind unverändert.
- **Unsichere Zonen, geklärt über Zugriffe und Link:**
  - R23-DATA 78d2..78da benutzt R23 selbst.
  - Das BSS cb5a..cb6c gehört zu O241 (g_tavern_crowd, g_cb5c), cb6c..cb6e zu O262. R23 beginnt bei cb6e.
  - R01-BSS 8b8e..8b96 bleibt bei R01.
- **SEG001-Link:** Die 4 Byte Abweichung (Segmentwort der markierten Aufrufe) kamen von einer veralteten lnk/asm1-Basis
  aus der Zeit vor KS. Der volle Build hat sie behoben.
- **O030 (BCC-Heap):** Der volle Build brach zuerst mit „_g_c4c5: mehrere Adressen“ ab. Mit den Basis-Headern läuft
  derselbe Vorlauf OVL198 sauber, das ist belegt. Die Ursache sind also die geänderten Kopfdateien, die OVL198 einliest.
  Neu ist ein Speicherbild-Vorlauf, der von den Headern unabhängig ist: frisch.txt `OVL030 @0115:2e8=8b,2f8=17`.
  tools/heapbild.py erzeugt LOAD.BIN, das Gleiche wie x030/t/Z0.BIN. tools/fill/FILL.ASM wird beim Build assembliert und
  lädt das Bild nach XMS-KB 0x115. build.sh und dsb.sh verstehen solche `@`-Vorläufe.
- Zu den „unbenutzt“-Objekten:
  - Alle Daten ohne Codeverweis liegen im zusammenhängenden Bereich ihres Moduls. Dass das stimmt, zeigen dscheck und
    der bytegleiche Link.
  - In den D-Modulen sind sie als `/* ds:xxxx, unbenutzt */` definiert, 133 Stück, davon 90 in O032.
  - Außerhalb von D-Modulen liegen nur O030 (bleibt Assembler) sowie R24 und R46 (Assembler).

## Offen
- **BSS 0xc948..0xcaba (57 Kampf-Globale, Reststück DRO131).** Über die Zugriffe lässt sich das nicht entscheiden:
  - Die Hauptnutzer liegen außerhalb des möglichen Fensters, nämlich O141, O144, O018 und O142.
  - Jedes Modul zwischen DMO103 und DMO140 ohne eigenes BSS kommt in Frage, also O104..O139, auch Module ohne Daten wie
    O112, O128, O134 und O137.
  - Allein von O131 benutzt wird nur ds:cab8.
  - Ich habe das Reststück deshalb gelassen statt zu raten.
- Für den Hauptbaum: Die Pfade in den Patches zeigen auf /root/sternenschweif. Im Arbeitsbaum waren sie auf
  /root/ds2_baum umgestellt, das ist zurückgedreht. tools/dosrun.sh ist unverändert.
