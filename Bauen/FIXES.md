# Fehlerbehebungen per Schalter (FIXES)

Stand: 19.10.2026. Quelle der Tabelle: `abschluss/fix_schalter.tsv` (erzeugt von `tools/mkfix.py` aus
`abschluss/fehler_raster.tsv`, `abschluss/fehler_liste.tsv` und `abschluss/fix_titel.tsv`).

**Stufen:** `FIX_BUGS=1` („wichtig“) schaltet alle Behebungen der Kategorien *kritisch* und *mittel* ein,
`FIX_BUGS=2` („alles“) zusätzlich die Kategorie *egal*. `FIX_EXPLOITS=0` lässt die als Exploit markierten Fehler
(die dem Spieler nützen) im Spiel; Vorgabe ist `FIX_EXPLOITS=1`, sobald `FIX_BUGS > 0`. Jeder Schalter `FIX_nnnn`
lässt sich einzeln übersteuern (`-DFIX_0020=0`). `FIX_BUGS=0` (Vorgabe) ergibt das bytegleiche Original.

**Spalten:** Stufe = ab welcher Stufe der Schalter an ist; umgesetzt = ja / teilweise / nein; „Änderung“ nennt die
Kommentare an den `#if FIX_nnnn`-Stellen im Code, „Grund“ erklärt Teilumsetzungen und Auslassungen.

## Übersicht

| Stufe | ja | teilweise | nein | kein Fehler |
|---|---|---|---|---|
| wichtig | 92 | 17 | 9 | – |
| alles | 103 | 13 | 24 | – |
| – (kein Fehler/gewollt) | – | – | – | 10 |

Nr. 6 und Nr. 212 sind dieselbe Stelle (`FIX_0006` folgt `FIX_0212`). Nr. 20 und Nr. 84 (Ware bezahlt, aber nicht erhalten) sind *kritisch* und damit in Stufe „wichtig“ (`FIX_BUGS=1`) enthalten.

**Nr. 352–376** (am Code bestätigte Spielermeldungen, `abschluss/fehler_netz_geprueft.tsv`) kamen während dieser Runde dazu. Sie waren beim Abschluss
schon eingebaut und sind in beiden Fix-EXEs enthalten (Standardbau weiter bytegleich, fixcheck und Startprobe in Ordnung), aber nur in der Cloud ohne Spieldaten
geprüft. Wer sie in dieser Runde draußen lassen will, schaltet sie einzeln ab (`-DFIX_0352=0` usw.). Was davon nicht umgesetzt ist (nächste Runde bzw. mit Grund), steht in der Spalte „umgesetzt“.

## Tabelle

| Nr | Titel | Kategorie | Exploit | Stufe | umgesetzt | Änderung / Grund | Dateien |
|---|---|---|---|---|---|---|---|
| 1 | Verzauberte Gegner patzen im Angriff ohne Folgen | mittel | nein | wichtig | ja | dangling else: Patzerfolgen 9..12 haengen am Test k >= 3 && k <= 8 | OVL160.CPP |
| 2 | Automatisches Steigern ignoriert die Grenze je Talent | mittel | nein | wichtig | ja | Steigerungen zaehlen (incs), nicht tries | OVL087.CPP |
| 3 | Klickflächen-Prüfung tauscht falsche Werte | egal | nein | alles | nein | im residenten Segment SEG012 kein Platz - die richtige Vertauschung kostet 2 Bytes (y2 liegt auf dem Stapel, y1 im Register), das Segment darf nicht laenger werden; der Zweig wird nie betreten (alle Aufrufer uebergeben geordnete y) |  |
| 4 | Nächtlicher Dieb bestiehlt nur den ersten Helden | mittel | nein | wichtig | ja | alle Helden durchgehen (!= wie in der ersten Schleife) | OVL179.CPP |
| 5 | Wegereignisse lösen an zufälligen Stellen aus | mittel | nein | wichtig | ja | r vorbelegen: zwischen den Stufen kein Zufallsereignis aus Stapelmuell | OVL210.CPP |
| 6 | Orks werfen abgenommene Ausrüstung weg (wie Nr. 212) | kritisch | nein | wichtig | ja | wie Nr. 212 | OVL263.CPP |
| 7 | Karte aufdecken: Grenzprüfung doppelt | egal | nein | alles | ja | Untergrenze wie in den Nachbarzeilen | SEG008.C |
| 8 | Zufallsbegegnungen in einer Geländeart falsch | mittel | nein | wichtig | ja | Bereich 6..15 (wie die Nachbarbereiche) | OVL212.CPP |
| 9 | Bild-Entpacker liefert Zufallswert zurück | egal | nein | alles | ja | Rueckgabe im RLE-Zweig setzen | OVL098.CPP |
| 10 | Heldenvorauswahl steht manchmal auf leerem Platz | egal | nein | alles | ja | best vorbelegen (kein Held punktet) | OVL058.CPP |
| 11 | Marktstand mit unbekanntem Feld | egal | nein | alles | ja | k fuer unbekannte Felder vorbelegen | OVL183.CPP |
| 12 | Orkgefängnis: bei Alarm zwei Kämpfe hintereinander | mittel | nein | wichtig | ja | else wie bei den Nachbarfeldern: bei Alarm nur ein Kampf | OVL266.C |
| 13 | Höhlenspinnen kommen bis zur Nestzerstörung wieder | mittel | nein | wichtig | ja | Kampf melden (ret = 1); Kampf melden (ret = 1); die Spinnen kommen weiter nach, bis das Nest an 0x225 zerstoert ist | OVL266.C |
| 14 | Absturz beim Schlüsselbund mit 18 oder mehr Schlüsseln | kritisch | nein | wichtig | ja | hoechstens 16 Namen sammeln (Feld names[17], Auswahl zeigt 16) | OVL048.CPP |
| 15 | Schlüsselbund/Rezeptbuch: falsche „schon drin“-Meldung | egal | nein | alles | ja | nicht gefundenes Rezept nicht als Fach 14 pruefen; alle 27 Faecher durchsuchen, nicht gefundenes Kraut nicht pruefen | OVL048.CPP |
| 16 | Einzelne Pixel in falscher Farbe | egal | nein | alles | ja | start dreimal addieren (Eintraege zu 3 Bytes); nur die g_view_pal_n belegten Farben durchsuchen (fehlender Sprung) | OVL096.C |
| 17 | Speicherleck beim Verlassen der 3D-Ansicht | egal | nein | alles | ja | Ortsart vor dem Zuruecksetzen merken; Pruefung auf die gemerkte Ortsart | OVL096.C |
| 18 | Flucht vom Randfeld wird manchmal verweigert | mittel | nein | wichtig | ja | Flucht vom Randfeld: Weg ist vorhanden (ok war uninitialisiert) | OVL150.C |
| 19 | NSC aufnehmen: Hilfsvariable doppelt benutzt | egal | nein | alles | nein | x ist nur ein Dummy-Ziel fuer Breite und Hoehe, wird nie gelesen (wie Nr. 96) - ohne Wirkung |  |
| 20 | Fahrender Händler: Ware bezahlt, aber nicht erhalten | kritisch | nein | wichtig | ja | Hilfsvariablen fuer die Abrechnung der gelieferten Ware; weiter verteilen und nur die gegebenen Stueck berechnen; bei Uebergepaeck nur die gelieferte Ware bezahlen | OVL069.CPP |
| 21 | Händler-Warenliste überlappt bei riesigem Sortiment | egal | nein | alles | ja | Waffen nur bis Platz 69 (ab 70 liegen die Ruestungen) | OVL069.CPP |
| 22 | Automap findet sehr weite Wege nicht | egal | nein | alles | ja | Puffer ist signed char: Suche bei 127 beenden statt leer bis 0x100 weiterzulaufen | OVL053.CPP |
| 23 | Versiegelte Schlösser: Erleichterung wirkungslos | egal | nein | alles | nein | die Halbierung nach dem Oeffnen ist toter Code ohne Wirkung; ob ein zugezaubertes Schloss leichter sein sollte, ist eine Designfrage (Vorziehen vor try_open wuerde das Spiel aendern) |  |
| 24 | Warenauswahl: Pfeil links springt an den Anfang | egal | nein | alles | ja | erste Spalte der ersten Seite: Cursor bleibt stehen (sel -= 10 ergab immer 0) | OVL069.CPP OVL229.C OVL230.CPP |
| 25 | Geld ins Minus: Millionen durch Gruppenteilung | kritisch | ja | wichtig | ja | mit Vorzeichen pruefen: mehr abziehen als vorhanden ergibt 0 statt ~42,9 Mio. Dukaten | OVL200.CPP |
| 26 | Gruppe entfernen ändert fremdes Byte | egal | nein | alles | ja | nur eine gefundene Gruppe verkleinern (nicht g_group_sizes[6]) | OVL200.CPP |
| 27 | Texteingabe: fehlende Endnull | egal | nein | alles | ja | Endnull nach strncpy | OVL111.CPP |
| 28 | Texteingabe: Rückgabewert fehlt | egal | nein | alles | nein | die gemeinte Bedingung ist unklar (unsicher), und der einzige Aufrufer (O110) wertet die Rueckgabe nicht aus - keine Wirkung |  |
| 29 | Kampfbewegung: Animationsfolge ohne Längengrenze | egal | nein | alles | ja | Laenge der Animationsfolge begrenzen wie in fig_enemy_move | OVL149.CPP |
| 30 | Reiseereignis Weg 136: gewollter Durchlauf | egal | nein | - | kein Fehler | gewollt: bewusstes Durchfallen (case 1 -> case 2) |  |
| 31 | Ortswechsel überschreibt den gemerkten alten Ort | egal | nein | alles | ja | Ortswechsel nur einmal: alten Ort behalten; Ortswechsel nur einmal: alten Dungeon behalten | OVL019.C |
| 32 | Absturz im versteckten Entwicklerbildschirm | mittel | nein | wichtig | teilweise | Eingabe auf buf[40] begrenzen (Enter passt immer noch) — <i>Eingabelaenge begrenzt (Stapelueberlauf behoben); die Indizes der Anzeige-/Setzbefehle bleiben ungeprueft - die Merkerfelder sind Fenster ins Datensegment ohne festgelegte Laenge (Entwicklerbildschirm)</i> | OVL011.CPP |
| 33 | Kalender: falscher Tagesüberlauf | egal | nein | alles | ja | Tagesueberlauf: Rest in den naechsten Monat | OVL250.CPP |
| 34 | Kalender: Tick-Überlauf verliert Restzeit | egal | nein | alles | ja | Tagesueberlauf ab einem vollen Tag, Rest behalten | OVL250.CPP |
| 35 | Kalender: Jahreswechsel ergibt Monat 13 | egal | nein | alles | nein | die beiden Ueberlaufzweige stellen die Namenlosen Tage verschieden dar (Tick-Zweig: Monat 13 mit Tag -5..0, Jahreswechsel erst bei Tag 0; Tageszweig: sofort Monat 1 und Jahr + 1) - welche Darstellung gilt, ist nicht belegbar; der Zweig ist ausserdem unerreichbar (einziger Aufrufer mit ticks = 0) |  |
| 36 | Reisekarte: Richtungswahl ohne Rückgabewert | egal | nein | alles | ja | r vorbelegen | OVL214.C |
| 37 | Absturz-Tagebucheintrag auch ohne Absturz | mittel | nein | wichtig | ja | Tagebucheintrag nur nach einem Absturz und nur, wenn der Held ueberlebt hat | OVL121.CPP |
| 38 | Kampffeld: Belegt-Prüfung ohne Wirkung | egal | nein | alles | ja | belegtes Feld bzw. ungueltiges Objekt: nicht setzen (wie DSA 1) | OVL147.CPP |
| 39 | Hänger bei völlig zugestelltem Kampffeld | mittel | nein | wichtig | ja | Suche begrenzen, danach das ganze Feld zeilenweise absuchen (kein Haenger) | OVL147.CPP |
| 40 | Parade Held gegen Held (kein Fehler) | egal | nein | - | kein Fehler | kein Fehler: hero_mr_target liefert den Verteidiger |  |
| 41 | Paradepatzer nennt den falschen Helden | egal | nein | alles | ja | Paradepatzer nennt den Verteidiger def | OVL158.CPP |
| 42 | Patzerfolge ohne Schaden geprüft | egal | nein | alles | ja | Folge nur nach Schaden (Abfrage in den Schadensblock) | OVL158.CPP |
| 43 | Steinbock frisst die letzte Süßigkeit umsonst | mittel | nein | wichtig | ja | Kennung vor dem Verbrauch lesen | OVL119.CPP |
| 44 | Zahleneingabe zeigt falsche erste Ziffer | egal | nein | alles | ja | Startwert auf die darstellbaren Stellen begrenzen (erste Ziffer sonst > 9) | OVL197.CPP |
| 45 | Kräuterstelle an Weg 165 beliebig oft ernten | mittel | ja | wichtig | ja | Probe mit dem eigenen Zaehler von Weg 165; Zaehler bis 40 (41 * 3 + 5 = 128 liefe im signed char ueber) | OVL122.CPP |
| 46 | Reisekarte: Wegsuche liest hinter der Tabelle | egal | nein | alles | ja | Ort ohne Weg: nichts tun statt hinter der Tabelle zu lesen | OVL209.CPP |
| 47 | Reisekarte: Wegzeichnung im Randfall falsch | egal | nein | alles | ja | Routenende erreicht: Schleife verlassen (idx nicht veraltet benutzen) | OVL209.CPP |
| 48 | Brenne toter Stoff ohne Fackel: Meldung zufällig | egal | nein | alles | ja | sel vorbelegen: ohne Fackel und Lampe immer die Meldung | OVL237.CPP |
| 49 | Sonderkampf: Lesen vor dem Nulltest | egal | nein | alles | ja | Nullpruefung vor dem ersten Zugriff | OVL021.CPP |
| 50 | Phex-Tempel: nur eine von vier Flächen animiert | egal | nein | alles | ja | alle vier Flaechen animieren (faces[k]) | OVL198.C |
| 51 | Herbergsdieb stiehlt nie Proviant, sondern Geld | mittel | nein | wichtig | ja | Suche nach -2 (Hand/Koerper 0..15) bzw. -3 (Rucksack 16..30) fuer jeden Helden in der Schleife | OVL201.CPP |
| 52 | Gegenstandssuche über alle Plätze kaputt | egal | nein | alles | ja | echte Suche ueber alle Plaetze statt Abbruch am ersten Platz | OVL201.CPP |
| 53 | Porträtleiste einer Teilgruppe zu breit/schmal | egal | nein | alles | ja | Heldenzahl der eigenen Gruppe | OVL201.CPP |
| 54 | Gegnerzauber bei leerer Zauberliste | egal | nein | alles | ja | ohne Zauber keine Zufallswahl mit Index -1 | OVL152.CPP |
| 55 | Fähre Hilvalla: zu wenig Geld zeigt Begrüßung erneut | egal | nein | alles | ja | bei zu wenig Geld g_on_boat = -1 wie in den Parallelorten | OVL126.CPP |
| 56 | Tagebuch drucken: Überlauf bei langen Zeilen | egal | nein | alles | ja | pad mit Vorzeichen, zu breite Zeilen unveraendert lassen | OVL105.C |
| 57 | Tagebuch-Heldennamen: Lesen hinter dem Feld | egal | nein | alles | ja | Grenze vor dem Zugriff pruefen | OVL105.C |
| 58 | Ortssymbole: unvollständige Änderungsprüfung | egal | nein | alles | ja | alle restlichen Plaetze pruefen (nicht old[n], bei n == 9 hinter dem Feld) | OVL188.CPP |
| 59 | NSC verlässt die Gruppe ohne Abschiedstext | mittel | nein | wichtig | ja | Abschiedstext auch anzeigen | OVL129.CPP |
| 60 | Teleport mit unbekannter Herkunft | egal | nein | alles | ja | x als Merker vorbelegen; ohne passenden Fall nicht mit uninitialisierten x/y/dir teleportieren | OVL129.CPP |
| 61 | Gruppe teilen bei sechs Gruppen | egal | nein | alles | ja | Suche auf die 6 Gruppen begrenzen, ohne freie Gruppe abbrechen | OVL075.CPP |
| 62 | Porträt ziehen liest vor der Tabelle | egal | nein | alles | ja | Untergrenze: nicht vor g_portrait_x[0] lesen | OVL075.CPP |
| 63 | Wirselkraut an Weg 201 beliebig oft ernten | mittel | ja | wichtig | ja | Probe mit dem eigenen Zaehler von Weg 201; Zaehler bis 40 (41 * 3 + 5 = 128 liefe im signed char ueber) | OVL123.CPP |
| 64 | Eine Gegner-Animation wird nie gezeigt | egal | nein | alles | nein | der tote Zweig TARGET_SPECIAL benutzt seinerseits enemy_target_enemy, das Ziele ab 30 falsch abbildet (g_enemies[id - 10]); eine Korrektur muesste O192 und alle Aufrufer mitziehen - Entfernen des toten Zweigs aendert nichts |  |
| 65 | Genitiv-s bei Namen mit Sonderzeichen falsch | egal | nein | alles | ja | Zeichen ab 0x80 ausser den grossen Umlauten gelten als Kleinbuchstaben | OVL076.CPP |
| 66 | DSA-1-Import: doppelte Prüfung | egal | nein | alles | nein | doppelter Block und wirkungslose Teilbedingung aendern nichts; ob im zweiten Block ein anderer Gegenstand gemeint war, ist nicht belegbar (unsicher) |  |
| 67 | Dialogskript: Zeiger nicht genullt | egal | nein | alles | ja | Zeiger nullen, ganzes Feld used[] loeschen | OVL260.CPP |
| 68 | Klangtreiber: kleines Speicherleck | egal | nein | alles | teilweise | Treiberpuffer freigeben, sobald AIL beendet ist; Namensliste nur bis zu ihrem Ende durchsuchen — <i>der Treiberpuffer wird nur freigegeben, wenn AIL beendet ist (davor kann der Treiber noch registriert sein - Freigeben waere gefaehrlich); drvname begrenzt</i> | OVL042.CPP |
| 69 | Kartenrätsel: toter Programmcode | egal | nein | alles | nein | toter Code ohne sichtbare Folge (doppelter Zweig, last_sel nie -1) - eine Bereinigung aendert kein Verhalten |  |
| 70 | Heiler nennt Krankheit vor der Zahlung (gewollt) | egal | nein | - | kein Fehler | gewollt: Diagnose im Fragetext genannt |  |
| 71 | Lagerhaus beim ersten Besuch manchmal gesperrt | mittel | nein | wichtig | ja | Datei mit Zugriffsmodus anlegen und nur bei Erfolg schreiben | OVL012.CPP |
| 72 | Boron-Wunder: Totenerweckung viel zu häufig | mittel | ja | wichtig | ja | r <= 3 bei laufendem Zeitzaehler tut nichts (keine Wiederbelebung) | OVL033.CPP |
| 73 | Kampfanimation: über 40 Animationen gleichzeitig | egal | nein | alles | ja | ohne freien XMS-Platz slot = -1 statt Muell; nur einen eigenen Platz freigeben; ohne Platz nicht ins XMS schreiben; Figur ohne XMS-Platz: Bild neu entpacken statt fremden Platz lesen | OVL016.CPP OVL112.CPP |
| 74 | Bildklasse: Randfehler ohne Aufrufweg | egal | nein | alles | teilweise | Archivobjekt wie im Standardkonstruktor setzen — <i>nur das Archivobjekt im Konstruktor gesetzt; read_data/check bleiben (decode liefert keinen Erfolgswert, der Pruefkopf im Objekt ist eher gewollt; kein Aufrufweg gefunden)</i> | OVL269.CPP |
| 75 | Meldung „versteckte Gegner“ zählt manchmal falsch | egal | nein | alles | ja | nur die n belegten Eintraege durchsuchen | OVL162.CPP |
| 76 | Sumpfrantzendokument gegen Gegner ohne Meldung | egal | nein | alles | ja | 0x14d wie in fig_use_item_hero | OVL162.CPP |
| 77 | Vorratslager: Debugtext als Mengenfrage | mittel | nein | wichtig | ja | Stiefel (3) und Seil (5) haben keinen Mengentext - immer 1 Stueck statt Debugmeldung; Beuteliste nach dem Zurueckschreiben freigeben (Speicherleck) | OVL116.CPP |
| 78 | Lagerhaus: Verderb nach einem Jahr verkehrt herum | mittel | nein | wichtig | ja | Verlust steigt mit dem Alter (<= statt >) | OVL100.CPP |
| 79 | Kampf: Figur läuft selten in Zufallsrichtung | mittel | nein | wichtig | ja | best vorbelegen (kein Weg -> Weg 0 statt Zufall); Marke -2 hinter die Endmarke -1 (wie DSA 1), nicht darueber; Zeiger nach dem Freigeben nullen | OVL146.CPP |
| 80 | Reisekarte: Position im Randfall falsch | egal | nein | alles | ja | len/step vorbelegen (Start == Ziel) | OVL213.CPP |
| 81 | Zufälliger Gegenstandsverlust fehlerhaft | egal | nein | alles | ja | fuer jeden Gegenstand neu auslosen | OVL202.CPP |
| 82 | Gegenstand an jeden Helden: Restmenge falsch | egal | nein | alles | ja | nur den Rest verteilen | OVL202.CPP |
| 83 | Gruppenverwaltung schreibt hinter das Feld | egal | nein | alles | ja | nur eine gefundene Gruppe vergroessern; nur eine gefundene Gruppe verkleinern | OVL202.CPP |
| 84 | Laden: Ware bezahlt, aber nicht erhalten | kritisch | nein | wichtig | ja | Stueckpreis vor einer Preisaenderung (build_wares) merken; weiter verteilen, nicht gelieferte Ware nicht berechnen | OVL226.CPP |
| 85 | Laden-Warenliste überlappt bei riesigem Sortiment | egal | nein | alles | ja | Waffen nur bis Platz 69 (ab 70 liegen die Ruestungen) | OVL226.CPP |
| 86 | CD-Zwischenspeicher bei Lesefehlern kaputt | egal | nein | alles | ja | unbekannter Typ: Nullzeiger statt Muellzeiger; unbekannter Typ: abbrechen statt mit Muellzeiger zu lesen; _creat pruefen, negatives Leseergebnis nicht schreiben | OVL003.CPP |
| 87 | Gegenstände scheinen durch Wände | egal | nein | alles | ja | else-Zweig des Quadranten "!rc" gespiegelt ergaenzt (Quadrant "!l11"); else-Zweig des Quadranten "!rc" gespiegelt ergaenzt (Quadrant "!rd"); else-Zweig des Quadranten "!rc" gespiegelt ergaenzt (Quadrant "!rb") | OVL255.C |
| 88 | Wegetexte: hängende Zeiger | egal | nein | alles | ja | sub_scene als short sichern; alle vier Zeiger nach dem Freigeben nullen | OVL211.C |
| 89 | Kobold Mumpitz: Auswahl liest außerhalb | egal | nein | alles | teilweise | jede negative Auswahl wiederholen (kein Zugriff auf h->inv[sel < 0]) — <i>dng11_kobold_send_away liest bei mehr Einzelgruppen weiter in die folgenden Zieltabellen (split/far) - diese Ziele sind im Original de facto festgelegt, eine Grenze wuerde das Verhalten aendern; nur die Auswahlschleife behoben</i> | OVL062.CPP |
| 90 | Kampf-Figurenliste: mehrere Randfehler | egal | nein | alles | teilweise | Sperre used[0] (id 0) wie im Konstruktor erhalten; Partner nur setzen, wenn er gefunden wurde — <i>clear() und hide/show behoben; add() mit prio -1, die fehlende Kopfknoten-id, getpos und die Spalte x = 24 bleiben - kein Aufrufer erreicht diese Faelle, die Liste ist Teil des Kampfablaufs</i> | OVL153.CPP |
| 91 | Dateizugriff: Handle-Leck und späte Fehlerprüfung | egal | nein | alles | teilweise | zweites Handle wieder schliessen; Lesefehler vor dem Verrechnen abfangen; id == count ist ungueltig; zweites Oeffnen des Archivs pruefen — <i>Namenslisten ueber 64 KB (find) bleiben abgeschnitten - die Listen der Spieldaten sind klein; Handle, Lesefehler, exists und open_in_arc behoben</i> | OVL037.CPP |
| 92 | Hintergrundanimation zeigt evtl. falsches Teilbild | egal | nein | alles | ja | Bildversatz als huge-Arithmetik (normalisiert); Teil k mit seinem eigenen Bildobjekt bmp[k] zeichnen | OVL054.CPP |
| 93 | Absturz beim Bauern mit dem gebrochenen Rad | kritisch | nein | wichtig | ja | nur ueber die gesammelten Helden laufen (keine NULL-Eintraege) | OVL118.CPP |
| 94 | Laden: Gepäckwarnung fehlt oder falsch | mittel | nein | wichtig | teilweise | Warnung nach der neu berechneten Platzzahl; Menge vorbelegen (stapelbare Ware mit Menge 1) — <i>Platzwarnung und Menge behoben; der Geldvergleich bleibt - mit Nr. 25 kann das Gruppengeld nicht mehr negativ werden, der Vergleich ist dann gleichwertig</i> | OVL228.CPP |
| 95 | Leeres Zunderkästchen: doppelte oder alte Meldung | egal | nein | alles | ja | Meldung nur einmal (am Ende), ohne Merker keinen alten Text zeigen | OVL252.CPP |
| 96 | Kampfbeginn: Hilfsvariable doppelt benutzt | egal | nein | - | kein Fehler | kein Fehler: Dummy-Ziel (+0x1a1 waere old_dlg_mode) |  |
| 97 | Kompassrose verschwindet im Randfall | egal | nein | alles | ja | letzter Ausweg sichtbar oben links statt unterhalb des Bildes | OVL220.CPP |
| 98 | Schlafender NSC steht wach im Kampf | mittel | nein | wichtig | ja | Schlafmerker am Platz k des Helden (NSC: 6); Zeilen nur bis 23 (Gitter 24 Zeilen); Feldgrenzen 0..23/0..24 und vor dem Zugriff pruefen | OVL140.CPP |
| 99 | Dialogfenster: Randfälle ohne Wirkung | egal | nein | alles | ja | bx2 vorbelegen (bei nd == 0 sonst uninitialisiert) | OVL190.CPP |
| 100 | Schenke: Andrang nach Mitternacht (unklar) | egal | nein | alles | nein | die Absicht ist offen (Andrang nach Mitternacht oder harmloser Rest) - die zweite Teilbedingung ist wirkungslos, eine geratene Aenderung wuerde das Schankverhalten aendern |  |
| 101 | Kräuterstelle an Weg 41: Zähler ohne Grenze | egal | nein | alles | ja | verirrtes Semikolon entfernt, Zaehler begrenzt | OVL115.CPP |
| 102 | Gewölbe des Namenlosen: Ereignisse zufällig | mittel | nein | wichtig | ja | Zeile mit g_map_x0 wie bei pos; Rueckgabe vorbelegen (keine Position / Sonderkampf) | OVL131.CPP |
| 103 | Tagebuchsuche: Esc sucht statt abzubrechen | egal | nein | alles | ja | ESC (-1) bricht ab (default: c1 = -1); nicht ueber das Stringende lesen | OVL103.CPP |
| 104 | Altsvellt-Sumpf: alte Ebene überschrieben | egal | nein | alles | ja | Dungeonwechsel einmal: old_dungeon behaelt den vorigen Dungeon | OVL195.CPP |
| 105 | Spinnennetze: Schreiben nach Adresse 0 | egal | nein | alles | ja | Wartezeit nicht negativ; unbekanntes Feld: nicht nach 0000:0000 schreiben | OVL265.CPP |
| 106 | Sonderkampf: Flächenangriff trifft niemanden | mittel | nein | wichtig | ja | Nullpruefung vor dem ersten Zugriff (wie O021); Angriff auf alle: Trefferliste mit den handlungsfaehigen Helden fuellen (war immer leer) | OVL141.CPP |
| 107 | Heldenerschaffung: uninitialisierte Werte | egal | nein | alles | teilweise | ohne Typ direkt verschieben (again war uninitialisiert) — <i>chargen_save_bg/restore_bg fuer Seiten ausserhalb 0..10 bleiben ohne Vorbelegung - g->page kommt nur aus 0..10</i> | OVL169.CPP |
| 108 | Auswahlliste ohne Grenzprüfung | egal | nein | alles | ja | Liste (350 Eintraege) nicht ueberfuellen | OVL225.CPP |
| 109 | Texteingabefeld: Grafikfehler und Zeichenmüll | mittel | nein | wichtig | teilweise | Mausbereich mit der Breite in Pixeln; Hintergrund im eigenen Zielpuffer scr loeschen; in Spalte 0 nicht s[-1] lesen/schreiben — <i>die toten Zweige in wrap (l >= nlines, Zweig nach memset) bleiben - der Ueberlauf landet bestimmungsgemaess in der Reservezeile lines[nlines]; Hintergrund, Mausbereich und s[-1] behoben</i> | OVL110.CPP |
| 110 | Stabzauber prüft eine Eigenschaft doppelt | mittel | nein | wichtig | ja | Stabzauber-Probe mit der dritten Eigenschaft a3; Textzeiger auch nach Abbruch/Fehlschlag nullen | OVL091.CPP |
| 111 | Gegnerischer Heilzauber verletzt Schwerverwundete | mittel | nein | wichtig | teilweise | Heilung mindestens 7, sonst der Fehlbetrag (nicht SPELL_RES_ABORT als Schaden) — <i>mon_spell_verwandlung_beenden verbraucht bei zu wenig AE weiterhin die ganze AE ohne Wirkung - das kann ein gewollter Fehlschlag sein; der Raster-Fix betrifft nur Balsam</i> | OVL194.CPP |
| 112 | Kriegerhüne: Wegetexte unnötig neu geladen | egal | nein | alles | ja | alle vier Zeiger nach dem Freigeben nullen | OVL113.CPP |
| 113 | Gegnerpatzer gegen bewusstlose Helden entfallen | egal | nein | alles | nein | unsicher - Patzer gegen einen nicht handlungsfaehigen Helden auszulassen kann Absicht sein; Patzer 12 ist (wie in O160) ein leerer Zweig |  |
| 114 | Lorenfahrt: Abspringen gefahrlos, aber ohne AP | mittel | nein | wichtig | ja | Probe mit der gesicherten Fahrtstrecke t; Schaden aus der gesicherten Fahrtstrecke t; AP aus der gesicherten Fahrtstrecke t | OVL063.CPP |
| 115 | Lore: Lesen vor der Gültigkeitsprüfung | egal | nein | alles | ja | nicht vor das Heldenfeld lesen | OVL063.CPP |
| 116 | Auto-Erschaffung: Akademiezauber doppelt bedacht | egal | nein | alles | ja | nicht ueber die Zeile (25 Talente) hinaus lesen; nicht ueber die Zeile (45 Zauber) hinaus lesen | OVL167.CPP |
| 117 | Heilkunde Wunden: Probe doppelt gewürfelt | mittel | nein | wichtig | nein | Doppelte Heilprobe und Schaden LE-1 sind in DSA 1 (seg103 skill_heal_wounds) identisch, also Spielregel; case -1 nur toter Code ohne Wirkung |  |
| 118 | Wasserfall-Spende: Vollheilung fast umsonst | mittel | ja | wichtig | ja | halbes Gruppengeld als long (nicht nur das untere Byte) | OVL114.CPP |
| 119 | Warzennashorn-Schaden und Kräuterprobe Weg 36 falsch | mittel | nein | wichtig | ja | Zaehler wie bei den Schwesterereignissen begrenzen; 3W6 einmal wuerfeln (Vergleich und Schaden aus demselben Wurf); Zaehler bis 40 (41 * 3 + 5 = 128 liefe im signed char ueber) | OVL114.CPP |
| 120 | Automatischer Kampf: Helden zaubern auf falsches Ziel | mittel | nein | wichtig | ja | Zielfeld nicht vertauschen (x = Spalte wie in O146 und ki_select_spell_target); Gegnerzauber nur mit Gegner als Ziel (sonst e uninitialisiert); Gegnerzauber nur mit Gegner als Ziel; Heldenzauber nur mit Held als Ziel (sonst th uninitialisiert); Heldenzauber nur mit Held als Ziel | OVL156.CPP |
| 121 | Sehr lange Reiseetappen dauern zu kurz | mittel | nein | wichtig | ja | 32-bittig rechnen (kein Ueberlauf bei hohem Tempo); Zeiger nach dem Freigeben nullen | OVL217.CPP |
| 122 | Ingerimms Schmiede: Meldung fehlt, „und“ falsch | mittel | nein | wichtig | ja | Meldung nach drei vergeblichen Versuchen (Schleife endet bei 3); "und" vor dem letzten aufgelisteten Namen: k nur fuer aufgelistete Helden zaehlen | OVL002.C |
| 123 | Unendlich Geld aus Truhen | kritisch | ja | wichtig | ja | nur die genommene Menge gutschreiben (Geldvermehrung) | OVL078.CPP |
| 124 | Reisetempo im Randfall falsch | mittel | nein | wichtig | ja | Wegpunktzahl aktualisieren; Vorgabe fuer unbekannte Wegart (mittleres Tempo); Vorgabe fuer unbekanntes Wetter (normal); Vorgabe fuer andere Startorte (langsames Schiffstempo) | OVL216.CPP |
| 125 | Zwischensequenz: Überblendung zufällig | egal | nein | alles | ja | fade und b vorbelegen; && statt \|\| (Palettenzweig fuer 0x36e/0x244); nur ein angelegtes Bild loeschen | OVL040.CPP |
| 126 | Großer AP-Bonus im Gespräch wird nie vergeben | mittel | nein | wichtig | ja | hoher Bonus nur beim ersten Mal | OVL013.CPP |
| 127 | Klangeinstellung: Port/IRQ-Eingabe verworfen | mittel | nein | wichtig | ja | Treiberliste auf 30 Eintraege begrenzen (einer bleibt fuer den Schluss); eingegebenen Port uebernehmen; eingegebenen IRQ uebernehmen | OVL043.CPP |
| 128 | Fahrender Händler: falsche Menge beim Zurücklegen | mittel | nein | wichtig | ja | Warnung mit den neu berechneten belegten Plaetzen nslots; Menge 1 auch bei stapelbarer Ware setzen | OVL068.CPP |
| 129 | Speicherfehler löscht die falsche Datei | mittel | nein | wichtig | ja | alle Datenbloecke auf Schreibfehler pruefen; Temp-Dateinamen nicht in path ablegen (path bleibt der Spielstand); LASTGAME am Namen erkennen; geloescht wird der Spielstand (path) | OVL106.CPP |
| 130 | Gulmondblatt: Spieltext durch Heldennamen ersetzt | mittel | nein | wichtig | ja | in den Ausgabepuffer schreiben statt in Text 74 | OVL254.CPP |
| 131 | Starker Zaubertrank nennt Lebenspunkte | egal | nein | alles | ja | Einheit Astralpunkte wie beim Zaubertrank; leere Flasche zurueck wie bei den anderen Giften | OVL254.CPP |
| 132 | Ortsszene: Musikwahl zufällig | egal | nein | alles | ja | Rueckgabe vorbelegen (0 wie die anderen Rueckrufe) | OVL259.CPP |
| 133 | Binge: Position doppelt gesetzt | egal | nein | alles | ja | Suche nicht vor den Anfang des Heldenfelds laufen lassen; Position nur einmal setzen (old_x/old_y behalten die vorherige) | OVL061.CPP |
| 134 | Schwarzmagier-Szene: Abschlusstext und Ton fehlen | mittel | nein | wichtig | teilweise | Animation vor der Auswahl anhalten wie im Zweig mit Salamanderstein (O055_032F fehlte) — <i>nur das fehlende ani_halt ergaenzt. Die leeren Rumpfe (Text/Sprache) bleiben leer: die passenden Texte (SEQUENCE.LTX 38-40 sind unbenutzt), Sprachnummern und Animationsschritte sind nicht belegbar; ein geratener Sprach- oder Animationsindex koennte die Endsequenz aufhaengen</i> | OVL007.C |
| 135 | Gespräch mit Gegenstandswahl lässt sich nicht abbrechen | mittel | nein | wichtig | ja | Abbruch der Helden-/Gegenstandswahl beendet die Abfrage (d->next = 7) | OVL102.CPP |
| 136 | Fenster springt beim Verschieben | egal | nein | alles | ja | Mausposition merken statt alter Fenster-x (kein Sprung beim naechsten Mauszug) | OVL205.CPP |
| 137 | Eolan-Ereignis (kein Fehler) | egal | nein | - | kein Fehler | kein Fehler: Zustand wird im Dialog gesetzt |  |
| 138 | Shurinknollen-Gift: Erholung extrem langsam | mittel | nein | wichtig | ja | Praezedenz: !(days % 12) | OVL089.CPP |
| 139 | Kellertür: Wandbild bei nicht quadratischer Karte | egal | nein | alles | ja | Spalte mit g_map_y0 (wie O063/O065) | OVL130.CPP |
| 140 | Pelzwand: Abzug beim falschen Gegenstand | mittel | nein | wichtig | ja | Wasser beim Traeger h2 an seiner eigenen Stelle k abziehen | OVL065.CPP |
| 141 | Animation: hängender Zeiger bei fehlender Datei | egal | nein | alles | ja | Zeiger nach dem Freigeben nullen (kein zweites Loeschen) | OVL050.CPP |
| 142 | Lagerplatz: Suche liest hinter der Tabelle | egal | nein | alles | ja | Grenze in die Suchschleife | OVL067.CPP |
| 143 | Zwergenbinge: Portal versetzt andere Teilgruppe | mittel | nein | wichtig | ja | Dungeonwechsel nur einmal (alten Dungeon behalten); nur Gruppen auf x 12..13 versetzen (&& statt \|\|), old_y behaelt die alte Zeile | OVL060.CPP |
| 144 | Tagebuch: Seitenzählung falsch, Überlauf möglich | mittel | nein | wichtig | teilweise | Seitenzaehlung mit denselben Steuerzeichen wie die Anzeige (0xa4..0xa8) — <i>nur die Steuerzeichen der Zeilenzaehlung an Anzeige/Druck angeglichen; die fehlende Laengenpruefung der 1000-Byte-Puffer bleibt - die Seitenentscheidung (20 Zeilen) faellt vor dem Zusammensetzen, ein sauberer Seitenumbruch an dieser Stelle braeuchte einen Umbau von append</i> | OVL104.CPP |
| 145 | Erschaffung: Eigenschaften verschieben falsch geprüft | mittel | nein | wichtig | ja | Spielraum zum Senken v - 8; Spielraum zum Senken v - 2 | OVL026.CPP |
| 146 | Spendenschrein: falscher Text, Zufallswert | mittel | nein | wichtig | ja | Text 135 bei Zahlung des ganzen Geldes (wie im ersten Zustand); n ausserhalb 1..4: kein Kampf, kein fight_result mit Muellwert; k ausserhalb 1..4: kein Kampf, kein fight_result mit Muellwert | OVL164.CPP |
| 147 | Heldenaktion mit unbekannter Zielart | egal | nein | alles | ja | unbekannte Zielart: Aktion abbrechen statt Muellzeiger e/t; Folge nur nach Schaden (wie O158) | OVL159.CPP |
| 148 | Nichtzauberer können das Zaubermenü öffnen | egal | nein | alles | ja | beim Wechsel auf Seite 4 neu auswaehlen (inv_reselect wie auf Seite 1); Zaubern nur fuer Zauberkundige (wie auf den anderen Seiten) | OVL093.CPP |
| 149 | Ankunft in Lowangen: erster Textteil fehlt | mittel | nein | wichtig | ja | txt[18] (beginnt mit @) an txt[13] anhaengen statt ueberschreiben | OVL185.CPP |
| 150 | Fähre: Zufallswert an unbekanntem Ort | egal | nein | alles | ja | r vorbelegen (Ort ohne Faehrfall) | OVL163.CPP |
| 151 | Zwergengrab: Frevel ohne Kriegsaxt | mittel | nein | wichtig | ja | Gunst und Frevel erst, wenn die Axt wirklich genommen wurde; Wandbild vorbelegen | OVL064.CPP |
| 152 | Alchimie/Schätzen: kleine Randfehler | egal | nein | alles | ja | Platz fuer alle 14 Rezepte; Fehlschlag: Mindestpreis 1 wie im Erfolgszweig | OVL090.CPP |
| 153 | Reise bricht unvermittelt ab | mittel | nein | wichtig | ja | Rueckgabe vorbelegen (nur rest == 1 setzt sie) | OVL038.CPP |
| 154 | Blaue Keuche steckt mit Dumpfschädel an | mittel | nein | wichtig | ja | Blaue Keuche steckt mit Blauer Keuche (3) an | OVL081.CPP |
| 155 | Herberge: Abbruch löscht die alte Zimmerbuchung | mittel | nein | wichtig | ja | Essen kostet mindestens 1 (0 war kostenlos); Abbruch stellt die stornierte Buchung wieder her (erstattetes Geld wird wieder abgezogen) | OVL178.CPP |
| 156 | Hafen-Feilschen (kein Fehler) | egal | nein | - | kein Fehler | gewollt: -1 passt zum anderen Menue |  |
| 157 | Orkfestung Ebene 3: ausgewürfelte Gegner fehlen | mittel | nein | wichtig | teilweise | Kampf melden (ret = 1) — <i>die Rueckgaben der Kaempfe (Halle, Orkveteran, Mumie) behoben; an 0x1516/0x1615 wird c in diesem Block nie gesetzt (immer 0) - die dritte Gruppe -1/0 hat dadurch keine Wirkung, Art und Zahl der gemeinten dritten Gegnergruppe sind nicht belegbar</i> | OVL268.CPP |
| 158 | Schenke: leerer Text, merkwürdiger Rundenzähler | egal | nein | alles | teilweise | den Text pruefen statt den Zeiger — <i>nur der Test auf leeren Text; die rounds-Behandlung nach "bezahlen" bleibt (ohne sichtbare Folge, Absicht nicht belegbar)</i> | OVL241.C |
| 159 | Armatrutz eine Stufe höher als die AE erlaubt | mittel | ja | wichtig | ja | groesstes k mit k * k <= AE | OVL236.CPP |
| 160 | Zwergenbinge: Gruppe falsch, fremder Gegenstand weg | mittel | nein | wichtig | ja | Gruppenindex der wieder aktiven Gruppe old; Dietriche dort entfernen, wo sie gefunden wurden (h) | OVL000.CPP |
| 161 | Automatische Zaubersteigerung lückenhaft | mittel | nein | wichtig | ja | erste Schleife ab 1, dritte Schleife wie bei den Elfen; Schulliste ab list[0] je einmal (list[0] nicht doppelt); erste Schleife ab 1 (nicht ab dem Endstand der vorigen Schleife) | OVL086.CPP |
| 162 | Reisekarte: Ortssuche ohne Grenze | egal | nein | alles | teilweise | nach erfolgloser Suche nicht hinter der Tabelle lesen — <i>nur das Lesen hinter der Wegtabelle (route_find/route_first) abgefangen; die Ortssuchen ohne Grenze bleiben (alle abgefragten Orte stehen in g_places, ein Abbruch braeuchte einen Fehlerwert bei allen Aufrufern), place_last_dest nimmt weiter die letzte Richtung (welche gemeint war, ist nicht belegbar)</i> | OVL219.CPP |
| 163 | Heldenzauber: Lesen hinter der Tabelle | egal | nein | alles | teilweise | nur die 12 Eintraege der Tabelle lesen — <i>nur der Lesezugriff hinter der Tabelle behoben; &w als Breite und Hoehe in heal_ani bleibt (Hoehe wird nie gelesen, ohne Wirkung - wie Nr. 19/96)</i> | OVL233.CPP |
| 164 | Ingerimms Schmiede: Schreiben hinter das Feld | egal | nein | alles | ja | nur in eine gefundene Gruppe schreiben (nicht g_groups[6]); Rueckgabe 1, wenn der Schluessel benutzt wurde | OVL001.C |
| 165 | DSA-1-Import: Gegenstände gehen verloren | mittel | nein | wichtig | ja | naechsten Platz frei machen (j++ wie beim Hexenbesen); negatives Geld auf 0 statt 1000 | OVL177.CPP |
| 166 | Nihilatio Gravitas über die AE hinaus | mittel | ja | wichtig | ja | Hoechststufe = kleinerer Wert von Stufe und AE/5 | OVL234.CPP |
| 167 | Sonderkampf-Sequenzen: doppeltes Freigeben | egal | nein | alles | ja | s vorbelegen (Ereignisse ohne Text); Merker und Zeiger nach dem Freigeben zuruecksetzen (kein zweites Freigeben) | OVL020.CPP |
| 168 | Kampfaufbau: leere Prüfung | egal | nein | alles | ja | leeres "if (!r) ;" auf das (evtl. uninitialisierte) r entfernt | OVL022.CPP |
| 169 | Gegenstand benutzen: fehlende Standardfälle | egal | nein | alles | ja | unbekannter Gegenstand: abbrechen statt Muellzeiger an sprintf; poison vorbelegen (kein Gift-/Oelgegenstand) | OVL047.CPP |
| 170 | Routenplanung ergibt falsche Route | mittel | nein | wichtig | ja | Suche beenden, ohne p->route[99] zu lesen; j bei n == 0 nicht veraltet benutzen; zweiten Weg mit buf[m + 1] pruefen (Kopierfehler); dem Weg wirklich folgen (v = next), Route nicht ueberlaufen lassen | OVL218.CPP |
| 171 | Heldensuche: ungenutzte fehlerhafte Funktion | egal | nein | alles | ja | am Gruppenende vorn weitersuchen (i = 0 statt h = cur); Nulltest vor hero_weapon_type2; inv[v] nur bei v >= 0 lesen; Nulltest vor get_item_pos | OVL088.CPP |
| 172 | Intro: Speicher-Merker nicht gesetzt | egal | nein | alles | ja | EMS-Merker im Konstruktor setzen | OVL027.CPP |
| 173 | Hexenspeichel heilt mehr, als die AE bezahlt | mittel | ja | wichtig | ja | AE-Obergrenze nach der Rechnung pruefen (wie spell_balsam) | OVL235.CPP |
| 174 | Unmut eines Gesprächspartners wird vergessen | mittel | nein | wichtig | ja | beim Einfuegen ebenso: nicht nach list[100] schreiben; nur innerhalb der 100 Eintraege schieben (list[100] ist counters[0].id) | OVL257.CPP |
| 175 | Zwergenskelett: falsches Pronomen | egal | nein | alles | nein | kein Fehler - in BLUT1.LTX[42] ("%s versucht, %s gut zuzureden, doch %s scheint ...") steht das zweite %s fuer den Helden, dem zugeredet wird (h, Dativ "ihm/ihr"); im Erfolgstext [43] bezieht sich das Pronomen dagegen auf h2 - beide Aufrufe sind richtig |  |
| 176 | Reisekarte: Grafikfehler beim Hochscrollen | egal | nein | alles | ja | gleicher Versatz view_x wie in den anderen Richtungen | OVL187.CPP |
| 177 | Wegliste: eine Meldung erscheint nie | mittel | nein | wichtig | nein | die unerreichbare Meldung ist FEATURE.LTX[30] "Ihr habt den Endpunkt der geplanten Route erreicht" - sie passt nicht in diesen Zweig (route[i + 1] ist dort gerade nicht das Routenende); welche Bedingung gemeint war, ist nicht belegbar, ein Umordnen zeigte die Meldung an Kreuzungen falsch an |  |
| 178 | Erschaffung: Grenzwerte uneinheitlich verglichen | egal | nein | alles | ja | Mindestwert wie den Hoechstwert mit Vorzeichen vergleichen | OVL170.CPP |
| 179 | Kein Hunger und Durst beim Rasten | mittel | nein | wichtig | ja | Zunahme bei Rast anteilig per Zufall statt ganzzahlig 0 | OVL005.CPP |
| 180 | Sieg über die Schwarzmagier wird nicht gemerkt | mittel | nein | wichtig | teilweise | Sieg merken wie in place_event_041 — <i>der fehlende Merker an Ort 100 ist behoben; die Erweiterung (fight_result liefert nie 2, ein Sieg setzt g_blackmage_done nirgends) bleibt offen - g_blackmage_done = 1 bedeutet "die Magier haben den Stein" (O264 laesst dann die Leiche mit dem Stein finden); nach einem Sieg den Merker zu setzen ergaebe einen zweiten Stein, welche Folge ein Sieg haben sollte, ist nicht belegbar</i> | OVL017.CPP |
| 181 | Kampf: einige Klänge fehlen | egal | nein | alles | ja | Heldenzweig: erstes Ereignis 8 (wie 11 im Gegnerzweig) statt der dort unmoeglichen 11 | OVL144.CPP |
| 182 | Orkfestung Ebene 1: Ereignisse zufällig | mittel | nein | wichtig | ja | Rueckgabe der Positionszweige nicht ueberschreiben, wenn kein Kampf stattfand | OVL261.CPP |
| 183 | Kampffeld: falsche Wände und Blockaden | mittel | nein | wichtig | teilweise | Gitter hat 24 Zeilen (0..23); Koordinaten (x, y) wie beim gelesenen Feld G[y + i][x - i]; Koordinaten (x, y) wie beim gelesenen Feld G[y + i][a6 - i] — <i>Zeilengrenze und vertauschte Koordinaten behoben; "G[1][x - 1]" bleibt (die Schleife beginnt bei x = 1, der Versatz wirkt beabsichtigt), fight_wall_shape findet n in den Spieldaten immer</i> | OVL139.CPP |
| 184 | Straßenüberfall: Lesen hinter der Liste | egal | nein | alles | ja | nur gefundene Helden benutzen (i < n), mode/v vorbelegen | OVL046.CPP |
| 185 | Gegenstände weitergeben: falscher Held geprüft | mittel | nein | wichtig | teilweise | Zielgegenstand am Quellplatz beim Quellhelden src pruefen; Zweihandpruefung am Quellplatz beim Quellhelden src; Last einheitlich mit KK + Mod (wie give_hero_new_item); Last einheitlich mit KK + Mod; Grenze vor dem Zugriff pruefen (nicht inv[31] lesen) — <i>Rueckgabe 1 bei leerem bzw. nicht ablegbarem Quellplatz bleibt (Aufrufer werten sie als 'erledigt' aus, eine Aenderung wuerde die Bedienung aendern)</i> | OVL085.CPP |
| 186 | Stadtereignisse: Text fehlt, gleiche Antworten | mittel | nein | wichtig | teilweise | k fuer unbekannte Felder vorbelegen; erste Herausforderung: Zaehler ist hier schon erhoeht (2) — <i>rematch zeigt fuer Antwort 2 und 3 weiterhin txt[0x43] - der gemeinte zweite Text ist nicht belegbar</i> | OVL166.CPP |
| 187 | „Sichern“ im Kampf wirkt erst eine Runde später | mittel | nein | wichtig | ja | erst die BP pruefen, dann auf 0 setzen (wie DSA 1) | OVL155.CPP |
| 188 | Zauberziel gegen Sondergegner falsch geprüft | mittel | nein | wichtig | ja | Ausnahme fuer Zauber Eigenschaften auch bei Sondergegnern 30..49; Obergrenze 50 fuer Sondergegner | OVL155.CPP |
| 189 | Sonderkampf-Animation läuft falsch | egal | nein | alles | teilweise | zweites Feld (9, 8) wie der Schluessel 0x908 — <i>nur das zweite Feld (9, 8) korrigiert; die Vorbelegung fuer unbekannte Schluessel (alle Positionen sind in der Sonderkampfkarte belegt) und das doppelte &w bleiben</i> | OVL142.CPP |
| 190 | Abspann: eine Zeile fehlt mit Sprachausgabe | egal | nein | alles | teilweise | nach dem letzten Eintrag nicht hinter das Feld lesen — <i>nur das Lesen hinter dem Feld in voice() behoben; die angeblich fehlende 9. Abspannzeile ist ein Leertext " " (alle 8 echten Zeilen werden gezeigt)</i> | OVL041.CPP |
| 191 | Lager: Wachen und Diebstahl fehlerhaft | mittel | nein | wichtig | teilweise | Anzahl einmal wuerfeln (Vergleich und Wert aus demselben Wurf); Abstand nur vergleichen, wenn d fuer diesen Weg gesetzt wurde; found je Wache zuruecksetzen; fehlende Gegenstaende nicht ueberspringen (k laeuft normal weiter); last vorbelegen; Meldung nur mit einem gefundenen Helden — <i>mode = -5 bei unbekannter Wachenzahl bleibt (Vorgabe ohne sichtbaren Fehler); Wurf, d, found, Diebstahlschleife und last behoben</i> | OVL072.CPP |
| 192 | Bolzenfalle im Phex-Tempel tötet hintere Helden | kritisch | nein | wichtig | ja | je Held 2 x (W6 + 6) LE; x summiert nur fuer die Ohnmachtsgrenze | OVL204.CPP |
| 193 | Lager: Entscheidung zufällig | mittel | nein | wichtig | ja | ohne gueltigen Helden nicht suchen (h vorbelegt); ans vorbelegen (Fehlerfaelle st == -1/-2) | OVL071.CPP |
| 194 | Orkfestung Ebene 3: Schaden bei jedem Betreten | mittel | nein | wichtig | teilweise | Rueckgabe der Positionszweige nicht ueberschreiben, wenn kein Kampf stattfand — <i>nur die ueberschriebene Rueckgabe behoben; der doppelte Zweig 0x1308 ist toter Code (Entfernen aendert nichts), der Schaden an 0x213 bei jedem Betreten kann gewollt sein</i> | OVL267.CPP |
| 195 | Abblenden: Palette nur teilweise gelöscht | egal | nein | alles | ja | ganze Palette schwarz setzen | OVL055.CPP |
| 196 | Treffen mit Teilgruppen unterwegs unzuverlässig | mittel | nein | wichtig | teilweise | Lage der anderen Gruppe nur verwenden, wenn sie in diesem Durchlauf auf demselben Weg gefunden wurde; Ja/Nein in eigener Variable — <i>Gruppentreffen behoben (Lage je Durchlauf, Antwort getrennt); der leere Zweig bei hurry == 0xff und der Vergleich "speed >= route_pos" bleiben - was dort gemeint war, ist nicht belegbar (ein geratener Inhalt wuerde das Reisetempo bzw. die Lage beim Nachtlager aendern)</i> | OVL215.CPP |
| 197 | Spielstand laden: eine Einstellung geht verloren | mittel | nein | wichtig | ja | alten Ortstyp uebernehmen (der neue ist hier immer 0); Suche auf die Zahl der Spielstaende begrenzen; isalnum mit unsigned char (nicht vor _ctype lesen); Ergebnis von _open pruefen | OVL107.CPP |
| 198 | Ohne EMS: falsche Pixel im Bildschirmrahmen | egal | nein | alles | ja | Quelle je 48-KB-Block weiterzaehlen; je Block 0xc000 Bytes lesen und Ziel weiterzaehlen; ungerade Laenge: erst den Hauptteil lesen, dann die 2 Bytes, last danach eintragen | OVL010.CPP |
| 199 | Unrathaufen im Orkgefängnis: endlos Gegenstände | kritisch | ja | wichtig | ja | nach vier Funden nichts mehr finden (kein switch ohne Fall mit Muellwerten) | OVL264.CPP |
| 200 | Automatische Zielwahl bevorzugt eine Seite | egal | nein | alles | teilweise | Zeilen nur bis 23; Umkreis symmetrisch bis sx + r bzw. sy + r; Ziel tx/ty nicht fuer die folgenden Felder ueberschreiben (Kopie) — <i>das verworfene Ergebnis von fig_list_add im Verschiebezweig bleibt - es steht in einem mehrzeiligen Makro, in das kein #if passt; ohne sichtbare Folge</i> | OVL145.CPP |
| 201 | Startausrüstung: Lesen hinter dem Eintrag | egal | nein | alles | ja | Grenze vor dem Zugriff pruefen | OVL084.CPP |
| 202 | 3D-Objekte: eine Palettenfarbe zu viel | egal | nein | alles | teilweise | genau nr_colors Palettenfarben lesen — <i>Typ 6 setzt den Bildzeiger der Objekte 16/17 auf sich selbst - wirkungslos, die eigentliche Absicht ist nicht belegbar; Palettenschleife behoben</i> | OVL097.CPP |
| 203 | Flammenschlüssel beliebig oft holbar | mittel | ja | wichtig | ja | nach dem Nehmen Zustand 4 wie in Zustand 1/2 (Schluessel nur einmal) | OVL132.CPP |
| 204 | Hesinde-Statue: geschwächter Held entgeht der Strafe | mittel | nein | wichtig | ja | zu wenig LE bestraft jeden Helden (nicht nur den Ersatzhelden) | OVL132.CPP |
| 205 | Totenkopf-Ornament: „%s“ statt Heldenname | egal | nein | alles | nein | kein Fehler - FINAL.LTX[79] enthaelt keinen Platzhalter %s (geprueft an spieltexte/texte/FINAL.txt), die Meldung ist ohne sprintf richtig |  |
| 206 | Absturz bei der Bengram-Verwechslung in Tjolmar | kritisch | nein | wichtig | ja | nur vorhandene Helden treffen (kein Schreiben ueber den Nullzeiger); Spende nur abziehen, wenn das Geld reicht | OVL239.CPP |
| 207 | Gruppentrennung mit den falschen Helden | mittel | nein | wichtig | ja | arr.h[1..k] fuellen, bevor sie abgetrennt werden; ueber die abgetrennten Helden arr.h[1..k] laufen; unbekannter Code: Grundfarbe statt Muell; unbekannter Code: Text wie 0xff statt Muellzeiger; r vorbelegen | OVL034.CPP |
| 208 | Automap ohne XMS/EMS kaputt, Notizreste | mittel | nein | wichtig | teilweise | Archiv nicht ein zweites Mal auslagern; freigegebene Zeiger zuruecksetzen (mem_free liefert 0); ganzen Notiztext (0xfc Bytes) leeren — <i>open() ohne XMS und ohne EMS bleibt ohne Fehlerbehandlung - die Automap braucht den Auslagerungsspeicher, ein sauberer Abbruch braeuchte Aenderungen bei allen Aufrufern; Notiztext, Zeiger und doppeltes Auslagern behoben</i> | OVL052.CPP |
| 209 | Speicherfehler bei über 160 MB XMS | egal | nein | alles | ja | hoechstens 10 Bloecke (Groesse von g_xms_handles) | SEG005.CPP |
| 210 | Textdateien laden ohne Fehlerprüfung | egal | nein | alles | nein | die Pruefungen (open, Suchgrenze) kosten 21 Bytes im residenten Segment SEG005 - kein Platz (das Segment darf nicht laenger werden, der freigeraeumte Totcode reicht nur fuer Nr. 209/230/231); nur bei kaputten Dateien |  |
| 211 | Bild laden mit zu kleinem Puffer (ungenutzt) | egal | nein | alles | ja | Puffer fuer w * h Bytes anlegen (statt 1 Byte) | SEG042.C |
| 212 | Orkgefängnis: beschlagnahmte Ausrüstung für immer weg | kritisch | nein | wichtig | ja | naechsten Platz der Ablage benutzen (n erhoehen) - die Gegenstaende bleiben erhalten | OVL263.CPP |
| 213 | Arax-Gift lähmt Gegner völlig | mittel | ja | wichtig | ja | nach unten begrenzen (mindestens 1 BP) | OVL079.CPP |
| 214 | Tanzen für Geld nutzt Musizieren | mittel | ja | wichtig | ja | Probe auf Tanzen | OVL006.CPP |
| 215 | Tollwut-Anfall: Beruhigen gelingt verkehrt herum | mittel | nein | wichtig | ja | Rueckgabe = Zahl der gescheiterten Helden: gelungen bei 0 | OVL082.CPP |
| 216 | Lowangen, zweites Stadttor: Unsinnstext | mittel | nein | wichtig | ja | NAKED.LTX hat 35 Texte: der sonst unbenutzte Text 34 ("Wie aus dem Boden gewachsen steht ploetzlich eine Gestalt vor Euch.") passt zum Gavron-Bild (vermutlich 0x22 statt 0x25) | OVL185.CPP |
| 217 | Bild-Entpacker im Speicher: Zufallswert | egal | nein | alles | nein | im residenten Segment SEG041 kein Platz (die Laengenberechnung kostet 17 Bytes); die Rueckgabe von nvf_process wird nirgends ausgewertet - folgenlos |  |
| 218 | Unratfund heißt immer „einen Dolch“ | egal | nein | alles | ja | Fundtext BLUT2.LTX[41]; Fundtext BLUT2.LTX[42]; Fundtext BLUT2.LTX[43] | OVL264.CPP |
| 219 | Gruppenwechsel überspringt NSC-Gruppen nicht | egal | nein | alles | ja | NSC-Zeiger auf g_heroes[6] (0x239a lag 0x20 Bytes davor) | OVL075.CPP |
| 220 | Meditation mit wenig LE kostet AE | mittel | nein | wichtig | ja | bei zu wenig LE keine Umwandlung (kein negativer AE-Gewinn) | OVL091.CPP |
| 221 | Finale: Überblendung von Schwarz (gewollt) | egal | nein | alles | nein | pal_from kommt aus genulltem Speicher (schwarze Palette) - Ueberblenden von/nach Schwarz ist wahrscheinlich gewollt (unsicher), kein Flackern |  |
| 222 | Steinschlag: danach evtl. falsche Wegtexte | egal | nein | alles | ja | die Ereignistexte des Weges wiederherstellen (SECTION2 nur fuer den Steinschlag) | OVL165.CPP |
| 223 | Orkschützen an Weg 250 (kein Fehler) | egal | nein | - | kein Fehler | kein Fehler: get_next_hero liefert nie 0 |  |
| 224 | Kampffiguren am Rand ungleich abgeschnitten | egal | nein | alles | ja | senkrecht symmetrisch: cy + 5 wie waagerecht cx + 5 | OVL112.CPP |
| 225 | Absturzbericht liest über den Speicherblock | egal | nein | alles | nein | der Absturzbericht kennt die Groesse des Blocks hinter ptr nicht; im Real Mode ist das Lesen ueber Blockgrenzen ungefaehrlich (der Auszug wird nur gelesen und in die Datei geschrieben) |  |
| 226 | Wasserfall über Weg 25 mit falschen Texten | egal | nein | alles | nein | unerreichbar (Weg 25 hat in SECTION.DAT kein Ereignis 25, tevent_025 laeuft nur ueber Weg 178 mit den richtigen Texten) - Verteiler und Texte bleiben |  |
| 227 | Dialogskript: Vergleich mit leerem Feld | egal | nein | alles | ja | last vorbelegen (wie nach TOPIC) | OVL260.CPP |
| 228 | Handbuchabfrage in der CD-Fassung ohne Wirkung | egal | nein | - | kein Fehler | gewollt: Schutz ueber cd_check, Rest der Disketten-Fassung |  |
| 229 | Zeitgeber prüft denselben Wert doppelt | egal | nein | - | kein Fehler | ohne Wirkung: doppelter Test, Absicht nicht belegbar |  |
| 230 | CD-Prüfung bei leerer PATH.CFG | egal | nein | alles | ja | leere PATH.CFG: kein Laufwerk statt Zufallswert (Vorbelegung statt Pruefung, spart Platz) | SEG005.CPP |
| 231 | CD-Prüfung bricht ohne Aufräumen ab | egal | nein | alles | ja | regulaeres Programmende (atexit-Funktionen laufen) | SEG005.CPP |
| 232 | Speichern: Existenzprüfung mit > statt >= | egal | nein | alles | ja | Handle 0 ist gueltig (gleiche Codelaenge, der ueberwachte Aufruf bleibt unveraendert) | SEG001.CPP |
| 233 | Kriegerhüne: Abschiedstext erscheint nie | mittel | nein | wichtig | nein | in welchem Zweig (bzw. bei welchem Gespraechsausgang von Dialog 0x82) der Merker ds:2595 gesetzt werden sollte, ist nicht belegbar - vermutlich setzte ihn eine Dialogaktion; ein geratener Zweig zeigte den Text womoeglich an der falschen Stelle |  |
| 300 | DIGPAK-Klangtreiber: Stapelfehler (ungenutzt) | egal | nein | alles | ja | nur das gepushte Wort abraeumen | SEG022.ASM |
| 301 | Zeichenart „Oder“ kopiert falsch (ungenutzt) | egal | nein | alles | ja | Oder-Schleife fortsetzen | SEG046.ASM |
| 302 | Rückwärtskopieren versetzt (ungenutzt) | egal | nein | alles | nein | eine richtige Rueckwaertskopie braucht Zeigerkorrekturen zwischen movsb/movsw/movsd und ein cld - zusaetzliche Befehle in einem residenten Segment (es darf nicht laenger werden, sonst verschieben sich alle folgenden residenten Adressen); kein Aufrufer benutzt das Rueckwaertsflag |  |
| 303 | Debug-Wächter zeigt immer Rot (toter Code) | egal | nein | - | kein Fehler | toter Debug-Code (Waechter nie eingehaengt) |  |
| 304 | XMS-Vorgabe ohne Rücksprung | egal | nein | alles | ja | nicht in xms_present weiterlaufen; SP = BP schon nach den pops (spart 2 Bytes fuer das retf oben) | SEG039.ASM |
| 305 | XMS-Kopie liest Einsprung aus falschem Segment | egal | nein | alles | nein | der Einsprung muesste ueber den Stapel gerufen werden (8 Bytes mehr im residenten Segment SEG039, kein Platz); alle Aufrufer uebergeben &g_xms_move im Datensegment - folgenlos |  |
| 306 | Flat-Modus ohne Fehlerprüfung | egal | nein | alles | nein | unerreichbar - flat_enter laeuft nur im Realmodus (im V86-Modus nimmt SEG005 immer den EMS-Zweig); eine Auswertung in den Aufrufern braeuchte zusaetzlichen residenten Code |  |
| 307 | Kopierroutine verliert ungerades Byte | egal | nein | alles | ja | ECX nicht sichern (C-Aufrufer halten darin nichts), Platz fuer jnc/movsb; gerade Laenge fertig | SEG035.ASM |
| 350 | Ohne EMS vereinzelt falsche Bodenpixel | egal | nein | alles | nein | der Pixelschritt ist VIEW_W-fach abgerollt; "adc esi,edx" ist 1 Byte laenger als "adc si,dx" - SEG024 wuerde um mehrere hundert Bytes wachsen und alle folgenden residenten Adressen und das Datensegment verschieben; Folge im Original nur einzelne falsche Bodenpixel ohne EMS |  |
| 351 | Tiefensortierung bei 129 Flächen | egal | nein | alles | ja | Verkettung next/depth hat nur Platz fuer 1..128 | SEG025.ASM |
| 352 | Lagerhaus: Waren kehren zurück, Lagerkosten explodieren | kritisch | nein | wichtig | teilweise | Gebuehr wirklich abbuchen und den Zeitstempel neu setzen (Store::run speichert am Ende); Platz 6 = NSC ueber group_get_npc (heroes[6] ist bei kleiner Gruppe leer); s->value enthaelt schon den alten Warenwert (store_deposit beginnt mit depot->value) - nicht verdoppeln; als verkauft/zerbrochen gemeldete Plaetze (take_qty -1) wirklich entfernen; Warenwert = Wert des Rests — <i>Einlagern (Warenwert nicht verdoppeln, NSC ueber group_get_npc), Gebuehr wirklich abbuchen und Zeitstempel setzen, Abholen (Restwert, verlorene Plaetze entfernen) behoben; die Anzeige in store_fetch_fill (nur die ersten count Plaetze) und der Klick auf ein leeres Portraet bleiben</i> | OVL012.CPP OVL099.CPP OVL100.CPP |
| 353 | Holzfällerlager: Abbrechen legt ein Stück dazu | mittel | ja | wichtig | ja | Abbruch (-1) aendert nichts; nur abziehen, was wirklich gegeben wurde | OVL116.CPP |
| 354 | Phexhelm und Phexschild vervielfachen | mittel | ja | wichtig | ja | Helm und Schild bei allen Gruppen suchen; nur zuruecklegen, was abgenommen wurde (keine Vervielfachung) | OVL204.CPP |
| 355 | Gefangennahme: Ringe und Amulette vervielfacht | egal | nein | alles | nein | die Ablage speichert je Gegenstand nur Kennung und Anzahl (struct stash_entry, im Spielstand) - Ladungen und Zustand gesondert zu speichern aendert das Spielstandformat |  |
| 356 | Geld verschwindet beim Vereinen von Gruppen | mittel | nein | wichtig | teilweise | Anteil nur ueber die Helden ohne NSC rechnen; der NSC bekommt keinen Anteil am Gruppengeld und behaelt sein eigenes Geld — <i>Anteil ohne NSC gerechnet, der NSC bekommt beim Umzug kein Gruppengeld und behaelt sein eigenes; die NSC-Abgabe in add_money beim Einbuchen eines Helden bleibt (Abgabe auf Einnahmen ist so gewollt, Einbuchen ist davon nicht zu unterscheiden)</i> | OVL200.CPP OVL202.CPP |
| 357 | Asthenilschwert per ESC behalten | mittel | ja | wichtig | ja | ESC bei der Frage tauschen/mitnehmen bricht ab, statt den Stein ohne Folgen nehmen zu lassen | OVL001.C |
| 358 | Schlüsselbund und Rezeptbuch verlieren im Lagerhaus den Inhalt | mittel | ja | wichtig | nein | es gibt keine Funktion, die einen ganzen Inventarplatz (mit Bits, Ladungen, Bruchfaktor) an die Gruppe uebergibt; eine eigene Verteilung mit Platzsuche, Traglast und Ablage-Abfrage waere ein Neubau von group_give_item |  |
| 359 | Geweihte Waffen verlieren „magisch“ | mittel | nein | wichtig | nein | wie Nr. 358 (Schmiede und Lager geben neue Gegenstaende aus); Teil (3) in O263 ist im Original folgenlos und haengt mit Nr. 355 zusammen |  |
| 360 | Kaputte und heile Gegenstände im selben Stapel | mittel | ja | wichtig | nein | offen fuer die naechste Runde (Stelle in move_item kommt zweimal vor, Einbau nicht mehr abgeschlossen) |  |
| 361 | Volles Inventar: Asthenilring oder magischer Wasserschlauch weg | kritisch | nein | wichtig | ja | Ring bei vollem Rucksack ueber die Gruppe geben (fragt nach Ablegen), Merker nur bei Erfolg; Schlauch bei vollem Rucksack ueber die Gruppe geben, Merker nur bei Erfolg | OVL060.CPP OVL066.C |
| 362 | Schlüssel verschwindet beim Abnehmen vom Bund | kritisch | nein | wichtig | ja | Schluessel nur vom Bund nehmen, wenn er wirklich ins Gepaeck kam | OVL048.CPP |
| 363 | Leere Gruppe hängt das Spiel auf | kritisch | nein | wichtig | ja | der letzte Held der Gruppe stuerzt nicht ab (leere Gruppe haengt das Spiel auf), er steigt wieder heraus; den letzten Helden der Gruppe nicht entfernen (leere Gruppe haengt das Spiel auf); der letzte Held der Gruppe geht nicht verloren; ohne zweiten Helden keine Probe ueber den Nullzeiger | OVL059.CPP OVL074.CPP OVL239.CPP |
| 364 | Horriphobus: Kampf endet nie | kritisch | nein | wichtig | ja | ohne Fluchtweg endet die Angst (sonst kann der Kampf nie enden) | OVL128.CPP |
| 365 | MR der NSC falsch (Korima, Thurazz) | egal | nein | alles | nein | die MR steht in den NSC-Dateien (Spieldaten); eine Neuberechnung beim Aufnehmen aenderte gespeicherte Werte laufender Spielstaende - egal |  |
| 366 | Tierkunde an den Spinnennetzen invertiert | egal | nein | alles | ja | warnen, wenn mindestens ein Held die Probe schafft (nicht, wenn einer scheitert) | OVL264.CPP |
| 367 | Hesinde-Wunder 8 und 9 bewirken nichts | mittel | nein | wichtig | nein | der Wuerfel steht im Datensegment (g_temple_miracle_dice, darf nicht geaendert werden), und welche Wunder fuer 8 und 9 gedacht waren, ist nicht belegbar |  |
| 368 | Hesinde-Statue als AP-Quelle | mittel | ja | wichtig | ja | 20 AP je Held nur nach einem Opfer (Merker gesetzt), nicht bei jeder Ablehnung | OVL132.CPP |
| 369 | Kräuterwirkungen verfallen nicht, Werte kippen | kritisch | ja | wichtig | ja | Wirkung von Platz 0 auch im Heldenbogen (g_dialog_depth != 0) wirklich zuruecknehmen | SEG002.C |
| 370 | Kräutersuche über „Alte Einstellungen“ ohne Malus | mittel | ja | wichtig | ja | 1-Bit-Merker setzen statt +1; 1-Bit-Merker setzen statt +1 (1 + 1 ergab 0) | OVL071.CPP |
| 371 | Stufenanstieg: Kampftalent steigert immer die Attacke | mittel | nein | wichtig | nein | unsicher - ob AT oder PA steigen soll, ist eine Regelfrage (DSA: Spielerwahl); eine Abfrage waere neue Bedienung, eine feste Verteilung eine Spielaenderung |  |
| 372 | Nichtzauberer bekommen Zaubersteigerungen angeboten | egal | nein | alles | ja | Zaubersteigerungen eines vorher gewaehlten Zaubertyps zuruecksetzen | OVL167.CPP |
| 373 | Weitere Importfehler (toter Held, Kampfwerte) | mittel | nein | wichtig | nein | unsicher - tote Helden und Kampfwerte beim DSA-1-Import; die Neuberechnung von RS/BE und AT/PA aus der Ausruestung braucht Funktionen, die der Import nicht kennt; ohne DSA-1-Spielstand nicht pruefbar |  |
| 374 | Markthalle Lowangen: Ereignis bleibt hängen | mittel | ja | wichtig | ja | Merker nach dem Kampf zuruecksetzen (bei leerem Kampf laeuft fight_finish nicht und der naechste Kampf endet sonst mit Bettlerflucht) | OVL184.CPP |
| 375 | Agdan-Quest: kein Lowangen-Schlüssel | kritisch | nein | wichtig | ja | Schluessel bei vollem Rucksack ueber die Gruppe geben (fragt nach Ablegen); Zustand nur bei Erfolg, sonst bietet Eolan ihn erneut an | OVL013.CPP |
| 376 | Textfehler „Rechstschritt“ | egal | nein | alles | nein | Tippfehler in der Spieldatei BLUT3.LTX - kein Programmcode |  |
