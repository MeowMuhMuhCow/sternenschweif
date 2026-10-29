/* s_fix.h - Fehlerbehebungen per Schalter (Projekt Sternenschweif, erzeugt von tools/mkfix.py - nicht von Hand aendern)
 *
 * Stufenschalter (BCC -D..., Vorgabe: Original):
 *   FIX_BUGS 0      Original, bytegleich mit SCHWEIF.EXE C1.02 (Standard)
 *   FIX_BUGS 1      "wichtig": alle Fehler mit Auswirkung kritisch oder mittel
 *   FIX_BUGS 2      "alles": dazu alle Fehler mit Auswirkung egal
 *   FIX_EXPLOITS 1  auch Fehler beheben, die dem Spieler nuetzen (Vorgabe 1, sobald FIX_BUGS > 0); 0 = drinlassen
 * Einzelschalter: jeder FIX_nnnn laesst sich uebersteuern, z. B. -DFIX_0020=0 (aus) oder -DFIX_0050=1 (an).
 * Keine Behebung aendert Strukturen, das Spielstandformat oder das Datensegment.
 *
 * Schalter   Stufe    Kategorie Exploit umgesetzt  Titel
 * ---------  -------  --------- ------- ---------  -----
 * FIX_0001   wichtig  mittel    -       ja         Verzauberte Gegner patzen im Angriff ohne Folgen
 * FIX_0002   wichtig  mittel    -       ja         Automatisches Steigern ignoriert die Grenze je Talent
 * FIX_0003   alles    egal      -       nein       Klickflaechen-Pruefung tauscht falsche Werte
 * FIX_0004   wichtig  mittel    -       ja         Naechtlicher Dieb bestiehlt nur den ersten Helden
 * FIX_0005   wichtig  mittel    -       ja         Wegereignisse loesen an zufaelligen Stellen aus
 * FIX_0006   wichtig  kritisch  -       ja         Orks werfen abgenommene Ausruestung weg (wie Nr. 212)
 * FIX_0007   alles    egal      -       ja         Karte aufdecken: Grenzpruefung doppelt
 * FIX_0008   wichtig  mittel    -       ja         Zufallsbegegnungen in einer Gelaendeart falsch
 * FIX_0009   alles    egal      -       ja         Bild-Entpacker liefert Zufallswert zurueck
 * FIX_0010   alles    egal      -       ja         Heldenvorauswahl steht manchmal auf leerem Platz
 * FIX_0011   alles    egal      -       ja         Marktstand mit unbekanntem Feld
 * FIX_0012   wichtig  mittel    -       ja         Orkgefaengnis: bei Alarm zwei Kaempfe hintereinander
 * FIX_0013   wichtig  mittel    -       ja         Hoehlenspinnen kommen bis zur Nestzerstoerung wieder
 * FIX_0014   wichtig  kritisch  -       ja         Absturz beim Schluesselbund mit 18 oder mehr Schluesseln
 * FIX_0015   alles    egal      -       ja         Schluesselbund/Rezeptbuch: falsche "schon drin"-Meldung
 * FIX_0016   alles    egal      -       ja         Einzelne Pixel in falscher Farbe
 * FIX_0017   alles    egal      -       ja         Speicherleck beim Verlassen der 3D-Ansicht
 * FIX_0018   wichtig  mittel    -       ja         Flucht vom Randfeld wird manchmal verweigert
 * FIX_0019   alles    egal      -       nein       NSC aufnehmen: Hilfsvariable doppelt benutzt
 * FIX_0020   wichtig  kritisch  -       ja         Fahrender Haendler: Ware bezahlt, aber nicht erhalten
 * FIX_0021   alles    egal      -       ja         Haendler-Warenliste ueberlappt bei riesigem Sortiment
 * FIX_0022   alles    egal      -       ja         Automap findet sehr weite Wege nicht
 * FIX_0023   alles    egal      -       nein       Versiegelte Schloesser: Erleichterung wirkungslos
 * FIX_0024   alles    egal      -       ja         Warenauswahl: Pfeil links springt an den Anfang
 * FIX_0025   wichtig  kritisch  ja      ja         Geld ins Minus: Millionen durch Gruppenteilung
 * FIX_0026   alles    egal      -       ja         Gruppe entfernen aendert fremdes Byte
 * FIX_0027   alles    egal      -       ja         Texteingabe: fehlende Endnull
 * FIX_0028   alles    egal      -       nein       Texteingabe: Rueckgabewert fehlt
 * FIX_0029   alles    egal      -       ja         Kampfbewegung: Animationsfolge ohne Laengengrenze
 * (30)       -        egal      -       kein Fehler Reiseereignis Weg 136: gewollter Durchlauf [gewollt: bewusstes Durchfallen (case 1 -> case 2)]
 * FIX_0031   alles    egal      -       ja         Ortswechsel ueberschreibt den gemerkten alten Ort
 * FIX_0032   wichtig  mittel    -       teilweise  Absturz im versteckten Entwicklerbildschirm
 * FIX_0033   alles    egal      -       ja         Kalender: falscher Tagesueberlauf
 * FIX_0034   alles    egal      -       ja         Kalender: Tick-Ueberlauf verliert Restzeit
 * FIX_0035   alles    egal      -       nein       Kalender: Jahreswechsel ergibt Monat 13
 * FIX_0036   alles    egal      -       ja         Reisekarte: Richtungswahl ohne Rueckgabewert
 * FIX_0037   wichtig  mittel    -       ja         Absturz-Tagebucheintrag auch ohne Absturz
 * FIX_0038   alles    egal      -       ja         Kampffeld: Belegt-Pruefung ohne Wirkung
 * FIX_0039   wichtig  mittel    -       ja         Haenger bei voellig zugestelltem Kampffeld
 * (40)       -        egal      -       kein Fehler Parade Held gegen Held (kein Fehler) [kein Fehler: hero_mr_target liefert den Verteidiger]
 * FIX_0041   alles    egal      -       ja         Paradepatzer nennt den falschen Helden
 * FIX_0042   alles    egal      -       ja         Patzerfolge ohne Schaden geprueft
 * FIX_0043   wichtig  mittel    -       ja         Steinbock frisst die letzte Suessigkeit umsonst
 * FIX_0044   alles    egal      -       ja         Zahleneingabe zeigt falsche erste Ziffer
 * FIX_0045   wichtig  mittel    ja      ja         Kraeuterstelle an Weg 165 beliebig oft ernten
 * FIX_0046   alles    egal      -       ja         Reisekarte: Wegsuche liest hinter der Tabelle
 * FIX_0047   alles    egal      -       ja         Reisekarte: Wegzeichnung im Randfall falsch
 * FIX_0048   alles    egal      -       ja         Brenne toter Stoff ohne Fackel: Meldung zufaellig
 * FIX_0049   alles    egal      -       ja         Sonderkampf: Lesen vor dem Nulltest
 * FIX_0050   alles    egal      -       ja         Phex-Tempel: nur eine von vier Flaechen animiert
 * FIX_0051   wichtig  mittel    -       ja         Herbergsdieb stiehlt nie Proviant, sondern Geld
 * FIX_0052   alles    egal      -       ja         Gegenstandssuche ueber alle Plaetze kaputt
 * FIX_0053   alles    egal      -       ja         Portraetleiste einer Teilgruppe zu breit/schmal
 * FIX_0054   alles    egal      -       ja         Gegnerzauber bei leerer Zauberliste
 * FIX_0055   alles    egal      -       ja         Faehre Hilvalla: zu wenig Geld zeigt Begruessung erneut
 * FIX_0056   alles    egal      -       ja         Tagebuch drucken: Ueberlauf bei langen Zeilen
 * FIX_0057   alles    egal      -       ja         Tagebuch-Heldennamen: Lesen hinter dem Feld
 * FIX_0058   alles    egal      -       ja         Ortssymbole: unvollstaendige Aenderungspruefung
 * FIX_0059   wichtig  mittel    -       ja         NSC verlaesst die Gruppe ohne Abschiedstext
 * FIX_0060   alles    egal      -       ja         Teleport mit unbekannter Herkunft
 * FIX_0061   alles    egal      -       ja         Gruppe teilen bei sechs Gruppen
 * FIX_0062   alles    egal      -       ja         Portraet ziehen liest vor der Tabelle
 * FIX_0063   wichtig  mittel    ja      ja         Wirselkraut an Weg 201 beliebig oft ernten
 * FIX_0064   alles    egal      -       nein       Eine Gegner-Animation wird nie gezeigt
 * FIX_0065   alles    egal      -       ja         Genitiv-s bei Namen mit Sonderzeichen falsch
 * FIX_0066   alles    egal      -       nein       DSA-1-Import: doppelte Pruefung
 * FIX_0067   alles    egal      -       ja         Dialogskript: Zeiger nicht genullt
 * FIX_0068   alles    egal      -       teilweise  Klangtreiber: kleines Speicherleck
 * FIX_0069   alles    egal      -       nein       Kartenraetsel: toter Programmcode
 * (70)       -        egal      -       kein Fehler Heiler nennt Krankheit vor der Zahlung (gewollt) [gewollt: Diagnose im Fragetext genannt]
 * FIX_0071   wichtig  mittel    -       ja         Lagerhaus beim ersten Besuch manchmal gesperrt
 * FIX_0072   wichtig  mittel    ja      ja         Boron-Wunder: Totenerweckung viel zu haeufig
 * FIX_0073   alles    egal      -       ja         Kampfanimation: ueber 40 Animationen gleichzeitig
 * FIX_0074   alles    egal      -       teilweise  Bildklasse: Randfehler ohne Aufrufweg
 * FIX_0075   alles    egal      -       ja         Meldung "versteckte Gegner" zaehlt manchmal falsch
 * FIX_0076   alles    egal      -       ja         Sumpfrantzendokument gegen Gegner ohne Meldung
 * FIX_0077   wichtig  mittel    -       ja         Vorratslager: Debugtext als Mengenfrage
 * FIX_0078   wichtig  mittel    -       ja         Lagerhaus: Verderb nach einem Jahr verkehrt herum
 * FIX_0079   wichtig  mittel    -       ja         Kampf: Figur laeuft selten in Zufallsrichtung
 * FIX_0080   alles    egal      -       ja         Reisekarte: Position im Randfall falsch
 * FIX_0081   alles    egal      -       ja         Zufaelliger Gegenstandsverlust fehlerhaft
 * FIX_0082   alles    egal      -       ja         Gegenstand an jeden Helden: Restmenge falsch
 * FIX_0083   alles    egal      -       ja         Gruppenverwaltung schreibt hinter das Feld
 * FIX_0084   wichtig  kritisch  -       ja         Laden: Ware bezahlt, aber nicht erhalten
 * FIX_0085   alles    egal      -       ja         Laden-Warenliste ueberlappt bei riesigem Sortiment
 * FIX_0086   alles    egal      -       ja         CD-Zwischenspeicher bei Lesefehlern kaputt
 * FIX_0087   alles    egal      -       ja         Gegenstaende scheinen durch Waende
 * FIX_0088   alles    egal      -       ja         Wegetexte: haengende Zeiger
 * FIX_0089   alles    egal      -       teilweise  Kobold Mumpitz: Auswahl liest ausserhalb
 * FIX_0090   alles    egal      -       teilweise  Kampf-Figurenliste: mehrere Randfehler
 * FIX_0091   alles    egal      -       teilweise  Dateizugriff: Handle-Leck und spaete Fehlerpruefung
 * FIX_0092   alles    egal      -       ja         Hintergrundanimation zeigt evtl. falsches Teilbild
 * FIX_0093   wichtig  kritisch  -       ja         Absturz beim Bauern mit dem gebrochenen Rad
 * FIX_0094   wichtig  mittel    -       teilweise  Laden: Gepaeckwarnung fehlt oder falsch
 * FIX_0095   alles    egal      -       ja         Leeres Zunderkaestchen: doppelte oder alte Meldung
 * (96)       -        egal      -       kein Fehler Kampfbeginn: Hilfsvariable doppelt benutzt [kein Fehler: Dummy-Ziel (+0x1a1 waere old_dlg_mode)]
 * FIX_0097   alles    egal      -       ja         Kompassrose verschwindet im Randfall
 * FIX_0098   wichtig  mittel    -       ja         Schlafender NSC steht wach im Kampf
 * FIX_0099   alles    egal      -       ja         Dialogfenster: Randfaelle ohne Wirkung
 * FIX_0100   alles    egal      -       nein       Schenke: Andrang nach Mitternacht (unklar)
 * FIX_0101   alles    egal      -       ja         Kraeuterstelle an Weg 41: Zaehler ohne Grenze
 * FIX_0102   wichtig  mittel    -       ja         Gewoelbe des Namenlosen: Ereignisse zufaellig
 * FIX_0103   alles    egal      -       ja         Tagebuchsuche: Esc sucht statt abzubrechen
 * FIX_0104   alles    egal      -       ja         Altsvellt-Sumpf: alte Ebene ueberschrieben
 * FIX_0105   alles    egal      -       ja         Spinnennetze: Schreiben nach Adresse 0
 * FIX_0106   wichtig  mittel    -       ja         Sonderkampf: Flaechenangriff trifft niemanden
 * FIX_0107   alles    egal      -       teilweise  Heldenerschaffung: uninitialisierte Werte
 * FIX_0108   alles    egal      -       ja         Auswahlliste ohne Grenzpruefung
 * FIX_0109   wichtig  mittel    -       teilweise  Texteingabefeld: Grafikfehler und Zeichenmuell
 * FIX_0110   wichtig  mittel    -       ja         Stabzauber prueft eine Eigenschaft doppelt
 * FIX_0111   wichtig  mittel    -       teilweise  Gegnerischer Heilzauber verletzt Schwerverwundete
 * FIX_0112   alles    egal      -       ja         Kriegerhuene: Wegetexte unnoetig neu geladen
 * FIX_0113   alles    egal      -       nein       Gegnerpatzer gegen bewusstlose Helden entfallen
 * FIX_0114   wichtig  mittel    -       ja         Lorenfahrt: Abspringen gefahrlos, aber ohne AP
 * FIX_0115   alles    egal      -       ja         Lore: Lesen vor der Gueltigkeitspruefung
 * FIX_0116   alles    egal      -       ja         Auto-Erschaffung: Akademiezauber doppelt bedacht
 * FIX_0117   wichtig  mittel    -       nein       Heilkunde Wunden: Probe doppelt gewuerfelt
 * FIX_0118   wichtig  mittel    ja      ja         Wasserfall-Spende: Vollheilung fast umsonst
 * FIX_0119   wichtig  mittel    -       ja         Warzennashorn-Schaden und Kraeuterprobe Weg 36 falsch
 * FIX_0120   wichtig  mittel    -       ja         Automatischer Kampf: Helden zaubern auf falsches Ziel
 * FIX_0121   wichtig  mittel    -       ja         Sehr lange Reiseetappen dauern zu kurz
 * FIX_0122   wichtig  mittel    -       ja         Ingerimms Schmiede: Meldung fehlt, "und" falsch
 * FIX_0123   wichtig  kritisch  ja      ja         Unendlich Geld aus Truhen
 * FIX_0124   wichtig  mittel    -       ja         Reisetempo im Randfall falsch
 * FIX_0125   alles    egal      -       ja         Zwischensequenz: Ueberblendung zufaellig
 * FIX_0126   wichtig  mittel    -       ja         Grosser AP-Bonus im Gespraech wird nie vergeben
 * FIX_0127   wichtig  mittel    -       ja         Klangeinstellung: Port/IRQ-Eingabe verworfen
 * FIX_0128   wichtig  mittel    -       ja         Fahrender Haendler: falsche Menge beim Zuruecklegen
 * FIX_0129   wichtig  mittel    -       ja         Speicherfehler loescht die falsche Datei
 * FIX_0130   wichtig  mittel    -       ja         Gulmondblatt: Spieltext durch Heldennamen ersetzt
 * FIX_0131   alles    egal      -       ja         Starker Zaubertrank nennt Lebenspunkte
 * FIX_0132   alles    egal      -       ja         Ortsszene: Musikwahl zufaellig
 * FIX_0133   alles    egal      -       ja         Binge: Position doppelt gesetzt
 * FIX_0134   wichtig  mittel    -       teilweise  Schwarzmagier-Szene: Abschlusstext und Ton fehlen
 * FIX_0135   wichtig  mittel    -       ja         Gespraech mit Gegenstandswahl laesst sich nicht abbrechen
 * FIX_0136   alles    egal      -       ja         Fenster springt beim Verschieben
 * (137)      -        egal      -       kein Fehler Eolan-Ereignis (kein Fehler) [kein Fehler: Zustand wird im Dialog gesetzt]
 * FIX_0138   wichtig  mittel    -       ja         Shurinknollen-Gift: Erholung extrem langsam
 * FIX_0139   alles    egal      -       ja         Kellertuer: Wandbild bei nicht quadratischer Karte
 * FIX_0140   wichtig  mittel    -       ja         Pelzwand: Abzug beim falschen Gegenstand
 * FIX_0141   alles    egal      -       ja         Animation: haengender Zeiger bei fehlender Datei
 * FIX_0142   alles    egal      -       ja         Lagerplatz: Suche liest hinter der Tabelle
 * FIX_0143   wichtig  mittel    -       ja         Zwergenbinge: Portal versetzt andere Teilgruppe
 * FIX_0144   wichtig  mittel    -       teilweise  Tagebuch: Seitenzaehlung falsch, Ueberlauf moeglich
 * FIX_0145   wichtig  mittel    -       ja         Erschaffung: Eigenschaften verschieben falsch geprueft
 * FIX_0146   wichtig  mittel    -       ja         Spendenschrein: falscher Text, Zufallswert
 * FIX_0147   alles    egal      -       ja         Heldenaktion mit unbekannter Zielart
 * FIX_0148   alles    egal      -       ja         Nichtzauberer koennen das Zaubermenue oeffnen
 * FIX_0149   wichtig  mittel    -       ja         Ankunft in Lowangen: erster Textteil fehlt
 * FIX_0150   alles    egal      -       ja         Faehre: Zufallswert an unbekanntem Ort
 * FIX_0151   wichtig  mittel    -       ja         Zwergengrab: Frevel ohne Kriegsaxt
 * FIX_0152   alles    egal      -       ja         Alchimie/Schaetzen: kleine Randfehler
 * FIX_0153   wichtig  mittel    -       ja         Reise bricht unvermittelt ab
 * FIX_0154   wichtig  mittel    -       ja         Blaue Keuche steckt mit Dumpfschaedel an
 * FIX_0155   wichtig  mittel    -       ja         Herberge: Abbruch loescht die alte Zimmerbuchung
 * (156)      -        egal      -       kein Fehler Hafen-Feilschen (kein Fehler) [gewollt: -1 passt zum anderen Menue]
 * FIX_0157   wichtig  mittel    -       teilweise  Orkfestung Ebene 3: ausgewuerfelte Gegner fehlen
 * FIX_0158   alles    egal      -       teilweise  Schenke: leerer Text, merkwuerdiger Rundenzaehler
 * FIX_0159   wichtig  mittel    ja      ja         Armatrutz eine Stufe hoeher als die AE erlaubt
 * FIX_0160   wichtig  mittel    -       ja         Zwergenbinge: Gruppe falsch, fremder Gegenstand weg
 * FIX_0161   wichtig  mittel    -       ja         Automatische Zaubersteigerung lueckenhaft
 * FIX_0162   alles    egal      -       teilweise  Reisekarte: Ortssuche ohne Grenze
 * FIX_0163   alles    egal      -       teilweise  Heldenzauber: Lesen hinter der Tabelle
 * FIX_0164   alles    egal      -       ja         Ingerimms Schmiede: Schreiben hinter das Feld
 * FIX_0165   wichtig  mittel    -       ja         DSA-1-Import: Gegenstaende gehen verloren
 * FIX_0166   wichtig  mittel    ja      ja         Nihilatio Gravitas ueber die AE hinaus
 * FIX_0167   alles    egal      -       ja         Sonderkampf-Sequenzen: doppeltes Freigeben
 * FIX_0168   alles    egal      -       ja         Kampfaufbau: leere Pruefung
 * FIX_0169   alles    egal      -       ja         Gegenstand benutzen: fehlende Standardfaelle
 * FIX_0170   wichtig  mittel    -       ja         Routenplanung ergibt falsche Route
 * FIX_0171   alles    egal      -       ja         Heldensuche: ungenutzte fehlerhafte Funktion
 * FIX_0172   alles    egal      -       ja         Intro: Speicher-Merker nicht gesetzt
 * FIX_0173   wichtig  mittel    ja      ja         Hexenspeichel heilt mehr, als die AE bezahlt
 * FIX_0174   wichtig  mittel    -       ja         Unmut eines Gespraechspartners wird vergessen
 * FIX_0175   alles    egal      -       nein       Zwergenskelett: falsches Pronomen
 * FIX_0176   alles    egal      -       ja         Reisekarte: Grafikfehler beim Hochscrollen
 * FIX_0177   wichtig  mittel    -       nein       Wegliste: eine Meldung erscheint nie
 * FIX_0178   alles    egal      -       ja         Erschaffung: Grenzwerte uneinheitlich verglichen
 * FIX_0179   wichtig  mittel    -       ja         Kein Hunger und Durst beim Rasten
 * FIX_0180   wichtig  mittel    -       teilweise  Sieg ueber die Schwarzmagier wird nicht gemerkt
 * FIX_0181   alles    egal      -       ja         Kampf: einige Klaenge fehlen
 * FIX_0182   wichtig  mittel    -       ja         Orkfestung Ebene 1: Ereignisse zufaellig
 * FIX_0183   wichtig  mittel    -       teilweise  Kampffeld: falsche Waende und Blockaden
 * FIX_0184   alles    egal      -       ja         Strassenueberfall: Lesen hinter der Liste
 * FIX_0185   wichtig  mittel    -       teilweise  Gegenstaende weitergeben: falscher Held geprueft
 * FIX_0186   wichtig  mittel    -       teilweise  Stadtereignisse: Text fehlt, gleiche Antworten
 * FIX_0187   wichtig  mittel    -       ja         "Sichern" im Kampf wirkt erst eine Runde spaeter
 * FIX_0188   wichtig  mittel    -       ja         Zauberziel gegen Sondergegner falsch geprueft
 * FIX_0189   alles    egal      -       teilweise  Sonderkampf-Animation laeuft falsch
 * FIX_0190   alles    egal      -       teilweise  Abspann: eine Zeile fehlt mit Sprachausgabe
 * FIX_0191   wichtig  mittel    -       teilweise  Lager: Wachen und Diebstahl fehlerhaft
 * FIX_0192   wichtig  kritisch  -       ja         Bolzenfalle im Phex-Tempel toetet hintere Helden
 * FIX_0193   wichtig  mittel    -       ja         Lager: Entscheidung zufaellig
 * FIX_0194   wichtig  mittel    -       teilweise  Orkfestung Ebene 3: Schaden bei jedem Betreten
 * FIX_0195   alles    egal      -       ja         Abblenden: Palette nur teilweise geloescht
 * FIX_0196   wichtig  mittel    -       teilweise  Treffen mit Teilgruppen unterwegs unzuverlaessig
 * FIX_0197   wichtig  mittel    -       ja         Spielstand laden: eine Einstellung geht verloren
 * FIX_0198   alles    egal      -       ja         Ohne EMS: falsche Pixel im Bildschirmrahmen
 * FIX_0199   wichtig  kritisch  ja      ja         Unrathaufen im Orkgefaengnis: endlos Gegenstaende
 * FIX_0200   alles    egal      -       teilweise  Automatische Zielwahl bevorzugt eine Seite
 * FIX_0201   alles    egal      -       ja         Startausruestung: Lesen hinter dem Eintrag
 * FIX_0202   alles    egal      -       teilweise  3D-Objekte: eine Palettenfarbe zu viel
 * FIX_0203   wichtig  mittel    ja      ja         Flammenschluessel beliebig oft holbar
 * FIX_0204   wichtig  mittel    -       ja         Hesinde-Statue: geschwaechter Held entgeht der Strafe
 * FIX_0205   alles    egal      -       nein       Totenkopf-Ornament: "%s" statt Heldenname
 * FIX_0206   wichtig  kritisch  -       ja         Absturz bei der Bengram-Verwechslung in Tjolmar
 * FIX_0207   wichtig  mittel    -       ja         Gruppentrennung mit den falschen Helden
 * FIX_0208   wichtig  mittel    -       teilweise  Automap ohne XMS/EMS kaputt, Notizreste
 * FIX_0209   alles    egal      -       ja         Speicherfehler bei ueber 160 MB XMS
 * FIX_0210   alles    egal      -       nein       Textdateien laden ohne Fehlerpruefung
 * FIX_0211   alles    egal      -       ja         Bild laden mit zu kleinem Puffer (ungenutzt)
 * FIX_0212   wichtig  kritisch  -       ja         Orkgefaengnis: beschlagnahmte Ausruestung fuer immer weg
 * FIX_0213   wichtig  mittel    ja      ja         Arax-Gift laehmt Gegner voellig
 * FIX_0214   wichtig  mittel    ja      ja         Tanzen fuer Geld nutzt Musizieren
 * FIX_0215   wichtig  mittel    -       ja         Tollwut-Anfall: Beruhigen gelingt verkehrt herum
 * FIX_0216   wichtig  mittel    -       ja         Lowangen, zweites Stadttor: Unsinnstext
 * FIX_0217   alles    egal      -       nein       Bild-Entpacker im Speicher: Zufallswert
 * FIX_0218   alles    egal      -       ja         Unratfund heisst immer "einen Dolch"
 * FIX_0219   alles    egal      -       ja         Gruppenwechsel ueberspringt NSC-Gruppen nicht
 * FIX_0220   wichtig  mittel    -       ja         Meditation mit wenig LE kostet AE
 * FIX_0221   alles    egal      -       nein       Finale: Ueberblendung von Schwarz (gewollt)
 * FIX_0222   alles    egal      -       ja         Steinschlag: danach evtl. falsche Wegtexte
 * (223)      -        egal      -       kein Fehler Orkschuetzen an Weg 250 (kein Fehler) [kein Fehler: get_next_hero liefert nie 0]
 * FIX_0224   alles    egal      -       ja         Kampffiguren am Rand ungleich abgeschnitten
 * FIX_0225   alles    egal      -       nein       Absturzbericht liest ueber den Speicherblock
 * FIX_0226   alles    egal      -       nein       Wasserfall ueber Weg 25 mit falschen Texten
 * FIX_0227   alles    egal      -       ja         Dialogskript: Vergleich mit leerem Feld
 * (228)      -        egal      -       kein Fehler Handbuchabfrage in der CD-Fassung ohne Wirkung [gewollt: Schutz ueber cd_check, Rest der Disketten-Fassung]
 * (229)      -        egal      -       kein Fehler Zeitgeber prueft denselben Wert doppelt [ohne Wirkung: doppelter Test, Absicht nicht belegbar]
 * FIX_0230   alles    egal      -       ja         CD-Pruefung bei leerer PATH.CFG
 * FIX_0231   alles    egal      -       ja         CD-Pruefung bricht ohne Aufraeumen ab
 * FIX_0232   alles    egal      -       ja         Speichern: Existenzpruefung mit > statt >=
 * FIX_0233   wichtig  mittel    -       nein       Kriegerhuene: Abschiedstext erscheint nie
 * FIX_0300   alles    egal      -       ja         DIGPAK-Klangtreiber: Stapelfehler (ungenutzt)
 * FIX_0301   alles    egal      -       ja         Zeichenart "Oder" kopiert falsch (ungenutzt)
 * FIX_0302   alles    egal      -       nein       Rueckwaertskopieren versetzt (ungenutzt)
 * (303)      -        egal      -       kein Fehler Debug-Waechter zeigt immer Rot (toter Code) [toter Debug-Code (Waechter nie eingehaengt)]
 * FIX_0304   alles    egal      -       ja         XMS-Vorgabe ohne Ruecksprung
 * FIX_0305   alles    egal      -       nein       XMS-Kopie liest Einsprung aus falschem Segment
 * FIX_0306   alles    egal      -       nein       Flat-Modus ohne Fehlerpruefung
 * FIX_0307   alles    egal      -       ja         Kopierroutine verliert ungerades Byte
 * FIX_0350   alles    egal      -       nein       Ohne EMS vereinzelt falsche Bodenpixel
 * FIX_0351   alles    egal      -       ja         Tiefensortierung bei 129 Flaechen
 * FIX_0352   wichtig  kritisch  -       teilweise  Lagerhaus: Waren kehren zurueck, Lagerkosten explodieren
 * FIX_0353   wichtig  mittel    ja      ja         Holzfaellerlager: Abbrechen legt ein Stueck dazu
 * FIX_0354   wichtig  mittel    ja      ja         Phexhelm und Phexschild vervielfachen
 * FIX_0355   alles    egal      -       nein       Gefangennahme: Ringe und Amulette vervielfacht
 * FIX_0356   wichtig  mittel    -       teilweise  Geld verschwindet beim Vereinen von Gruppen
 * FIX_0357   wichtig  mittel    ja      ja         Asthenilschwert per ESC behalten
 * FIX_0358   wichtig  mittel    ja      nein       Schluesselbund und Rezeptbuch verlieren im Lagerhaus den Inhalt
 * FIX_0359   wichtig  mittel    -       nein       Geweihte Waffen verlieren "magisch"
 * FIX_0360   wichtig  mittel    ja      nein       Kaputte und heile Gegenstaende im selben Stapel
 * FIX_0361   wichtig  kritisch  -       ja         Volles Inventar: Asthenilring oder magischer Wasserschlauch weg
 * FIX_0362   wichtig  kritisch  -       ja         Schluessel verschwindet beim Abnehmen vom Bund
 * FIX_0363   wichtig  kritisch  -       ja         Leere Gruppe haengt das Spiel auf
 * FIX_0364   wichtig  kritisch  -       ja         Horriphobus: Kampf endet nie
 * FIX_0365   alles    egal      -       nein       MR der NSC falsch (Korima, Thurazz)
 * FIX_0366   alles    egal      -       ja         Tierkunde an den Spinnennetzen invertiert
 * FIX_0367   wichtig  mittel    -       nein       Hesinde-Wunder 8 und 9 bewirken nichts
 * FIX_0368   wichtig  mittel    ja      ja         Hesinde-Statue als AP-Quelle
 * FIX_0369   wichtig  kritisch  ja      ja         Kraeuterwirkungen verfallen nicht, Werte kippen
 * FIX_0370   wichtig  mittel    ja      ja         Kraeutersuche ueber "Alte Einstellungen" ohne Malus
 * FIX_0371   wichtig  mittel    -       nein       Stufenanstieg: Kampftalent steigert immer die Attacke
 * FIX_0372   alles    egal      -       ja         Nichtzauberer bekommen Zaubersteigerungen angeboten
 * FIX_0373   wichtig  mittel    -       nein       Weitere Importfehler (toter Held, Kampfwerte)
 * FIX_0374   wichtig  mittel    ja      ja         Markthalle Lowangen: Ereignis bleibt haengen
 * FIX_0375   wichtig  kritisch  -       ja         Agdan-Quest: kein Lowangen-Schluessel
 * FIX_0376   alles    egal      -       nein       Textfehler "Rechstschritt"
 */
#ifndef S_FIX_H
#define S_FIX_H

#ifndef FIX_BUGS
#define FIX_BUGS 0
#endif
#ifndef FIX_EXPLOITS
#if FIX_BUGS > 0
#define FIX_EXPLOITS 1
#else
#define FIX_EXPLOITS 0
#endif
#endif

/* Vorgaben je Stufe (X = Exploit: nur mit FIX_EXPLOITS) */
#define FIX_STUFE_W  (FIX_BUGS >= 1)
#define FIX_STUFE_A  (FIX_BUGS >= 2)
#define FIX_STUFE_WX (FIX_BUGS >= 1 && FIX_EXPLOITS)
#define FIX_STUFE_AX (FIX_BUGS >= 2 && FIX_EXPLOITS)

#ifndef FIX_0001
#define FIX_0001 FIX_STUFE_W
#endif
#ifndef FIX_0002
#define FIX_0002 FIX_STUFE_W
#endif
#ifndef FIX_0003
#define FIX_0003 FIX_STUFE_A
#endif
#ifndef FIX_0004
#define FIX_0004 FIX_STUFE_W
#endif
#ifndef FIX_0005
#define FIX_0005 FIX_STUFE_W
#endif
#ifndef FIX_0006
#define FIX_0006 FIX_0212	/* dieselbe Stelle */
#endif
#ifndef FIX_0007
#define FIX_0007 FIX_STUFE_A
#endif
#ifndef FIX_0008
#define FIX_0008 FIX_STUFE_W
#endif
#ifndef FIX_0009
#define FIX_0009 FIX_STUFE_A
#endif
#ifndef FIX_0010
#define FIX_0010 FIX_STUFE_A
#endif
#ifndef FIX_0011
#define FIX_0011 FIX_STUFE_A
#endif
#ifndef FIX_0012
#define FIX_0012 FIX_STUFE_W
#endif
#ifndef FIX_0013
#define FIX_0013 FIX_STUFE_W
#endif
#ifndef FIX_0014
#define FIX_0014 FIX_STUFE_W
#endif
#ifndef FIX_0015
#define FIX_0015 FIX_STUFE_A
#endif
#ifndef FIX_0016
#define FIX_0016 FIX_STUFE_A
#endif
#ifndef FIX_0017
#define FIX_0017 FIX_STUFE_A
#endif
#ifndef FIX_0018
#define FIX_0018 FIX_STUFE_W
#endif
#ifndef FIX_0019
#define FIX_0019 FIX_STUFE_A
#endif
#ifndef FIX_0020
#define FIX_0020 FIX_STUFE_W
#endif
#ifndef FIX_0021
#define FIX_0021 FIX_STUFE_A
#endif
#ifndef FIX_0022
#define FIX_0022 FIX_STUFE_A
#endif
#ifndef FIX_0023
#define FIX_0023 FIX_STUFE_A
#endif
#ifndef FIX_0024
#define FIX_0024 FIX_STUFE_A
#endif
#ifndef FIX_0025
#define FIX_0025 FIX_STUFE_WX
#endif
#ifndef FIX_0026
#define FIX_0026 FIX_STUFE_A
#endif
#ifndef FIX_0027
#define FIX_0027 FIX_STUFE_A
#endif
#ifndef FIX_0028
#define FIX_0028 FIX_STUFE_A
#endif
#ifndef FIX_0029
#define FIX_0029 FIX_STUFE_A
#endif
#ifndef FIX_0031
#define FIX_0031 FIX_STUFE_A
#endif
#ifndef FIX_0032
#define FIX_0032 FIX_STUFE_W
#endif
#ifndef FIX_0033
#define FIX_0033 FIX_STUFE_A
#endif
#ifndef FIX_0034
#define FIX_0034 FIX_STUFE_A
#endif
#ifndef FIX_0035
#define FIX_0035 FIX_STUFE_A
#endif
#ifndef FIX_0036
#define FIX_0036 FIX_STUFE_A
#endif
#ifndef FIX_0037
#define FIX_0037 FIX_STUFE_W
#endif
#ifndef FIX_0038
#define FIX_0038 FIX_STUFE_A
#endif
#ifndef FIX_0039
#define FIX_0039 FIX_STUFE_W
#endif
#ifndef FIX_0041
#define FIX_0041 FIX_STUFE_A
#endif
#ifndef FIX_0042
#define FIX_0042 FIX_STUFE_A
#endif
#ifndef FIX_0043
#define FIX_0043 FIX_STUFE_W
#endif
#ifndef FIX_0044
#define FIX_0044 FIX_STUFE_A
#endif
#ifndef FIX_0045
#define FIX_0045 FIX_STUFE_WX
#endif
#ifndef FIX_0046
#define FIX_0046 FIX_STUFE_A
#endif
#ifndef FIX_0047
#define FIX_0047 FIX_STUFE_A
#endif
#ifndef FIX_0048
#define FIX_0048 FIX_STUFE_A
#endif
#ifndef FIX_0049
#define FIX_0049 FIX_STUFE_A
#endif
#ifndef FIX_0050
#define FIX_0050 FIX_STUFE_A
#endif
#ifndef FIX_0051
#define FIX_0051 FIX_STUFE_W
#endif
#ifndef FIX_0052
#define FIX_0052 FIX_STUFE_A
#endif
#ifndef FIX_0053
#define FIX_0053 FIX_STUFE_A
#endif
#ifndef FIX_0054
#define FIX_0054 FIX_STUFE_A
#endif
#ifndef FIX_0055
#define FIX_0055 FIX_STUFE_A
#endif
#ifndef FIX_0056
#define FIX_0056 FIX_STUFE_A
#endif
#ifndef FIX_0057
#define FIX_0057 FIX_STUFE_A
#endif
#ifndef FIX_0058
#define FIX_0058 FIX_STUFE_A
#endif
#ifndef FIX_0059
#define FIX_0059 FIX_STUFE_W
#endif
#ifndef FIX_0060
#define FIX_0060 FIX_STUFE_A
#endif
#ifndef FIX_0061
#define FIX_0061 FIX_STUFE_A
#endif
#ifndef FIX_0062
#define FIX_0062 FIX_STUFE_A
#endif
#ifndef FIX_0063
#define FIX_0063 FIX_STUFE_WX
#endif
#ifndef FIX_0064
#define FIX_0064 FIX_STUFE_A
#endif
#ifndef FIX_0065
#define FIX_0065 FIX_STUFE_A
#endif
#ifndef FIX_0066
#define FIX_0066 FIX_STUFE_A
#endif
#ifndef FIX_0067
#define FIX_0067 FIX_STUFE_A
#endif
#ifndef FIX_0068
#define FIX_0068 FIX_STUFE_A
#endif
#ifndef FIX_0069
#define FIX_0069 FIX_STUFE_A
#endif
#ifndef FIX_0071
#define FIX_0071 FIX_STUFE_W
#endif
#ifndef FIX_0072
#define FIX_0072 FIX_STUFE_WX
#endif
#ifndef FIX_0073
#define FIX_0073 FIX_STUFE_A
#endif
#ifndef FIX_0074
#define FIX_0074 FIX_STUFE_A
#endif
#ifndef FIX_0075
#define FIX_0075 FIX_STUFE_A
#endif
#ifndef FIX_0076
#define FIX_0076 FIX_STUFE_A
#endif
#ifndef FIX_0077
#define FIX_0077 FIX_STUFE_W
#endif
#ifndef FIX_0078
#define FIX_0078 FIX_STUFE_W
#endif
#ifndef FIX_0079
#define FIX_0079 FIX_STUFE_W
#endif
#ifndef FIX_0080
#define FIX_0080 FIX_STUFE_A
#endif
#ifndef FIX_0081
#define FIX_0081 FIX_STUFE_A
#endif
#ifndef FIX_0082
#define FIX_0082 FIX_STUFE_A
#endif
#ifndef FIX_0083
#define FIX_0083 FIX_STUFE_A
#endif
#ifndef FIX_0084
#define FIX_0084 FIX_STUFE_W
#endif
#ifndef FIX_0085
#define FIX_0085 FIX_STUFE_A
#endif
#ifndef FIX_0086
#define FIX_0086 FIX_STUFE_A
#endif
#ifndef FIX_0087
#define FIX_0087 FIX_STUFE_A
#endif
#ifndef FIX_0088
#define FIX_0088 FIX_STUFE_A
#endif
#ifndef FIX_0089
#define FIX_0089 FIX_STUFE_A
#endif
#ifndef FIX_0090
#define FIX_0090 FIX_STUFE_A
#endif
#ifndef FIX_0091
#define FIX_0091 FIX_STUFE_A
#endif
#ifndef FIX_0092
#define FIX_0092 FIX_STUFE_A
#endif
#ifndef FIX_0093
#define FIX_0093 FIX_STUFE_W
#endif
#ifndef FIX_0094
#define FIX_0094 FIX_STUFE_W
#endif
#ifndef FIX_0095
#define FIX_0095 FIX_STUFE_A
#endif
#ifndef FIX_0097
#define FIX_0097 FIX_STUFE_A
#endif
#ifndef FIX_0098
#define FIX_0098 FIX_STUFE_W
#endif
#ifndef FIX_0099
#define FIX_0099 FIX_STUFE_A
#endif
#ifndef FIX_0100
#define FIX_0100 FIX_STUFE_A
#endif
#ifndef FIX_0101
#define FIX_0101 FIX_STUFE_A
#endif
#ifndef FIX_0102
#define FIX_0102 FIX_STUFE_W
#endif
#ifndef FIX_0103
#define FIX_0103 FIX_STUFE_A
#endif
#ifndef FIX_0104
#define FIX_0104 FIX_STUFE_A
#endif
#ifndef FIX_0105
#define FIX_0105 FIX_STUFE_A
#endif
#ifndef FIX_0106
#define FIX_0106 FIX_STUFE_W
#endif
#ifndef FIX_0107
#define FIX_0107 FIX_STUFE_A
#endif
#ifndef FIX_0108
#define FIX_0108 FIX_STUFE_A
#endif
#ifndef FIX_0109
#define FIX_0109 FIX_STUFE_W
#endif
#ifndef FIX_0110
#define FIX_0110 FIX_STUFE_W
#endif
#ifndef FIX_0111
#define FIX_0111 FIX_STUFE_W
#endif
#ifndef FIX_0112
#define FIX_0112 FIX_STUFE_A
#endif
#ifndef FIX_0113
#define FIX_0113 FIX_STUFE_A
#endif
#ifndef FIX_0114
#define FIX_0114 FIX_STUFE_W
#endif
#ifndef FIX_0115
#define FIX_0115 FIX_STUFE_A
#endif
#ifndef FIX_0116
#define FIX_0116 FIX_STUFE_A
#endif
#ifndef FIX_0117
#define FIX_0117 FIX_STUFE_W
#endif
#ifndef FIX_0118
#define FIX_0118 FIX_STUFE_WX
#endif
#ifndef FIX_0119
#define FIX_0119 FIX_STUFE_W
#endif
#ifndef FIX_0120
#define FIX_0120 FIX_STUFE_W
#endif
#ifndef FIX_0121
#define FIX_0121 FIX_STUFE_W
#endif
#ifndef FIX_0122
#define FIX_0122 FIX_STUFE_W
#endif
#ifndef FIX_0123
#define FIX_0123 FIX_STUFE_WX
#endif
#ifndef FIX_0124
#define FIX_0124 FIX_STUFE_W
#endif
#ifndef FIX_0125
#define FIX_0125 FIX_STUFE_A
#endif
#ifndef FIX_0126
#define FIX_0126 FIX_STUFE_W
#endif
#ifndef FIX_0127
#define FIX_0127 FIX_STUFE_W
#endif
#ifndef FIX_0128
#define FIX_0128 FIX_STUFE_W
#endif
#ifndef FIX_0129
#define FIX_0129 FIX_STUFE_W
#endif
#ifndef FIX_0130
#define FIX_0130 FIX_STUFE_W
#endif
#ifndef FIX_0131
#define FIX_0131 FIX_STUFE_A
#endif
#ifndef FIX_0132
#define FIX_0132 FIX_STUFE_A
#endif
#ifndef FIX_0133
#define FIX_0133 FIX_STUFE_A
#endif
#ifndef FIX_0134
#define FIX_0134 FIX_STUFE_W
#endif
#ifndef FIX_0135
#define FIX_0135 FIX_STUFE_W
#endif
#ifndef FIX_0136
#define FIX_0136 FIX_STUFE_A
#endif
#ifndef FIX_0138
#define FIX_0138 FIX_STUFE_W
#endif
#ifndef FIX_0139
#define FIX_0139 FIX_STUFE_A
#endif
#ifndef FIX_0140
#define FIX_0140 FIX_STUFE_W
#endif
#ifndef FIX_0141
#define FIX_0141 FIX_STUFE_A
#endif
#ifndef FIX_0142
#define FIX_0142 FIX_STUFE_A
#endif
#ifndef FIX_0143
#define FIX_0143 FIX_STUFE_W
#endif
#ifndef FIX_0144
#define FIX_0144 FIX_STUFE_W
#endif
#ifndef FIX_0145
#define FIX_0145 FIX_STUFE_W
#endif
#ifndef FIX_0146
#define FIX_0146 FIX_STUFE_W
#endif
#ifndef FIX_0147
#define FIX_0147 FIX_STUFE_A
#endif
#ifndef FIX_0148
#define FIX_0148 FIX_STUFE_A
#endif
#ifndef FIX_0149
#define FIX_0149 FIX_STUFE_W
#endif
#ifndef FIX_0150
#define FIX_0150 FIX_STUFE_A
#endif
#ifndef FIX_0151
#define FIX_0151 FIX_STUFE_W
#endif
#ifndef FIX_0152
#define FIX_0152 FIX_STUFE_A
#endif
#ifndef FIX_0153
#define FIX_0153 FIX_STUFE_W
#endif
#ifndef FIX_0154
#define FIX_0154 FIX_STUFE_W
#endif
#ifndef FIX_0155
#define FIX_0155 FIX_STUFE_W
#endif
#ifndef FIX_0157
#define FIX_0157 FIX_STUFE_W
#endif
#ifndef FIX_0158
#define FIX_0158 FIX_STUFE_A
#endif
#ifndef FIX_0159
#define FIX_0159 FIX_STUFE_WX
#endif
#ifndef FIX_0160
#define FIX_0160 FIX_STUFE_W
#endif
#ifndef FIX_0161
#define FIX_0161 FIX_STUFE_W
#endif
#ifndef FIX_0162
#define FIX_0162 FIX_STUFE_A
#endif
#ifndef FIX_0163
#define FIX_0163 FIX_STUFE_A
#endif
#ifndef FIX_0164
#define FIX_0164 FIX_STUFE_A
#endif
#ifndef FIX_0165
#define FIX_0165 FIX_STUFE_W
#endif
#ifndef FIX_0166
#define FIX_0166 FIX_STUFE_WX
#endif
#ifndef FIX_0167
#define FIX_0167 FIX_STUFE_A
#endif
#ifndef FIX_0168
#define FIX_0168 FIX_STUFE_A
#endif
#ifndef FIX_0169
#define FIX_0169 FIX_STUFE_A
#endif
#ifndef FIX_0170
#define FIX_0170 FIX_STUFE_W
#endif
#ifndef FIX_0171
#define FIX_0171 FIX_STUFE_A
#endif
#ifndef FIX_0172
#define FIX_0172 FIX_STUFE_A
#endif
#ifndef FIX_0173
#define FIX_0173 FIX_STUFE_WX
#endif
#ifndef FIX_0174
#define FIX_0174 FIX_STUFE_W
#endif
#ifndef FIX_0175
#define FIX_0175 FIX_STUFE_A
#endif
#ifndef FIX_0176
#define FIX_0176 FIX_STUFE_A
#endif
#ifndef FIX_0177
#define FIX_0177 FIX_STUFE_W
#endif
#ifndef FIX_0178
#define FIX_0178 FIX_STUFE_A
#endif
#ifndef FIX_0179
#define FIX_0179 FIX_STUFE_W
#endif
#ifndef FIX_0180
#define FIX_0180 FIX_STUFE_W
#endif
#ifndef FIX_0181
#define FIX_0181 FIX_STUFE_A
#endif
#ifndef FIX_0182
#define FIX_0182 FIX_STUFE_W
#endif
#ifndef FIX_0183
#define FIX_0183 FIX_STUFE_W
#endif
#ifndef FIX_0184
#define FIX_0184 FIX_STUFE_A
#endif
#ifndef FIX_0185
#define FIX_0185 FIX_STUFE_W
#endif
#ifndef FIX_0186
#define FIX_0186 FIX_STUFE_W
#endif
#ifndef FIX_0187
#define FIX_0187 FIX_STUFE_W
#endif
#ifndef FIX_0188
#define FIX_0188 FIX_STUFE_W
#endif
#ifndef FIX_0189
#define FIX_0189 FIX_STUFE_A
#endif
#ifndef FIX_0190
#define FIX_0190 FIX_STUFE_A
#endif
#ifndef FIX_0191
#define FIX_0191 FIX_STUFE_W
#endif
#ifndef FIX_0192
#define FIX_0192 FIX_STUFE_W
#endif
#ifndef FIX_0193
#define FIX_0193 FIX_STUFE_W
#endif
#ifndef FIX_0194
#define FIX_0194 FIX_STUFE_W
#endif
#ifndef FIX_0195
#define FIX_0195 FIX_STUFE_A
#endif
#ifndef FIX_0196
#define FIX_0196 FIX_STUFE_W
#endif
#ifndef FIX_0197
#define FIX_0197 FIX_STUFE_W
#endif
#ifndef FIX_0198
#define FIX_0198 FIX_STUFE_A
#endif
#ifndef FIX_0199
#define FIX_0199 FIX_STUFE_WX
#endif
#ifndef FIX_0200
#define FIX_0200 FIX_STUFE_A
#endif
#ifndef FIX_0201
#define FIX_0201 FIX_STUFE_A
#endif
#ifndef FIX_0202
#define FIX_0202 FIX_STUFE_A
#endif
#ifndef FIX_0203
#define FIX_0203 FIX_STUFE_WX
#endif
#ifndef FIX_0204
#define FIX_0204 FIX_STUFE_W
#endif
#ifndef FIX_0205
#define FIX_0205 FIX_STUFE_A
#endif
#ifndef FIX_0206
#define FIX_0206 FIX_STUFE_W
#endif
#ifndef FIX_0207
#define FIX_0207 FIX_STUFE_W
#endif
#ifndef FIX_0208
#define FIX_0208 FIX_STUFE_W
#endif
#ifndef FIX_0209
#define FIX_0209 FIX_STUFE_A
#endif
#ifndef FIX_0210
#define FIX_0210 FIX_STUFE_A
#endif
#ifndef FIX_0211
#define FIX_0211 FIX_STUFE_A
#endif
#ifndef FIX_0212
#define FIX_0212 FIX_STUFE_W
#endif
#ifndef FIX_0213
#define FIX_0213 FIX_STUFE_WX
#endif
#ifndef FIX_0214
#define FIX_0214 FIX_STUFE_WX
#endif
#ifndef FIX_0215
#define FIX_0215 FIX_STUFE_W
#endif
#ifndef FIX_0216
#define FIX_0216 FIX_STUFE_W
#endif
#ifndef FIX_0217
#define FIX_0217 FIX_STUFE_A
#endif
#ifndef FIX_0218
#define FIX_0218 FIX_STUFE_A
#endif
#ifndef FIX_0219
#define FIX_0219 FIX_STUFE_A
#endif
#ifndef FIX_0220
#define FIX_0220 FIX_STUFE_W
#endif
#ifndef FIX_0221
#define FIX_0221 FIX_STUFE_A
#endif
#ifndef FIX_0222
#define FIX_0222 FIX_STUFE_A
#endif
#ifndef FIX_0224
#define FIX_0224 FIX_STUFE_A
#endif
#ifndef FIX_0225
#define FIX_0225 FIX_STUFE_A
#endif
#ifndef FIX_0226
#define FIX_0226 FIX_STUFE_A
#endif
#ifndef FIX_0227
#define FIX_0227 FIX_STUFE_A
#endif
#ifndef FIX_0230
#define FIX_0230 FIX_STUFE_A
#endif
#ifndef FIX_0231
#define FIX_0231 FIX_STUFE_A
#endif
#ifndef FIX_0232
#define FIX_0232 FIX_STUFE_A
#endif
#ifndef FIX_0233
#define FIX_0233 FIX_STUFE_W
#endif
#ifndef FIX_0300
#define FIX_0300 FIX_STUFE_A
#endif
#ifndef FIX_0301
#define FIX_0301 FIX_STUFE_A
#endif
#ifndef FIX_0302
#define FIX_0302 FIX_STUFE_A
#endif
#ifndef FIX_0304
#define FIX_0304 FIX_STUFE_A
#endif
#ifndef FIX_0305
#define FIX_0305 FIX_STUFE_A
#endif
#ifndef FIX_0306
#define FIX_0306 FIX_STUFE_A
#endif
#ifndef FIX_0307
#define FIX_0307 FIX_STUFE_A
#endif
#ifndef FIX_0350
#define FIX_0350 FIX_STUFE_A
#endif
#ifndef FIX_0351
#define FIX_0351 FIX_STUFE_A
#endif
#ifndef FIX_0352
#define FIX_0352 FIX_STUFE_W
#endif
#ifndef FIX_0353
#define FIX_0353 FIX_STUFE_WX
#endif
#ifndef FIX_0354
#define FIX_0354 FIX_STUFE_WX
#endif
#ifndef FIX_0355
#define FIX_0355 FIX_STUFE_A
#endif
#ifndef FIX_0356
#define FIX_0356 FIX_STUFE_W
#endif
#ifndef FIX_0357
#define FIX_0357 FIX_STUFE_WX
#endif
#ifndef FIX_0358
#define FIX_0358 FIX_STUFE_WX
#endif
#ifndef FIX_0359
#define FIX_0359 FIX_STUFE_W
#endif
#ifndef FIX_0360
#define FIX_0360 FIX_STUFE_WX
#endif
#ifndef FIX_0361
#define FIX_0361 FIX_STUFE_W
#endif
#ifndef FIX_0362
#define FIX_0362 FIX_STUFE_W
#endif
#ifndef FIX_0363
#define FIX_0363 FIX_STUFE_W
#endif
#ifndef FIX_0364
#define FIX_0364 FIX_STUFE_W
#endif
#ifndef FIX_0365
#define FIX_0365 FIX_STUFE_A
#endif
#ifndef FIX_0366
#define FIX_0366 FIX_STUFE_A
#endif
#ifndef FIX_0367
#define FIX_0367 FIX_STUFE_W
#endif
#ifndef FIX_0368
#define FIX_0368 FIX_STUFE_WX
#endif
#ifndef FIX_0369
#define FIX_0369 FIX_STUFE_WX
#endif
#ifndef FIX_0370
#define FIX_0370 FIX_STUFE_WX
#endif
#ifndef FIX_0371
#define FIX_0371 FIX_STUFE_W
#endif
#ifndef FIX_0372
#define FIX_0372 FIX_STUFE_A
#endif
#ifndef FIX_0373
#define FIX_0373 FIX_STUFE_W
#endif
#ifndef FIX_0374
#define FIX_0374 FIX_STUFE_WX
#endif
#ifndef FIX_0375
#define FIX_0375 FIX_STUFE_W
#endif
#ifndef FIX_0376
#define FIX_0376 FIX_STUFE_A
#endif

#endif
