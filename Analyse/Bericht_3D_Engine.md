# Bericht Auftrag C: die 3D-Engine (SEG024, SEG025)

Stand 19.10.2026. Ergebnis: beide Module sind lesbar und **bytegleich** (Prüfung siehe unten).

| Modul | vorher | nachher | Inhalt |
|---|---|---|---|
| SEG024 | 10.011 Zeilen, 1.151 Adressmarken | 1.545 Zeilen, 12 Prozeduren, 74 benannte Marken | Kanten verfolgen, Flächen füllen |
| SEG025 | 1.666 Zeilen, 44 Adressmarken | 1.056 Zeilen, 4 Prozeduren, 43 benannte Marken | drehen, abschneiden, projizieren, sortieren |

Die meisten der 10.000 Zeilen von SEG024 waren dieselben vier Pixelschritte, 134- bzw. 208-mal hintereinander, dazu vier
Sprungtabellen, die der Disassembler als Unsinnsbefehle gelesen hatte. Attic hat das sicher nicht von Hand getippt,
sondern mit Makros (`rept`) erzeugt. So steht es jetzt auch im Quelltext.

## Wie die Engine funktioniert

Das Bild der 3D-Ansicht (Stadt, Dungeon) ist 208 × 134 Pixel groß. Jedes Kartenfeld hat ein 3D-Objekt aus ebenen,
konvexen Vielecken (Flächen). Pro Bild ruft die C-Seite (SEG012 `draw_object`) für jedes sichtbare Objekt erst
SEG025 und dann für jede Fläche SEG024 auf.

### 1. Drehen (SEG025 `obj_project`)

Klassische Koordinatentransformation: Lage des Objekts minus Augenpunkt, dann drei ebene Drehungen nacheinander
(x/z-Ebene = Blickrichtung, y/z = Neigung, x/y = Seitenneigung) mit den negierten Kamerawinkeln. Danach steht die
Kamera im Ursprung und blickt entlang +z.

- Winkel sind 10 Bit (1024 = 360°), Sinus kommt aus einer Tabelle mit 1024 Werten in Festkomma 1.15
  (sin · 32768), Kosinus ist dieselbe Tabelle um 256 Einträge versetzt.
- Eine Multiplikation mit sin/cos ist `imul` (16 × 16 → 32 Bit) und `shld dx,ax,1`, also (a · s) >> 15.
- Jeder Eckpunkt wird mit (Kamerawinkel + Objektwinkel) gedreht und der gedrehte Objektursprung addiert.
- Bewegte Eckpunkte (Türen, Animationen) haben statt x die Marke 7FFFh und holen ihre Lage aus einer Liste,
  abhängig von einer Phase des Objekts.

### 2. Abschneiden an der Augenebene (SEG025 `clip_face`)

Ecken hinter dem Auge (z < 0) lassen sich nicht projizieren (Division durch etwas ≤ 0). Jede Fläche mit so einer
Ecke wird an der Ebene z = 0 abgeschnitten (Sutherland-Hodgman): Kante für Kante bleibt die vordere Ecke stehen,
und wo eine Kante die Ebene durchstößt, entsteht ein neuer Punkt mit t = |z_a| / |z_a − z_b| (x, y, u, v linear
interpoliert, z = 0). Die beschnittene Fläche ist eine Kopie im Codesegment, höchstens 16 je Objekt.

### 3. Projektion

Zentralprojektion auf den Schirm, Mitte (104, 67):

  x' = 104 + x · (scale/16) · d / (z + d),   y' = 67 + y · (scale/16) · d / (z + d)

mit scale = 90 und d = 32 (`view_set_projection(90, 32)` aus O014/O046). d ist der Augabstand: ein Punkt bei z = 0
liegt nicht im Unendlichen, sondern wird mit Faktor 1 abgebildet.

### 4. Sichtbarkeit und Reihenfolge

- **Rückseiten:** Bei einseitigen Flächen (Flag 1) entscheidet das Vorzeichen des Kreuzprodukts der ersten drei
  Bildpunkte, ob man auf die Vorder- oder Rückseite blickt.
- **Ganz außerhalb:** Liegen alle Ecken links, rechts, über oder unter dem Fenster, fällt die Fläche weg.
- **Maleralgorithmus:** Die verbleibenden Flächen kommen in eine verkettete Liste, absteigend sortiert nach dem
  größten z ihrer Ecken. Gezeichnet wird von hinten nach vorn, nahe Flächen übermalen ferne. Einen z-Puffer gibt es
  nicht.

### 5. Kantentabellen (SEG024 `poly_edges`, `edge_trace`)

Eine konvexe Fläche schneidet jede Bildschirmspalte in genau einem Stück. Es reicht also, je Spalte das obere und
das untere Ende zu kennen. Dafür gibt es acht Tabellen mit je 208 Einträgen (`g_edge_lo/_hi` und an beiden Enden
z, u, v). `poly_edges` geht die Kanten der Fläche ab, `edge_trace` rechnet entlang jeder Kante Spalte für Spalte
die Höhe y, die Tiefe z und die Texturkoordinaten u, v aus und trägt sie ein, wenn sie oben bzw. unten neue
Extremwerte sind.

Rechenweise: Schritt = Differenz / Spaltenzahl in Festkomma 16.16, vom Bruchteil nur das obere Byte. Der Bruchteil
wird mit `add` aufaddiert, der Überlauf (Carry) per `adc` auf den ganzzahligen Teil übertragen. Startwert 0,5,
damit gerundet wird.

### 6. Der Trick mit Wänden und Böden

Texturen perspektivisch richtig aufzutragen kostet eine Division pro Pixel. Die Engine spart sie mit einer
geometrischen Beobachtung:

- Auf einer **senkrechten Wand** hat jede Bildschirmspalte überall dieselbe Tiefe z (die Kamera neigt sich kaum).
  Entlang einer Spalte ist die Abbildung dann exakt linear. Wände werden deshalb **spaltenweise** gefüllt
  (`poly_tex_cols`).
- Auf einem **waagerechten Boden** oder einer Decke hat jede Bildschirm**zeile** dieselbe Tiefe. Dafür vertauscht
  `edge_trace` einfach x und y (Parameter „horizontal“), die Tabellen beschreiben dann Zeilen, und `poly_tex_rows`
  füllt zeilenweise.
- Die Perspektive steckt nur noch in den Kanten: `edge_trace_persp` interpoliert u, v, z entlang der Kante nicht
  linear im Bild, sondern linear im Raum:

  wert(s) = wert₀ + (wert₁ − wert₀) · s · w₀ / ((1 − s) · w₁ + s · w₀),   w = z + d

  Zähler und Nenner wachsen je Spalte um feste Schritte, pro Spalte bleiben drei Divisionen statt einer pro Pixel.
  Das braucht man nur für nahe Flächen (SEG012: z unter 100 bzw. 400), ferne Kanten laufen linear.

### 7. Füllen (SEG024 `poly_tex_*`, `poly_flat`)

Je Spalte (bzw. Zeile):

1. **Nebel und Licht:** Stufe = Nebeltabelle[z / 16] + Helligkeit der Fläche (0..7), höchstens 31. Jede Stufe hat
   eine Farbumsetztabelle mit 256 Einträgen; der Pixel wird über diese Tabelle „abgedunkelt“ (`mov bl,fs:[bx]`).
2. **Texturschritt:** v (Texturzeile) nach Bresenham: ganzer Schritt in Zeilen × Texturbreite, Rest als 8-Bit-
   Bruchteil, beim Überlauf eine Texturzeile mehr. u (Texturspalte) mit 8-Bit-Bruchteil. Kein Multiplizieren,
   kein Dividieren im inneren Schritt.
3. **Abschneiden am Fensterrand:** oben wird die Textur nur vorgerechnet, unten die Pixelzahl gekürzt.
4. **Abgerollte Schleife:** Statt einer Schleife steht der Pixelschritt 134-mal (Spalte) bzw. 208-mal (Zeile)
   hintereinander. Für n Pixel springt die Routine mitten hinein, auf Schritt 134 − n: Adresse aus einer Tabelle
   holen, auf den Stapel legen, `retn`. So entfallen Zähler und Rücksprung pro Pixel.
5. **Durchsicht:** Die `_m`-Varianten lassen Texturfarbe 0 aus (Bäume, Gitter).

`poly_flat` füllt einfarbige Flächen (ohne Nebel), 16 Pixel je Schleifendurchlauf.

### 8. Selbstveränderlicher Code

Die Texturen liegen entweder im EMS-Fenster (16-Bit-Adresse) oder ohne EMS im Speicher über 1 MB, erreichbar über
den „Flat Real Mode“ (SEG035 setzt die Segmentgrenzen auf 4 GB, Zugriff mit 32-Bit-Adresse ESI). Die Pixelschritte
lesen das Texel mit `mov bl,[esi]` (Bytes 67 8A 1E). Ist EMS da, überschreibt jede Füllroutine beim ersten Aufruf
alle ihre 134 bzw. 208 Schritte mit `mov bl,ds:[si]` (3E 8A 1C, gleich lang). Die Adressen der Stellen liefert
dieselbe Sprungtabelle.

## Prozeduren

| Linkname | Name | Aufgabe |
|---|---|---|
| F_R24_0000 | edge_trace_vars | keine Funktion: 16 Bytes Variablen der Kantenverfolgung im Codesegment |
| F_R24_0010 | edge_trace | eine Kante linear in die Kantentabellen eintragen |
| F_R24_025A | edge_trace_persp | eine Kante perspektivisch korrekt eintragen (nahe Flächen) |
| F_R24_04F3 | poly_edges | alle Kanten einer Fläche eintragen (ruft edge_trace), früher poly_setup |
| F_R24_0560 | poly_edges_persp | dasselbe mit edge_trace_persp, früher poly_setup_clip |
| F_R24_05CD | edges_clear | Kantentabellen leeren, BE-Name fill_smth |
| F_R24_05F6 | fill_scene | 3D-Fenster im Zeichenpuffer mit einer Farbe füllen (BE) |
| F_R24_0627 | copy_3d_to_screen | 3D-Fenster nach A000:028F auf den Bildschirm kopieren (BE) |
| F_R24_0668 | poly_tex_cols | texturierte Fläche spaltenweise (Wände), früher poly_tex |
| F_R24_198A | poly_tex_cols_m | dasselbe mit Durchsicht, früher poly_tex_m |
| F_R24_2FCE | poly_tex_rows | texturierte Fläche zeilenweise (Boden, Decke), früher poly_tex_p |
| F_R24_4B29 | poly_tex_rows_m | dasselbe mit Durchsicht, früher poly_tex_mp |
| F_R24_6C35 | poly_flat | einfarbige Fläche spaltenweise |
| F_R25_04B6 | clip_face | Fläche an der Ebene z = 0 abschneiden (Registerschnittstelle, nur aus obj_project) |
| F_R25_06C4 | view_set_projection | Maßstab und Augabstand setzen (90, 32), früher r25_06c4 |
| F_R25_08E4 | obj_project | Objekt drehen, abschneiden, projizieren, Flächen prüfen und sortieren |
| F_R25_0FF4 | tex_copy_flat | Speicher mit 32-Bit-Adressen kopieren (Texturen ohne EMS) |

Die Linknamen F_R24_xxxx/F_R25_xxxx bleiben, weil die Assembler-Fassungen der Aufrufer sie benutzen. Der
sprechende Name ist der Prozedurname im Quelltext und der C-Name in `s_proto.h`.

## Daten

| Adresse | Name | Inhalt |
|---|---|---|
| ds:78DA … 843A | g_edge_lo, g_edge_hi, g_edge_lo_z/_hi_z, g_edge_lo_u/_lo_v, g_edge_hi_u/_hi_v | die acht Kantentabellen (je 208 Worte) |
| ds:85DA, 85E0 | g_cur_tex, g_cur_tex_w | Lage und Breite der aktuellen Textur |
| ds:8C78, 8C7A | g_proj_dist, g_proj_scale (neu) | Projektion d = 32, scale = 90 |
| ds:8C7C, 8C7E, 8C82 | g_nr_vis_faces, g_nr_clip_faces (früher g_8c7e), g_nr_vtx | Zähler je Objekt |
| ds:94A6 | g_face_depth (früher g_94a6) | Sortierschlüssel je Fläche (größtes z) |
| ds:94C4 | g_face_list_seg | Segmentwort von g_face_list (eigenes Public) |
| SEG024 CS:0000 | et_* | Schritte, Zähler und Bruchteil-Akkus der Kantenverfolgung |
| SEG024 CS:0658 | tx_* | Texturschritte der Füllroutinen |
| SEG024 CS:187E, 2EC2, 4989, 6A95 | JT_COLS, JT_COLS_M, JT_ROWS, JT_ROWS_M | Sprungtabellen (134 bzw. 208 Einträge) |
| SEG025 CS:000A | pj_scale, pj_dist | Projektion (Kopie für obj_project) |
| SEG025 CS:000E | clip_buf | 16 beschnittene Flächen à 72 Bytes |
| SEG025 CS:048E | cl_* | Ecken, Schnittpunkt und Texturkoordinaten der gerade geschnittenen Kante |
| SEG025 CS:06DE | face_ptrs, face_ptrs_len | Zeiger auf die Flächen des aktuellen Objekts |

Flächenblock (Segment g_face_seg): +000h Verkettung (1-basiert), +102h Tiefe, +204h Punktpuffer (x, y, z, w).

## Besonderheiten

- **Attics Werkzeug verrät sich.** Mehrere Bytefolgen, die wie Handarbeit aussehen, sind TASM-Spuren:
  Vorwärtssprünge bekommen NOPs angehängt (TASM reserviert erst die lange Form), `jecxz` mit fünf NOPs heißt,
  dass Attic `JUMPS` eingeschaltet hatte, und `nop / push cs / call near` ist TLINKs Umbau eines far-Aufrufs.
  Die Auffüllung `xchg bx,bx` vor den abgerollten Schleifen ist vermutlich `align 4`. Details in LEITFADEN_C.md.
- **Was nicht geht:** `align` und `dup` erzeugen in der OBJ einen Satztyp (LIDATA), den unser Linkwerkzeug
  `objorder.py` nicht kennt. Deshalb Füllbefehle per Makro und Nullbereiche per `rept`.
- **Variablen im Codesegment** werden als feste Versätze angesprochen wie im Original (kein Fixup); vor ihrer
  Definition benutzte Tabellen prüft ein Makro `AT` beim Assemblieren auf die richtige Lage.
- **Für die C-Seite (Auftrag A / Hauptsitzung):**
  - In SEG012 `draw_object` heißt der letzte Parameter von poly_edges `persp`. Er bedeutet „zeilenweise füllen“
    (waagerechte Fläche), nicht „perspektivisch“. Die Perspektive entscheidet die Wahl poly_edges/poly_edges_persp.
  - O032 legt ds:8C74..8C7B als `char g_8c74[8]` („unbenutzt“) an. Dort liegen aber g_proj_dist (8C78) und
    g_proj_scale (8C7A). Ein Umbau in O032 ist nicht Teil dieses Auftrags.
  - Alte Namen bleiben über `#define` im Block `/* Abschlusswelle C */` am Ende von s_proto.h gültig.

## Fehler im Original (ORIGINALFEHLER.md)

| # | Wo | Was |
|---|---|---|
| 350 | poly_tex_rows | der u-Schritt läuft nur in 16 Bit; ohne EMS kann eine Bodentextur über einer 64-KB-Grenze falsche Pixel zeigen (niedrig) |
| 351 | obj_project | die 129. sichtbare Fläche eines Objekts überschreibt x des ersten Punkts (Off-by-one, praktisch unerreichbar) |

Dazu Kuriositäten ohne Wirkung (Vergleich ohne Sprung, einpixelige Spalten fehlen bei Texturen, Teiler „Spaltenzahl“
statt „Abstände“).

## Prüfung

- Einzeltest je Änderung: beide Module mit TASM 3.1 /ml assembliert, OBJ gegen den Ausgangsstand verglichen
  (Codebytes, Fixups, Publics) → gleich. Gegenprobe mit einer absichtlich falschen Konstante schlägt an.
- `NUR_ASM=1 bash tools/build.sh` → „Stufe 1 (alles Assembler): BYTEGLEICH“.
- voller Build (`bash tools/build.sh`, 301 C-Module, 145 mit eigenen Daten): „Stufe 1 (alles Assembler): BYTEGLEICH“
  und „BYTEGLEICH mit dem Original“, MD5 `da3b59ce86a82e4b8bb14d36ed37d4d0`.
- Patches (`asm.patch`, `names.patch`, `andere.patch`, `-p1`) gegen eine frische Kopie von `ausgang.tgz`
  geprüft: `patch --dry-run` sauber, angewendet identisch mit dem Arbeitsstand.

## Offen

- C-Seite: Parametername `persp` in SEG012, Aufteilung von `g_8c74[8]` in O032, Kommentare in SEG012/O096 auf die
  neuen Namen ziehen (die `#define`s halten alles übersetzbar).
- face3d +0x08..0x0C (Normale in Kamerakoordinaten) und face_ent +0x04..0x09 sind nur als Kommentar beschrieben,
  nicht als eigene Felder (sonst ändern sich Feldnamen in fremden Modulen).
- ORIGINALFEHLER.md: die Zählzeile im Kopf („232 Einträge“) zieht die Hauptsitzung nach dem Zusammenführen nach.
