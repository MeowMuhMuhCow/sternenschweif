# Sternenschweif: Fehler im Original

**Das Schwarze Auge 2: Sternenschweif** (Attic, 1994), deutsche CD-Fassung C1.02. Alle Fehler, die bei der vollständigen Rekonstruktion des Quelltexts gefunden wurden. Jeder Eintrag ist am Quelltext geprüft. Stand 19.10.2026, MooMoo mit Claude.

**Spoiler-Warnung:** Die Liste nennt Orte, Ereignisse und Lösungen aus dem ganzen Spiel.

## Übersicht

268 Fehler, eingeordnet nach zwei Fragen: *Ist das sicher ein Fehler oder vielleicht Absicht?* und *Wie stark wirkt er sich im Spiel aus?*

| | kritisch | mittel | egal |
|---|---:|---:|---:|
| **sicher falsch** | 14 | 97 | 134 |
| **unsicher falsch** | 4 | 3 | 16 |

| Begriff | Bedeutung |
|---|---|
| sicher falsch | Der Code widerspricht sich selbst oder seiner offensichtlichen Absicht |
| unsicher | Könnte Absicht von Attic sein (Spielbalance, Design). Die Begründung steht dabei |
| kritisch | Absturz, Speicher überschrieben, Spielstand oder Ausrüstung verloren, Spieler betrogen oder plötzlich reich, Quest blockiert |
| mittel | spürbar falsches Spielverhalten: Text erscheint nie, Ereignis falsch, Werte falsch |
| egal | ohne sichtbare Folge, unerreichbar, toter Code |
| Bekannt | im Netz schon beschrieben? *ja* = derselbe Fehler, *teilweise* = Symptom gemeldet, Ursache anders oder unklar, *nein* = in den durchgesehenen Sammlungen nicht gefunden |
| Exploit | Fehler, der dem Spieler nützt und gezielt ausnutzbar ist (22 Stück, siehe Cheat-Liste) |

**Im Netz bekannt:** 34 ja, 6 teilweise, 228 nein. Durchgesehen wurden vor allem der Crystals-DSA-Foren-Thread „Bugs in Sternenschweif“ (2006–2020), die Truhenpatch-Threads, Tipps-und-Tricks-Sammlungen, die Patch-Hinweise von Attic, GOG, Wikipedia und Komplettlösungen (Liste im Anhang).

9 Einträge haben sich bei der Prüfung als Absicht oder als kein Fehler erwiesen (Nr. 30, 40, 70, 137, 156, 223, 228, 229, 303). Sie stehen der Vollständigkeit halber in der Liste.

## Cheat-Liste: Exploits zum Nachstellen

Alle Schritte sind am Quelltext und an den Spieldaten geprüft.

### Nr. 25: Geld ins Minus: Millionen durch Gruppenteilung

*Gruppe & Inventar · sicher, kritisch*

Muss die Gruppe mehr zahlen, als sie hat, zeigt die Geldanzeige rund 42,9 Millionen Dukaten, und abgespaltene Helden bringen echte Millionen mit.

**So geht es:** 1. Gruppengeld unter 1 Dukat (100 Heller) bringen. 2. In einer Stadt außer Gashok umherlaufen, bis ein Bettler, eine Bettlerin oder ein Krüppel um eine Gabe bittet, und „Ja“ wählen (alternativ: in einer einfachen Herberge in Lowangen im Schlafsaal übernachten, bis ein Dieb Geld stiehlt). 3. Die Anzeige springt auf rund 42,9 Mio. Dukaten, Läden melden aber „zu wenig Geld“. 4. Gruppe teilen und mindestens einen Helden abspalten: die neue Gruppe hat mehrere Millionen Dukaten und kann frei einkaufen. 5. Danach die Gruppen wieder vereinen (das Geld wird verrechnet, die gekaufte Ware bleibt).

**Im Netz bekannt:** ja: Tommek 30.04.2007: ganzes Geld im Tempel spenden, dann Bettler Geld geben -> ca. '4 Mio. Dukaten' im Inventar (unbestätigt); NRS 2016 führt '4-Millionen-Geldbug' als bekannt, nicht behoben. Gruppenteilungs-Trick und Ursache (unsigned-Abzug) nicht beschrieben (https://www.crystals-dsa-foren.de/printthread.php?tid=46&page=2)

### Nr. 45: Kräuterstelle an Weg 165 beliebig oft ernten

*Reise & Ereignisse · sicher, mittel*

An einer Kräuterstelle nahe Lowangen findet man bei jeder Durchreise erneut Wirselkraut, Einbeeren und eine Alraune, ohne dass es schwerer wird.

**So geht es:** 1. Weg 165 der Reisekarte (nicht weit von Lowangen) bereisen; der beste Pflanzenkundige der Gruppe würfelt eine leichte Probe. 2. Bei Erfolg gibt es 2 Wirselkraut, 3 Einbeeren und 1 Alraune. 3. Den Weg hin und zurück wiederholen – die Probe wird hier nie schwerer. 4. Die Kräuterstelle an Weg 153 (bei Tiefhusen) dabei möglichst nicht abernten: ihr Zähler bestimmt die Schwierigkeit an Weg 165.

**Im Netz bekannt:** nein

### Nr. 63: Wirselkraut an Weg 201 beliebig oft ernten

*Reise & Ereignisse · sicher, mittel*

An einer Kräuterstelle findet man bei jeder Durchreise wieder Wirselkraut, die Probe wird nie schwerer.

**So geht es:** 1. Weg 201 der Reisekarte bereisen; der beste Pflanzenkundige der Gruppe würfelt. 2. Bei Erfolg gibt es 1 Wirselkraut. 3. Hin und zurück wiederholen – die Schwierigkeit hängt an der Kräuterstelle bei Kvirasim (Weg 3) und steigt hier nie. 4. Den Fund bei Kvirasim vorher nicht oft abernten. Gleich verhalten sich die Kräuterstellen an den Wegen 63, 84, 127 und 140.

**Im Netz bekannt:** nein

### Nr. 72: Boron-Wunder: Totenerweckung viel zu häufig

*Städte & Orte · sicher, mittel*

Nach dem Boron-Wunder „Schutz vor Untoten“ erwecken weitere Boron-Wunder drei Tage lang fast immer einen toten Helden.

**So geht es:** 1. In einem Borontempel Borons Ansehen durch Spenden erhöhen (jede Wunderbitte kostet 10 Punkte Ansehen). 2. Um ein Wunder bitten, bis „Boron gewährt Euch Schutz vor Untoten“ erscheint. 3. In den folgenden drei Tagen mit einem toten Helden in der Gruppe weiter um Wunder bitten: jedes gewährte Wunder (5 von 6 statt 2 von 6) holt ihn ins Leben zurück.

**Im Netz bekannt:** teilweise: Forum 27.11.2015 zitiert NLT-Wiki: nach 'Schutz vor Untoten' werden weitere Boron-Wunder als Totenauferweckung gewertet – als Spielmechanik beschrieben, nicht als Fehler; erhöhte Wahrscheinlichkeit nicht als Bug erkannt. NLT-Wiki selbst nicht abrufbar (robots.txt) (https://www.crystals-dsa-foren.de/showthread.php?tid=5131)

### Nr. 118: Wasserfall-Spende: Vollheilung fast umsonst

*Reise & Ereignisse · sicher, mittel*

Die Spende am Wasserfall kostet statt der Hälfte des Geldes höchstens 63 Heller, manchmal bringt sie sogar Geld – und die Gruppe wird voll geheilt.

**So geht es:** 1. Auf Weg 178 der Reisekarte zum Wasserfall reisen. 2. „Spenden“ wählen. 3. Statt der Hälfte des Gruppengelds werden höchstens 63 Heller abgezogen, je nach Geldbetrag gibt es sogar bis zu 64 Heller dazu; alle Helden werden voll geheilt. 4. Das Ereignis bleibt bestehen und lässt sich bei jeder Durchreise wiederholen.

**Im Netz bekannt:** teilweise: Gaddezwerch 23.08.2006: Spende am Tsa-Wasserfall westl. Arsingen meldet 'Hälfte gespendet', Guthaben stieg aber (angeblich um 50 %), Solo-Magier nicht betroffen; NRS 2016 listet 'Spenden am Tsa-Wasserfall' als bekannt, nicht behoben. Ursache (nur unterstes Byte) unbekannt, Betrag falsch gedeutet (https://www.crystals-dsa-foren.de/showthread.php?tid=46)

### Nr. 123: Unendlich Geld aus Truhen

*Dungeons · sicher, kritisch*

Nimmt man aus einem Geldstapel in einer Truhe nur einen Teil, bekommt man trotzdem den ganzen Stapel gutgeschrieben – immer wieder.

**So geht es:** 1. Im Keller unter dem Haus des Namenlosen-Dieners in Tjolmar eine der vier Truhen mit 100 Dukaten öffnen (Schloss knacken, aufbrechen oder Foramen). 2. In der Liste die Dukaten wählen und als Menge 1 eingeben. 3. Die Gruppe erhält den ganzen Stapel (100 Dukaten), in der Truhe bleiben 99. 4. Immer wieder 1 eingeben, bis die Truhe leer ist: zusammen 5050 statt 100 Dukaten je Truhe. Klappt bei jeder Truhe mit einem Dukatenstapel.

**Im Netz bekannt:** teilweise: Lumpi 11.03.2007 ('Tims Bug'): in Ingerimms Schatz bringt jedes Nehmen aus der Truhe 'zusätzlich 5000 Dukaten' – von Kunar/Zurgrimm/Crystal nicht reproduziert; als 'Ingerimm-Schatz'-Bug in Bug-Listen geführt. Möglicherweise derselbe Mechanismus (100er-Stapel ergibt 5050), Truhe in Tjolmar und Ursache nicht beschrieben (https://crystals-dsa-foren.de/showthread.php?page=4&tid=84)

### Nr. 159: Armatrutz eine Stufe höher als die AE erlaubt

*Magie · sicher, mittel*

Armatrutz bietet eine Stufe an, die mehr AE kostet, als der Held hat; der Zauber wirkt trotzdem voll und verbraucht nur die vorhandene AE.

**So geht es:** 1. Mit einem Zauberkundigen Armatrutz wirken, wenn seine AE keine Quadratzahl ist (z. B. AE 5, 10 oder 17). 2. Die angebotene Höchststufe wählen (bei AE 5: Stufe 3 für eigentlich 9 AE). 3. Gelingt die Probe, wirkt der volle Rüstungsschutz, die AE sinkt nur auf 0.

**Im Netz bekannt:** nein

### Nr. 166: Nihilatio Gravitas über die AE hinaus

*Magie · sicher, mittel*

Nihilatio Gravitas bietet Stufen bis zur Heldenstufe an, auch wenn die AE dafür nicht reicht; der Zauber wirkt trotzdem voll.

**So geht es:** 1. Einen Zauberkundigen hoher Stufe mit wenig AE Nihilatio Gravitas wirken lassen. 2. Die angebotene Höchststufe wählen – sie entspricht der Heldenstufe, auch wenn 5 AE je Stufe nicht reichen. 3. Gelingt die Probe, wirkt die volle Stufe, die AE sinkt nur auf 0.

**Im Netz bekannt:** nein

### Nr. 173: Hexenspeichel heilt mehr, als die AE bezahlt

*Magie · sicher, mittel*

Hexenspeichel müsste doppelt so viel AE kosten, wie er heilt, wird aber nie durch die vorhandene AE begrenzt – man heilt für wenig AE viel.

**So geht es:** 1. Mit einem Helden, der Hexenspeichel beherrscht, den Zauber auf einen verwundeten Gefährten wirken. 2. Als Einsatz die gesamte vorhandene AE eingeben. 3. Gelingt die Probe, heilt der Zauber so viele LE wie eingegeben (bis zur vollen LE), obwohl er das Doppelte kosten müsste; die AE sinkt nur auf 0.

**Im Netz bekannt:** nein

### Nr. 199: Unrathaufen im Orkgefängnis: endlos Gegenstände

*Dungeons · sicher, kritisch*

Nach vier Funden im Unrathaufen des Orkgefängnisses bringt jede weitere Suche einen unbestimmten Gegenstand – beliebig oft, mit Absturzgefahr.

**So geht es:** 1. Im Orkgefängnis in den Blutzinnen (zweite Ebene der Orkfestung) einen der beiden Unrathaufen durchsuchen (Frage mit „Ja“ beantworten). 2. So oft wiederholen, bis die vier regulären Funde gemacht sind (je Suche 5 %). 3. Danach liefert jede Suche sofort einen unbestimmten Gegenstand, beliebig oft. Vorher speichern: Unsinnsgegenstände oder ein Absturz sind möglich.

**Im Netz bekannt:** ja: Gaddezwerch 23.08.2006, von Boomer in C1.02 bestätigt: Schröterhaufen (Spinnenkönigin-Etage) liefert nach Dolch, Orkschmuck, Bronzeflasche, Schlüssel bei jeder weiteren Suche 'ein Schild'; als Trick bei Lumpi 2007; NRS-Patch 2016 behebt 'Schilde im Schröterhaufen'. Absturzgefahr/Müllzeiger nicht erkannt (https://www.crystals-dsa-foren.de/showthread.php?tid=46)

### Nr. 203: Flammenschlüssel beliebig oft holbar

*Dungeons · sicher, mittel*

Am aufgetauten Brunnen im Gewölbe unter Tjolmar kann man den Flammenschlüssel immer wieder nehmen.

**So geht es:** 1. Im Gewölbe unter dem Haus in Tjolmar (erste Ebene) den Eisblock mit der Kriegerin per „Hartes schmelze“ auftauen. 2. Bei der Frage, ob Ihr den flammendroten Schlüssel nehmen wollt, „Nein“ wählen (oder einige Zeit verstreichen lassen). 3. Danach den Brunnen erneut untersuchen und „Ja“ wählen: jedes Mal gibt es einen weiteren Flammenschlüssel. Ein Schlüssel ist allerdings nur rund 20 Heller wert.

**Im Netz bekannt:** ja: Kunar 28.08.2008: die Abfrage zum Flammenschlüssel (Namenlosentempel, nach Hartes schmelze) erscheint unendlich oft, Inventare lassen sich damit füllen. Ursache unbekannt (https://www.crystals-dsa-foren.de/printthread.php?tid=46&page=5)

### Nr. 213: Arax-Gift lähmt Gegner völlig

*Kampf · sicher, mittel*

Jeder Treffer mit einer Arax-vergifteten Waffe senkt die Bewegungspunkte des Gegners ohne Untergrenze – nach einigen Treffern kann er sich nicht mehr bewegen.

**So geht es:** 1. Eine Waffe mit Araxgift vergiften. 2. Im Kampf denselben Gegner mehrmals treffen: jeder Treffer kostet ihn 2 Bewegungspunkte (dazu je 1 Punkt Attacke und Parade), ohne Untergrenze. 3. Nach einigen Treffern hat er keine Bewegungspunkte mehr und bleibt stehen – dann mit Fernkampf oder Zaubern erledigen oder in Ruhe fliehen.

**Im Netz bekannt:** teilweise: Silencer 29.01.2013: Araxgift schwächt Gegner 'bis zur völligen Bewegungsunfähigkeit' – aber für Riva (DSA 3) und als normale Giftwirkung beschrieben, nicht als Fehler; für Sternenschweif nichts gefunden (https://www.crystals-dsa-foren.de/showthread.php?tid=4018)

### Nr. 214: Tanzen für Geld nutzt Musizieren

*Helden & Werte · sicher, mittel*

Beim Tanzen für Geld zählt nicht der Tanzen-Wert, sondern Musizieren – ein guter Musiker verdient doppelt.

**So geht es:** 1. Einen Helden mit hohem Musizieren-Wert wählen (der Tanzen-Wert ist egal, zum Tanzen braucht er kein Instrument). 2. Über „Talent anwenden“ erst Musizieren (mit Instrument), dann Tanzen einsetzen. 3. Beides wird auf Musizieren geprüft und bringt je 100–300 Heller; die Wartezeiten beider Talente laufen getrennt.

**Im Netz bekannt:** ja: Fíonlaighrí 19.12.2015 (mit Bright Eyes beobachtet): alle Tanzen-Proben werden auf Musizieren geprüft; Tiefhusen 09.12.2024 hält es dagegen für einen Fehler in Bright Eyes. Unser Befund: Fehler im Original (skill_dance) (https://www.crystals-dsa-foren.de/printthread.php?tid=46&page=10)

### Nr. 353: Holzfällerlager: Abbrechen legt ein Stück dazu

*Reise & Ereignisse · sicher, mittel*

Bricht man die Mengenabfrage im verlassenen Holzfällerlager ab, liegt danach ein Gegenstand mehr dort.

**So geht es:** 1. Weg 49 bereisen, das verlassene Holzfällerlager durchsuchen („Ja“). 2. Einen Eintrag mit mindestens 2 Stück wählen (z. B. Decken). 3. Bei der Frage „Wieviele … wollt Ihr mitnehmen?“ mit Rechtsklick/ESC abbrechen: die Liste zeigt ein Stück mehr. 4. Beliebig wiederholen (bis höchstens 127), dann die gewünschte Menge mitnehmen.

**Im Netz bekannt:** ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt (https://www.crystals-dsa-foren.de/printthread.php?tid=46&page=10)

### Nr. 354: Phexhelm und Phexschild vervielfachen

*Dungeons · sicher, mittel*

Gibt man Phexhelm und Phexschild einem abgetrennten Helden, behält er sie, und der Poltergeist legt trotzdem neue in die Truhe.

**So geht es:** 1. Im Phex-Tempel aus der Schatzkammer (Truhe 0x30) Phexhelm und Phexschild nehmen. 2. Die Gruppe teilen und Helm und Schild einem Helden der anderen Gruppe geben, diesen Helden aus dem Bereich bringen. 3. Mit der Hauptgruppe warten, bis der Poltergeist erscheint: er findet nichts, legt aber Helm und Schild neu in die Truhe. 4. Truhe erneut plündern, wiederholen.

**Im Netz bekannt:** ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt

### Nr. 357: Asthenilschwert per ESC behalten

*Dungeons · sicher, mittel*

Mit ESC bei der Tauschfrage behält man Salamanderstein und Asthenilschwert, ohne dass es als Frevel gilt.

**So geht es:** 1. In Ingerimms Schmiede den Golem schmieden lassen. 2. Die Truhe mit dem Salamanderstein öffnen und „Ja“ (Stein nehmen) wählen. 3. Bei „tauschen? / oder den Stein einfach mitnehmen?“ ESC drücken. 4. In der Ablage den Stein nehmen. 5. Den Golem fertig schmieden lassen: er übergibt das Asthenilschwert, kein Frevel.

**Im Netz bekannt:** ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt

### Nr. 358: Schlüsselbund und Rezeptbuch verlieren im Lagerhaus den Inhalt

*Städte & Orte · sicher, mittel*

Aus dem Lagerhaus kommen Gegenstände neu zurück: Schlüsselbund und Rezeptbuch sind leer, Kaputtes ist repariert, Geweihtes ist nicht mehr magisch.

**So geht es:** 1. Kaputte Stiefel oder eine schartige Waffe in einem Lagerhaus einlagern (100 Heller Gebühr). 2. Sofort wieder abholen: die Teile sind wie neu (billiger als jeder Schmied). Achtung: Schlüsselbund und Rezeptbuch nie einlagern, sie kommen leer zurück.

**Im Netz bekannt:** ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt

### Nr. 360: Kaputte und heile Gegenstände im selben Stapel

*Gruppe & Inventar · sicher, mittel*

Zieht man zerbrochene auf heile Gegenstände derselben Art (oder umgekehrt), übernimmt der ganze Stapel den Zustand des Zielstapels.

**So geht es:** 1. Einen heilen und einen zerbrochenen stapelbaren Gegenstand derselben Art besitzen (z. B. Pfeile/Wurfwaffen). 2. Den zerbrochenen auf den heilen Stapel ziehen: der neue Stapel ist heil.

**Im Netz bekannt:** ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt

### Nr. 368: Hesinde-Statue als AP-Quelle

*Dungeons · sicher, mittel*

Lehnt man das Opfer an der Hesinde-Statue ab, bekommt jeder Held trotzdem 20 AP, beliebig oft.

**So geht es:** 1. Hesindes Ansehen auf mindestens 100 bringen (Spenden im Hesinde-Tempel). 2. Im Gewölbe unter Tjolmar zur Hesinde-Statue gehen. 3. Die Frage nach dem Opfer mit „Nein“ beantworten: jeder Held erhält 20 AP. 4. Weggehen und wieder auf das Feld treten, beliebig wiederholen.

**Im Netz bekannt:** ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt

### Nr. 369: Kräuterwirkungen verfallen nicht, Werte kippen

*Helden & Werte · sicher, kritisch*

Isst ein Held viele Kräuter hintereinander, bleiben ihre Boni dauerhaft und können ins Negative umschlagen.

**So geht es:** 1. Mit einem Helden im Heldenbogen zwei Atmonblüten essen (20 Wirkungsplätze voll). 2. Weitere Atmonblüten, Menchal oder Belmart essen: jede weitere Wirkung überschreibt Platz 0, dessen Bonus nicht mehr abgezogen wird. 3. Wiederholen: Talente 9-18 bzw. MR steigen dauerhaft (Vorsicht: ab 128 werden die Werte negativ).

**Im Netz bekannt:** ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt

### Nr. 370: Kräutersuche über „Alte Einstellungen“ ohne Malus

*Reise & Ereignisse · sicher, mittel*

Lässt man über „Alte Einstellungen“ Kräuter suchen, erholt sich der Sammler voll.

**So geht es:** 1. Im Lager einmal einen Helden zum Kräutersuchen einteilen und rasten. 2. In den folgenden Nächten „Rast mit Aufträgen“ mit den alten Einstellungen wählen. 3. Der Sammler findet Kräuter und regeneriert ohne den Abzug von 2 Punkten.

**Im Netz bekannt:** ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt

### Nr. 374: Markthalle Lowangen: Ereignis bleibt hängen

*Städte & Orte · sicher, mittel*

Nach einem zweiten Besuch der Markthalle endet der nächste Kampf schon nach dem ersten toten Gegner, und alle anderen fliehen.

**So geht es:** 1. In Lowangen die Markthalle betreten, das Angebot ablehnen und gegen die Bettler kämpfen. 2. Die Markthalle erneut betreten, bis wieder die Bettler kommen, und kämpfen wollen: Text ohne Kampf. 3. Im nächsten schweren Kampf einen Gegner töten: alle übrigen fliehen, der Kampf ist gewonnen (mit Beute und AP).

**Im Netz bekannt:** ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt

## Sicher falsch, kritisch (14)

### Helden & Werte

**Nr. 369: Kräuterwirkungen verfallen nicht, Werte kippen** · **Exploit**  
Isst ein Held viele Kräuter hintereinander, bleiben ihre Boni dauerhaft und können ins Negative umschlagen.  
*Nachstellen:* 1. Mit einem Helden im Heldenbogen zwei Atmonblüten essen (20 Wirkungsplätze voll). 2. Weitere Atmonblüten, Menchal oder Belmart essen: jede weitere Wirkung überschreibt Platz 0, dessen Bonus nicht mehr abgezogen wird. 3. Wiederholen: Talente 9-18 bzw. MR steigen dauerhaft (Vorsicht: ab 128 werden die Werte negativ).  
*Im Netz bekannt:* ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt  

### Gruppe & Inventar

**Nr. 14: Absturz beim Schlüsselbund mit 18 oder mehr Schlüsseln**  
Hängen 18 oder mehr verschiedene Schlüssel am Schlüsselbund, stürzt das Spiel beim Benutzen ab.  
*Nachstellen:* 1. Mindestens 18 verschiedene Schlüssel sammeln und an den Schlüsselbund hängen (Schlüssel mit dem Schlüsselbund benutzen). 2. Den Schlüsselbund allein benutzen, um einen Schlüssel abzunehmen. 3. Beim Aufbau der Auswahlliste läuft der Speicher über: Absturz oder Datenmüll. Ebenso, wenn ein Held 18 oder mehr noch nicht eingehängte Schlüssel trägt und den Schlüsselbund mit einem Schlüssel benutzt.  
*Im Netz bekannt:* nein  

**Nr. 25: Geld ins Minus: Millionen durch Gruppenteilung** · **Exploit**  
Muss die Gruppe mehr zahlen, als sie hat, zeigt die Geldanzeige rund 42,9 Millionen Dukaten, und abgespaltene Helden bringen echte Millionen mit.  
*Nachstellen:* 1. Gruppengeld unter 1 Dukat (100 Heller) bringen. 2. In einer Stadt außer Gashok umherlaufen, bis ein Bettler, eine Bettlerin oder ein Krüppel um eine Gabe bittet, und „Ja“ wählen (alternativ: in einer einfachen Herberge in Lowangen im Schlafsaal übernachten, bis ein Dieb Geld stiehlt). 3. Die Anzeige springt auf rund 42,9 Mio. Dukaten, Läden melden aber „zu wenig Geld“. 4. Gruppe teilen und mindestens einen Helden abspalten: die neue Gruppe hat mehrere Millionen Dukaten und kann frei einkaufen. 5. Danach die Gruppen wieder vereinen (das Geld wird verrechnet, die gekaufte Ware bleibt).  
*Im Netz bekannt:* ja: Tommek 30.04.2007: ganzes Geld im Tempel spenden, dann Bettler Geld geben -> ca. '4 Mio. Dukaten' im Inventar (unbestätigt); NRS 2016 führt '4-Millionen-Geldbug' als bekannt, nicht behoben. Gruppenteilungs-Trick und Ursache (unsigned-Abzug) nicht beschrieben (https://www.crystals-dsa-foren.de/printthread.php?tid=46&page=2)  

**Nr. 362: Schlüssel verschwindet beim Abnehmen vom Bund**  
Nimmt man bei voller Gruppe einen Schlüssel vom Bund und lehnt das Ablegen ab, ist der Schlüssel verloren.  
*Nachstellen:* 1. Alle Rucksäcke der Gruppe füllen. 2. Den Schlüsselbund allein benutzen und einen Schlüssel wählen. 3. Bei „Soll ein Held etwas ablegen?“ „Nein“ wählen: der Schlüssel ist weder im Bund noch im Gepäck (schlimmstenfalls ein für die Handlung nötiger Schlüssel).  
*Im Netz bekannt:* ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt  

**Nr. 363: Leere Gruppe hängt das Spiel auf**  
Verschwindet der letzte Held einer Teilgruppe (Geldscheißer, Absturz im Schacht), hängt das Spiel.  
*Nachstellen:* 1. Eine Teilgruppe mit nur einem Helden bilden. 2. Entweder mit ihm den Geldscheißer brauen (Rezept aus dem Phex-Tempel) oder in der Binge (Ebene 16) mit Seil dreimal tiefer abseilen. 3. Die Gruppe ist leer: Speichern geht nicht mehr, Gruppenwechsel oder Laufen führt zu Grafikfehlern oder Hänger.  
*Im Netz bekannt:* ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt  

### Städte & Orte

**Nr. 206: Absturz bei der Bengram-Verwechslung in Tjolmar**  
Wird ein Zwerg in Tjolmar für den Verräter Bengram gehalten und ist kaum noch ein Held einsatzfähig, kann das Spiel abstürzen; die Bettler-Spende zieht außerdem auch ohne Geld ab.  
*Nachstellen:* 1. Gruppe aus mindestens drei Helden mit einem männlichen Zwerg; von den übrigen ist höchstens einer handlungsfähig (die anderen tot oder bewusstlos). 2. In Tjolmar umherlaufen, bis jemand ruft „Das ist doch Bengram, einer der Verräter!“. 3. Mindestens ein Held scheitert an der Mutprobe: der Text zeigt Unsinnsnamen, ein Zeitgeber-Eintrag im Speicher wird verändert, und das Spiel stürzt kurz darauf ab. (Die Bettler-Spende derselben Datei treibt das Geld ins Minus, siehe Nr. 25.)  
*Im Netz bekannt:* nein  

**Nr. 352: Lagerhaus: Waren kehren zurück, Lagerkosten explodieren**  
Nach mehreren Einlagerungen werden die Lagerkosten absurd hoch und sperren das Lager, als verloren gemeldete Waren liegen beim nächsten Besuch wieder da.  
*Nachstellen:* 1. In einem Lagerhaus Gegenstände einlagern. 2. Weitere Male je ein Teil einlagern: der Wert im Depot verdoppelt sich jedes Mal. 3. Einige Tage warten und das Lagerhaus wieder betreten: die angezeigte Gebühr wächst mit jeder Einlagerung, wird aber nie abgebucht. 4. Übersteigt sie das Gruppengeld, ist das Lager für immer gesperrt. 5. Nach mehr als 365 Tagen Lagerdauer abholen: die als verkauft gemeldeten Teile liegen beim nächsten Besuch wieder im Depot.  
*Im Netz bekannt:* ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt (https://www.crystals-dsa-foren.de/showthread.php?tid=949)  

### Reise & Ereignisse

**Nr. 93: Absturz beim Bauern mit dem gebrochenen Rad**  
Hilft man dem Bauern mit dem gebrochenen Rad, kann das Spiel abstürzen oder Unsinnsnamen zeigen, und der Text nennt nur drei Helden.  
*Nachstellen:* 1. Mit weniger als sieben handlungsfähigen Helden (also fast immer) Weg 97 der Reisekarte bereisen (Gegend zwischen Finsterkoppen und Lowangen). 2. Am Wegesrand sitzt ein Bauer neben seinem schiefen Wagen: „dem Mann Eure Hilfe anbieten“ wählen. 3. Je nach Speicherinhalt erscheinen Meldungen mit Unsinnsnamen, oder das Spiel stürzt kurz darauf ab (z. B. beim nächsten Klang). Das Ereignis kommt nur einmal, also vorher speichern.  
*Im Netz bekannt:* nein  

### Dungeons

**Nr. 6: Orks werfen abgenommene Ausrüstung weg (wie Nr. 212)**  
Was die Orks bei der Gefangennahme abnehmen, ist für immer weg, die Truhen mit der Ausrüstung sind leer.  
*Nachstellen:* Wie Nr. 212: 1. Unterwegs vom Orkheer gefangen nehmen lassen. 2. Am ersten Tag im Orkgefängnis nehmen die Orks Kräuter, Tränke, Magisches, Rüstungen und Waffen ab. 3. Nach dem Ausbruch die Truhen mit der Ausrüstung öffnen: leer.  
*Im Netz bekannt:* ja: Gleiche Stelle wie Nr. 212: 'Truhen-Bug' Blutzinnen seit 2007 bekannt (Kunar); NRS 2011 nennt genau diese Ursache (Gegenstände in Ablage gelegt, Zählvariable nie erhöht); per Fanpatch behoben (https://www.crystals-dsa-foren.de/showthread.php?tid=46)  

**Nr. 123: Unendlich Geld aus Truhen** · **Exploit**  
Nimmt man aus einem Geldstapel in einer Truhe nur einen Teil, bekommt man trotzdem den ganzen Stapel gutgeschrieben – immer wieder.  
*Nachstellen:* 1. Im Keller unter dem Haus des Namenlosen-Dieners in Tjolmar eine der vier Truhen mit 100 Dukaten öffnen (Schloss knacken, aufbrechen oder Foramen). 2. In der Liste die Dukaten wählen und als Menge 1 eingeben. 3. Die Gruppe erhält den ganzen Stapel (100 Dukaten), in der Truhe bleiben 99. 4. Immer wieder 1 eingeben, bis die Truhe leer ist: zusammen 5050 statt 100 Dukaten je Truhe. Klappt bei jeder Truhe mit einem Dukatenstapel.  
*Im Netz bekannt:* teilweise: Lumpi 11.03.2007 ('Tims Bug'): in Ingerimms Schatz bringt jedes Nehmen aus der Truhe 'zusätzlich 5000 Dukaten' – von Kunar/Zurgrimm/Crystal nicht reproduziert; als 'Ingerimm-Schatz'-Bug in Bug-Listen geführt. Möglicherweise derselbe Mechanismus (100er-Stapel ergibt 5050), Truhe in Tjolmar und Ursache nicht beschrieben (https://crystals-dsa-foren.de/showthread.php?page=4&tid=84)  

**Nr. 192: Bolzenfalle im Phex-Tempel tötet hintere Helden**  
Die Bolzenfalle im Phex-Tempel trifft jeden weiteren Helden härter; ab dem dritten ist sie sicher tödlich.  
*Nachstellen:* 1. Im Phex-Tempel (in den Hensger die Gruppe lockt) nicht am Opferstock spenden. 2. Den Gang mit der Falle von der Seite betreten, auf der man zuerst das Auslösefeld überquert (von der anderen Seite ist sie entschärft). 3. Auf dem Fallenfeld verliert der erste Held etwa 21–36 LE, der zweite etwa 50–80, ab dem dritten ist sie sicher tödlich.  
*Im Netz bekannt:* nein  

**Nr. 199: Unrathaufen im Orkgefängnis: endlos Gegenstände** · **Exploit**  
Nach vier Funden im Unrathaufen des Orkgefängnisses bringt jede weitere Suche einen unbestimmten Gegenstand – beliebig oft, mit Absturzgefahr.  
*Nachstellen:* 1. Im Orkgefängnis in den Blutzinnen (zweite Ebene der Orkfestung) einen der beiden Unrathaufen durchsuchen (Frage mit „Ja“ beantworten). 2. So oft wiederholen, bis die vier regulären Funde gemacht sind (je Suche 5 %). 3. Danach liefert jede Suche sofort einen unbestimmten Gegenstand, beliebig oft. Vorher speichern: Unsinnsgegenstände oder ein Absturz sind möglich.  
*Im Netz bekannt:* ja: Gaddezwerch 23.08.2006, von Boomer in C1.02 bestätigt: Schröterhaufen (Spinnenkönigin-Etage) liefert nach Dolch, Orkschmuck, Bronzeflasche, Schlüssel bei jeder weiteren Suche 'ein Schild'; als Trick bei Lumpi 2007; NRS-Patch 2016 behebt 'Schilde im Schröterhaufen'. Absturzgefahr/Müllzeiger nicht erkannt (https://www.crystals-dsa-foren.de/showthread.php?tid=46)  

**Nr. 212: Orkgefängnis: beschlagnahmte Ausrüstung für immer weg**  
Alles, was die Orks bei der Gefangennahme abnehmen (Kräuter, Tränke, Magisches, Rüstungen, Waffen), ist verloren – die Truhen damit sind leer.  
*Nachstellen:* 1. Unterwegs auf das Orkheer treffen und sich ergeben oder kämpfen – beides führt in die Gefangenschaft. 2. Am ersten Tag in der Zelle des Orkgefängnisses (Blutzinnen) nehmen die Orks Kräuter, Tränke, magische Gegenstände, Rüstungen, Waffen und den Rest ab. 3. Nach dem Ausbruch die Truhen im Gefängnis bzw. die Ablage auf der dritten Ebene der Orkfestung öffnen: sie sind leer. Wer das vermeiden will, lagert Wichtiges vorher ein (z. B. im Lagerhaus).  
*Im Netz bekannt:* ja: Bekanntester Bug: nach Gefangennahme Ausrüstungstruhen leer (Kunar 12.03.2007, 'gravierendster Bug'); NRS 08.07.2011 Ursache im Code: Zähler der Ablage wird nicht erhöht; Fanpatch 2011, 'finaler Truhenpatch' 2016; Workaround Gruppenteilung per Rechtsklick; engl. Walkthrough: 'losing all equipment permanently' (https://www.crystals-dsa-foren.de/showthread.php?tid=46)  

**Nr. 361: Volles Inventar: Asthenilring oder magischer Wasserschlauch weg**  
Hat der Held keinen Platz, sind Asthenilring bzw. magischer Wasserschlauch ohne Meldung und für immer verloren.  
*Nachstellen:* 1. Mit vollem Rucksack (16 Plätze belegt) beim Kohlebecken in Dungeon 10 den Ring aus der Glut ziehen. 2. Die Probe gelingt, Text kommt, der Ring fehlt. 3. Das Becken liefert ihn nie wieder. Gleich beim Tauchen im Becken (Dungeon 15).  
*Im Netz bekannt:* ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt  

## Sicher falsch, mittel (97)

### Kampf

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 1 | Verzauberte Gegner patzen im Angriff ohne Folgen | Greift ein verzauberter Gegner einen anderen Gegner an und patzt, verletzt er sich nie selbst. | nein |
| 18 | Flucht vom Randfeld wird manchmal verweigert | Steht ein Held schon am Rand des Kampffelds, scheitert die Flucht gelegentlich mit „kein Weg“. | nein |
| 39 | Hänger bei völlig zugestelltem Kampffeld | Bei einem ungewöhnlich vollen Kampffeld kann das Spiel beim Aufstellen einer Figur hängen bleiben. | nein |
| 79 | Kampf: Figur läuft selten in Zufallsrichtung | Selten macht eine Figur im Kampf einen Schritt in eine zufällige Richtung oder bleibt einen Schritt zu früh stehen. | nein |
| 98 | Schlafender NSC steht wach im Kampf | Schläft ein NSC bei einem Überfall, steht er bei kleiner Gruppe trotzdem wach im Kampf. | nein |
| 106 | Sonderkampf: Flächenangriff trifft niemanden | In einem Sonderkampf bleibt ein Angriff des Gegners auf alle Helden wirkungslos – der Kampf ist leichter als gedacht. | nein |
| 111 | Gegnerischer Heilzauber verletzt Schwerverwundete | Gegnerische Heiler verletzen ihre schwer verwundeten Verbündeten, statt sie zu heilen. | nein |
| 120 | Automatischer Kampf: Helden zaubern auf falsches Ziel | Im automatischen Kampf zaubern Helden manchmal auf ein falsches Ziel oder Feld. | nein |
| 183 | Kampffeld: falsche Wände und Blockaden | Auf manchen Kampffeldern blockieren unsichtbare Hindernisse Wege, oder vorhandene fehlen. | nein |
| 187 | „Sichern“ im Kampf wirkt erst eine Runde später | Wählt man im Kampf „Sichern“, bekommt der Held in dieser Runde keinen Paradebonus, er wartet nur. | nein |
| 213 | Arax-Gift lähmt Gegner völlig **(Exploit)** | Jeder Treffer mit einer Arax-vergifteten Waffe senkt die Bewegungspunkte des Gegners ohne Untergrenze – nach einigen Treffern kann er sich nicht mehr bewegen. | teilweise |

### Magie

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 110 | Stabzauber prüft eine Eigenschaft doppelt | Die Probe für Stabzauber prüft eine Eigenschaft doppelt und eine andere gar nicht. | nein |
| 159 | Armatrutz eine Stufe höher als die AE erlaubt **(Exploit)** | Armatrutz bietet eine Stufe an, die mehr AE kostet, als der Held hat; der Zauber wirkt trotzdem voll und verbraucht nur die vorhandene AE. | nein |
| 166 | Nihilatio Gravitas über die AE hinaus **(Exploit)** | Nihilatio Gravitas bietet Stufen bis zur Heldenstufe an, auch wenn die AE dafür nicht reicht; der Zauber wirkt trotzdem voll. | nein |
| 173 | Hexenspeichel heilt mehr, als die AE bezahlt **(Exploit)** | Hexenspeichel müsste doppelt so viel AE kosten, wie er heilt, wird aber nie durch die vorhandene AE begrenzt – man heilt für wenig AE viel. | nein |
| 188 | Zauberziel gegen Sondergegner falsch geprüft | Gegen besondere Gegner werden manche Zauber und Gegenstände im Kampf fälschlich erlaubt oder verweigert. | nein |
| 220 | Meditation mit wenig LE kostet AE | Meditiert ein Held mit weniger als 8 LE, verliert er Astralenergie, statt sie zu gewinnen. | nein |

### Helden & Werte

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 2 | Automatisches Steigern ignoriert die Grenze je Talent | Beim automatischen Stufenanstieg kann ein Talent in einer Stufe öfter steigen als erlaubt. | nein |
| 117 | Heilkunde Wunden: Probe doppelt gewürfelt | Heilkunde Wunden gelingt seltener als vorgesehen. | nein |
| 138 | Shurinknollen-Gift: Erholung extrem langsam | Nach einer Vergiftung mit Shurinknolle kehren Körperkraft und Gewandtheit extrem langsam zurück. | nein |
| 145 | Erschaffung: Eigenschaften verschieben falsch geprüft | Das Verschieben von Eigenschaftspunkten in der Heldenerschaffung wird manchmal verweigert, obwohl es ginge, oder angeboten, obwohl es nicht geht. | nein |
| 154 | Blaue Keuche steckt mit Dumpfschädel an | Wer an Blauer Keuche erkrankt, steckt die anderen mit Dumpfschädel an. | nein |
| 161 | Automatische Zaubersteigerung lückenhaft | Beim automatischen Stufenanstieg werden manche Zauber übergangen oder doppelt bedacht. | nein |
| 165 | DSA-1-Import: Gegenstände gehen verloren | Beim Import von Helden aus der Schicksalsklinge verlieren manche Heldentypen Gegenstände, und ein Wert wird falsch auf 1000 gesetzt. | teilweise |
| 179 | Kein Hunger und Durst beim Rasten | Während Rast und Schlaf bekommen die Helden weder Hunger noch Durst. | nein |
| 214 | Tanzen für Geld nutzt Musizieren **(Exploit)** | Beim Tanzen für Geld zählt nicht der Tanzen-Wert, sondern Musizieren – ein guter Musiker verdient doppelt. | ja |
| 215 | Tollwut-Anfall: Beruhigen gelingt verkehrt herum | Bei einem Tollwut-Anfall gelingt das Beruhigen nur, wenn mindestens ein Held an seiner Charisma-Probe scheitert; bestehen alle, misslingt es. | nein |

### Gruppe & Inventar

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 130 | Gulmondblatt: Spieltext durch Heldennamen ersetzt | Benutzt ein Held ein Gulmondblatt, erscheint eine alte, falsche Meldung, und ein Spieltext wird durch seinen Namen überschrieben. | nein |
| 185 | Gegenstände weitergeben: falscher Held geprüft | Beim Weitergeben von Gegenständen wird manchmal der falsche Held geprüft, und die Traglast wird uneinheitlich berechnet. | nein |
| 207 | Gruppentrennung mit den falschen Helden | Bei bestimmten Ereignissen, die die Gruppe aufteilen, landen die falschen Helden in der neuen Gruppe. | nein |
| 356 | Geld verschwindet beim Vereinen von Gruppen | Nach dem Trennen und Vereinen mit einem NSC fehlt ein Teil des Gruppengelds, je nachdem, von welcher Gruppe aus man vereint. | ja |
| 360 | Kaputte und heile Gegenstände im selben Stapel **(Exploit)** | Zieht man zerbrochene auf heile Gegenstände derselben Art (oder umgekehrt), übernimmt der ganze Stapel den Zustand des Zielstapels. | ja |

### Händler & Dienste

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 94 | Laden: Gepäckwarnung fehlt oder falsch | Die Kaufmaske warnt nicht zuverlässig, wenn die Ware nicht mehr ins Gepäck passt. | nein |
| 128 | Fahrender Händler: falsche Menge beim Zurücklegen | Beim fahrenden Händler stimmt beim Zurücklegen stapelbarer Ware manchmal die Menge nicht, und die Gepäckwarnung fehlt. | nein |
| 359 | Geweihte Waffen verlieren „magisch“ | Lässt man eine geweihte Waffe beim Schmied reparieren, ist sie danach nicht mehr magisch. | ja |

### Städte & Orte

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 4 | Nächtlicher Dieb bestiehlt nur den ersten Helden | Bei einem nächtlichen Diebstahl im Schlafsaal fehlen nur Sachen aus dem Rucksack des ersten Helden. | nein |
| 51 | Herbergsdieb stiehlt nie Proviant, sondern Geld | In einfachen Herbergen nimmt der Dieb nie Proviantpakete, sondern morgens immer Geld. | nein |
| 71 | Lagerhaus beim ersten Besuch manchmal gesperrt | Beim ersten Besuch eines Lagerhauses ist es mit etwa 4 % Wahrscheinlichkeit sofort für Monate geschlossen bzw. ausgeraubt. | nein |
| 72 | Boron-Wunder: Totenerweckung viel zu häufig **(Exploit)** | Nach dem Boron-Wunder „Schutz vor Untoten“ erwecken weitere Boron-Wunder drei Tage lang fast immer einen toten Helden. | teilweise |
| 78 | Lagerhaus: Verderb nach einem Jahr verkehrt herum | Wer Waren knapp über ein Jahr einlagert, verliert fast alles, wer knapp 15 Monate wartet, fast nichts. | nein |
| 149 | Ankunft in Lowangen: erster Textteil fehlt | Bei einer Ankunft in Lowangen fehlt der erste Teil des Begrüßungstexts. | nein |
| 155 | Herberge: Abbruch löscht die alte Zimmerbuchung | Bricht man in der Herberge eine Umbuchung ab, ist die alte Zimmerbuchung weg (das Geld kommt zurück). | nein |
| 186 | Stadtereignisse: Text fehlt, gleiche Antworten | Bei einem Stadtereignis erscheint ein Text nie, und zwei Antworten führen zum selben Text. | nein |
| 216 | Lowangen, zweites Stadttor: Unsinnstext | Bei der Ankunft am zweiten Stadttor von Lowangen erscheint ein unsinniger Text. | nein |
| 358 | Schlüsselbund und Rezeptbuch verlieren im Lagerhaus den Inhalt **(Exploit)** | Aus dem Lagerhaus kommen Gegenstände neu zurück: Schlüsselbund und Rezeptbuch sind leer, Kaputtes ist repariert, Geweihtes ist nicht mehr magisch. | ja |
| 367 | Hesinde-Wunder 8 und 9 bewirken nichts | Bei Hesinde, Firun und Ifirn bleiben 2 von 10 gewährten Wundern ohne Wirkung, das Ansehen ist trotzdem verbraucht. | ja |
| 374 | Markthalle Lowangen: Ereignis bleibt hängen **(Exploit)** | Nach einem zweiten Besuch der Markthalle endet der nächste Kampf schon nach dem ersten toten Gegner, und alle anderen fliehen. | ja |

### Reise & Ereignisse

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 5 | Wegereignisse lösen an zufälligen Stellen aus | Manche Wegereignisse treten unterwegs an unerwarteten Stellen oder zusätzlich auf. | nein |
| 8 | Zufallsbegegnungen in einer Geländeart falsch | In einer Geländeart fällt jede sechste Zufallsbegegnung aus, und eine Gegnerart kommt öfter als gedacht. | nein |
| 37 | Absturz-Tagebucheintrag auch ohne Absturz | Auf einem Weg steht bei jeder Durchreise im Tagebuch, ein Held sei knapp einem Absturz entgangen – oft mit Unsinnsnamen, auch wenn nichts passiert ist. | nein |
| 43 | Steinbock frisst die letzte Süßigkeit umsonst | Bietet man dem Steinbock die letzte Lakritze oder das letzte Bonbon an, ist es weg, aber er nimmt das Angebot nicht an. | ja |
| 45 | Kräuterstelle an Weg 165 beliebig oft ernten **(Exploit)** | An einer Kräuterstelle nahe Lowangen findet man bei jeder Durchreise erneut Wirselkraut, Einbeeren und eine Alraune, ohne dass es schwerer wird. | nein |
| 63 | Wirselkraut an Weg 201 beliebig oft ernten **(Exploit)** | An einer Kräuterstelle findet man bei jeder Durchreise wieder Wirselkraut, die Probe wird nie schwerer. | nein |
| 77 | Vorratslager: Debugtext als Mengenfrage | Im Vorratslager erscheint bei zwei Funden statt der Mengenfrage ein Text wie „k=3, i=2“. | nein |
| 118 | Wasserfall-Spende: Vollheilung fast umsonst **(Exploit)** | Die Spende am Wasserfall kostet statt der Hälfte des Geldes höchstens 63 Heller, manchmal bringt sie sogar Geld – und die Gruppe wird voll geheilt. | teilweise |
| 119 | Warzennashorn-Schaden und Kräuterprobe Weg 36 falsch | Das Warzennashorn richtet manchmal weniger als den vorgesehenen Mindestschaden an; an der Kräuterstelle bei Tjolmar ändert sich für die meisten Gruppen nichts Spürbares. | nein |
| 121 | Sehr lange Reiseetappen dauern zu kurz | Sehr lange Etappen auf der Reisekarte können viel zu schnell vorbei sein. | nein |
| 124 | Reisetempo im Randfall falsch | In Randfällen stimmt das Reisetempo nicht, oder die Zahl der Wegpunkte wird nicht aktualisiert. | nein |
| 134 | Schwarzmagier-Szene: Abschlusstext und Ton fehlen | In einem Zweig der Schwarzmagier-Szene fehlen Abschlusstext und Klang. | nein |
| 146 | Spendenschrein: falscher Text, Zufallswert | Am Spendenschrein erscheint ein Text im falschen Moment, und Folgeereignisse fallen teils zufällig aus. | nein |
| 153 | Reise bricht unvermittelt ab | Eine Reise wird gelegentlich ohne Grund unterbrochen, als hätte man Rast befohlen. | nein |
| 170 | Routenplanung ergibt falsche Route | Die geplante Route auf der Reisekarte ist in manchen Fällen falsch. | nein |
| 177 | Wegliste: eine Meldung erscheint nie | Beim Abgehen einer geplanten Route fehlt eine Hinweismeldung. | nein |
| 180 | Sieg über die Schwarzmagier wird nicht gemerkt | Besiegt man die Schwarzmagier an der zweiten Stelle, wird das Gesprächsthema „Reisende“ nie freigeschaltet. | nein |
| 191 | Lager: Wachen und Diebstahl fehlerhaft | Beim Rasten im Lager gilt eine erfolgreiche Wache für alle folgenden, und ein Dieb überspringt Gegenstände. | nein |
| 193 | Lager: Entscheidung zufällig | Manche Entscheidungen im Lager fallen zufällig aus. | nein |
| 196 | Treffen mit Teilgruppen unterwegs unzuverlässig | Treffen geteilte Gruppen unterwegs aufeinander, wird das nicht zuverlässig erkannt. | nein |
| 233 | Kriegerhüne: Abschiedstext erscheint nie | Nach der Begegnung mit dem Kriegerhünen fehlt der Text, wie sein Geschimpfe hinter der Biegung verklingt. | nein |
| 353 | Holzfällerlager: Abbrechen legt ein Stück dazu **(Exploit)** | Bricht man die Mengenabfrage im verlassenen Holzfällerlager ab, liegt danach ein Gegenstand mehr dort. | ja |
| 370 | Kräutersuche über „Alte Einstellungen“ ohne Malus **(Exploit)** | Lässt man über „Alte Einstellungen“ Kräuter suchen, erholt sich der Sammler voll. | ja |

### Dungeons

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 12 | Orkgefängnis: bei Alarm zwei Kämpfe hintereinander | Auf einem Feld im Orkgefängnis folgt nach einem Alarm-Kampf sofort ein zweiter, auch nach Flucht oder Niederlage. | nein |
| 59 | NSC verlässt die Gruppe ohne Abschiedstext | Ist ein NSC im Gewölbe des Namenlosen-Kults unter Tjolmar nicht handlungsfähig, verlässt er die Gruppe kommentarlos. | nein |
| 102 | Gewölbe des Namenlosen: Ereignisse zufällig | Im Gewölbe des Namenlosen-Kults unter Tjolmar werden Zufallsbegegnungen und Kartenereignisse manchmal zufällig ausgelöst oder unterdrückt. | nein |
| 114 | Lorenfahrt: Abspringen gefahrlos, aber ohne AP | Beim Abspringen von der Lore in der Binge gibt es nie Schaden, aber auch nie Abenteuerpunkte. | nein |
| 122 | Ingerimms Schmiede: Meldung fehlt, „und“ falsch | In Ingerimms Schmiede fehlt eine Meldung, und Aufzählungen von Heldennamen setzen das „und“ manchmal falsch. | nein |
| 140 | Pelzwand: Abzug beim falschen Gegenstand | An der kalten Pelzwand in der Binge wird beim falschen Gegenstand abgezogen, im schlimmsten Fall an einem Zauberwert des Helden. | nein |
| 143 | Zwergenbinge: Portal versetzt andere Teilgruppe | Bei geteilter Gruppe kann das Portal in der Zwergenbinge eine andere Teilgruppe in eine falsche Zeile oder Wand versetzen. | teilweise |
| 151 | Zwergengrab: Frevel ohne Kriegsaxt | Nimmt man im Zwergengrab der Binge die Kriegsaxt bei vollem Gepäck, zürnt Ingerimm, obwohl man die Axt gar nicht bekommt. | nein |
| 157 | Orkfestung Ebene 3: ausgewürfelte Gegner fehlen | Auf zwei Feldern der dritten Ebene der Orkfestung erscheinen ausgewürfelte Zusatzgegner nie, und nach manchen Kämpfen lösen Ereignisse zufällig aus. | nein |
| 160 | Zwergenbinge: Gruppe falsch, fremder Gegenstand weg | Nach der Altarvision in der Zwergenbinge passen aktive Gruppe und Gruppennummer nicht zusammen, und an der morschen Truhe kann ein fremder Gegenstand verloren gehen. | nein |
| 182 | Orkfestung Ebene 1: Ereignisse zufällig | Auf der ersten Ebene der Orkfestung werden Folgeereignisse und Zufallsbegegnungen falsch ausgelöst oder unterdrückt. | nein |
| 194 | Orkfestung Ebene 3: Schaden bei jedem Betreten | Auf einem Feld der dritten Ebene der Orkfestung nehmen die Helden bei jedem Betreten Schaden, und Ereignisse lösen teils zufällig aus. | nein |
| 203 | Flammenschlüssel beliebig oft holbar **(Exploit)** | Am aufgetauten Brunnen im Gewölbe unter Tjolmar kann man den Flammenschlüssel immer wieder nehmen. | ja |
| 204 | Hesinde-Statue: geschwächter Held entgeht der Strafe | An der Hesinde-Statue im Gewölbe unter Tjolmar bricht ein ausgewählter Held mit weniger als 7 LE nicht zusammen, sondern bekommt die Belohnung. | nein |
| 354 | Phexhelm und Phexschild vervielfachen **(Exploit)** | Gibt man Phexhelm und Phexschild einem abgetrennten Helden, behält er sie, und der Poltergeist legt trotzdem neue in die Truhe. | ja |
| 357 | Asthenilschwert per ESC behalten **(Exploit)** | Mit ESC bei der Tauschfrage behält man Salamanderstein und Asthenilschwert, ohne dass es als Frevel gilt. | ja |
| 368 | Hesinde-Statue als AP-Quelle **(Exploit)** | Lehnt man das Opfer an der Hesinde-Statue ab, bekommt jeder Held trotzdem 20 AP, beliebig oft. | ja |

### Dialoge

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 126 | Großer AP-Bonus im Gespräch wird nie vergeben | Bei zwei Gesprächsbelohnungen gibt es immer nur den kleinen Abenteuerpunkte-Bonus, nie den großen. | nein |
| 135 | Gespräch mit Gegenstandswahl lässt sich nicht abbrechen | In einem Gespräch, das einen Gegenstand verlangt, lässt sich die Auswahl nicht abbrechen; trägt niemand etwas, hängt das Spiel. | nein |
| 174 | Unmut eines Gesprächspartners wird vergessen | Hat man einen Gesprächspartner verärgert, kann sein Unmut später ohne Grund vergessen sein. | nein |

### Oberfläche

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 109 | Texteingabefeld: Grafikfehler und Zeichenmüll | In Texteingabefeldern (z. B. Notizen) gibt es Grafikfehler, und Enter am Zeilenanfang kann ein Zeichen der Vorzeile verändern. | nein |
| 144 | Tagebuch: Seitenzählung falsch, Überlauf möglich | Die Seitenzahlen im Tagebuch stimmen nicht immer, und sehr lange Einträge auf vollen Seiten können das Spiel stören. | nein |
| 208 | Automap ohne XMS/EMS kaputt, Notizreste | Ohne XMS- oder EMS-Speicher funktioniert die Automap nicht richtig, und gelöschte Kartennotizen hinterlassen Reste im Spielstand. | nein |

### Klang & Grafik

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 127 | Klangeinstellung: Port/IRQ-Eingabe verworfen | In der Klangeinstellung wird bei manchen Sound-Blaster-Treibern der eingegebene Port bzw. IRQ nicht übernommen. | nein |

### Technik & Speicher

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 32 | Absturz im versteckten Entwicklerbildschirm | Im versteckten Entwicklerbildschirm (beide Umschalttasten) stürzt das Spiel ab, wenn man mehr als 40 Zeichen eintippt. | nein |
| 129 | Speicherfehler löscht die falsche Datei | Ist der Datenträger beim Speichern voll, bleibt der kaputte Spielstand stehen, und eine andere Datei wird gelöscht. | nein |
| 197 | Spielstand laden: eine Einstellung geht verloren | Nach dem Laden eines Spielstands ist eine Spieleinstellung zurückgesetzt. | nein |

## Unsicher falsch, kritisch (4)

### Kampf

**Nr. 364: Horriphobus: Kampf endet nie**  
Ist der letzte handlungsfähige Held verängstigt und eingeschlossen, läuft der Kampf ohne Ende weiter.  
*Nachstellen:* 1. Im Kampf einen eigenen Helden per Horriphobus verängstigen lassen (Gegnerzauber) und dafür sorgen, dass er eingeschlossen ist. 2. Die übrigen Helden fallen aus. 3. Kann kein Gegner ihn erreichen, laufen die Runden endlos, nur ein Programmabbruch hilft.  
*Warum unsicher:* „Verängstigt“ (scared) gilt bis zum Kampfende, gelöscht wird es nur beim Aufräumen in O080 oder bei gefesselten bzw. kampfunfähigen Helden. Ein verängstigter Held versucht in jedem Zug zu fliehen (fight_path_search). Ohne Fluchtweg passiert nichts, er verliert nur seine BP. fig_get_first_active_hero zählt ihn weiter als handlungsfähig, also endet der Kampf nicht. Die Kampfschleife in Fight::run hat keine Rundengrenze und keine Patt-Erkennung. Können die Gegner ihn nicht erreichen oder sind selbst verängstigt und eingeschlossen, dreht sich der Kampf für immer.  
*Im Netz bekannt:* ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt  

### Händler & Dienste

**Nr. 20: Fahrender Händler: Ware bezahlt, aber nicht erhalten**  
Kauft man beim fahrenden Händler mehr, als ins Gepäck passt, zahlt man alles, bekommt aber nur einen Teil.  
*Warum unsicher:* Peddler::trade: passt ein Posten nicht mehr ins Gepaeck, bricht die Verteilung mit `break` ab (auch alle folgenden Posten fehlen), abgezogen wird trotzdem der volle Preis aller Posten. Absicht denkbar, weil danach eigens SHOP.LTX[19] „Ihr habt mehr gekauft, als Ihr tragen koennt!“ erscheint und die Kaufmaske eigentlich warnt (deren Warnung ist aber selbst fehlerhaft, Nr. 94/128). Fuer den Spieler: bezahlt, Ware weg.  
*Im Netz bekannt:* nein  

**Nr. 84: Laden: Ware bezahlt, aber nicht erhalten**  
Kauft man im Laden mehr, als ins Gepäck passt, zahlt man alles, bekommt aber nur einen Teil.  
*Warum unsicher:* Shop::trade wie Nr. 20: passt eine Ware nicht, endet die Verteilung, der volle Preis (abzueglich Rabatt) wird trotzdem abgezogen; danach Meldung „Ihr habt mehr gekauft, als Ihr tragen koennt!“. Absicht denkbar (Warnung in der Kaufmaske, Meldung danach), fuer den Spieler aber Geld weg ohne Ware. Nachstellen: im Laden mehr Ware waehlen, als das Gepaeck fasst (z. B. 20 schwere Ruestungen bei vollem Inventar), kaufen.  
*Im Netz bekannt:* nein  

### Dialoge

**Nr. 375: Agdan-Quest: kein Lowangen-Schlüssel**  
Hat der erste Held keinen Platz, bekommt die Gruppe den Lowangen-Schlüssel nicht, obwohl das Gespräch ihn übergibt.  
*Nachstellen:* 1. Den ersten Helden der Gruppe voll bepacken. 2. Das Gespräch bis zur Übergabe des Lowangen-Schlüssels führen. 3. Der Schlüssel fehlt, die Tür zum Geheimgang an der Burgruine bleibt verschlossen.  
*Warum unsicher:* Der Schlüssel wird mit give_hero_new_item direkt an den ersten handlungsfähigen Helden gegeben, das Ergebnis wird nicht geprüft. Ist dessen Rucksack voll oder die Traglast erreicht, gibt es keinen Schlüssel, g_eolan_state = 2 und der Tagebucheintrag stehen trotzdem. Der Geheimgang an der Burgruine (O125, group_find_key) bleibt dann zu. Ob das die Ursache der Meldung war, ist nicht belegt.  
*Im Netz bekannt:* ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt  

## Unsicher falsch, mittel (3)

### Helden & Werte

**Nr. 371: Stufenanstieg: Kampftalent steigert immer die Attacke**  
Steigert man beim Stufenanstieg ein Kampftalent, wächst nur die Attacke, die Parade nie.  
*Warum unsicher:* Der Stufenanstieg erhöht ein Kampftalent immer um 1 (add_skill_value(…, 1)). add_skill_value verteilt die Hälfte auf AT und PA und schlägt den ungeraden Rest der AT zu. Bei +1 bekommt die AT also immer +1 und die PA nie etwas. Eine Wahl gibt es nirgends. Damit steigt die PA durch Stufenanstiege nie, nur über den Grundwert. Ob Attic das so wollte, ist offen (nach den DSA-Regeln wählt der Spieler).  
*Im Netz bekannt:* ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt  

**Nr. 373: Weitere Importfehler (toter Held, Kampfwerte)**  
Beim Import aus der Schicksalsklinge kommen auch tote Helden mit, und Rüstungs- und Kampfwerte können verschoben sein.  
*Warum unsicher:* Teilweise bestätigt. (1) Toter Held: Die Importschleife prüft nur group_pos != 0 und npc_id == 0, nicht den Tod. Der Satz wird ab nr_items per memcpy (set_from_nr_items) übernommen, das dead-Bit also mit. (2) Kampfwerte: Derselbe memcpy übernimmt AT/PA-Werte, rs_bonus, rs_be und die Kampfmodifikatoren roh. check_hero setzt danach nur MR, Eigenschaften und den AT/PA-Grundwert neu (calc_atpa_base), Rüstungsschutz und BE der angelegten Rüstung werden nicht neu berechnet. Die Amulett-MR (-5) und das Travia-Amulett ließen sich ohne DSA-1-Spielstand nicht klären.  
*Im Netz bekannt:* ja: im Netz gemeldet, bei der Rekonstruktion am Code bestätigt  

### Dungeons

**Nr. 13: Höhlenspinnen kommen bis zur Nestzerstörung wieder**  
Die Höhlenspinnen im Orkgefängnis greifen bei jedem Betreten des Feldes erneut an, bis das Nest zerstört ist.  
*Warum unsicher:* Feld 0x124 startet event_fight2 (Hoehlenspinnen 0x26/0x27) ohne ret = 1 und ohne Merker. Der Merker g_dng07_nest_destroyed wird aber sehr wohl gesetzt, und zwar in O264 beim Zerstoeren des Nests an 0x225 – die alte Beschreibung ist da falsch. Absicht moeglich: die Spinnen kommen nach, bis das Nest zerstoert ist. Echt fehlerhaft ist nur die Rueckgabe 0 trotz Kampf (wie bei den Schroeterfeldern 0x1b14../0x1d17../0x171c.. und 0xa0d).  
*Im Netz bekannt:* nein  

## Sicher falsch, egal (134)

### Kampf

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 29 | Kampfbewegung: Animationsfolge ohne Längengrenze | Praktisch nichts; nur ein konstruierter Laufweg könnte die Kampfanimation stören. | nein |
| 38 | Kampffeld: Belegt-Prüfung ohne Wirkung | Nichts Sichtbares. | nein |
| 41 | Paradepatzer nennt den falschen Helden | Patzt ein verzauberter Held bei der Parade gegen einen anderen Helden, nennt der Kampftext den Angreifer. | nein |
| 42 | Patzerfolge ohne Schaden geprüft | Nichts Sichtbares. | nein |
| 49 | Sonderkampf: Lesen vor dem Nulltest | Nichts Sichtbares. | nein |
| 54 | Gegnerzauber bei leerer Zauberliste | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 75 | Meldung „versteckte Gegner“ zählt manchmal falsch | Selten fehlt ein Gegner in der Meldung über versteckte Gegner. | nein |
| 76 | Sumpfrantzendokument gegen Gegner ohne Meldung | Benutzt man das Sumpfrantzendokument im Kampf gegen einen Gegner, fehlt der Hinweis, dass es nichts nützt. | nein |
| 90 | Kampf-Figurenliste: mehrere Randfehler | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 147 | Heldenaktion mit unbekannter Zielart | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 168 | Kampfaufbau: leere Prüfung | Nichts Sichtbares. | nein |
| 200 | Automatische Zielwahl bevorzugt eine Seite | Die automatische Zielwahl im Kampf bevorzugt leicht eine Seite. | nein |

### Magie

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 48 | Brenne toter Stoff ohne Fackel: Meldung zufällig | Ohne Fackel oder Lampe erscheint beim Zauber mal die Hinweismeldung, mal passiert gar nichts. | nein |
| 148 | Nichtzauberer können das Zaubermenü öffnen | Auch Helden ohne Magie können im Heldenbogen das Zaubermenü öffnen, scheitern dann aber. | nein |
| 163 | Heldenzauber: Lesen hinter der Tabelle | Nichts Sichtbares. | nein |

### Helden & Werte

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 107 | Heldenerschaffung: uninitialisierte Werte | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 116 | Auto-Erschaffung: Akademiezauber doppelt bedacht | Bei der automatischen Heldenerschaffung werden manche Akademiezauber bevorzugt gesteigert. | nein |
| 152 | Alchimie/Schätzen: kleine Randfehler | Kaum sichtbar; Schätzen kann einen Preis von 0 nennen. | nein |
| 178 | Erschaffung: Grenzwerte uneinheitlich verglichen | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 201 | Startausrüstung: Lesen hinter dem Eintrag | Nichts Sichtbares. | nein |
| 365 | MR der NSC falsch (Korima, Thurazz) | Korima und Thurazz zeigen zu Beginn eine andere MR als nach dem ersten Kraut oder Amulett. | ja |
| 372 | Nichtzauberer bekommen Zaubersteigerungen angeboten | Ein Nichtzauberer, der in der Erschaffung zuerst als Zauberer angelegt war, bekommt beim Stufenanstieg Zauberabfragen. | ja |

### Gruppe & Inventar

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 15 | Schlüsselbund/Rezeptbuch: falsche „schon drin“-Meldung | Kombiniert man einen unpassenden Gegenstand mit Schlüsselbund oder Rezeptbuch, heißt es manchmal fälschlich, er sei schon darin. | nein |
| 26 | Gruppe entfernen ändert fremdes Byte | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 52 | Gegenstandssuche über alle Plätze kaputt | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 61 | Gruppe teilen bei sechs Gruppen | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 81 | Zufälliger Gegenstandsverlust fehlerhaft | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 82 | Gegenstand an jeden Helden: Restmenge falsch | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 83 | Gruppenverwaltung schreibt hinter das Feld | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 95 | Leeres Zunderkästchen: doppelte oder alte Meldung | Benutzt man ein leeres Zunderkästchen, erscheint die Meldung doppelt oder ein alter, unpassender Text. | nein |
| 131 | Starker Zaubertrank nennt Lebenspunkte | Beim starken Zaubertrank spricht die Meldung von Lebens- statt Astralpunkten, und beim Lotosgift bleibt keine leere Flasche zurück. | nein |
| 169 | Gegenstand benutzen: fehlende Standardfälle | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 171 | Heldensuche: ungenutzte fehlerhafte Funktion | Nichts Sichtbares. | nein |
| 219 | Gruppenwechsel überspringt NSC-Gruppen nicht | Beim Gruppenwechsel werden Teilgruppen, in denen nur noch der NSC handlungsfähig ist, nicht übersprungen. | nein |
| 355 | Gefangennahme: Ringe und Amulette vervielfacht | Im Original gehen die Sachen ohnehin verloren (Nr. 212), nach einer Korrektur kämen Ringe und Amulette mehrfach zurück. | ja |

### Händler & Dienste

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 21 | Händler-Warenliste überlappt bei riesigem Sortiment | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 85 | Laden-Warenliste überlappt bei riesigem Sortiment | Nichts – der Fehler tritt im Spiel nicht auf. | nein |

### Städte & Orte

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 11 | Marktstand mit unbekanntem Feld | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 55 | Fähre Hilvalla: zu wenig Geld zeigt Begrüßung erneut | Hat man an der Fährstation in Hilvalla zu wenig Geld, erscheint beim nächsten Mal wieder der lange Begrüßungstext. | nein |
| 158 | Schenke: leerer Text, merkwürdiger Rundenzähler | Nichts Sichtbares. | nein |
| 184 | Straßenüberfall: Lesen hinter der Liste | Nichts Sichtbares. | nein |

### Reise & Ereignisse

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 101 | Kräuterstelle an Weg 41: Zähler ohne Grenze | Der Wirselkraut-Fund an einem Weg bei Finsterkoppen wird mit jeder Durchreise schwerer, nach sehr vielen Durchreisen schwankt die Schwierigkeit. | nein |
| 112 | Kriegerhüne: Wegetexte unnötig neu geladen | Nichts Sichtbares. | nein |
| 142 | Lagerplatz: Suche liest hinter der Tabelle | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 150 | Fähre: Zufallswert an unbekanntem Ort | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 222 | Steinschlag: danach evtl. falsche Wegtexte | Nach dem Freigraben eines Steinschlags können weitere Ereignisse auf demselben Wegstück falsche Texte zeigen. | nein |
| 226 | Wasserfall über Weg 25 mit falschen Texten | Nichts – der Fehler tritt im Spiel nicht auf. | nein |

### Dungeons

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 23 | Versiegelte Schlösser: Erleichterung wirkungslos | Nichts Spürbares; eine vorgesehene Erleichterung bei zugezauberten Schlössern greift nie. | nein |
| 31 | Ortswechsel überschreibt den gemerkten alten Ort | Nichts Sichtbares. | nein |
| 60 | Teleport mit unbekannter Herkunft | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 89 | Kobold Mumpitz: Auswahl liest außerhalb | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 104 | Altsvellt-Sumpf: alte Ebene überschrieben | Nichts Sichtbares. | nein |
| 105 | Spinnennetze: Schreiben nach Adresse 0 | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 115 | Lore: Lesen vor der Gültigkeitsprüfung | Nichts Sichtbares. | nein |
| 133 | Binge: Position doppelt gesetzt | Nichts Sichtbares. | nein |
| 164 | Ingerimms Schmiede: Schreiben hinter das Feld | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 175 | Zwergenskelett: falsches Pronomen | Bei einer misslungenen Probe am Zwergenskelett in der Orkfestung steht ein falsches Pronomen im Text. | nein |
| 205 | Totenkopf-Ornament: „%s“ statt Heldenname | Am Totenkopf-Ornament im Gewölbe unter Tjolmar steht in einem Text „%s“ statt des Heldennamens. | nein |
| 218 | Unratfund heißt immer „einen Dolch“ | Im Unrathaufen des Orkgefängnisses heißt jeder Fund „einen Dolch“, obwohl man auch Schmuck, einen goldenen Schlüssel oder eine Bronzeflasche erhält. | ja |
| 366 | Tierkunde an den Spinnennetzen invertiert | Vor den Höhlenspinnen wird gewarnt, wenn jemand die Tierkunde-Probe verpatzt, und nicht, wenn sie gelingt. | ja |
| 376 | Textfehler „Rechstschritt“ | Ein Truhentext in den Blutzinnen schreibt „Rechstschritt“. | ja |

### Dialoge

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 67 | Dialogskript: Zeiger nicht genullt | Nichts Sichtbares. | nein |
| 227 | Dialogskript: Vergleich mit leerem Feld | Nichts – der Fehler tritt im Spiel nicht auf. | nein |

### Oberfläche

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 3 | Klickflächen-Prüfung tauscht falsche Werte | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 7 | Karte aufdecken: Grenzprüfung doppelt | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 10 | Heldenvorauswahl steht manchmal auf leerem Platz | In manchen Heldenauswahlen steht der Cursor auf einem leeren Platz statt auf dem passenden Helden. | nein |
| 22 | Automap findet sehr weite Wege nicht | Ein Klick auf ein sehr weit entferntes Ziel der Automap ergibt „kein Weg“. | nein |
| 24 | Warenauswahl: Pfeil links springt an den Anfang | In der Warenauswahl springt der Cursor mit Pfeil links aus der ersten Spalte nach oben links. | nein |
| 27 | Texteingabe: fehlende Endnull | Nichts Sichtbares. | nein |
| 36 | Reisekarte: Richtungswahl ohne Rückgabewert | Nichts Sichtbares. | nein |
| 44 | Zahleneingabe zeigt falsche erste Ziffer | In manchen Zahlenfeldern erscheint bei großen Vorgabewerten ein unsinniges Zeichen als erste Ziffer. | ja |
| 46 | Reisekarte: Wegsuche liest hinter der Tabelle | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 47 | Reisekarte: Wegzeichnung im Randfall falsch | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 53 | Porträtleiste einer Teilgruppe zu breit/schmal | Bei geteilter Gruppe wird die Porträtleiste einer nicht aktiven Gruppe manchmal in falscher Breite gezeichnet. | nein |
| 56 | Tagebuch drucken: Überlauf bei langen Zeilen | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 57 | Tagebuch-Heldennamen: Lesen hinter dem Feld | Nichts Sichtbares. | nein |
| 58 | Ortssymbole: unvollständige Änderungsprüfung | Höchstens ein überflüssiges Neuzeichnen der Symbolleiste. | nein |
| 62 | Porträt ziehen liest vor der Tabelle | Nichts Sichtbares. | nein |
| 65 | Genitiv-s bei Namen mit Sonderzeichen falsch | Bei Heldennamen mit Sonderzeichen (außer ä, ö, ü) steht das Genitiv-s falsch oder fehlt. | nein |
| 69 | Kartenrätsel: toter Programmcode | Nichts Sichtbares. | nein |
| 80 | Reisekarte: Position im Randfall falsch | In einem Randfall wird die Gruppe auf der Reisekarte an einer falschen Stelle gezeigt. | nein |
| 97 | Kompassrose verschwindet im Randfall | In seltenen Fällen ist die Kompassrose nicht zu sehen. | nein |
| 103 | Tagebuchsuche: Esc sucht statt abzubrechen | Drückt man in der Tagebuchsuche Esc, startet eine Suche, statt abzubrechen. | nein |
| 108 | Auswahlliste ohne Grenzprüfung | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 136 | Fenster springt beim Verschieben | Beim Ziehen eines Fensters springt es an eine falsche Stelle. | nein |
| 162 | Reisekarte: Ortssuche ohne Grenze | Nichts – der Fehler tritt im Spiel nicht auf. | nein |

### Klang & Grafik

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 9 | Bild-Entpacker liefert Zufallswert zurück | Nichts Sichtbares. | nein |
| 16 | Einzelne Pixel in falscher Farbe | Vereinzelt erscheinen Bildpunkte in einer leicht falschen Farbe. | nein |
| 50 | Phex-Tempel: nur eine von vier Flächen animiert | Im Phex-Tempel bewegt sich von vier animierten Wandflächen nur eine. | nein |
| 64 | Eine Gegner-Animation wird nie gezeigt | Eine Schuss- bzw. Zauberanimation der Gegner fehlt. | nein |
| 68 | Klangtreiber: kleines Speicherleck | Nichts Sichtbares; beim Neuanlegen der Klänge geht etwas Speicher verloren. | nein |
| 73 | Kampfanimation: über 40 Animationen gleichzeitig | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 74 | Bildklasse: Randfehler ohne Aufrufweg | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 87 | Gegenstände scheinen durch Wände | In der 3D-Ansicht sieht man in drei von vier Blickrichtungen manchmal Objekte durch Wände. | nein |
| 92 | Hintergrundanimation zeigt evtl. falsches Teilbild | Höchstens ein falsches Einzelbild in einer Hintergrundanimation. | nein |
| 125 | Zwischensequenz: Überblendung zufällig | In einer Zwischensequenz ist die Überblendung mal schneller, mal langsamer. | nein |
| 132 | Ortsszene: Musikwahl zufällig | In einer Ortsszene kann die falsche Musik laufen. | nein |
| 139 | Kellertür: Wandbild bei nicht quadratischer Karte | Nur auf nicht quadratischen Karten erscheint ein Türbild im Keller versetzt. | nein |
| 141 | Animation: hängender Zeiger bei fehlender Datei | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 167 | Sonderkampf-Sequenzen: doppeltes Freigeben | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 172 | Intro: Speicher-Merker nicht gesetzt | Nichts Sichtbares. | nein |
| 176 | Reisekarte: Grafikfehler beim Hochscrollen | Beim Hochscrollen der Reisekarte entstehen Grafikfehler. | nein |
| 181 | Kampf: einige Klänge fehlen | Bei einigen Kampfereignissen fehlen die Klänge. | nein |
| 189 | Sonderkampf-Animation läuft falsch | Eine Animation im Sonderkampf zeigt ein falsches Feld. | nein |
| 190 | Abspann: eine Zeile fehlt mit Sprachausgabe | Mit Sprachausgabe fehlt im Abspann eine Zeile. | nein |
| 195 | Abblenden: Palette nur teilweise gelöscht | Nichts Sichtbares. | nein |
| 198 | Ohne EMS: falsche Pixel im Bildschirmrahmen | Ohne EMS-Speicher erscheinen nach dem Auslagern einige falsche Pixel unten links im Bildschirmrahmen. | nein |
| 202 | 3D-Objekte: eine Palettenfarbe zu viel | Nichts Sichtbares. | nein |
| 211 | Bild laden mit zu kleinem Puffer (ungenutzt) | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 217 | Bild-Entpacker im Speicher: Zufallswert | Nichts Sichtbares. | nein |
| 224 | Kampffiguren am Rand ungleich abgeschnitten | Kampffiguren am Bildrand werden ungleichmäßig abgeschnitten. | nein |
| 300 | DIGPAK-Klangtreiber: Stapelfehler (ungenutzt) | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 301 | Zeichenart „Oder“ kopiert falsch (ungenutzt) | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 302 | Rückwärtskopieren versetzt (ungenutzt) | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 350 | Ohne EMS vereinzelt falsche Bodenpixel | Ohne EMS-Speicher können in der 3D-Ansicht vereinzelt Bodenpixel falsch sein. | nein |
| 351 | Tiefensortierung bei 129 Flächen | Nichts – der Fehler tritt im Spiel nicht auf. | nein |

### Technik & Speicher

| Nr. | Fehler | Was der Spieler merkt | Bekannt |
|-----:|-------------------------|---------------------------------|----------|
| 17 | Speicherleck beim Verlassen der 3D-Ansicht | Nichts Sichtbares; bei jedem Verlassen geht ein wenig Speicher verloren. | nein |
| 33 | Kalender: falscher Tagesüberlauf | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 34 | Kalender: Tick-Überlauf verliert Restzeit | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 35 | Kalender: Jahreswechsel ergibt Monat 13 | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 86 | CD-Zwischenspeicher bei Lesefehlern kaputt | Nur bei einer fehlerhaften CD kann eine kaputte Zwischenspeicher-Datei entstehen. | nein |
| 88 | Wegetexte: hängende Zeiger | Nichts Sichtbares. | nein |
| 91 | Dateizugriff: Handle-Leck und späte Fehlerprüfung | Nur bei Lesefehlern der Spieldaten können falsche Daten geladen werden. | nein |
| 209 | Speicherfehler bei über 160 MB XMS | Nur auf Rechnern mit mehr als 160 MB XMS-Speicher wird beim Start Speicher überschrieben. | nein |
| 210 | Textdateien laden ohne Fehlerprüfung | Nur bei beschädigten Spieldateien können Texte falsch geladen werden. | nein |
| 225 | Absturzbericht liest über den Speicherblock | Nur nach einem Absturz kann der Absturzbericht einen Folgeabsturz auslösen. | nein |
| 230 | CD-Prüfung bei leerer PATH.CFG | Bei leerer Konfigurationsdatei meldet das Spiel meist, es gebe kein CD-ROM-Laufwerk. | nein |
| 231 | CD-Prüfung bricht ohne Aufräumen ab | Nichts Sichtbares. | nein |
| 232 | Speichern: Existenzprüfung mit > statt >= | Nichts Sichtbares. | nein |
| 303 | Debug-Wächter zeigt immer Rot (toter Code) *(kein Fehler)* | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 304 | XMS-Vorgabe ohne Rücksprung | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 305 | XMS-Kopie liest Einsprung aus falschem Segment | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 306 | Flat-Modus ohne Fehlerprüfung | Nichts – der Fehler tritt im Spiel nicht auf. | nein |
| 307 | Kopierroutine verliert ungerades Byte | Nichts – der Fehler tritt im Spiel nicht auf. | nein |

## Unsicher falsch, egal (16)

### Kampf

**Nr. 40: Parade Held gegen Held (kein Fehler)**  
Nichts, die Parade rechnet richtig.  
*Warum unsicher:* p2 = hero_mr_target(att) ist kein Fehler: hero_mr_target liefert das ZIEL des Angreifers (enemy_id), im Kampf Held gegen Held also den Verteidiger – genau wie e1/e2 = enemy_target_enemy(att) in O160 und wie DSA1. Die Parade selbst rechnet ohnehin mit def (hero_pa(def)).  
*Im Netz bekannt:* nein  

**Nr. 96: Kampfbeginn: Hilfsvariable doppelt benutzt**  
Nichts Sichtbares.  
*Warum unsicher:* fight_quick_start uebergibt `&f->pic_w, &f->pic_w` als Breite und Hoehe an nvfile_load; die Hoehe wird nirgends gebraucht, zweimal dasselbe Muellziel kann Absicht sein (wie Nr. 19). Die alte Vermutung „gemeint &f->f1a1“ ist falsch: +0x1a1 ist old_dlg_mode (O022/O143 dlg_set16) – dorthin zu schreiben wuerde den Dialogmodus zerstoeren. f4/f8 werden ohne Nullen freigegeben, danach aber nicht mehr benutzt.  
*Im Netz bekannt:* nein  

**Nr. 113: Gegnerpatzer gegen bewusstlose Helden entfallen**  
Patzt ein Gegner gegen einen bewusstlosen Helden, hat das keine Folgen.  
*Warum unsicher:* O161: Patzer des Gegners werden nur ausgewertet, wenn check_hero(h) fuer den angegriffenen Helden gilt; Patzer 12 ist ein leerer Zweig (wie O160). Kann Absicht sein (gegen einen bewusstlosen Helden wird nicht „gekaempft“); die Wirkung ist gering.  
*Im Netz bekannt:* nein  

### Helden & Werte

**Nr. 66: DSA-1-Import: doppelte Prüfung**  
Nichts Sichtbares.  
*Warum unsicher:* DSA-1-Import: `len < 0x6da && len < 0x6ca` (erste Haelfte wirkungslos, Ergebnis aber korrekt) und der Block „0xd7 → Platz 8“ steht zweimal. Ob beim zweiten Block ein anderer Gegenstand gemeint war, laesst sich nicht belegen – es kann auch ein harmloses Duplikat sein.  
*Im Netz bekannt:* nein  

### Gruppe & Inventar

**Nr. 19: NSC aufnehmen: Hilfsvariable doppelt benutzt**  
Nichts Sichtbares.  
*Warum unsicher:* nvfile_load bekommt `&x, &x` als Breite/Hoehe-Ziel. x wird danach nie gelesen, es ist nur ein Muellziel – zweimal dieselbe Dummy-Variable kann Absicht sein (wie O220 rose_alloc). Keine Wirkung.  
*Im Netz bekannt:* nein  

### Händler & Dienste

**Nr. 70: Heiler nennt Krankheit vor der Zahlung (gewollt)**  
Nichts Fehlerhaftes; der Heiler nennt Krankheit oder Gift, auch wenn man danach ablehnt.  
*Warum unsicher:* cure_disease/cure_poison setzen das Merkbit „diagnosed“ vor der Rueckfrage. Das ist kein „gilt als behandelt“: der Heiler nennt die Krankheit bzw. das Gift im Fragetext (txt[11] mit g_ttx[k + 43]), danach ist sie bekannt. Folgerichtig und wohl gewollt.  
*Im Netz bekannt:* nein  

### Städte & Orte

**Nr. 100: Schenke: Andrang nach Mitternacht (unklar)**  
Zwischen Mitternacht und 2 Uhr gelten Schenken als ruhig.  
*Warum unsicher:* Tavern::set_crowd: `t < 12 Uhr // (t >= 2 Uhr && t < 3 Uhr)` (bzw. 17 Uhr) – die zweite Teilbedingung ist in der ersten enthalten. Was gemeint war, ist offen (z. B. Andrang nach Mitternacht bis 2 Uhr); denkbar ist auch ein harmloser Rest. Folge nur: 0–2 Uhr gilt als ruhige Zeit.  
*Im Netz bekannt:* nein  

**Nr. 156: Hafen-Feilschen (kein Fehler)**  
Nichts, die Menüs des Feilschens stimmen.  
*Warum unsicher:* port_haggle: das „− 1“ bei res == 1 ist gewollt – dieses Menue hat als ersten Punkt TIEFHUS[44] „Dann eben gar nicht“ (→ 0, nichts), danach [45]/[46], die mit −1 genau auf dieselben Werte 1/2 fallen wie [45]/[46] in den anderen Menues. Das lokale Textobjekt (dieselbe Datei 0x21d) und `v < 0 → 1` (0 Dukaten moeglich) sind harmlos.  
*Im Netz bekannt:* nein  

### Reise & Ereignisse

**Nr. 30: Reiseereignis Weg 136: gewollter Durchlauf**  
Nichts, das Verhalten ist gewollt.  
*Warum unsicher:* tevent_136: case 1 faellt bewusst in case 2: ohne Fernwaffe fragt case 1 erneut („anpirschen“ setzt r = 1), case 2 wertet dann r == 1 aus; nach gelungenem Schuss ist r = -1 und case 2 tut nichts. Das ist erkennbar gewollt, kein Fehler.  
*Im Netz bekannt:* nein  

**Nr. 223: Orkschützen an Weg 250 (kein Fehler)**  
Nichts; bei kleiner Gruppe werden dieselben Helden mehrfach getroffen, was gewollt sein kann.  
*Warum unsicher:* tevent_250: get_next_hero liefert nie 0 (die Schleife sucht bis zu einem belegten Platz und laeuft bei nr_heroes auf 0 um) – der beschriebene Nullfall tritt nicht ein. Bei kleiner Gruppe werden dieselben Helden mehrfach getroffen, was bei „vier Orks schiessen“ gewollt sein kann.  
*Im Netz bekannt:* nein  

### Dialoge

**Nr. 137: Eolan-Ereignis (kein Fehler)**  
Nichts, das Ereignis funktioniert.  
*Warum unsicher:* Ev::eolan case 1: `talk(..., 0x90, 1, 0); if (g_eolan_state == 2) group_leave_place(1);` ist sehr wohl erreichbar – die Dialogaktion talk_give_lowangen_key (O013) setzt waehrend des Gespraechs g_eolan_state = 2. Die alte Einschaetzung „nie wahr“ stimmt nicht, kein Fehler.  
*Im Netz bekannt:* nein  

### Oberfläche

**Nr. 28: Texteingabe: Rückgabewert fehlt**  
Nichts Sichtbares.  
*Warum unsicher:* insert_char: `if (++col == old) return 1;` – old ist col vor dem Erhoehen, waere nur wahr, wenn wrap(line) col um eins zuruecksetzt (denkbar, daher unsicher). Der einzige Aufrufer (O110) wertet die Rueckgabe nicht aus.  
*Im Netz bekannt:* nein  

**Nr. 99: Dialogfenster: Randfälle ohne Wirkung**  
Nichts Sichtbares.  
*Warum unsicher:* Dlg::choose verwirft bei n == 0 das Ergebnis von input_text und liefert 0; mit n == 0 dient choose aber nur als Meldungsfenster (z. B. tevent_097), die Rueckgabe wird nicht ausgewertet – vermutlich gewollt. num_loop mit nd == 0 (bx2 uninitialisiert) wird nicht benutzt.  
*Im Netz bekannt:* nein  

### Klang & Grafik

**Nr. 221: Finale: Überblendung von Schwarz (gewollt)**  
Nichts Fehlerhaftes; die Überblendungen im Finale laufen über Schwarz.  
*Warum unsicher:* Finale::fade_wait: pal_from wird angelegt, aber nie gefuellt. Es kommt aus new (farcalloc, genullt) und ist damit eine schwarze Palette – die Ueberblendungen laufen von bzw. nach Schwarz, was durchaus gewollt aussieht. Kein Flackern.  
*Im Netz bekannt:* nein  

### Technik & Speicher

**Nr. 228: Handbuchabfrage in der CD-Fassung ohne Wirkung**  
Nichts; die CD-Fassung fragt nie nach dem Handbuch.  
*Warum unsicher:* CopyProt (SEG003): ok = Zufall 3..21, copyprot_check beendet nur bei `!ok2 && !ok`, die Abfrage CopyProt::query wird in der CD-Fassung nie aufgerufen. Sehr wahrscheinlich Absicht (Schutz ueber cd_check), der Code ist nur Rest.  
*Im Netz bekannt:* nein  

**Nr. 229: Zeitgeber prüft denselben Wert doppelt**  
Nichts Sichtbares.  
*Warum unsicher:* timer_isr: `(loctype = g_group->loctype) != 0 && loctype != 0` prueft denselben Wert zweimal. Ob beim zweiten Test ein anderer Wert gemeint war, ist nicht belegbar; so ist er nur ueberfluessig.  
*Im Netz bekannt:* nein  

## Anhang: Durchgesehene Quellen im Netz

| Nr | Sammlung | URL | Stand |
|---|---|---|---|
| S1 | Crystals-DSA-Foren, Thread „Bugs in Sternenschweif“ (tid=46), alle 12 Seiten, 2006–2020 | https://www.crystals-dsa-foren.de/showthread.php?tid=46 und printthread.php?tid=46&page=1…12 | gelesen |
| S2 | „Korrektur für Fehler Leere Truhen in Blutzinnen“ (NRS 2011) | https://www.crystals-dsa-foren.de/showthread.php?tid=2981 | gelesen |
| S3 | „Sternenschweif: Finaler Truhenpatch für Blutzinnen“ (NRS 2016, mit Liste „bekannt, nicht behoben“) | https://www.crystals-dsa-foren.de/showthread.php?tid=5183 | gelesen |
| S4 | „nicht ganz alltägliche Tips und Tricks“, Seite 4 von 7 (Lumpi-Sammlung 2007) | https://crystals-dsa-foren.de/showthread.php?page=4&tid=84 | nur Seite 4 gelesen |
| S5 | „NLT Patches von attic“ (offizielle Patches DSAPAT01–04, Changelog) | https://www.crystals-dsa-foren.de/showthread.php?tid=862 | gelesen, LIESMICH selbst nicht zitiert |
| S6 | Themen-Threads: Geld (tid=2524, 121), AP-Farm (5386), Depot (949), verdeckte Proben (4404 S. 1/2/4), Kräuter/Gifte (4018), Hexenspeichel (2812), Wunder (5131, 1126, 4409), Reiseereignisse (477), Steinbock (3042), Phex-Tempel (73 S. 4/10, 1248), Tempel des Namenlosen (157 S. 4), Import (116 S. 7), Unterschiede (2075), Kurioses (5134), Patzer (5363), Krankheiten (pid=144275), Abstürze (1824, 1855) | crystals-dsa-foren.de | gelesen |
| S7 | Englisch: bigorrin.org Walkthrough und Linkseite | https://www.bigorrin.org/star2.htm , http://www.bigorrin.org/star3.htm | gelesen |
| S8 | GOG-Forum: „Kid faces bug“, „Old Woman is not there“ | https://www.gog.com/forum/realms_of_arkania_series/kid_faces_bug | gelesen |
| S9 | Wikipedia (en), wiki-aventurica Komplettlösung, GameSpot Cheats, PC Player 01/1995 (archive.org) | https://en.wikipedia.org/wiki/Realms_of_Arkania:_Star_Trail , https://de.wiki-aventurica.de/wiki/Sternenschweif_(1994)/Komplettl%C3%B6sung | gelesen, praktisch keine Bugs |

**Nicht erreichbar:** NLT-Wiki (nlt-wiki.crystals-dsa-foren.de, u. a. „Die tödlichen Fallen“, Phex-Tempel-Karte) und nlt-hilfe.crystals-dsa-foren.de sperren den Abruf per robots.txt. Kunars Reiseführer (muenster.de/~kunar/dsa2/) wurde nicht freigegeben. CRPG-Addict-Blog (Star Trail, 2026) hängt in einer Weiterleitungsschleife. xeen.de-Lösung ebenso. Archivansichten des Forums (archive/index.php) liefern nur die Navigation. kultboy.com und GitHub (Bright-Eyes) sind aus dem Container gesperrt. Steam-Threads betreffen nur das HD-Remake. Nicht geprüft: die Seiten 2–48 von „Reverse Engineering der NLT II“ (tid=5383), die übrigen Seiten von tid=84 und Spielezeitschriften außer PC Player 01/95.

## Anhang: Meldungen aus dem Netz, am Code geprüft

45 Fehlermeldungen von Spielern aus Foren, die bei uns zunächst fehlten, wurden am Quelltext geprüft. Die bestätigten stehen oben in der Liste (Nr. 352 ff., Spalte „Bekannt“ = ja).

| Ergebnis | Anzahl |
|---|---:|
| bestätigt | 25 |
| anders | 4 |
| widerlegt | 7 |
| unklar | 9 |

| Meldung | Ergebnis | Erklärung |
|------------------|-------------|------------------------------------------|
| Korima: verkaufte Waffen bleiben im Gepäck | widerlegt | Wird Korima (NSC, npc_id 7) über die Heldenwahl gewählt, setzt shop_sell_screen s->hero = 6, und Shop::trade nimmt für i == 6 ausdrücklich group_get_npc (Z. 351) und entfernt die Ware mit drop_item. Schwert und Speer liegen bei Korima im Rucksack (Platz 20 und 17, KORIMA.NPC) und sind normale, ablegbare Gegenstände. Der Laden nimmt sie also korrekt ab. Ein ähnliches Symptom gibt es aber im Lagerhaus: store_deposit_commit (O099 Z. 311/312) nimmt für den NSC g_group->heroes[6] statt group_get_npc. Der NSC steht nach group::add_hero immer auf dem letzten belegten Platz, bei weniger als sechs Helden ist heroes[6] leer. Dann kopiert das Spiel Speicherinhalt ab 0000:01A8 als Gegenstand ins Depot, drop_item schreibt in die Interrupttabelle, und der NSC behält seine Sachen. Vermutlich liegt hier eine Verwechslung vor (siehe Nr. 352). |
| Lagerhaus: Waren kehren zurück, Lagerkosten explodieren | bestätigt (Nr. 352) | Drei Fehler wirken zusammen. (1) store_deposit beginnt mit s->value = depot->value und zählt die neue Ware dazu, store_deposit_commit addiert dann s->value noch einmal auf depot->value: Bei jedem Einlagern verdoppelt sich der gespeicherte Warenwert (neu = 2 x alt + Zugang). Die Tagesgebühr ist Tage x (Wert / 300 + 1) Heller, sie wächst also exponentiell mit der Zahl der Einlagerungen. (2) check_fee zeigt nur „Ihr bezahlt Eure Schulden“ (DEPOT.LTX[37]), zieht aber nie Geld ab und setzt den Zeitstempel nicht zurück. Ist die Gebühr höher als das Gruppengeld, sperrt „Zahlt erst einmal Eure Schulden!“ das Lager für immer. Beim Abholen rechnet store_fetch_commit zudem depot->value -= Wert des Rests statt Wert = Rest. (3) Gegenstände, die store_fetch_check als verkauft oder zerbrochen meldet (take_qty = -1), werden nie entfernt: store_fetch_commit ruft group_give_item(id, -1), das 0 liefert, und übernimmt den Platz in den Rest. store_fetch_fill zeigt außerdem nur die ersten count = nr_items - verloren Plätze. Beim nächsten Besuch tauchen die „verlorenen“ und die ausgeblendeten Waren wieder auf. Latent: take_qty/sel_qty sind signed char (ab 128 Stück Überlauf, bei Stapeln bis 99 nicht erreichbar). Nebenbefunde: Einlagern beim NSC liest und beschreibt g_group->heroes[6] (bei kleiner Gruppe Nullzeiger), und ein Klick auf ein leeres Porträt setzt s->hero, ohne es zurückzunehmen (O099 Z. 239-252), dann wird beim falschen Helden abgebucht. |
| Holzfällerlager: Abbrechen legt ein Stück dazu | bestätigt (Nr. 353) | Bei mehr als einem Stück fragt dlg_input nach der Menge. Rechtsklick/ESC liefert -1. Die Prüfung „loot[i].n < amount“ ist dann falsch, also folgt „loot[i].n -= amount“ (n + 1) und group_give_item(id, -1), das nichts gibt. Jeder Abbruch erhöht den Vorrat im Lager um eins, die Datei FEAT049.DAT wird am Ende so zurückgeschrieben. Mit dem Vorrat lassen sich danach beliebig viele Decken, Äxte und Trockennahrung mitnehmen. Ab 127 kippt der signed-char-Zähler ins Negative und der Eintrag verschwindet. |
| Phexhelm und Phexschild vervielfachen | bestätigt (Nr. 354) | Nach 2000 Takten sucht der Poltergeist Helm und Schild nur in der aktiven Gruppe (group_find_item(g_group, …)) und nimmt sie mit group_drop_item dort weg. Danach legt er unabhängig davon neue Exemplare in Truhe 0x30 (ch->set_item(ITEM_PHEX_HELMET/SHIELD)). Trägt ein Held einer abgetrennten Gruppe die Teile, findet er sie nicht (Text PHEXTEMP[66] „Ihr seid schlauer …“), der Held behält sie, und die Truhe ist wieder gefüllt. |
| Gefangennahme: Ringe und Amulette vervielfacht | bestätigt (Nr. 355) | Zurgrimms Vermutung stimmt. Die Ablage speichert je Gegenstand nur id und Anzahl (struct stash_entry). O263 trägt als Anzahl h->inv[k].get_quantity() ein, bei nicht stapelbaren Gegenständen mit Ladungen (Ringe, Stirnreif, Amulett, Rezeptbuch) steht dort aber die Zahl der Ladungen. Stash::menu gibt beim Herausnehmen so viele Exemplare per group_give_item aus. Im unveränderten C1.02 ist das verdeckt, weil Nr. 212 (Zähler n bleibt 0) alle Ablagen leert. Sichtbar wird es erst mit einem Fanpatch, der Nr. 212 behebt (S2/S3). |
| Geld verschwindet beim Vereinen von Gruppen | bestätigt (Nr. 356) | Wechselt ein Held die Gruppe, gibt remove_hero ihm money / nr_heroes mit, und add_hero bucht es per add_money in der neuen Gruppe ein. nr_heroes zählt aber den NSC mit. Zieht der NSC selbst um (Vereinen von der NSC-losen Teilgruppe aus, Flucht/Versprengung per make_group, Umkehr an den heißen Platten O001 Z. 409), überschreibt set_money(share) sein eigenes Geld mit seinem Anteil, add_hero bucht für NSC nichts ein (if (!x->npc_id)): dieser Anteil ist weg. Darum hängt der Verlust davon ab, von welcher Teilgruppe aus vereint wird (a). Flieht Helen in einer Siebenergruppe allein, verliert die Gruppe genau 1/7 (c). Bei zwei Helden plus NSC ist es die Hälfte (b). Zusätzlich zweigt add_money beim Einbuchen in eine Gruppe mit NSC dessen money_share (bei allen NSC 1 %) ab. |
| Asthenilschwert per ESC behalten | bestätigt (Nr. 357) | Bei der Frage „tauschen oder einfach mitnehmen?“ liefert ESC r = -1. stash_show läuft trotzdem, man kann den Salamanderstein aus der Ablage nehmen, und g_dng16_stone_taken wird gesetzt. Da r weder 1 noch 2 ist, bleibt g_dng16_artifact_traded 0 und es gibt keinen Frevel (g_dng16_sacrilege). Ist der Golem fertig, gibt er das Asthenilschwert aus (txt[71], weil nicht getauscht). Ergebnis: Stein und Schwert ohne Frevel. Verdoppeln: Wählt man „tauschen“, während ein Held einer anderen Gruppe das Schwert trägt, findet group_find_item(g_group, …) es nicht, der Tausch gilt trotzdem, und Ablage 0x3e (enthält laut SPOIL.DAT ein Asthenilschwert) gibt ein zweites heraus (dann mit Frevel). |
| Taschendiebstahl beim fahrenden Händler | widerlegt | Die fahrenden Händler gibt es nur als Ereignisorte in Lowangen (Ev::peddler_redhead/falkert, Ortsart 17 LOC_SPECIAL). select_skill bietet Taschendiebstahl nur in Schenken mit Andrang und in Läden (LOC_MERCHANT) an, beim Händler also gar nicht. Im Laden wirft use_skill nach dem Erwischtwerden hinaus (g_shop_banned, shop_leave), auch aus der Charakteransicht. Auffällig, aber nicht gemeldet: skill_pickpocket setzt beim Erwischtwerden g_group->money = 0, ohne das Geld der einzelnen Helden mitzuändern. |
| Schlüsselbund und Rezeptbuch verlieren im Lagerhaus den Inhalt | bestätigt (Nr. 358) | Beim Einlagern kopiert store_deposit_commit den ganzen Inventarplatz (27 Bytes) ins Depot. Beim Abholen gibt store_fetch_commit aber nur group_give_item(g_group, id, Menge) aus, also einen fabrikneuen Gegenstand. Alles, was im Platz stand, geht verloren: die Belegungsbits von Schlüsselbund und Rezeptbuch (key_bits +0x0b, box_has), broken, rs_lost, Bruchfaktor, Magie-Bit (geweiht), Giftladungen, Lichtdauer. Das erklärt auch Meldungen 10 und 11 (Lagerteil). Hinweis: ITEM_KEYRING (0x145) ist laut ITEMS.LTX der Schlüsselbund, der Name im Quelltext ist irreführend. |
| Lagerhaus repariert Gegenstände | anders | Gleiche Ursache wie Nr. 358 (Meldung 9): Abholen erzeugt neue Gegenstände, broken, rs_lost und Bruchfaktor werden dabei zurückgesetzt. Ein Pfandhaus gibt es in Sternenschweif nicht, gemeint ist das Lagerhaus. |
| Geweihte Waffen verlieren „magisch“ | bestätigt (Nr. 359) | Die Weihe steckt nur im Magie-Bit des Inventarplatzes (inv_slot.magic), nicht in der Gegenstandsart. (1) Die Schmiede gibt reparierte Teile per group_give_item(repair->item_id, 1) neu aus, das Bit ist weg. (2) Das Lagerhaus macht es ebenso (Nr. 358). (3) Die Gefangennahme ordnet nach g_items[id].is_magic() ein, also nach der Gegenstandsart: eine geweihte Waffe landet bei den normalen Waffen (Ablage 0x36) und nicht bei den magischen Dingen (0x34). Da die Ablagen wegen Nr. 212 ohnehin leer bleiben, ist Teil (3) im Original folgenlos. |
| Kaputte und heile Gegenstände im selben Stapel | bestätigt (Nr. 360) | Beim Stapeln gleichartiger Gegenstände addiert move_item nur die Mengen (und Giftladungen) auf den Zielplatz pos1. Die Bits des Quellplatzes (broken, magic, magic_revealed) und rs_lost/Bruchfaktor werden nicht verglichen: der Zustand des Zielstapels gilt danach für alle. Legt man Kaputtes auf Heiles, ist alles heil, umgekehrt alles kaputt. |
| Volles Inventar: Asthenilring oder magischer Wasserschlauch weg | bestätigt (Nr. 361) | Beide Stellen geben den Gegenstand mit give_hero_new_item(h, …) direkt an einen Helden und werten das Ergebnis nicht aus. Ist dessen Rucksack voll oder die Traglast erreicht, wird nichts gegeben, der Merker g_dng10_ring_taken bzw. g_dng14_waterskin_found steht aber trotzdem. Der Fund ist für immer verloren, ohne Abfrage zum Ablegen (die group_give_item stellen würde). |
| Schlüssel verschwindet beim Abnehmen vom Bund | bestätigt (Nr. 362) | Beim Abnehmen ruft use_keyring group_give_item(Schlüssel, 1) und löscht danach unbedingt das Bit im Schlüsselbund (box_set(…, 0)). Bei vollem Inventar fragt group_give_item, ob jemand etwas ablegt. Wer „Nein“ sagt, bekommt nichts (Rückgabe 0), das Bit ist trotzdem gelöscht: der Schlüssel ist weg. (ITEM_KEYRING 0x145 ist der Schlüsselbund.) |
| Unendlich viele Drachenschlüssel im Namenlosentempel | anders | Einen „Drachenschlüssel“ gibt es in Sternenschweif nicht (ITEMS.LTX kennt keinen, die Drachenklaue wird in O129 nur einmal vergeben, Merker g_dragon_claw_found). Die einzige endlose Schlüsselquelle im Gewölbe unter Tjolmar (dem Namenlosentempel) ist der flammendrote Schlüssel am Brunnen: im Zustand 3 gibt es bei jedem „Ja“ einen weiteren, der Zustand bleibt 3. Vermutlich ist dieser gemeint, also bekannte Nr. 203. |
| Leere Gruppe hängt das Spiel auf | bestätigt (Nr. 363) | Alle drei Stellen entfernen einen Helden mit group_set_slot (= group::dismiss_hero), ohne zu prüfen, ob er der letzte der Gruppe ist. Geldscheißer: Gelingt das Brauen, verschwindet der Held mit dem höchsten Goldgier-Wert, bei einem Einzelhelden also er selbst. Schacht in der Binge: Bei der dritten Abseilstufe stürzt der erste Held ab, auch wenn er allein ist. dismiss_hero setzt nr_heroes auf 0, die Gruppe bleibt aber aktiv und g_nr_groups unverändert. Viele Funktionen setzen danach einen Helden voraus (get_first_hero liefert 0): Streifen, Hänger, kein Speichern. Dasselbe kann das Stadtereignis „lured_away“ auslösen. |
| Helden in Wänden gefangen | unklar | Zellenänderungen (Geheimtür offen, Statue zerstört) schreibt set_cell nur in die Zellentabelle g_cells im Speicher. Ob sie nach dem Laden wieder angewandt werden, hängt an den jeweiligen Merkern und am gespeicherten Kartenzustand. Ohne den konkreten Spielstand ist nicht zu entscheiden, welche der drei Stellen betroffen ist. Transversalis als einziger Ausweg passt dazu, dass die Gruppe auf einer nicht begehbaren Zelle steht. |
| Gruppenwechsel: Teilgruppe steht am Ort der anderen | unklar | Im selben Dungeon bzw. derselben Stadt setzen O014 und O046 nach group_switch_next die Kamera mit cam_set_from_group neu, bei anderem Ort wird neu geladen. Ein Fehler, der fast bei jedem Wechsel auftritt, ist im Code nicht zu sehen. Die Meldung stammt aus DOSBox (S1 S. 11) und ist nicht nachgestellt. Bekannt ist nur Nr. 219 (NSC-Gruppen werden nicht übersprungen). |
| Torwachen umgehen | unklar | Zufallsereignisse wie die Bengram-Verwechslung versprengen die Gruppe (group_scatter) per Automap::random_cell auf ein begehbares Feld bis zu 8 Schritte entfernt. Begehbarkeit kennt keine Torwachen. Auffällig: random_cell kennt für Stadt 3 (Finsterkoppen) und Dungeon 9 die eigene Liste g_walk_town3 nicht (Automap::travel schon) und benutzt g_walk_default mit ganz anderen Feldtypen. Ob dadurch Felder außerhalb der Mauern erreichbar werden, hängt an den Kartendaten und ist ohne sie nicht zu entscheiden. |
| Absturz, wenn Helden auf Nachschubfeldern stehen | anders | Nachrückende Gegner (round_appear) suchen mit fig_find_free_cell ein freies Feld. Die Suche prüft nur die vier Achsenstrahlen ab dem Nachschubfeld und hat kein Abbruchkriterium: sind diese vier Linien bis zum Rand belegt, läuft sie endlos (Hänger, bekannt als Nr. 39). Bei Zweifeldgegnern wird das Schwanzfeld zudem ohne Randprüfung gelesen. Steht ein Held genau auf dem Nachschubfeld in einem engen Gang, ist das der wahrscheinlichste Weg zum „Absturz“. Zusätzlich setzt place_obj_on_cb ohne wirksame Belegt-Prüfung (Nr. 38). |
| Horriphobus: Kampf endet nie | bestätigt (Nr. 364) | „Verängstigt“ (scared) gilt bis zum Kampfende, gelöscht wird es nur beim Aufräumen in O080 oder bei gefesselten bzw. kampfunfähigen Helden. Ein verängstigter Held versucht in jedem Zug zu fliehen (fight_path_search). Ohne Fluchtweg passiert nichts, er verliert nur seine BP. fig_get_first_active_hero zählt ihn weiter als handlungsfähig, also endet der Kampf nicht. Die Kampfschleife in Fight::run hat keine Rundengrenze und keine Patt-Erkennung. Können die Gegner ihn nicht erreichen oder sind selbst verängstigt und eingeschlossen, dreht sich der Kampf für immer. |
| Endkampf Arkandor: Beschwörungen nicht gesperrt | widerlegt | Heptagon (21) und Elementare beschwören (24) stehen in g_spells_no_sfight. Beide haben Zielart 4, das Kampfmenü lässt als Ziel nur den Zaubernden selbst zu (TARGET_ID_SELF). Im Sonderkampf 0x493 greift dann die Sperre (target == caster) mit Text 0x107. Lücken: Krähenruf (22) und Skelettarius (23) fehlen in der Liste, und bei Zielen auf andere Helden prüft die Sperre gar nicht. Ein Absturz beim Feueratem durch Heptagon ist am Code nicht nachvollziehbar. Möglicherweise stammt die Meldung aus einer gepatchten Fassung (vgl. Meldung 45). |
| Eisenschild/Plattenzeug: Parademalus wirkt auf die Attacke | widerlegt | Schilde und Rüstungen haben in Sternenschweif nur RS und BE (g_armors_table). Die BE wird nach DSA-Regel je zur Hälfte von AT und PA abgezogen (rs_be / 2, ungerade Rest auf die PA). Ein reiner Parademalus existiert nicht, die Wirkung auf die AT ist gewollt. |
| Kampfmagier vor Lowangen kehren nach dem Sieg wieder | anders | Nr. 180 greift zu kurz. fight_result liefert nur 0 (Sieg) oder 1 (Flucht oder Niederlage), nie 2. Damit sind die Zweige „if (r == 2)“ in BEIDEN Funktionen tot, und ein Sieg setzt g_blackmage_done an keiner Stelle. Solange die Gruppe den Stein trägt, erscheinen die Magier bei jedem Durchreisen von Ort 41 und Ort 100 neu. Die gesunkene MR ist am Code nicht erklärbar (vermutlich Gegnerzauber im Kampf). Nr. 180 sollte entsprechend erweitert werden. |
| Analüs/Odem Arcanum zerstören Bildschirm und Heldenliste | unklar | Beide Zauber legen nichts dauerhaft an (select_item, Texte im Cast-Objekt). Ein Speicherverlust oder Überlauf je Anwendung ist nicht zu sehen. Die Meldung ist sporadisch und nur für die Einzel-CD-Fassung, das kann auch am CD-Cache (Nr. 86) oder an EMS/XMS liegen. |
| MR der NSC falsch (Korima, Thurazz) | bestätigt (Nr. 365) | Die NSC-Dateien enthalten eine MR, die nicht zur Formel des Spiels passt. calc_mr rechnet (KL + MU + Stufe) / 3 - 2 x AG + Typzuschlag: Korima -4 statt gespeichert -7, Thurazz -3 statt 0, Vinsalter 1 statt 2. Alle anderen NSC stimmen. Jede Neuberechnung (add_mr_mod, z. B. durch Menchal/Belmart, Amulette, Stufenanstieg) ersetzt den Dateiwert durch den Formelwert, darum „korrigieren“ Kräuter die MR. Thorgrim gibt es in Sternenschweif nicht. |
| Tierkunde an den Spinnennetzen invertiert | bestätigt (Nr. 366) | Am Feld 0x1118 (scharfer Geruch) warnt der Held mit der besten Tierkunde nur, wenn count_failed_skill(…, TA_TIERKUNDE, 6) > 0 ist, also wenn mindestens ein Held die Probe NICHT schafft. Gemeint war die Zahl der gelungenen Proben. Wer die Probe schafft, merkt nichts. |
| Akrobatik/Körperbeherrschung auf KL/MU/MU | widerlegt | Die Talenttabelle prüft Akrobatik (9) auf MU/GE/KK und Körperbeherrschung (11) auf MU/IN/GE, wie im Handbuch. Die Beobachtung stammt aus Bright Eyes, Tiefhusen vermutet dort richtig einen Bright-Eyes-Fehler. |
| Hesinde-Wunder 8 und 9 bewirken nichts | bestätigt (Nr. 367) | Hesinde würfelt W10 (g_temple_miracle_dice[6] = 10), der Höchstwert 10 ist die Wiederbelebung. Behandelt werden aber nur 1-3 (Analüs), 4-6 (Abtrünnige) und 7 (MR). Bei 8 und 9 bleibt der Text leer, es folgt „Eure Bitten werden nicht erhört“, die 10 Punkte Ansehen sind trotzdem weg. Firun und Ifirn haben dieselbe Lücke (W10, Fälle bis 7). Alle anderen Götter decken ihren Würfel vollständig ab. |
| Hesinde-Statue als AP-Quelle | bestätigt (Nr. 368) | Ab Hesinde-Ansehen 100 vergibt die Statue am Ende immer group_add_ap_each(g_group, 20), auch wenn man das Opfer ablehnt. Der Merker g_dng01_hesinde_done, der die Statue sperrt (Z. 306), wird aber nur beim Opfern gesetzt. Jede Ablehnung bringt also 20 AP je Held und kostet 15 Minuten. Nr. 204 betrifft die Probe beim Opfern, nicht diese Lücke. |
| Kräuterwirkungen verfallen nicht, Werte kippen | bestätigt (Nr. 369) | Jeder Held hat 20 Wirkungsplätze. Sind alle belegt, setzt get_free_effect die Dauer von Platz 0 auf 1 und ruft effects_tick(h, 1), damit er sofort abläuft. effects_tick tut aber nichts, solange g_dialog_depth != 0, und im Heldenbogen (wo man Kräuter isst) ist es mindestens 1. Platz 0 wird dann überschrieben, ohne dass seine Wirkung zurückgenommen wird: der Bonus bleibt für immer. Atmonblüte belegt 10 Plätze auf einmal, ab der dritten Blüte wird jede weitere Wirkung dauerhaft. Die Werte sind signed char und kippen ab 128 ins Negative. Gulmond wirkt dagegen absichtlich nur, solange die KK nicht schon erhöht ist (kk->cur > kk->normal). |
| Kräutersuche über „Alte Einstellungen“ ohne Malus | bestätigt (Nr. 370) | Im Auftragsmodus setzt gather_herbs selbst h->rest_herbs = 1. Direkt danach ruft rest_jobs h[0]->set_rest_herbs(h[0]->rest_herbs + 1) auf. rest_herbs ist ein 1-Bit-Feld, 1 + 1 ergibt 0: der Merker für Erholung -2 ist gelöscht. Bei der Nahrungssuche (rest_food) wechselt das Bit ebenso, wenn derselbe Held beide Nahrungsaufträge hat. Die Sache mit der vierten Nacht ist am Code nicht nachvollzogen. |
| Giftschlange im Sumpf: LE 18/18 statt Vergiftung | unklar | Ein Biss gibt zu 50 % Gift 7 (Lotusgift: 24 Stunden lang je Stunde 2W6 LE) oder W4 LE Schaden. Keine Stelle setzt LE und LE-Maximum auf einen festen Wert. Wie ein Krieger dauerhaft auf 18/18 kommt, ist am Code nicht zu erkennen. Vermutlich spielt ein anderer Vorgang mit (z. B. Tod und Wiederbelebung durch die hohe Giftwirkung, die Wiederbelebung gibt LE 7). |
| Stufenanstieg: Kampftalent steigert immer die Attacke | bestätigt (Nr. 371) | Der Stufenanstieg erhöht ein Kampftalent immer um 1 (add_skill_value(…, 1)). add_skill_value verteilt die Hälfte auf AT und PA und schlägt den ungeraden Rest der AT zu. Bei +1 bekommt die AT also immer +1 und die PA nie etwas. Eine Wahl gibt es nirgends. Damit steigt die PA durch Stufenanstiege nie, nur über den Grundwert. Ob Attic das so wollte, ist offen (nach den DSA-Regeln wählt der Spieler). |
| Nichtzauberer bekommen Zaubersteigerungen angeboten | bestätigt (Nr. 372) | In der Heldenerschaffung setzt chargen_set_type_values spell_incs nur bei Zauberkundigen. Wählt man zuerst einen zauberkundigen Typ und wechselt dann zu einem anderen, bleibt dessen Zahl stehen (der aus Schick bekannte, nicht zurückgesetzte Zähler). Beim Stufenanstieg läuft die Zauberschleife von Lvl::inc_points bei spell_incs != 0 ohne is_mage()-Prüfung: select_spell meldet „Das ist magiebegabten Helden vorbehalten“, danach folgt das Menü „Ende/automatisch/abbrechen“. |
| Import: Kindergesichter | unklar | Das Porträt wird über eine Prüfsumme des DSA-1-Bildes in Tabelle 0x7b gesucht, sonst gibt es das erste Porträt des Typs. Eine Stelle, die Kinderbilder wählt, gibt es im Code nicht. Der Workaround (Byte 0xB7 hinter „CHAR“ im Spielstand von 0x10 auf 0) betrifft ein Feld des gespeicherten Heldensatzes, das am Code nicht eindeutig zuzuordnen ist. Die Meldung betrifft vermutlich frühe Fassungen, für die es einen eigenen Patch gab. |
| Weitere Importfehler (toter Held, Kampfwerte) | bestätigt (Nr. 373) | Teilweise bestätigt. (1) Toter Held: Die Importschleife prüft nur group_pos != 0 und npc_id == 0, nicht den Tod. Der Satz wird ab nr_items per memcpy (set_from_nr_items) übernommen, das dead-Bit also mit. (2) Kampfwerte: Derselbe memcpy übernimmt AT/PA-Werte, rs_bonus, rs_be und die Kampfmodifikatoren roh. check_hero setzt danach nur MR, Eigenschaften und den AT/PA-Grundwert neu (calc_atpa_base), Rüstungsschutz und BE der angelegten Rüstung werden nicht neu berechnet. Die Amulett-MR (-5) und das Travia-Amulett ließen sich ohne DSA-1-Spielstand nicht klären. |
| Siebenquell/Ailian: Salamanderstein weg, Gavron fehlt, kein Ende | widerlegt | Bei der Ankunft in Lowangen nimmt Gavron den Stein (NAKED.LTX[11] „Und mit ihm der Salamanderstein!“), das ist Handlung. Ist Antharon tot, setzt das Spiel g_low_stone_delivered = -2, Gavron erscheint dann absichtlich nie in Schenke 14 (Bedingung == 1). Das Spielende zählt g_low_stone_days nur an Tagen, an denen die Gruppe in Lowangen ist, und greift erst ab Tag 12. Wer den Stein zurückholt, setzt den Zähler auf 0. „Nach 10 Tagen kein Ende“ ist damit erwartetes Verhalten. |
| Markthalle Lowangen: Ereignis bleibt hängen | bestätigt (Nr. 374) | Vor dem Bettlerkampf setzt tribute g_fight_end = 2 („nach dem ersten Toten fliehen alle“). Zurückgesetzt wird es nur in fight_finish. Sind die Bettler aus dem ersten Kampf weg, liefert fight_count_monsters beim nächsten Mal 0, und Fight::run kehrt sofort mit 1 zurück, ohne fight_finish. Man sieht nur den Text ohne Kampf, g_fight_end bleibt 2. Im nächsten beliebigen Kampf löst der erste tote Gegner g_fight_flee_all aus: alle Gegner fliehen mit dem Bettlertext (txt[0x56]). Da fight_finish den Kampf als Sieg wertet, sterben die noch stehenden Gegner, man bekommt Beute und AP. |
| Agdan-Quest: kein Lowangen-Schlüssel | bestätigt (Nr. 375) | Der Schlüssel wird mit give_hero_new_item direkt an den ersten handlungsfähigen Helden gegeben, das Ergebnis wird nicht geprüft. Ist dessen Rucksack voll oder die Traglast erreicht, gibt es keinen Schlüssel, g_eolan_state = 2 und der Tagebucheintrag stehen trotzdem. Der Geheimgang an der Burgruine (O125, group_find_key) bleibt dann zu. Ob das die Ursache der Meldung war, ist nicht belegt. |
| Thurazz: falscher Ausgang, Tagebuch zu früh | unklar | Thurazz (NSC 1) wird an beiden Ausgängen der Orkfestung verabschiedet (Ebene 1 und Ebene 3). groups_check_npc schreibt für NSC 1 bei jedem Abschiedsgrund denselben Tagebuchtext GLOBLTXT[380] „… aus dieser Orkenhöhle herausgefunden …“, also auch, wenn er nach zwei Monaten oder bei Aufnahme eines anderen NSC geht. Welcher Ausgang „falsch“ ist, lässt sich ohne Handlungsvorgabe nicht entscheiden. Nebenbefund: An beiden Ausgängen liest group_get_npc(g_group)->get_npc_id() ohne Nullprüfung (ohne NSC wird Adresse 0000:009B gelesen). |
| Golem/Salamanderstein: Tauschmeldung fehlt | widerlegt | Eine eigene Tauschmeldung gibt es in keinem Fall: Wählt man „tauschen“, nimmt das Spiel das Schwert stumm weg (oder merkt sich den Tausch, wenn es noch nicht fertig ist). Ist das Schwert beim Tausch noch nicht fertig, sagt der Golem beim Fertigstellen BINGE7[70] „… legt es in die Truhe in der östlichen Kammer“, das ist die vorgesehene Meldung. |
| Textfehler „Rechstschritt“ | bestätigt (Nr. 376) | Die Truhe mit dem Orkbanner beschreibt „eine Fahne von gut 4 Rechstschritt Größe“ (gemeint: Rechtschritt). Die Angabe „Westen statt Osten“ in den Wegbeschreibungen nach Svellmia und Norhus fand sich in keinem Spieltext (POINT1/POINT2, TALKS_07/TALKS_09, Dialoge). |
| Sammelliste des englischen Walkthroughs | unklar | Die Liste ist zu unbestimmt für eine Prüfung einzelner Stellen (Truhe/Skelett im Sumpf nur schrittweise erreichbar, Netz im Sumpf, schrittweises Gehen in Tjolmar, „Wow!“-Türen, nie öffnende Truhe, Automap-Absturz). Die Drehung um 180° nach dem Besuch eines Hauses ist gewollt: O030 dreht die Gruppe, wenn ein Ort nicht betreten wurde, damit sie nicht sofort wieder hineinläuft. Zur Automap vgl. Nr. 208. |
| Offizielle Patches DSAPAT03/04 | unklar | Der Changelog nennt einen CD-Lesefehler beim Ebenenwechsel und einen Fehler bei der Dämonenbeschwörung im Kampf, beide für Fassungen bis C1.02c. Unsere C1.02 liegt davor und dürfte beide noch enthalten. Ohne den Patch ist keine Stelle sicher zuzuordnen. Spur für die Beschwörung: O137 setzt das beschworene Wesen auf h->target_x/y, bei Heptagon (Zielart 4) ist das das Feld des Zaubernden. place_obj_on_cb trägt es auf einem belegten Feld nicht ein (Nr. 38), das Wesen steht dann ohne Kampffeld-Eintrag auf dem Helden. Spur für den CD-Fehler: Nr. 86 (CD-Cache bei Lesefehlern). |

## Technische Details

Codestelle, genaue Beschreibung, Begründung der Einordnung und Fix-Vorschlag je Nummer: `Analyse/Originalfehler.md` und `Analyse/Fehler_Raster.tsv` im Quelltext-Paket. Die Fehler sind im rekonstruierten Quelltext bewusst so nachgebaut, damit er Byte für Byte das Original ergibt. Behebbar werden sie über Schalter (`FIX_BUGS`, `FIX_EXPLOITS`, je Fehler einzeln).

