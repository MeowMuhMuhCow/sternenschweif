# Sternenschweif – Bugfix-Fassung: Änderungsliste

**Version 1.1 vom 30.10.2026** · inoffiziell · zur deutschen GOG-Fassung C1.02 · **im Spiel noch ungetestet, Tester willkommen**

## So geht's

Nötig: Windows und **„Realms of Arkania 2 - Star Trail Classic“ von GOG** (deutsche Fassung). Kein Python, kein Compiler.

| | |
|---|---|
| 1 | Das Bugfix-Paket herunterladen (bei GitHub unter **Releases**, oder **Code → Download ZIP**) und irgendwohin entpacken |
| 2 | **`PATCHEN.bat`** doppelklicken. Es findet den GOG-Ordner (oder fragt), prüft die eigene `SCHWEIF.EXE` per Prüfsumme und baut daraus die Bugfix-EXEs. **Das Original wird nicht verändert** |
| 3 | **`SCHWEIF_FIX_WICHTIG_STARTEN.bat`** doppelklicken und spielen. Wer wirklich alles behoben haben will: `SCHWEIF_FIX_ALLES_STARTEN.bat` |

Die Spielstände der Bugfix-Fassung liegen in eigenen Ordnern unter `Build\`. Alte Spielstände lassen sich dorthin kopieren und weiterspielen, das Format ist unverändert. Die echten Spielstände im GOG-Ordner bleiben unberührt.

| Fassung | behebt |
|---|---|
| **WICHTIG** (empfohlen) | kritische und mittlere Fehler: Abstürze, Hänger, verlorene Ausrüstung, falsche Preise, Fehler, die man im Spiel merkt |
| **ALLES** | zusätzlich viele kleine Fehler ohne große Spielwirkung |

## Neu in Version 1.1

Abgleich mit der Zauberliste im Lösungsbuch zur Schicksalsklinge. Zauber, die dort beschrieben sind, im Spiel aber nichts oder zu wenig tun:

- **Visibili macht die ganze Gruppe unsichtbar**, wie das Lösungsbuch zur Schicksalsklinge den Zauber beschreibt. Sind alle Helden unsichtbar, fällt der Kampf aus. Bisher wurde nur der Zaubernde unsichtbar, und in einer Gruppe gab es trotzdem jeden Kampf (Wunsch aus dem Crystals-Forum) – Nr. 377 (WICHTIG)
- **Gardianum schützt jetzt.** Der Schild (Stufe + eingesetzte AE) fängt feindliche Fulminictus und Ignifaxius ab, bis er verbraucht ist. Bisher kostete er nur AE – Nr. 378 (WICHTIG)

## Was ist neu

- Orks im Gefängnis werfen die abgenommene Ausrüstung nicht mehr weg, sie liegt danach in den Truhen – Nr. 212
- Lagerhaus: der Warenwert verdoppelt sich nicht mehr bei jedem Einlagern, die Lagerkosten explodieren nicht *(teilweise)* – Nr. 352
- Laden und fahrender Händler: man zahlt nur noch für die Ware, die man auch bekommt (auch Nr. 20) – Nr. 84
- Kräuterwirkungen verfallen wieder ordentlich, die Werte kippen nicht mehr – Nr. 369
- Der Kampf gegen Horriphobus endet wieder – Nr. 364
- Schlüsselbund: kein Absturz mehr ab 18 Schlüsseln, kein verschwundener Schlüssel beim Abnehmen (auch Nr. 362) – Nr. 14
- Eine leere Gruppe hängt das Spiel nicht mehr auf – Nr. 363
- Herberge: eine abgebrochene Umbuchung behält das alte Zimmer – Nr. 155 ✅ *im Spiel getestet*
- Kein Sprung auf 42,9 Millionen Dukaten mehr, wenn das Geld unter null fällt (Exploit) – Nr. 25

## Überblick

| | ganz behoben | teilweise | Summe |
|---|---:|---:|---:|
| WICHTIG (steckt auch in ALLES) | 94 | 17 | 111 |
| nur in ALLES | 103 | 13 | 116 |
| **ALLES gesamt** | **197** | **30** | **227** |

Davon 20 **Exploits** (Fehler, die dem Spieler nützen, z. B. Geld oder Werte aus dem Nichts). Sie sind standardmäßig mit behoben. Wer sie behalten will, baut selbst mit `FIX_EXPLOITS 0` (siehe README).

## Alle Änderungen nach Spielbereich

*Stufe W = schon in WICHTIG, A = nur in ALLES. 💰 = Exploit.*

### Dungeons (34)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 6 | **Orks werfen abgenommene Ausrüstung weg (wie Nr. 212)** – Was die Orks bei der Gefangennahme abnehmen, ist für immer weg, die Truhen mit der Ausrüstung sind leer. | W |
| 12 | **Orkgefängnis: bei Alarm zwei Kämpfe hintereinander** – Auf einem Feld im Orkgefängnis folgt nach einem Alarm-Kampf sofort ein zweiter, auch nach Flucht oder Niederlage. | W |
| 13 | **Höhlenspinnen kommen bis zur Nestzerstörung wieder** – Die Höhlenspinnen im Orkgefängnis greifen bei jedem Betreten des Feldes erneut an, bis das Nest zerstört ist. | W |
| 59 | **NSC verlässt die Gruppe ohne Abschiedstext** – Ist ein NSC im Gewölbe des Namenlosen-Kults unter Tjolmar nicht handlungsfähig, verlässt er die Gruppe kommentarlos. | W |
| 102 | **Gewölbe des Namenlosen: Ereignisse zufällig** – Im Gewölbe des Namenlosen-Kults unter Tjolmar werden Zufallsbegegnungen und Kartenereignisse manchmal zufällig ausgelöst oder unterdrückt. | W |
| 114 | **Lorenfahrt: Abspringen gefahrlos, aber ohne AP** – Beim Abspringen von der Lore in der Binge gibt es nie Schaden, aber auch nie Abenteuerpunkte. | W |
| 122 | **Ingerimms Schmiede: Meldung fehlt, „und“ falsch** – In Ingerimms Schmiede fehlt eine Meldung, und Aufzählungen von Heldennamen setzen das „und“ manchmal falsch. | W |
| 123 | **Unendlich Geld aus Truhen** 💰 – Nimmt man aus einem Geldstapel in einer Truhe nur einen Teil, bekommt man trotzdem den ganzen Stapel gutgeschrieben – immer wieder. | W |
| 140 | **Pelzwand: Abzug beim falschen Gegenstand** – An der kalten Pelzwand in der Binge wird beim falschen Gegenstand abgezogen, im schlimmsten Fall an einem Zauberwert des Helden. | W |
| 143 | **Zwergenbinge: Portal versetzt andere Teilgruppe** – Bei geteilter Gruppe kann das Portal in der Zwergenbinge eine andere Teilgruppe in eine falsche Zeile oder Wand versetzen. | W |
| 151 | **Zwergengrab: Frevel ohne Kriegsaxt** – Nimmt man im Zwergengrab der Binge die Kriegsaxt bei vollem Gepäck, zürnt Ingerimm, obwohl man die Axt gar nicht bekommt. | W |
| 157 | **Orkfestung Ebene 3: ausgewürfelte Gegner fehlen** *(teilweise)* – Auf zwei Feldern der dritten Ebene der Orkfestung erscheinen ausgewürfelte Zusatzgegner nie, und nach manchen Kämpfen lösen Ereignisse zufällig aus. | W |
| 160 | **Zwergenbinge: Gruppe falsch, fremder Gegenstand weg** – Nach der Altarvision in der Zwergenbinge passen aktive Gruppe und Gruppennummer nicht zusammen, und an der morschen Truhe kann ein fremder Gegenstand verloren gehen. | W |
| 182 | **Orkfestung Ebene 1: Ereignisse zufällig** – Auf der ersten Ebene der Orkfestung werden Folgeereignisse und Zufallsbegegnungen falsch ausgelöst oder unterdrückt. | W |
| 192 | **Bolzenfalle im Phex-Tempel tötet hintere Helden** – Die Bolzenfalle im Phex-Tempel trifft jeden weiteren Helden härter; ab dem dritten ist sie sicher tödlich. | W |
| 194 | **Orkfestung Ebene 3: Schaden bei jedem Betreten** *(teilweise)* – Auf einem Feld der dritten Ebene der Orkfestung nehmen die Helden bei jedem Betreten Schaden, und Ereignisse lösen teils zufällig aus. | W |
| 199 | **Unrathaufen im Orkgefängnis: endlos Gegenstände** 💰 – Nach vier Funden im Unrathaufen des Orkgefängnisses bringt jede weitere Suche einen unbestimmten Gegenstand – beliebig oft, mit Absturzgefahr. | W |
| 203 | **Flammenschlüssel beliebig oft holbar** 💰 – Am aufgetauten Brunnen im Gewölbe unter Tjolmar kann man den Flammenschlüssel immer wieder nehmen. | W |
| 204 | **Hesinde-Statue: geschwächter Held entgeht der Strafe** – An der Hesinde-Statue im Gewölbe unter Tjolmar bricht ein ausgewählter Held mit weniger als 7 LE nicht zusammen, sondern bekommt die Belohnung. | W |
| 212 | **Orkgefängnis: beschlagnahmte Ausrüstung für immer weg** – Alles, was die Orks bei der Gefangennahme abnehmen (Kräuter, Tränke, Magisches, Rüstungen, Waffen), ist verloren – die Truhen damit sind leer. | W |
| 354 | **Phexhelm und Phexschild vervielfachen** 💰 – Gibt man Phexhelm und Phexschild einem abgetrennten Helden, behält er sie, und der Poltergeist legt trotzdem neue in die Truhe. | W |
| 357 | **Asthenilschwert per ESC behalten** 💰 – Mit ESC bei der Tauschfrage behält man Salamanderstein und Asthenilschwert, ohne dass es als Frevel gilt. | W |
| 361 | **Volles Inventar: Asthenilring oder magischer Wasserschlauch weg** – Hat der Held keinen Platz, sind Asthenilring bzw. magischer Wasserschlauch ohne Meldung und für immer verloren. | W |
| 368 | **Hesinde-Statue als AP-Quelle** 💰 – Lehnt man das Opfer an der Hesinde-Statue ab, bekommt jeder Held trotzdem 20 AP, beliebig oft. | W |
| 31 | **Ortswechsel überschreibt den gemerkten alten Ort** – Nichts Sichtbares. | A |
| 60 | **Teleport mit unbekannter Herkunft** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 89 | **Kobold Mumpitz: Auswahl liest außerhalb** *(teilweise)* – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 104 | **Altsvellt-Sumpf: alte Ebene überschrieben** – Nichts Sichtbares. | A |
| 105 | **Spinnennetze: Schreiben nach Adresse 0** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 115 | **Lore: Lesen vor der Gültigkeitsprüfung** – Nichts Sichtbares. | A |
| 133 | **Binge: Position doppelt gesetzt** – Nichts Sichtbares. | A |
| 164 | **Ingerimms Schmiede: Schreiben hinter das Feld** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 218 | **Unratfund heißt immer „einen Dolch“** – Im Unrathaufen des Orkgefängnisses heißt jeder Fund „einen Dolch“, obwohl man auch Schmuck, einen goldenen Schlüssel oder eine Bronzeflasche erhält. | A |
| 366 | **Tierkunde an den Spinnennetzen invertiert** – Vor den Höhlenspinnen wird gewarnt, wenn jemand die Tierkunde-Probe verpatzt, und nicht, wenn sie gelingt. | A |

### Reise & Ereignisse (27)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 5 | **Wegereignisse lösen an zufälligen Stellen aus** – Manche Wegereignisse treten unterwegs an unerwarteten Stellen oder zusätzlich auf. | W |
| 8 | **Zufallsbegegnungen in einer Geländeart falsch** – In einer Geländeart fällt jede sechste Zufallsbegegnung aus, und eine Gegnerart kommt öfter als gedacht. | W |
| 37 | **Absturz-Tagebucheintrag auch ohne Absturz** – Auf einem Weg steht bei jeder Durchreise im Tagebuch, ein Held sei knapp einem Absturz entgangen – oft mit Unsinnsnamen, auch wenn nichts passiert ist. | W |
| 43 | **Steinbock frisst die letzte Süßigkeit umsonst** – Bietet man dem Steinbock die letzte Lakritze oder das letzte Bonbon an, ist es weg, aber er nimmt das Angebot nicht an. | W |
| 45 | **Kräuterstelle an Weg 165 beliebig oft ernten** 💰 – An einer Kräuterstelle nahe Lowangen findet man bei jeder Durchreise erneut Wirselkraut, Einbeeren und eine Alraune, ohne dass es schwerer wird. | W |
| 63 | **Wirselkraut an Weg 201 beliebig oft ernten** 💰 – An einer Kräuterstelle findet man bei jeder Durchreise wieder Wirselkraut, die Probe wird nie schwerer. | W |
| 77 | **Vorratslager: Debugtext als Mengenfrage** – Im Vorratslager erscheint bei zwei Funden statt der Mengenfrage ein Text wie „k=3, i=2“. | W |
| 93 | **Absturz beim Bauern mit dem gebrochenen Rad** – Hilft man dem Bauern mit dem gebrochenen Rad, kann das Spiel abstürzen oder Unsinnsnamen zeigen, und der Text nennt nur drei Helden. | W |
| 118 | **Wasserfall-Spende: Vollheilung fast umsonst** 💰 – Die Spende am Wasserfall kostet statt der Hälfte des Geldes höchstens 63 Heller, manchmal bringt sie sogar Geld – und die Gruppe wird voll geheilt. | W |
| 119 | **Warzennashorn-Schaden und Kräuterprobe Weg 36 falsch** – Das Warzennashorn richtet manchmal weniger als den vorgesehenen Mindestschaden an; an der Kräuterstelle bei Tjolmar ändert sich für die meisten Gruppen nichts Spürbares. | W |
| 121 | **Sehr lange Reiseetappen dauern zu kurz** – Sehr lange Etappen auf der Reisekarte können viel zu schnell vorbei sein. | W |
| 124 | **Reisetempo im Randfall falsch** – In Randfällen stimmt das Reisetempo nicht, oder die Zahl der Wegpunkte wird nicht aktualisiert. | W |
| 134 | **Schwarzmagier-Szene: Abschlusstext und Ton fehlen** *(teilweise)* – In einem Zweig der Schwarzmagier-Szene fehlen Abschlusstext und Klang. | W |
| 146 | **Spendenschrein: falscher Text, Zufallswert** – Am Spendenschrein erscheint ein Text im falschen Moment, und Folgeereignisse fallen teils zufällig aus. | W |
| 153 | **Reise bricht unvermittelt ab** – Eine Reise wird gelegentlich ohne Grund unterbrochen, als hätte man Rast befohlen. | W |
| 170 | **Routenplanung ergibt falsche Route** – Die geplante Route auf der Reisekarte ist in manchen Fällen falsch. | W |
| 180 | **Sieg über die Schwarzmagier wird nicht gemerkt** *(teilweise)* – Besiegt man die Schwarzmagier an der zweiten Stelle, wird das Gesprächsthema „Reisende“ nie freigeschaltet. | W |
| 191 | **Lager: Wachen und Diebstahl fehlerhaft** *(teilweise)* – Beim Rasten im Lager gilt eine erfolgreiche Wache für alle folgenden, und ein Dieb überspringt Gegenstände. | W |
| 193 | **Lager: Entscheidung zufällig** – Manche Entscheidungen im Lager fallen zufällig aus. | W |
| 196 | **Treffen mit Teilgruppen unterwegs unzuverlässig** *(teilweise)* – Treffen geteilte Gruppen unterwegs aufeinander, wird das nicht zuverlässig erkannt. | W |
| 353 | **Holzfällerlager: Abbrechen legt ein Stück dazu** 💰 – Bricht man die Mengenabfrage im verlassenen Holzfällerlager ab, liegt danach ein Gegenstand mehr dort. | W |
| 370 | **Kräutersuche über „Alte Einstellungen“ ohne Malus** 💰 – Lässt man über „Alte Einstellungen“ Kräuter suchen, erholt sich der Sammler voll. | W |
| 101 | **Kräuterstelle an Weg 41: Zähler ohne Grenze** – Der Wirselkraut-Fund an einem Weg bei Finsterkoppen wird mit jeder Durchreise schwerer, nach sehr vielen Durchreisen schwankt die Schwierigkeit. | A |
| 112 | **Kriegerhüne: Wegetexte unnötig neu geladen** – Nichts Sichtbares. | A |
| 142 | **Lagerplatz: Suche liest hinter der Tabelle** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 150 | **Fähre: Zufallswert an unbekanntem Ort** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 222 | **Steinschlag: danach evtl. falsche Wegtexte** – Nach dem Freigraben eines Steinschlags können weitere Ereignisse auf demselben Wegstück falsche Texte zeigen. | A |

### Klang & Grafik (27)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 127 | **Klangeinstellung: Port/IRQ-Eingabe verworfen** – In der Klangeinstellung wird bei manchen Sound-Blaster-Treibern der eingegebene Port bzw. IRQ nicht übernommen. | W |
| 9 | **Bild-Entpacker liefert Zufallswert zurück** – Nichts Sichtbares. | A |
| 16 | **Einzelne Pixel in falscher Farbe** – Vereinzelt erscheinen Bildpunkte in einer leicht falschen Farbe. | A |
| 50 | **Phex-Tempel: nur eine von vier Flächen animiert** – Im Phex-Tempel bewegt sich von vier animierten Wandflächen nur eine. | A |
| 68 | **Klangtreiber: kleines Speicherleck** *(teilweise)* – Nichts Sichtbares; beim Neuanlegen der Klänge geht etwas Speicher verloren. | A |
| 73 | **Kampfanimation: über 40 Animationen gleichzeitig** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 74 | **Bildklasse: Randfehler ohne Aufrufweg** *(teilweise)* – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 87 | **Gegenstände scheinen durch Wände** – In der 3D-Ansicht sieht man in drei von vier Blickrichtungen manchmal Objekte durch Wände. | A |
| 92 | **Hintergrundanimation zeigt evtl. falsches Teilbild** – Höchstens ein falsches Einzelbild in einer Hintergrundanimation. | A |
| 125 | **Zwischensequenz: Überblendung zufällig** – In einer Zwischensequenz ist die Überblendung mal schneller, mal langsamer. | A |
| 132 | **Ortsszene: Musikwahl zufällig** – In einer Ortsszene kann die falsche Musik laufen. | A |
| 139 | **Kellertür: Wandbild bei nicht quadratischer Karte** – Nur auf nicht quadratischen Karten erscheint ein Türbild im Keller versetzt. | A |
| 141 | **Animation: hängender Zeiger bei fehlender Datei** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 167 | **Sonderkampf-Sequenzen: doppeltes Freigeben** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 172 | **Intro: Speicher-Merker nicht gesetzt** – Nichts Sichtbares. | A |
| 176 | **Reisekarte: Grafikfehler beim Hochscrollen** – Beim Hochscrollen der Reisekarte entstehen Grafikfehler. | A |
| 181 | **Kampf: einige Klänge fehlen** – Bei einigen Kampfereignissen fehlen die Klänge. | A |
| 189 | **Sonderkampf-Animation läuft falsch** *(teilweise)* – Eine Animation im Sonderkampf zeigt ein falsches Feld. | A |
| 190 | **Abspann: eine Zeile fehlt mit Sprachausgabe** *(teilweise)* – Mit Sprachausgabe fehlt im Abspann eine Zeile. | A |
| 195 | **Abblenden: Palette nur teilweise gelöscht** – Nichts Sichtbares. | A |
| 198 | **Ohne EMS: falsche Pixel im Bildschirmrahmen** – Ohne EMS-Speicher erscheinen nach dem Auslagern einige falsche Pixel unten links im Bildschirmrahmen. | A |
| 202 | **3D-Objekte: eine Palettenfarbe zu viel** *(teilweise)* – Nichts Sichtbares. | A |
| 211 | **Bild laden mit zu kleinem Puffer (ungenutzt)** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 224 | **Kampffiguren am Rand ungleich abgeschnitten** – Kampffiguren am Bildrand werden ungleichmäßig abgeschnitten. | A |
| 300 | **DIGPAK-Klangtreiber: Stapelfehler (ungenutzt)** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 301 | **Zeichenart „Oder“ kopiert falsch (ungenutzt)** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 351 | **Tiefensortierung bei 129 Flächen** – Nichts – der Fehler tritt im Spiel nicht auf. | A |

### Oberfläche (25)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 109 | **Texteingabefeld: Grafikfehler und Zeichenmüll** *(teilweise)* – In Texteingabefeldern (z. B. Notizen) gibt es Grafikfehler, und Enter am Zeilenanfang kann ein Zeichen der Vorzeile verändern. | W |
| 144 | **Tagebuch: Seitenzählung falsch, Überlauf möglich** *(teilweise)* – Die Seitenzahlen im Tagebuch stimmen nicht immer, und sehr lange Einträge auf vollen Seiten können das Spiel stören. | W |
| 208 | **Automap ohne XMS/EMS kaputt, Notizreste** *(teilweise)* – Ohne XMS- oder EMS-Speicher funktioniert die Automap nicht richtig, und gelöschte Kartennotizen hinterlassen Reste im Spielstand. | W |
| 7 | **Karte aufdecken: Grenzprüfung doppelt** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 10 | **Heldenvorauswahl steht manchmal auf leerem Platz** – In manchen Heldenauswahlen steht der Cursor auf einem leeren Platz statt auf dem passenden Helden. | A |
| 22 | **Automap findet sehr weite Wege nicht** – Ein Klick auf ein sehr weit entferntes Ziel der Automap ergibt „kein Weg“. | A |
| 24 | **Warenauswahl: Pfeil links springt an den Anfang** – In der Warenauswahl springt der Cursor mit Pfeil links aus der ersten Spalte nach oben links. | A |
| 27 | **Texteingabe: fehlende Endnull** – Nichts Sichtbares. | A |
| 36 | **Reisekarte: Richtungswahl ohne Rückgabewert** – Nichts Sichtbares. | A |
| 44 | **Zahleneingabe zeigt falsche erste Ziffer** – In manchen Zahlenfeldern erscheint bei großen Vorgabewerten ein unsinniges Zeichen als erste Ziffer. | A |
| 46 | **Reisekarte: Wegsuche liest hinter der Tabelle** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 47 | **Reisekarte: Wegzeichnung im Randfall falsch** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 53 | **Porträtleiste einer Teilgruppe zu breit/schmal** – Bei geteilter Gruppe wird die Porträtleiste einer nicht aktiven Gruppe manchmal in falscher Breite gezeichnet. | A |
| 56 | **Tagebuch drucken: Überlauf bei langen Zeilen** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 57 | **Tagebuch-Heldennamen: Lesen hinter dem Feld** – Nichts Sichtbares. | A |
| 58 | **Ortssymbole: unvollständige Änderungsprüfung** – Höchstens ein überflüssiges Neuzeichnen der Symbolleiste. | A |
| 62 | **Porträt ziehen liest vor der Tabelle** – Nichts Sichtbares. | A |
| 65 | **Genitiv-s bei Namen mit Sonderzeichen falsch** – Bei Heldennamen mit Sonderzeichen (außer ä, ö, ü) steht das Genitiv-s falsch oder fehlt. | A |
| 80 | **Reisekarte: Position im Randfall falsch** – In einem Randfall wird die Gruppe auf der Reisekarte an einer falschen Stelle gezeigt. | A |
| 97 | **Kompassrose verschwindet im Randfall** – In seltenen Fällen ist die Kompassrose nicht zu sehen. | A |
| 99 | **Dialogfenster: Randfälle ohne Wirkung** – Nichts Sichtbares. | A |
| 103 | **Tagebuchsuche: Esc sucht statt abzubrechen** – Drückt man in der Tagebuchsuche Esc, startet eine Suche, statt abzubrechen. | A |
| 108 | **Auswahlliste ohne Grenzprüfung** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 136 | **Fenster springt beim Verschieben** – Beim Ziehen eines Fensters springt es an eine falsche Stelle. | A |
| 162 | **Reisekarte: Ortssuche ohne Grenze** *(teilweise)* – Nichts – der Fehler tritt im Spiel nicht auf. | A |

### Kampf (24)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 1 | **Verzauberte Gegner patzen im Angriff ohne Folgen** – Greift ein verzauberter Gegner einen anderen Gegner an und patzt, verletzt er sich nie selbst. | W |
| 18 | **Flucht vom Randfeld wird manchmal verweigert** – Steht ein Held schon am Rand des Kampffelds, scheitert die Flucht gelegentlich mit „kein Weg“. | W |
| 39 | **Hänger bei völlig zugestelltem Kampffeld** – Bei einem ungewöhnlich vollen Kampffeld kann das Spiel beim Aufstellen einer Figur hängen bleiben. | W |
| 79 | **Kampf: Figur läuft selten in Zufallsrichtung** – Selten macht eine Figur im Kampf einen Schritt in eine zufällige Richtung oder bleibt einen Schritt zu früh stehen. | W |
| 98 | **Schlafender NSC steht wach im Kampf** – Schläft ein NSC bei einem Überfall, steht er bei kleiner Gruppe trotzdem wach im Kampf. | W |
| 106 | **Sonderkampf: Flächenangriff trifft niemanden** – In einem Sonderkampf bleibt ein Angriff des Gegners auf alle Helden wirkungslos – der Kampf ist leichter als gedacht. | W |
| 111 | **Gegnerischer Heilzauber verletzt Schwerverwundete** *(teilweise)* – Gegnerische Heiler verletzen ihre schwer verwundeten Verbündeten, statt sie zu heilen. | W |
| 120 | **Automatischer Kampf: Helden zaubern auf falsches Ziel** – Im automatischen Kampf zaubern Helden manchmal auf ein falsches Ziel oder Feld. | W |
| 183 | **Kampffeld: falsche Wände und Blockaden** *(teilweise)* – Auf manchen Kampffeldern blockieren unsichtbare Hindernisse Wege, oder vorhandene fehlen. | W |
| 187 | **„Sichern“ im Kampf wirkt erst eine Runde später** – Wählt man im Kampf „Sichern“, bekommt der Held in dieser Runde keinen Paradebonus, er wartet nur. | W |
| 213 | **Arax-Gift lähmt Gegner völlig** 💰 – Jeder Treffer mit einer Arax-vergifteten Waffe senkt die Bewegungspunkte des Gegners ohne Untergrenze – nach einigen Treffern kann er sich nicht mehr bewegen. | W |
| 364 | **Horriphobus: Kampf endet nie** – Ist der letzte handlungsfähige Held verängstigt und eingeschlossen, läuft der Kampf ohne Ende weiter. | W |
| 29 | **Kampfbewegung: Animationsfolge ohne Längengrenze** – Praktisch nichts; nur ein konstruierter Laufweg könnte die Kampfanimation stören. | A |
| 38 | **Kampffeld: Belegt-Prüfung ohne Wirkung** – Nichts Sichtbares. | A |
| 41 | **Paradepatzer nennt den falschen Helden** – Patzt ein verzauberter Held bei der Parade gegen einen anderen Helden, nennt der Kampftext den Angreifer. | A |
| 42 | **Patzerfolge ohne Schaden geprüft** – Nichts Sichtbares. | A |
| 49 | **Sonderkampf: Lesen vor dem Nulltest** – Nichts Sichtbares. | A |
| 54 | **Gegnerzauber bei leerer Zauberliste** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 75 | **Meldung „versteckte Gegner“ zählt manchmal falsch** – Selten fehlt ein Gegner in der Meldung über versteckte Gegner. | A |
| 76 | **Sumpfrantzendokument gegen Gegner ohne Meldung** – Benutzt man das Sumpfrantzendokument im Kampf gegen einen Gegner, fehlt der Hinweis, dass es nichts nützt. | A |
| 90 | **Kampf-Figurenliste: mehrere Randfehler** *(teilweise)* – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 147 | **Heldenaktion mit unbekannter Zielart** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 168 | **Kampfaufbau: leere Prüfung** – Nichts Sichtbares. | A |
| 200 | **Automatische Zielwahl bevorzugt eine Seite** *(teilweise)* – Die automatische Zielwahl im Kampf bevorzugt leicht eine Seite. | A |

### Gruppe & Inventar (20)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 14 | **Absturz beim Schlüsselbund mit 18 oder mehr Schlüsseln** – Hängen 18 oder mehr verschiedene Schlüssel am Schlüsselbund, stürzt das Spiel beim Benutzen ab. | W |
| 25 | **Geld ins Minus: Millionen durch Gruppenteilung** 💰 – Muss die Gruppe mehr zahlen, als sie hat, zeigt die Geldanzeige rund 42,9 Millionen Dukaten, und abgespaltene Helden bringen echte Millionen mit. | W |
| 130 | **Gulmondblatt: Spieltext durch Heldennamen ersetzt** – Benutzt ein Held ein Gulmondblatt, erscheint eine alte, falsche Meldung, und ein Spieltext wird durch seinen Namen überschrieben. | W |
| 185 | **Gegenstände weitergeben: falscher Held geprüft** *(teilweise)* – Beim Weitergeben von Gegenständen wird manchmal der falsche Held geprüft, und die Traglast wird uneinheitlich berechnet. | W |
| 207 | **Gruppentrennung mit den falschen Helden** – Bei bestimmten Ereignissen, die die Gruppe aufteilen, landen die falschen Helden in der neuen Gruppe. | W |
| 356 | **Geld verschwindet beim Vereinen von Gruppen** *(teilweise)* – Nach dem Trennen und Vereinen mit einem NSC fehlt ein Teil des Gruppengelds, je nachdem, von welcher Gruppe aus man vereint. | W |
| 362 | **Schlüssel verschwindet beim Abnehmen vom Bund** – Nimmt man bei voller Gruppe einen Schlüssel vom Bund und lehnt das Ablegen ab, ist der Schlüssel verloren. | W |
| 363 | **Leere Gruppe hängt das Spiel auf** – Verschwindet der letzte Held einer Teilgruppe (Geldscheißer, Absturz im Schacht), hängt das Spiel. | W |
| 15 | **Schlüsselbund/Rezeptbuch: falsche „schon drin“-Meldung** – Kombiniert man einen unpassenden Gegenstand mit Schlüsselbund oder Rezeptbuch, heißt es manchmal fälschlich, er sei schon darin. | A |
| 26 | **Gruppe entfernen ändert fremdes Byte** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 52 | **Gegenstandssuche über alle Plätze kaputt** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 61 | **Gruppe teilen bei sechs Gruppen** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 81 | **Zufälliger Gegenstandsverlust fehlerhaft** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 82 | **Gegenstand an jeden Helden: Restmenge falsch** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 83 | **Gruppenverwaltung schreibt hinter das Feld** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 95 | **Leeres Zunderkästchen: doppelte oder alte Meldung** – Benutzt man ein leeres Zunderkästchen, erscheint die Meldung doppelt oder ein alter, unpassender Text. | A |
| 131 | **Starker Zaubertrank nennt Lebenspunkte** – Beim starken Zaubertrank spricht die Meldung von Lebens- statt Astralpunkten, und beim Lotosgift bleibt keine leere Flasche zurück. | A |
| 169 | **Gegenstand benutzen: fehlende Standardfälle** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 171 | **Heldensuche: ungenutzte fehlerhafte Funktion** – Nichts Sichtbares. | A |
| 219 | **Gruppenwechsel überspringt NSC-Gruppen nicht** – Beim Gruppenwechsel werden Teilgruppen, in denen nur noch der NSC handlungsfähig ist, nicht übersprungen. | A |

### Helden & Werte (16)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 2 | **Automatisches Steigern ignoriert die Grenze je Talent** – Beim automatischen Stufenanstieg kann ein Talent in einer Stufe öfter steigen als erlaubt. | W |
| 138 | **Shurinknollen-Gift: Erholung extrem langsam** – Nach einer Vergiftung mit Shurinknolle kehren Körperkraft und Gewandtheit extrem langsam zurück. | W |
| 145 | **Erschaffung: Eigenschaften verschieben falsch geprüft** – Das Verschieben von Eigenschaftspunkten in der Heldenerschaffung wird manchmal verweigert, obwohl es ginge, oder angeboten, obwohl es nicht geht. | W |
| 154 | **Blaue Keuche steckt mit Dumpfschädel an** – Wer an Blauer Keuche erkrankt, steckt die anderen mit Dumpfschädel an. | W |
| 161 | **Automatische Zaubersteigerung lückenhaft** – Beim automatischen Stufenanstieg werden manche Zauber übergangen oder doppelt bedacht. | W |
| 165 | **DSA-1-Import: Gegenstände gehen verloren** – Beim Import von Helden aus der Schicksalsklinge verlieren manche Heldentypen Gegenstände, und ein Wert wird falsch auf 1000 gesetzt. | W |
| 179 | **Kein Hunger und Durst beim Rasten** – Während Rast und Schlaf bekommen die Helden weder Hunger noch Durst. | W |
| 214 | **Tanzen für Geld nutzt Musizieren** 💰 – Beim Tanzen für Geld zählt nicht der Tanzen-Wert, sondern Musizieren – ein guter Musiker verdient doppelt. | W |
| 215 | **Tollwut-Anfall: Beruhigen gelingt verkehrt herum** – Bei einem Tollwut-Anfall gelingt das Beruhigen nur, wenn mindestens ein Held an seiner Charisma-Probe scheitert; bestehen alle, misslingt es. | W |
| 369 | **Kräuterwirkungen verfallen nicht, Werte kippen** 💰 – Isst ein Held viele Kräuter hintereinander, bleiben ihre Boni dauerhaft und können ins Negative umschlagen. | W |
| 107 | **Heldenerschaffung: uninitialisierte Werte** *(teilweise)* – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 116 | **Auto-Erschaffung: Akademiezauber doppelt bedacht** – Bei der automatischen Heldenerschaffung werden manche Akademiezauber bevorzugt gesteigert. | A |
| 152 | **Alchimie/Schätzen: kleine Randfehler** – Kaum sichtbar; Schätzen kann einen Preis von 0 nennen. | A |
| 178 | **Erschaffung: Grenzwerte uneinheitlich verglichen** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 201 | **Startausrüstung: Lesen hinter dem Eintrag** – Nichts Sichtbares. | A |
| 372 | **Nichtzauberer bekommen Zaubersteigerungen angeboten** – Ein Nichtzauberer, der in der Erschaffung zuerst als Zauberer angelegt war, bekommt beim Stufenanstieg Zauberabfragen. | A |

### Städte & Orte (16)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 4 | **Nächtlicher Dieb bestiehlt nur den ersten Helden** – Bei einem nächtlichen Diebstahl im Schlafsaal fehlen nur Sachen aus dem Rucksack des ersten Helden. | W |
| 51 | **Herbergsdieb stiehlt nie Proviant, sondern Geld** – In einfachen Herbergen nimmt der Dieb nie Proviantpakete, sondern morgens immer Geld. | W |
| 71 | **Lagerhaus beim ersten Besuch manchmal gesperrt** – Beim ersten Besuch eines Lagerhauses ist es mit etwa 4 % Wahrscheinlichkeit sofort für Monate geschlossen bzw. ausgeraubt. | W |
| 72 | **Boron-Wunder: Totenerweckung viel zu häufig** 💰 – Nach dem Boron-Wunder „Schutz vor Untoten“ erwecken weitere Boron-Wunder drei Tage lang fast immer einen toten Helden. | W |
| 78 | **Lagerhaus: Verderb nach einem Jahr verkehrt herum** – Wer Waren knapp über ein Jahr einlagert, verliert fast alles, wer knapp 15 Monate wartet, fast nichts. | W |
| 149 | **Ankunft in Lowangen: erster Textteil fehlt** – Bei einer Ankunft in Lowangen fehlt der erste Teil des Begrüßungstexts. | W |
| 155 | **Herberge: Abbruch löscht die alte Zimmerbuchung** ✅ – Bricht man in der Herberge eine Umbuchung ab, ist die alte Zimmerbuchung weg (das Geld kommt zurück). | W |
| 186 | **Stadtereignisse: Text fehlt, gleiche Antworten** *(teilweise)* – Bei einem Stadtereignis erscheint ein Text nie, und zwei Antworten führen zum selben Text. | W |
| 206 | **Absturz bei der Bengram-Verwechslung in Tjolmar** – Wird ein Zwerg in Tjolmar für den Verräter Bengram gehalten und ist kaum noch ein Held einsatzfähig, kann das Spiel abstürzen; die Bettler-Spende zieht außerdem auch ohne Geld ab. | W |
| 216 | **Lowangen, zweites Stadttor: Unsinnstext** – Bei der Ankunft am zweiten Stadttor von Lowangen erscheint ein unsinniger Text. | W |
| 352 | **Lagerhaus: Waren kehren zurück, Lagerkosten explodieren** *(teilweise)* – Nach mehreren Einlagerungen werden die Lagerkosten absurd hoch und sperren das Lager, als verloren gemeldete Waren liegen beim nächsten Besuch wieder da. | W |
| 374 | **Markthalle Lowangen: Ereignis bleibt hängen** 💰 – Nach einem zweiten Besuch der Markthalle endet der nächste Kampf schon nach dem ersten toten Gegner, und alle anderen fliehen. | W |
| 11 | **Marktstand mit unbekanntem Feld** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 55 | **Fähre Hilvalla: zu wenig Geld zeigt Begrüßung erneut** – Hat man an der Fährstation in Hilvalla zu wenig Geld, erscheint beim nächsten Mal wieder der lange Begrüßungstext. | A |
| 158 | **Schenke: leerer Text, merkwürdiger Rundenzähler** *(teilweise)* – Nichts Sichtbares. | A |
| 184 | **Straßenüberfall: Lesen hinter der Liste** – Nichts Sichtbares. | A |

### Technik & Speicher (15)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 32 | **Absturz im versteckten Entwicklerbildschirm** *(teilweise)* – Im versteckten Entwicklerbildschirm (beide Umschalttasten) stürzt das Spiel ab, wenn man mehr als 40 Zeichen eintippt. | W |
| 129 | **Speicherfehler löscht die falsche Datei** – Ist der Datenträger beim Speichern voll, bleibt der kaputte Spielstand stehen, und eine andere Datei wird gelöscht. | W |
| 197 | **Spielstand laden: eine Einstellung geht verloren** – Nach dem Laden eines Spielstands ist eine Spieleinstellung zurückgesetzt. | W |
| 17 | **Speicherleck beim Verlassen der 3D-Ansicht** – Nichts Sichtbares; bei jedem Verlassen geht ein wenig Speicher verloren. | A |
| 33 | **Kalender: falscher Tagesüberlauf** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 34 | **Kalender: Tick-Überlauf verliert Restzeit** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 86 | **CD-Zwischenspeicher bei Lesefehlern kaputt** – Nur bei einer fehlerhaften CD kann eine kaputte Zwischenspeicher-Datei entstehen. | A |
| 88 | **Wegetexte: hängende Zeiger** – Nichts Sichtbares. | A |
| 91 | **Dateizugriff: Handle-Leck und späte Fehlerprüfung** *(teilweise)* – Nur bei Lesefehlern der Spieldaten können falsche Daten geladen werden. | A |
| 209 | **Speicherfehler bei über 160 MB XMS** – Nur auf Rechnern mit mehr als 160 MB XMS-Speicher wird beim Start Speicher überschrieben. | A |
| 230 | **CD-Prüfung bei leerer PATH.CFG** – Bei leerer Konfigurationsdatei meldet das Spiel meist, es gebe kein CD-ROM-Laufwerk. | A |
| 231 | **CD-Prüfung bricht ohne Aufräumen ab** – Nichts Sichtbares. | A |
| 232 | **Speichern: Existenzprüfung mit > statt >=** – Nichts Sichtbares. | A |
| 304 | **XMS-Vorgabe ohne Rücksprung** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 307 | **Kopierroutine verliert ungerades Byte** – Nichts – der Fehler tritt im Spiel nicht auf. | A |

### Magie (11)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 110 | **Stabzauber prüft eine Eigenschaft doppelt** – Die Probe für Stabzauber prüft eine Eigenschaft doppelt und eine andere gar nicht. | W |
| 159 | **Armatrutz eine Stufe höher als die AE erlaubt** 💰 – Armatrutz bietet eine Stufe an, die mehr AE kostet, als der Held hat; der Zauber wirkt trotzdem voll und verbraucht nur die vorhandene AE. | W |
| 166 | **Nihilatio Gravitas über die AE hinaus** 💰 – Nihilatio Gravitas bietet Stufen bis zur Heldenstufe an, auch wenn die AE dafür nicht reicht; der Zauber wirkt trotzdem voll. | W |
| 173 | **Hexenspeichel heilt mehr, als die AE bezahlt** 💰 – Hexenspeichel müsste doppelt so viel AE kosten, wie er heilt, wird aber nie durch die vorhandene AE begrenzt – man heilt für wenig AE viel. | W |
| 188 | **Zauberziel gegen Sondergegner falsch geprüft** – Gegen besondere Gegner werden manche Zauber und Gegenstände im Kampf fälschlich erlaubt oder verweigert. | W |
| 220 | **Meditation mit wenig LE kostet AE** – Meditiert ein Held mit weniger als 8 LE, verliert er Astralenergie, statt sie zu gewinnen. | W |
| 377 | **Visibili wirkt nur auf den Zaubernden** – Visibili macht nur den Zaubernden unsichtbar, und in einer Gruppe gibt es trotzdem jeden Kampf. | W |
| 378 | **Gardianum schützt nicht** – Gardianum meldet „Schutzschirm aktiv“, kostet AE, wehrt aber nichts ab. | W |
| 48 | **Brenne toter Stoff ohne Fackel: Meldung zufällig** – Ohne Fackel oder Lampe erscheint beim Zauber mal die Hinweismeldung, mal passiert gar nichts. | A |
| 148 | **Nichtzauberer können das Zaubermenü öffnen** – Auch Helden ohne Magie können im Heldenbogen das Zaubermenü öffnen, scheitern dann aber. | A |
| 163 | **Heldenzauber: Lesen hinter der Tabelle** *(teilweise)* – Nichts Sichtbares. | A |

### Händler & Dienste (6)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 20 | **Fahrender Händler: Ware bezahlt, aber nicht erhalten** – Kauft man beim fahrenden Händler mehr, als ins Gepäck passt, zahlt man alles, bekommt aber nur einen Teil. | W |
| 84 | **Laden: Ware bezahlt, aber nicht erhalten** – Kauft man im Laden mehr, als ins Gepäck passt, zahlt man alles, bekommt aber nur einen Teil. | W |
| 94 | **Laden: Gepäckwarnung fehlt oder falsch** *(teilweise)* – Die Kaufmaske warnt nicht zuverlässig, wenn die Ware nicht mehr ins Gepäck passt. | W |
| 128 | **Fahrender Händler: falsche Menge beim Zurücklegen** – Beim fahrenden Händler stimmt beim Zurücklegen stapelbarer Ware manchmal die Menge nicht, und die Gepäckwarnung fehlt. | W |
| 21 | **Händler-Warenliste überlappt bei riesigem Sortiment** – Nichts – der Fehler tritt im Spiel nicht auf. | A |
| 85 | **Laden-Warenliste überlappt bei riesigem Sortiment** – Nichts – der Fehler tritt im Spiel nicht auf. | A |

### Dialoge (6)

| Nr | Was behoben ist | Stufe |
|---:|---|:---:|
| 126 | **Großer AP-Bonus im Gespräch wird nie vergeben** – Bei zwei Gesprächsbelohnungen gibt es immer nur den kleinen Abenteuerpunkte-Bonus, nie den großen. | W |
| 135 | **Gespräch mit Gegenstandswahl lässt sich nicht abbrechen** – In einem Gespräch, das einen Gegenstand verlangt, lässt sich die Auswahl nicht abbrechen; trägt niemand etwas, hängt das Spiel. | W |
| 174 | **Unmut eines Gesprächspartners wird vergessen** – Hat man einen Gesprächspartner verärgert, kann sein Unmut später ohne Grund vergessen sein. | W |
| 375 | **Agdan-Quest: kein Lowangen-Schlüssel** – Hat der erste Held keinen Platz, bekommt die Gruppe den Lowangen-Schlüssel nicht, obwohl das Gespräch ihn übergibt. | W |
| 67 | **Dialogskript: Zeiger nicht genullt** – Nichts Sichtbares. | A |
| 227 | **Dialogskript: Vergleich mit leerem Feld** – Nichts – der Fehler tritt im Spiel nicht auf. | A |

## Bewusst nicht geändert

33 gemeldete Fehler bleiben, wie sie sind. Die Gründe: Die Absicht von Attic ist unklar (Spielbalance), die Behebung würde Spielstände oder Spieldaten verändern, sie hätte keine Wirkung, oder sie wäre eine neue Spielmechanik statt einer Fehlerbehebung. Jeder einzelne Grund steht in [Bauen/FIXES.md](Bauen/FIXES.md).

## Etwas gefunden?

Alle Behebungen sind am Quelltext entworfen und technisch geprüft, aber **noch nicht ausgiebig gespielt**. Wenn sich etwas falsch anfühlt oder abstürzt: bitte mit der **Nummer** aus dieser Liste melden (GitHub Issues oder im Crystals-DSA-Forum). Mit `FIX_nnnn=0` lässt sich jede einzelne Behebung abschalten, so lässt sich ein Problem schnell eingrenzen.
