# Abweichungen zum Original

Nur Fehlerbehebungen aus `Port/src/inc/s_fix.h`. Jede ist ein Schalter.

Keine Behebung ändert Strukturen, das Spielstandformat oder das Datensegment.

## Stufen

| Bau | Bedeutung | EXE |
| :--- | :--- | :--- |
| `python3 Port/bauen_port.py original` | Stufe 0, Verhalten der CD-Fassung C1.02 | `SCHWEIF_PORT_ORIG.EXE` |
| `python3 Port/bauen_port.py wichtig` | Stufe 1, Folgen kritisch oder mittel | `SCHWEIF_PORT_WICHTIG.EXE` |
| `python3 Port/bauen_port.py alles` | Stufe 2, dazu die übrigen | `SCHWEIF_PORT_ALLES.EXE` und `SCHWEIF_PORT.EXE` |

Ein einzelner Schalter lässt sich übersteuern, zum Beispiel `python3 Port/bauen_port.py alles -DFIX_0020=0`. `1` schaltet ihn an, `0` aus. Die drei Stufen sind die vorgesehene Auswahl.

## Liste

| Schalter | Stufe | Gewicht | Titel |
| :--- | :--- | :--- | :--- |
| `FIX_0001` | wichtig | mittel | Verzauberte Gegner patzen im Angriff ohne Folgen |
| `FIX_0002` | wichtig | mittel | Automatisches Steigern ignoriert die Grenze je Talent |
| `FIX_0003` | alles | egal | Klickflaechen-Pruefung tauscht falsche Werte |
| `FIX_0004` | wichtig | mittel | Naechtlicher Dieb bestiehlt nur den ersten Helden |
| `FIX_0005` | wichtig | mittel | Wegereignisse loesen an zufaelligen Stellen aus |
| `FIX_0006` | wichtig | kritisch | Orks werfen abgenommene Ausruestung weg (wie Nr. 212) |
| `FIX_0007` | alles | egal | Karte aufdecken: Grenzpruefung doppelt |
| `FIX_0008` | wichtig | mittel | Zufallsbegegnungen in einer Gelaendeart falsch |
| `FIX_0009` | alles | egal | Bild-Entpacker liefert Zufallswert zurueck |
| `FIX_0010` | alles | egal | Heldenvorauswahl steht manchmal auf leerem Platz |
| `FIX_0011` | alles | egal | Marktstand mit unbekanntem Feld |
| `FIX_0012` | wichtig | mittel | Orkgefaengnis: bei Alarm zwei Kaempfe hintereinander |
| `FIX_0013` | wichtig | mittel | Hoehlenspinnen kommen bis zur Nestzerstoerung wieder |
| `FIX_0014` | wichtig | kritisch | Absturz beim Schluesselbund mit 18 oder mehr Schluesseln |
| `FIX_0015` | alles | egal | Schluesselbund/Rezeptbuch: falsche "schon drin"-Meldung |
| `FIX_0016` | alles | egal | Einzelne Pixel in falscher Farbe |
| `FIX_0017` | alles | egal | Speicherleck beim Verlassen der 3D-Ansicht |
| `FIX_0018` | wichtig | mittel | Flucht vom Randfeld wird manchmal verweigert |
| `FIX_0019` | alles | egal | NSC aufnehmen: Hilfsvariable doppelt benutzt |
| `FIX_0020` | wichtig | kritisch | Fahrender Haendler: Ware bezahlt, aber nicht erhalten |
| `FIX_0021` | alles | egal | Haendler-Warenliste ueberlappt bei riesigem Sortiment |
| `FIX_0022` | alles | egal | Automap findet sehr weite Wege nicht |
| `FIX_0023` | alles | egal | Versiegelte Schloesser: Erleichterung wirkungslos |
| `FIX_0024` | alles | egal | Warenauswahl: Pfeil links springt an den Anfang |
| `FIX_0025` | wichtig | kritisch | Geld ins Minus: Millionen durch Gruppenteilung |
| `FIX_0026` | alles | egal | Gruppe entfernen aendert fremdes Byte |
| `FIX_0027` | alles | egal | Texteingabe: fehlende Endnull |
| `FIX_0028` | alles | egal | Texteingabe: Rueckgabewert fehlt |
| `FIX_0029` | alles | egal | Kampfbewegung: Animationsfolge ohne Laengengrenze |
| `FIX_0031` | alles | egal | Ortswechsel ueberschreibt den gemerkten alten Ort |
| `FIX_0032` | wichtig | mittel | Absturz im versteckten Entwicklerbildschirm |
| `FIX_0033` | alles | egal | Kalender: falscher Tagesueberlauf |
| `FIX_0034` | alles | egal | Kalender: Tick-Ueberlauf verliert Restzeit |
| `FIX_0035` | alles | egal | Kalender: Jahreswechsel ergibt Monat 13 |
| `FIX_0036` | alles | egal | Reisekarte: Richtungswahl ohne Rueckgabewert |
| `FIX_0037` | wichtig | mittel | Absturz-Tagebucheintrag auch ohne Absturz |
| `FIX_0038` | alles | egal | Kampffeld: Belegt-Pruefung ohne Wirkung |
| `FIX_0039` | wichtig | mittel | Haenger bei voellig zugestelltem Kampffeld |
| `FIX_0041` | alles | egal | Paradepatzer nennt den falschen Helden |
| `FIX_0042` | alles | egal | Patzerfolge ohne Schaden geprueft |
| `FIX_0043` | wichtig | mittel | Steinbock frisst die letzte Suessigkeit umsonst |
| `FIX_0044` | alles | egal | Zahleneingabe zeigt falsche erste Ziffer |
| `FIX_0045` | wichtig | mittel | Kraeuterstelle an Weg 165 beliebig oft ernten |
| `FIX_0046` | alles | egal | Reisekarte: Wegsuche liest hinter der Tabelle |
| `FIX_0047` | alles | egal | Reisekarte: Wegzeichnung im Randfall falsch |
| `FIX_0048` | alles | egal | Brenne toter Stoff ohne Fackel: Meldung zufaellig |
| `FIX_0049` | alles | egal | Sonderkampf: Lesen vor dem Nulltest |
| `FIX_0050` | alles | egal | Phex-Tempel: nur eine von vier Flaechen animiert |
| `FIX_0051` | wichtig | mittel | Herbergsdieb stiehlt nie Proviant, sondern Geld |
| `FIX_0052` | alles | egal | Gegenstandssuche ueber alle Plaetze kaputt |
| `FIX_0053` | alles | egal | Portraetleiste einer Teilgruppe zu breit/schmal |
| `FIX_0054` | alles | egal | Gegnerzauber bei leerer Zauberliste |
| `FIX_0055` | alles | egal | Faehre Hilvalla: zu wenig Geld zeigt Begruessung erneut |
| `FIX_0056` | alles | egal | Tagebuch drucken: Ueberlauf bei langen Zeilen |
| `FIX_0057` | alles | egal | Tagebuch-Heldennamen: Lesen hinter dem Feld |
| `FIX_0058` | alles | egal | Ortssymbole: unvollstaendige Aenderungspruefung |
| `FIX_0059` | wichtig | mittel | NSC verlaesst die Gruppe ohne Abschiedstext |
| `FIX_0060` | alles | egal | Teleport mit unbekannter Herkunft |
| `FIX_0061` | alles | egal | Gruppe teilen bei sechs Gruppen |
| `FIX_0062` | alles | egal | Portraet ziehen liest vor der Tabelle |
| `FIX_0063` | wichtig | mittel | Wirselkraut an Weg 201 beliebig oft ernten |
| `FIX_0064` | alles | egal | Eine Gegner-Animation wird nie gezeigt |
| `FIX_0065` | alles | egal | Genitiv-s bei Namen mit Sonderzeichen falsch |
| `FIX_0066` | alles | egal | DSA-1-Import: doppelte Pruefung |
| `FIX_0067` | alles | egal | Dialogskript: Zeiger nicht genullt |
| `FIX_0068` | alles | egal | Klangtreiber: kleines Speicherleck |
| `FIX_0069` | alles | egal | Kartenraetsel: toter Programmcode |
| `FIX_0071` | wichtig | mittel | Lagerhaus beim ersten Besuch manchmal gesperrt |
| `FIX_0072` | wichtig | mittel | Boron-Wunder: Totenerweckung viel zu haeufig |
| `FIX_0073` | alles | egal | Kampfanimation: ueber 40 Animationen gleichzeitig |
| `FIX_0074` | alles | egal | Bildklasse: Randfehler ohne Aufrufweg |
| `FIX_0075` | alles | egal | Meldung "versteckte Gegner" zaehlt manchmal falsch |
| `FIX_0076` | alles | egal | Sumpfrantzendokument gegen Gegner ohne Meldung |
| `FIX_0077` | wichtig | mittel | Vorratslager: Debugtext als Mengenfrage |
| `FIX_0078` | wichtig | mittel | Lagerhaus: Verderb nach einem Jahr verkehrt herum |
| `FIX_0079` | wichtig | mittel | Kampf: Figur laeuft selten in Zufallsrichtung |
| `FIX_0080` | alles | egal | Reisekarte: Position im Randfall falsch |
| `FIX_0081` | alles | egal | Zufaelliger Gegenstandsverlust fehlerhaft |
| `FIX_0082` | alles | egal | Gegenstand an jeden Helden: Restmenge falsch |
| `FIX_0083` | alles | egal | Gruppenverwaltung schreibt hinter das Feld |
| `FIX_0084` | wichtig | kritisch | Laden: Ware bezahlt, aber nicht erhalten |
| `FIX_0085` | alles | egal | Laden-Warenliste ueberlappt bei riesigem Sortiment |
| `FIX_0086` | alles | egal | CD-Zwischenspeicher bei Lesefehlern kaputt |
| `FIX_0087` | alles | egal | Gegenstaende scheinen durch Waende |
| `FIX_0088` | alles | egal | Wegetexte: haengende Zeiger |
| `FIX_0089` | alles | egal | Kobold Mumpitz: Auswahl liest ausserhalb |
| `FIX_0090` | alles | egal | Kampf-Figurenliste: mehrere Randfehler |
| `FIX_0091` | alles | egal | Dateizugriff: Handle-Leck und spaete Fehlerpruefung |
| `FIX_0092` | alles | egal | Hintergrundanimation zeigt evtl. falsches Teilbild |
| `FIX_0093` | wichtig | kritisch | Absturz beim Bauern mit dem gebrochenen Rad |
| `FIX_0094` | wichtig | mittel | Laden: Gepaeckwarnung fehlt oder falsch |
| `FIX_0095` | alles | egal | Leeres Zunderkaestchen: doppelte oder alte Meldung |
| `FIX_0097` | alles | egal | Kompassrose verschwindet im Randfall |
| `FIX_0098` | wichtig | mittel | Schlafender NSC steht wach im Kampf |
| `FIX_0099` | alles | egal | Dialogfenster: Randfaelle ohne Wirkung |
| `FIX_0100` | alles | egal | Schenke: Andrang nach Mitternacht (unklar) |
| `FIX_0101` | alles | egal | Kraeuterstelle an Weg 41: Zaehler ohne Grenze |
| `FIX_0102` | wichtig | mittel | Gewoelbe des Namenlosen: Ereignisse zufaellig |
| `FIX_0103` | alles | egal | Tagebuchsuche: Esc sucht statt abzubrechen |
| `FIX_0104` | alles | egal | Altsvellt-Sumpf: alte Ebene ueberschrieben |
| `FIX_0105` | alles | egal | Spinnennetze: Schreiben nach Adresse 0 |
| `FIX_0106` | wichtig | mittel | Sonderkampf: Flaechenangriff trifft niemanden |
| `FIX_0107` | alles | egal | Heldenerschaffung: uninitialisierte Werte |
| `FIX_0108` | alles | egal | Auswahlliste ohne Grenzpruefung |
| `FIX_0109` | wichtig | mittel | Texteingabefeld: Grafikfehler und Zeichenmuell |
| `FIX_0110` | wichtig | mittel | Stabzauber prueft eine Eigenschaft doppelt |
| `FIX_0111` | wichtig | mittel | Gegnerischer Heilzauber verletzt Schwerverwundete |
| `FIX_0112` | alles | egal | Kriegerhuene: Wegetexte unnoetig neu geladen |
| `FIX_0113` | alles | egal | Gegnerpatzer gegen bewusstlose Helden entfallen |
| `FIX_0114` | wichtig | mittel | Lorenfahrt: Abspringen gefahrlos, aber ohne AP |
| `FIX_0115` | alles | egal | Lore: Lesen vor der Gueltigkeitspruefung |
| `FIX_0116` | alles | egal | Auto-Erschaffung: Akademiezauber doppelt bedacht |
| `FIX_0117` | wichtig | mittel | Heilkunde Wunden: Probe doppelt gewuerfelt |
| `FIX_0118` | wichtig | mittel | Wasserfall-Spende: Vollheilung fast umsonst |
| `FIX_0119` | wichtig | mittel | Warzennashorn-Schaden und Kraeuterprobe Weg 36 falsch |
| `FIX_0120` | wichtig | mittel | Automatischer Kampf: Helden zaubern auf falsches Ziel |
| `FIX_0121` | wichtig | mittel | Sehr lange Reiseetappen dauern zu kurz |
| `FIX_0122` | wichtig | mittel | Ingerimms Schmiede: Meldung fehlt, "und" falsch |
| `FIX_0123` | wichtig | kritisch | Unendlich Geld aus Truhen |
| `FIX_0124` | wichtig | mittel | Reisetempo im Randfall falsch |
| `FIX_0125` | alles | egal | Zwischensequenz: Ueberblendung zufaellig |
| `FIX_0126` | wichtig | mittel | Grosser AP-Bonus im Gespraech wird nie vergeben |
| `FIX_0127` | wichtig | mittel | Klangeinstellung: Port/IRQ-Eingabe verworfen |
| `FIX_0128` | wichtig | mittel | Fahrender Haendler: falsche Menge beim Zuruecklegen |
| `FIX_0129` | wichtig | mittel | Speicherfehler loescht die falsche Datei |
| `FIX_0130` | wichtig | mittel | Gulmondblatt: Spieltext durch Heldennamen ersetzt |
| `FIX_0131` | alles | egal | Starker Zaubertrank nennt Lebenspunkte |
| `FIX_0132` | alles | egal | Ortsszene: Musikwahl zufaellig |
| `FIX_0133` | alles | egal | Binge: Position doppelt gesetzt |
| `FIX_0134` | wichtig | mittel | Schwarzmagier-Szene: Abschlusstext und Ton fehlen |
| `FIX_0135` | wichtig | mittel | Gespraech mit Gegenstandswahl laesst sich nicht abbrechen |
| `FIX_0136` | alles | egal | Fenster springt beim Verschieben |
| `FIX_0138` | wichtig | mittel | Shurinknollen-Gift: Erholung extrem langsam |
| `FIX_0139` | alles | egal | Kellertuer: Wandbild bei nicht quadratischer Karte |
| `FIX_0140` | wichtig | mittel | Pelzwand: Abzug beim falschen Gegenstand |
| `FIX_0141` | alles | egal | Animation: haengender Zeiger bei fehlender Datei |
| `FIX_0142` | alles | egal | Lagerplatz: Suche liest hinter der Tabelle |
| `FIX_0143` | wichtig | mittel | Zwergenbinge: Portal versetzt andere Teilgruppe |
| `FIX_0144` | wichtig | mittel | Tagebuch: Seitenzaehlung falsch, Ueberlauf moeglich |
| `FIX_0145` | wichtig | mittel | Erschaffung: Eigenschaften verschieben falsch geprueft |
| `FIX_0146` | wichtig | mittel | Spendenschrein: falscher Text, Zufallswert |
| `FIX_0147` | alles | egal | Heldenaktion mit unbekannter Zielart |
| `FIX_0148` | alles | egal | Nichtzauberer koennen das Zaubermenue oeffnen |
| `FIX_0149` | wichtig | mittel | Ankunft in Lowangen: erster Textteil fehlt |
| `FIX_0150` | alles | egal | Faehre: Zufallswert an unbekanntem Ort |
| `FIX_0151` | wichtig | mittel | Zwergengrab: Frevel ohne Kriegsaxt |
| `FIX_0152` | alles | egal | Alchimie/Schaetzen: kleine Randfehler |
| `FIX_0153` | wichtig | mittel | Reise bricht unvermittelt ab |
| `FIX_0154` | wichtig | mittel | Blaue Keuche steckt mit Dumpfschaedel an |
| `FIX_0155` | wichtig | mittel | Herberge: Abbruch loescht die alte Zimmerbuchung |
| `FIX_0157` | wichtig | mittel | Orkfestung Ebene 3: ausgewuerfelte Gegner fehlen |
| `FIX_0158` | alles | egal | Schenke: leerer Text, merkwuerdiger Rundenzaehler |
| `FIX_0159` | wichtig | mittel | Armatrutz eine Stufe hoeher als die AE erlaubt |
| `FIX_0160` | wichtig | mittel | Zwergenbinge: Gruppe falsch, fremder Gegenstand weg |
| `FIX_0161` | wichtig | mittel | Automatische Zaubersteigerung lueckenhaft |
| `FIX_0162` | alles | egal | Reisekarte: Ortssuche ohne Grenze |
| `FIX_0163` | alles | egal | Heldenzauber: Lesen hinter der Tabelle |
| `FIX_0164` | alles | egal | Ingerimms Schmiede: Schreiben hinter das Feld |
| `FIX_0165` | wichtig | mittel | DSA-1-Import: Gegenstaende gehen verloren |
| `FIX_0166` | wichtig | mittel | Nihilatio Gravitas ueber die AE hinaus |
| `FIX_0167` | alles | egal | Sonderkampf-Sequenzen: doppeltes Freigeben |
| `FIX_0168` | alles | egal | Kampfaufbau: leere Pruefung |
| `FIX_0169` | alles | egal | Gegenstand benutzen: fehlende Standardfaelle |
| `FIX_0170` | wichtig | mittel | Routenplanung ergibt falsche Route |
| `FIX_0171` | alles | egal | Heldensuche: ungenutzte fehlerhafte Funktion |
| `FIX_0172` | alles | egal | Intro: Speicher-Merker nicht gesetzt |
| `FIX_0173` | wichtig | mittel | Hexenspeichel heilt mehr, als die AE bezahlt |
| `FIX_0174` | wichtig | mittel | Unmut eines Gespraechspartners wird vergessen |
| `FIX_0175` | alles | egal | Zwergenskelett: falsches Pronomen |
| `FIX_0176` | alles | egal | Reisekarte: Grafikfehler beim Hochscrollen |
| `FIX_0177` | wichtig | mittel | Wegliste: eine Meldung erscheint nie |
| `FIX_0178` | alles | egal | Erschaffung: Grenzwerte uneinheitlich verglichen |
| `FIX_0179` | wichtig | mittel | Kein Hunger und Durst beim Rasten |
| `FIX_0180` | wichtig | mittel | Sieg ueber die Schwarzmagier wird nicht gemerkt |
| `FIX_0181` | alles | egal | Kampf: einige Klaenge fehlen |
| `FIX_0182` | wichtig | mittel | Orkfestung Ebene 1: Ereignisse zufaellig |
| `FIX_0183` | wichtig | mittel | Kampffeld: falsche Waende und Blockaden |
| `FIX_0184` | alles | egal | Strassenueberfall: Lesen hinter der Liste |
| `FIX_0185` | wichtig | mittel | Gegenstaende weitergeben: falscher Held geprueft |
| `FIX_0186` | wichtig | mittel | Stadtereignisse: Text fehlt, gleiche Antworten |
| `FIX_0187` | wichtig | mittel | "Sichern" im Kampf wirkt erst eine Runde spaeter |
| `FIX_0188` | wichtig | mittel | Zauberziel gegen Sondergegner falsch geprueft |
| `FIX_0189` | alles | egal | Sonderkampf-Animation laeuft falsch |
| `FIX_0190` | alles | egal | Abspann: eine Zeile fehlt mit Sprachausgabe |
| `FIX_0191` | wichtig | mittel | Lager: Wachen und Diebstahl fehlerhaft |
| `FIX_0192` | wichtig | kritisch | Bolzenfalle im Phex-Tempel toetet hintere Helden |
| `FIX_0193` | wichtig | mittel | Lager: Entscheidung zufaellig |
| `FIX_0194` | wichtig | mittel | Orkfestung Ebene 3: Schaden bei jedem Betreten |
| `FIX_0195` | alles | egal | Abblenden: Palette nur teilweise geloescht |
| `FIX_0196` | wichtig | mittel | Treffen mit Teilgruppen unterwegs unzuverlaessig |
| `FIX_0197` | wichtig | mittel | Spielstand laden: eine Einstellung geht verloren |
| `FIX_0198` | alles | egal | Ohne EMS: falsche Pixel im Bildschirmrahmen |
| `FIX_0199` | wichtig | kritisch | Unrathaufen im Orkgefaengnis: endlos Gegenstaende |
| `FIX_0200` | alles | egal | Automatische Zielwahl bevorzugt eine Seite |
| `FIX_0201` | alles | egal | Startausruestung: Lesen hinter dem Eintrag |
| `FIX_0202` | alles | egal | 3D-Objekte: eine Palettenfarbe zu viel |
| `FIX_0203` | wichtig | mittel | Flammenschluessel beliebig oft holbar |
| `FIX_0204` | wichtig | mittel | Hesinde-Statue: geschwaechter Held entgeht der Strafe |
| `FIX_0205` | alles | egal | Totenkopf-Ornament: "%s" statt Heldenname |
| `FIX_0206` | wichtig | kritisch | Absturz bei der Bengram-Verwechslung in Tjolmar |
| `FIX_0207` | wichtig | mittel | Gruppentrennung mit den falschen Helden |
| `FIX_0208` | wichtig | mittel | Automap ohne XMS/EMS kaputt, Notizreste |
| `FIX_0209` | alles | egal | Speicherfehler bei ueber 160 MB XMS |
| `FIX_0210` | alles | egal | Textdateien laden ohne Fehlerpruefung |
| `FIX_0211` | alles | egal | Bild laden mit zu kleinem Puffer (ungenutzt) |
| `FIX_0212` | wichtig | kritisch | Orkgefaengnis: beschlagnahmte Ausruestung fuer immer weg |
| `FIX_0213` | wichtig | mittel | Arax-Gift laehmt Gegner voellig |
| `FIX_0214` | wichtig | mittel | Tanzen fuer Geld nutzt Musizieren |
| `FIX_0215` | wichtig | mittel | Tollwut-Anfall: Beruhigen gelingt verkehrt herum |
| `FIX_0216` | wichtig | mittel | Lowangen, zweites Stadttor: Unsinnstext |
| `FIX_0217` | alles | egal | Bild-Entpacker im Speicher: Zufallswert |
| `FIX_0218` | alles | egal | Unratfund heisst immer "einen Dolch" |
| `FIX_0219` | alles | egal | Gruppenwechsel ueberspringt NSC-Gruppen nicht |
| `FIX_0220` | wichtig | mittel | Meditation mit wenig LE kostet AE |
| `FIX_0221` | alles | egal | Finale: Ueberblendung von Schwarz (gewollt) |
| `FIX_0222` | alles | egal | Steinschlag: danach evtl. falsche Wegtexte |
| `FIX_0224` | alles | egal | Kampffiguren am Rand ungleich abgeschnitten |
| `FIX_0225` | alles | egal | Absturzbericht liest ueber den Speicherblock |
| `FIX_0226` | alles | egal | Wasserfall ueber Weg 25 mit falschen Texten |
| `FIX_0227` | alles | egal | Dialogskript: Vergleich mit leerem Feld |
| `FIX_0230` | alles | egal | CD-Pruefung bei leerer PATH.CFG |
| `FIX_0231` | alles | egal | CD-Pruefung bricht ohne Aufraeumen ab |
| `FIX_0232` | alles | egal | Speichern: Existenzpruefung mit > statt >= |
| `FIX_0233` | wichtig | mittel | Kriegerhuene: Abschiedstext erscheint nie |
| `FIX_0300` | alles | egal | DIGPAK-Klangtreiber: Stapelfehler (ungenutzt) |
| `FIX_0301` | alles | egal | Zeichenart "Oder" kopiert falsch (ungenutzt) |
| `FIX_0302` | alles | egal | Rueckwaertskopieren versetzt (ungenutzt) |
| `FIX_0304` | alles | egal | XMS-Vorgabe ohne Ruecksprung |
| `FIX_0305` | alles | egal | XMS-Kopie liest Einsprung aus falschem Segment |
| `FIX_0306` | alles | egal | Flat-Modus ohne Fehlerpruefung |
| `FIX_0307` | alles | egal | Kopierroutine verliert ungerades Byte |
| `FIX_0350` | alles | egal | Ohne EMS vereinzelt falsche Bodenpixel |
| `FIX_0351` | alles | egal | Tiefensortierung bei 129 Flaechen |
| `FIX_0352` | wichtig | kritisch | Lagerhaus: Waren kehren zurueck, Lagerkosten explodieren |
| `FIX_0353` | wichtig | mittel | Holzfaellerlager: Abbrechen legt ein Stueck dazu |
| `FIX_0354` | wichtig | mittel | Phexhelm und Phexschild vervielfachen |
| `FIX_0355` | alles | egal | Gefangennahme: Ringe und Amulette vervielfacht |
| `FIX_0356` | wichtig | mittel | Geld verschwindet beim Vereinen von Gruppen |
| `FIX_0357` | wichtig | mittel | Asthenilschwert per ESC behalten |
| `FIX_0358` | wichtig | mittel | Schluesselbund und Rezeptbuch verlieren im Lagerhaus den Inhalt |
| `FIX_0359` | wichtig | mittel | Geweihte Waffen verlieren "magisch" |
| `FIX_0360` | wichtig | mittel | Kaputte und heile Gegenstaende im selben Stapel |
| `FIX_0361` | wichtig | kritisch | Volles Inventar: Asthenilring oder magischer Wasserschlauch weg |
| `FIX_0362` | wichtig | kritisch | Schluessel verschwindet beim Abnehmen vom Bund |
| `FIX_0363` | wichtig | kritisch | Leere Gruppe haengt das Spiel auf |
| `FIX_0364` | wichtig | kritisch | Horriphobus: Kampf endet nie |
| `FIX_0365` | alles | egal | MR der NSC falsch (Korima, Thurazz) |
| `FIX_0366` | alles | egal | Tierkunde an den Spinnennetzen invertiert |
| `FIX_0367` | wichtig | mittel | Hesinde-Wunder 8 und 9 bewirken nichts |
| `FIX_0368` | wichtig | mittel | Hesinde-Statue als AP-Quelle |
| `FIX_0369` | wichtig | kritisch | Kraeuterwirkungen verfallen nicht, Werte kippen |
| `FIX_0370` | wichtig | mittel | Kraeutersuche ueber "Alte Einstellungen" ohne Malus |
| `FIX_0371` | wichtig | mittel | Stufenanstieg: Kampftalent steigert immer die Attacke |
| `FIX_0372` | alles | egal | Nichtzauberer bekommen Zaubersteigerungen angeboten |
| `FIX_0373` | wichtig | mittel | Weitere Importfehler (toter Held, Kampfwerte) |
| `FIX_0374` | wichtig | mittel | Markthalle Lowangen: Ereignis bleibt haengen |
| `FIX_0375` | wichtig | kritisch | Agdan-Quest: kein Lowangen-Schluessel |
| `FIX_0376` | alles | egal | Textfehler "Rechstschritt" |
| `FIX_0377` | wichtig | mittel | Visibili wirkt nur auf den Zaubernden |
| `FIX_0378` | wichtig | mittel | Gardianum schuetzt nicht |

Anzahl: 260.

