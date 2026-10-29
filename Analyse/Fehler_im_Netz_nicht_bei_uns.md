# Sternenschweif: Bugs aus dem Netz, die nicht in unserer Liste stehen

Stand 19.10.2026. Alle Einträge sind Meldungen von Spielern. Nichts davon ist am Code geprüft, und manche sind unbestätigt (so markiert). Viele Meldungen stammen aus Spielständen, die mit Hex-Editor oder DOSBox bearbeitet wurden, und können daher auch Bedienfehler sein.

## Durchgesehene Sammlungen (S1–S9)

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

## Geld und Vervielfältigung

1. **Korima: verkaufte Waffen bleiben im Gepäck.** Verkauft man Schwert und Speer der NSC Korima, gibt es rund 30 Dukaten, die Waffen bleiben aber bei ihr. Das lässt sich beliebig wiederholen. Quelle: S1 Seite 5 (Sznooze, 2008). https://www.crystals-dsa-foren.de/printthread.php?tid=46&page=5
2. **Lagerhaus: Gegenstände doppelt, absurde Lagerkosten.** Nach dem Abholen tauchen Teile der früher eingelagerten Ware wieder im Depot auf. Die Lagerkosten summieren sich bis 190.257 Dukaten. 2014 und 2018 erneut nachgestellt. https://www.crystals-dsa-foren.de/showthread.php?tid=949
3. **Holzfällerlager: ein Gegenstand mehr.** Stückzahl wählen, dann Rechtsklick (zurück): die Auswahl zeigt einen Gegenstand mehr. Beliebig wiederholbar, solange mindestens 2 übrig sind. Möglicherweise verwandt mit unserer Nr. 123 (Truhe, Teilmenge). S1 Seite 10 (Lippens die Ente, 2015). https://www.crystals-dsa-foren.de/printthread.php?tid=46&page=10
4. **Phexhelm/Phexschild vervielfachen.** Einem abgetrennten Helden Helm/Schild geben. Der Poltergeist legt die Teile in die Truhe zurück, der Held behält seine Kopie. S4 (Lumpi 2007), S1 Seite 10 (Zurgrimm 2015, Verweis auf Phex-Tempel-Thread #48–52).
5. **Ringe, Stirnreif, Amulett und leere Rezeptbücher werden bei der Gefangennahme vervielfacht.** Zurgrimm vermutet als Ursache, dass Anzahl und Ladungen am selben Platz gespeichert werden. S2.
6. **Geld verschwindet beim Vereinen von Gruppen.** (a) Mit NSC: der Betrag hängt davon ab, von welcher Teilgruppe aus vereint wird (Boomer 2006, S1 Seite 1). (b) Heiße Platten, Zwergenmine Ebene 6: zurückgegangene Helden verlieren beim Vereinen fast die Hälfte (Grimli 2012, S1 Seite 10). (c) Helen nimmt bei der Flucht 1/7 des Gruppengelds mit (Kunar 2010, S1 Seite 8). Zum Code: money_share/Gruppenvereinigung prüfen.
7. **Asthenilschwert per „ESC-Bug“ mitnehmen oder verdoppeln.** Unbestätigt, nur aus zweiter Hand (Zurgrimm über JackyD, 2007). S4.
8. **Taschendiebstahl beim fahrenden Händler beliebig oft.** Auch nach dem Erwischtwerden wird man nicht hinausgeworfen, solange man über die Charakteransicht stiehlt. S1 Seite 9 (Hanbastan 2011).

## Gegenstände und Inventar

9. **Schlüsselbund und Rezeptbuch verlieren im Lagerhaus ihren Inhalt.** Gemeldet von Gaddezwerch 2006, derboo 2010 und Lumpi 2007. NRS 2016: Fehler beim Zurückholen aus dem Depot, im Patch behoben. S1 Seite 1/8, S3, S4.
10. **Lagerhaus/Pfandhaus repariert Gegenstände.** Verschlissene Stiefel sind nach dem Ein- und Auslagern wieder neu. S1 Seite 1, S4.
11. **Geweihte Waffen verlieren „magisch“.** Bei Reparatur oder Einlagerung geht der Zusatz verloren. Die Orks vor Lowangen nehmen sie wie normale Waffen. S1 Seite 7 (RalfNr 2009), S4.
12. **Zerbrochene und heile Gegenstände im selben Stapel.** Der Zustand wird auf den ganzen Stapel übertragen, in beide Richtungen. S1 Seite 8 (Pirador 2010).
13. **Volles Inventar: Asthenilring oder magischer Wasserschlauch geht ohne Abfrage verloren.** S1 Seite 9 (Hanbastan 2011).
14. **Schlüssel verschwindet beim Herausnehmen aus dem Bund.** Bei vollem Inventar fragt das Spiel nach Wegwerfen. Wer verneint, verliert den Schlüssel. S1 Seite 6 (Greekmill 2008, „vielleicht-Bug“).
15. **Unendlich viele Drachenschlüssel beim Verlassen des Namenlosentempels.** Von Zurgrimm als Bug bestätigt. Möglicherweise verwandt mit unserer Nr. 203 (Flammenschlüssel), Kunar nennt beide aber getrennt. S1 Seite 5 (Kunar 2008).

## Gruppe, Bewegung und Hänger

16. **Leere Gruppe hängt das Spiel auf.** Braut ein einzelner Held den Geldscheißer oder seilt sich der letzte Held im Schacht der Zwergenbinge ab, bleibt eine Gruppe ohne Helden. Speichern geht nicht mehr. Wechseln oder Laufen führt zu Streifen bzw. zum Hänger. S1 Seite 4 (Kunar 2008), Seite 9 (Xep624/Zurgrimm 2011).
17. **Helden in Wänden gefangen.** Betroffen sind eine Geheimtür auf Blutzinnen Ebene 1, die nach dem Laden wieder auftaucht, die Brazoragh/Gold-Statue in den Blutzinnen und Bäume im Sumpf nach dem Öffnen der Automap. Befreien kann nur Transversalis, ohne Magier ist das Spiel damit verloren. S1 Seite 4/7/9.
18. **Gruppenwechsel: Teilgruppe steht am Ort der anderen.** Tritt fast bei jedem Wechsel auf, auch in Wildnis und Städten (DOSBox). S1 Seite 11 (HoraK 2017). Unsere Nr. 219 und 143 betreffen anderes.
19. **Torwachen umgehen.** Ein Zufallsereignis versetzt die Gruppe hinter die Torwache bzw. überschreibt den Wachtext. Danach lässt sich die Stadtkarte außerhalb der Mauern begehen, mit Grafikfehlern am Rand (Lowangen, Tiefhusen, Finsterkoppen). S1 Seite 5/6/12.
20. **Absturz, wenn Helden auf Nachschubfeldern der Gegner stehen.** S1 Seite 10 (Asgrimm 2013).

## Kampf und Magie

21. **Horriphobus: eingeschlossener eigener Held bleibt für immer verzaubert.** Der Kampf endet dann nie, er dreht sich im Kreis. Nur ein Programmabbruch hilft. S1 Seite 9 (exi 2011, Hendrik bestätigt).
22. **Endkampf Arkandor: Beschwörungszauber nicht gesperrt.** Mit Heptagon und Ähnlichem verlässt der Drache sein Lager, und das Spiel stürzt beim Feueratem ab. S6 tid=5134 (Jandor 2015). https://www.crystals-dsa-foren.de/showthread.php?tid=5134
23. **Eisenschild/Plattenzeug: angezeigter Parademalus wirkt auf die Attacke.** Laut Kunar ist das längst bekannt (tid=94). S1 Seite 6 (Greekmill 2008).
24. **Kampfmagier vor Lowangen kehren nach dem Sieg wieder.** Laut Zurgrimm gewollt, solange man den echten Salamanderstein trägt. Nach dem Wiederauftreten ist die MR gesunken. S1 Seite 11 (draconisa 2016), tid=5386. Bezug zu unserer Nr. 180 (blackmage_done) prüfen.
25. **Analüs/Odem Arcanum etwa 20-mal hintereinander zerstört Bildschirm und Heldenliste.** Sporadisch, Einzel-CD-Fassung. S1 Seite 4 (nn23 2007).

## Werte, Proben und Wunder

26. **MR der NSC falsch** (Korima −7 statt −4, Thurazz, Thorgrim). Kräuter wie Menchal/Belmart „korrigieren“ den Wert. S1 Seite 5/8.
27. **Tierkunde-Probe an den Spinnennetzen (Blutzinnen) invertiert.** Bei Misserfolg werden die Spinnen erkannt. S6 tid=4404 Seite 1 (Tiefhusen 2020, Beobachtung mit Bright Eyes).
28. **Akrobatik/Körperbeherrschung auf KL/MU/MU statt laut Handbuch.** Beobachtet mit Bright Eyes. Tiefhusen hält es 2024 für einen Bright-Eyes-Fehler. S1 Seite 10, S6 tid=4404 Seite 4.
29. **Hesinde-Wunder 8 und 9 bewirken nichts.** Meldung „Eure Bitten werden nicht erhört“, Bug vermutet. https://www.crystals-dsa-foren.de/showthread.php?tid=4409
30. **Hesinde-Statue im Namenlosentempel als AP-Farm.** Jedes Ablehnen des Angebots bringt Abenteuerpunkte, laut Pennywise fünf Stufen in 15 Minuten. S4 (2007), tid=5386. Am Code mit unserer Nr. 204 abgleichen.
31. **Kräuter-Abbau und Überlauf.** Gulmond, Atmon, Menchal und Belmart werden ab einer Zahl von Anwendungen nicht mehr abgebaut, und die Werte kippen ab 128 ins Negative. Gulmond wirkt angeblich nur einmal. S1 Seite 12 (Finsterkamm 2020, z. T. für Teil 1/3).
32. **Kräutersuche über „Alte Einstellungen“ ohne Regenerationsmalus.** Ebenso übernimmt ein Held in der vierten Nacht alle drei Lageraufgaben. S1 Seite 8 (titan 2010), Seite 11 (Tiefhusen 2016).
33. **Giftschlange in den Svelltsümpfen.** Der Krieger bekommt dauerhaft LE 18/18 statt einer Vergiftung. S1 Seite 9 (Hanbastan 2011).
34. **Beim Stufenanstieg steigt bei Kampftalenten immer die Attacke, es gibt keine Wahl.** Möglicherweise gewollt. S1 Seite 1/3, tid=2075.
35. **Nichtzauberer bekommen beim Stufenanstieg Hinweise auf Zaubersteigerungen.** Vermutet werden Restpunkte aus der Erschaffung. In Schick ist das als nicht zurückgesetzter Zähler bekannt. S1 Seite 10 (Silencer 2012).

## Import aus der Schicksalsklinge

36. **„Kindergesichter“.** Importierte Helden bekommen Kinderporträts. Workaround: Byte 0xB7 hinter „CHAR“ von 10 auf 00 setzen. Es gab dafür einen eigenen Patch. S8, S7 (star3.htm).
37. **Weitere Importfehler.** Mit beiden Amuletten (gegen Verwandlung und gegen Kampfzauber) bekommt der Held dauerhaft −5 MR. Das Traviaamulett wird nichtmagisch. Die Kampfmodifikatoren werden nicht zurückgesetzt (RS −1 bis −4, AT/PA verschoben). Ein toter Held wird mit importiert. S1 Seite 10/12, tid=116 Seite 7, tid=2075.

## Handlung und Text

38. **Siebenquell/Ailian (Lowangen): Salamanderstein verschwindet.** Auch Gavron bleibt weg, und nach 10 Tagen kommt kein Ende. S1 Seite 7 (Tensaya, Gaddezwerch 2009).
39. **Markthalle Lowangen: Ereignis bleibt hängen.** Beim erneuten Betreten erscheint der Text ohne Kampf. Der nächste beliebige Kampf endet nach dem ersten toten Gegner mit dem Markthallen-Text. Das lässt sich ausnutzen. S1 Seite 4/8 (Asgrimm 2008, Kunar 2009 reproduziert).
40. **Agdan-Quest: kein Lowangen-Schlüssel.** S1 Seite 7 (2009).
41. **Thurazz.** Er verlässt die Gruppe am falschen Ausgang, und der Tagebucheintrag „aus der Orkhöhle herausgefunden“ erscheint zu früh. S1 Seite 2 (Kunar 2007).
42. **Golem/Salamanderstein.** Holt man den Stein, bevor das Asthenilschwert fertig ist, fehlt die Tauschmeldung. S1 Seite 5 (2008).
43. **Textfehler.** „Rechst“ statt „Rechtsschritt“ (S1 Seite 8). Westen statt Osten in den Wegbeschreibungen zu den Salamandersteinen (Svellmia, Norhus, S1 Seite 2).
44. **Englischer Walkthrough (S7) nennt außerdem:** Truhe und Skelett im Sumpf ohne „schrittweises Gehen“ unerreichbar, Netz im Sumpf, schrittweises Gehen im Tjolmar-Dungeon defekt, zwei „Wow!“-Türen und eine nie öffnende Truhe in den Blutzinnen, Häuser in Lowangen drehen die Gruppe um 180°, Absturz der Automap.

## Offizielle Patches (S5)

45. Der Changelog der attic-Patches DSAPAT03/04 (Diskette bis V1.04, CD bis C1.02c) nennt nur zwei Fehler: einen CD-Lesefehler beim Wechsel der Dungeon-Ebene und einen bei der Dämonenbeschwörung im Kampf. Dazu kommen pauschal „verbliebene Unzulänglichkeiten und Textfehler“. Unsere C1.02 sollte beide enthalten. Die LIESMICH selbst konnte ich nicht lesen.
