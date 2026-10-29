# Leitfaden: Assembler-Modul → bytegleiches C/C++ (Borland C++ 3.1)

Gesammelt aus 70+ fertigen Modulen von Projekt Sternenschweif (Stand 18.10.2026).

## Ablauf je Modul
1. Funktionsgrenzen: `grep ONNN q004/zerl/funktionen.tsv` (Overlays: Spalte 3 = Offset im Segment).
2. Erste Fassung schreiben, Optionen per Massentest (Stapel ≤ 32 BCC-Aufrufe je BAT): -O -Oe -Ol -Oc -Og -Ov -Z -Oi jeweils mit -3 -Ot, CPU -1/-2/-3 prüfen.
3. Funktion für Funktion mit `tools/funcdiff.py` / Seiten-Disassembly (`sd.py`) auf 0 Abweichungen bringen.
4. `tools/ccmp.py REF.OBJ NEU.OBJ` muss GLEICH melden, `tools/cobjfix.py NEU.OBJ REF.OBJ MAP OUT.FIX` muss durchlaufen.

## Namen und Daten
- Externe Namen frei (cobjfix ordnet über Fixup-Stellen und Datenadressen zu, setzt Overlay-Publics in Originalreihenfolge).
- Daten: `extern` mit Adressnamen (g_b77f). Adresse ohne eigenes Public: Element/Versatz eines Nachbar-Arrays (`g_3587[1]`).
- Code-Adresse ohne eigenes Public: `(char far *)R23_000F + 0x428`.
- KEINE eigenen Daten im Modul: Strings, initialisierte Variablen, globale Objekte liegen im Datensegment → extern. Statischer Initialisierer eines globalen Objekts als normale Funktion mit nachgebildetem Konstruktor (OVL224.CPP).
- Far-Daten außerhalb DGROUP: `extern ... far` (g_far_data, g_far_bss).
- Bekannte Klassen: O243 Dateiklasse, O248 Textklasse, R42 Bildklasse (10 Bytes; Bmp(w,h)=R42_00D8, Bmp(w,h,p)=R42_004B, data()=R42_072D, Destruktor R42_03B3), O047-Objekt 0x22 Bytes, Heldenstruktur 0x5EF Bytes (Statusbits `unsigned short b:1`), R07_041c/R07_046c = new/delete.

## Handgeschriebenes TASM (dann kein C)
Epilog `mov sp,bp / pop bp` obwohl -3 nötig (BCC macht `leave`), NOP-Auffüllung nach kurzen Sprüngen, Funktionen ohne Stackrahmen, Sichern von cx/bx. DOS-Aufrufe sind dagegen C: `_AH = 0x0d; geninterrupt(0x21);`.

Lesbar machen, ohne Bytes zu ändern (Abschlusswelle B, 16 ASM-Module):
- Linknamen `F_Rnn_xxxx`/`F_Onnn_xxxx` bleiben (Verweise aller Module, Datenstücke, `ovlpub.py`, cobjfix). Der sprechende Name steht als nahe Marke direkt hinter `F_… label far` (gleiche Adresse, kein Public, also keine Änderung an OBJ/MAP).
- Daten über `equ`-Konstanten ansprechen (`ds:[g_xms_entry]`), nie über Marken: Marken erzeugten Fixups, die das Original nicht hat. Ebenso Daten im Codesegment (AIL, Flat Real Mode) und unmittelbare Code-Offsets (`API_timer_ofs`).
- Datenbereiche, die asm2tasm als Befehle zerlegt hat, als `db`/`dw`/`dd` schreiben; Nullfelder mit einem `rept`-Makro (`ZEROS n`), nicht mit `dup`: `dup` ergibt LIDATA-Sätze, an denen `tools/objorder.py` abbricht.
- `[reg+Offset]` mit Offset < 80h bleibt `db`, wenn das Original die Verschiebung 16 Bit breit codiert (TASM nähme 8 Bit).
- Wiederholte Befehlsfolgen dürfen Makros werden (SEG033 `GET_BIT`, SEG022 `POP_F`, `PUSH_C`, `ADD_PTR`, `FAR_TO_HUGE` wie in Miles' AIL.MAC); Marken im Makro mit `local @@x`, dann bleiben `@@`-Marken des Rufers gültig. Vorwärtssprünge im Makro ergeben dieselben Bytes (kurz + NOP).
- `locals` statt `; keine lokalen Marken`; `@@`-Marken gelten bis zur nächsten normalen Marke (also auch bis zum nächsten sprechenden Namen).
- Schnellprüfung je Modul: nur dieses Modul assemblieren und Segmentbytes, Fixups und Publics der OBJ gegen die alte OBJ vergleichen; am Ende `NUR_ASM=1 bash tools/build.sh`.

## C++-Muster (Attic hat viel C++ benutzt)
- Konstruktor: `if (this == 0) this = operator new(n)` + toter Epilog. Destruktor: `(flags & 1) → delete` automatisch.
- Member-Objekte werden automatisch konstruiert/zerstört; lokale Objekte rückwärts zerstört; lokale Klassenobjekte auf geraden Adressen; `return x` mit Objekt im Rumpf geht über Zwischenwert vor dem Destruktor.
- inline-Funktionen erklären rätselhafte Locals: jede Parameterkopie eines inline-Setters/-Getters/`abs()` ist ein eigener Local.
- inline-Prädikat `return f >= 7;` → `mov ax,1 / xor ax,ax / or ax,ax`. inline-Getter auf char → `mov al / cbw / or ax,ax`; auf int als Byte gespeichert → `mov ax,[..] / mov [g],al`.
- `unsigned char`-Bitfelder nur in C++.

## Locals und Register
- Reihenfolge im Stack: Locals der Funktionsebene (erste bei [bp-2]), dann inline-Kopien und -Og-Zwischenwerte, dann Locals innerer Blöcke (bzw. Makro-Blöcke).
- -Og-Zwischenwerte: entstehen nur, wenn ein Wert mehrfach im selben Ausdruck bzw. Modul vorkommt; mehrfach indiziertes Tabellenelement → Offset in SI/DI, Segment jedes Mal nachgeladen.
- Register: explizite `register`-Locals bekommen SI vor register-Parametern; eine Variable ohne `register` landet dann ggf. in DI. Mit -Og bekommen nur `register`-Variablen sicher SI/DI. Ohne Aufrufe legt BCC Variablen in CX/DX; Merker in DX nur, wenn im inneren Block deklariert.
- Wert dreimal im Ausdruck → -Og-Register (`cmp di,mem`); echte register-Variable dreht zu `cmp mem,di`.

## Ausdrücke (jede Schreibweise ergibt anderen Code)
- `x -= 1` (mov/dec/mov) vs `x--` (dec mem) vs `x += (signed char)-1` (add byte 0ffh).
- `!g` (mov al/cbw/or) vs `g == 0` (cmp byte); `(signed short)c > 1` → cbw/cmp.
- `m & v` vs `v & m`; `a * b` vs `b * a`; `n *= 2` (imul) vs `n = n * 2` (shl); `<< 5` vs `* 32`; `c = 0 - c` vs `c = -c` (neg).
- signed vs unsigned (long: ja/jna, movzx/movsx); `(signed long)sizeof(x)` für movsx/imul.
- Zuweisung getrennt (`v = f(); if (v != -1)` → cmp si) vs eingebettet (→ cmp ax); `sel--; if (sel == -1)` vs `--sel == -1`.
- Zuweisungsketten werden rechts→links gespeichert. Zuweisungen lieber trennen (`pos = a << 8; pos |= b;`).
- `(signed char)(a + b)` → add al / cbw. `if (..) x = 10; else ;` → toter `jmp short`.
- for-Initialisierung `for (i = 0, n = 0, ...)` bestimmt die Reihenfolge der xor.
- Argumente werden von rechts gepusht, auch vor inneren Aufrufen.

## Kontrollfluss
- Toter Epilog nach return = if/else mit returns; `if .. return 1; else return 0;` → zwei tote Epiloge; Epilogkopien brauchen meist -Og (oder -Ov).
- Bedingte Zuweisung mit getrennten Stores = if/else, nicht `?:`. Verschachtelter `?:` → mov ax/jmp zum gemeinsamen Epilog.
- switch: Fälle in Quellreihenfolge, `default: break;` erzeugt eigenen Sprung, gleiche Rümpfe bleiben getrennte Fälle.
- Funktionen weiter oben im Modul → `push cs / call near`, weiter unten → far call (TLINK macht nop/push cs/call near): Reihenfolge in der Datei muss stimmen.
- `#pragma option -Og` usw. kann mitten in der Datei wechseln.

## Bibliothek
- -Oi: memcpy/strcpy/strcat/strchr als Intrinsics; Strukturkopie far→DS über SCOPY@, umgekehrt rep movsw.
- long-Rückgaben mit -3: `shl eax,10h / shrd eax,edx,10h` nach Aufrufen, `shld edx,eax,10h` beim return.
- Speicher-zu-Speicher-Kopie von far-Zeigern mit -3 als `mov eax`.

## Originalfehler
Zusaetzlich im Ergebnisbericht melden; zentrale Liste: Analyse/Originalfehler.md.
Bugs des Originals (dangling else, falsches Feld, Schleife mit falscher Bedingung, uninitialisierte Rückgabe) bewusst nachbauen und im Kopfkommentar vermerken.

## Quellen für Namen
Bright Eyes `src/custom/schick/rewrite_m302de` (DSA 1, viele Entsprechungen, z.B. DSA1 seg096 = O056, seg041 = O079) und `src/custom/schweif` (DSA-2-Namen).

## Nachträge (Welle K)
- Wert in Bedingung UND Zuweisung (`if (!(d & 0x80)) old = d;`) → -Og-Zwischenwert vor dem `les` des Ziels; eigene Variable → `les` zuerst.
- Konstanter Tabellenindex → -Og-Zwischenwert `(int)tab[k].x`; variabler Index → neu gerechnet (dann kein inline, sondern Makro/Ausdruck).
- huge-Zeiger-Member: Nulltest `mov ax,[off] / or ax,[seg]` statt `cmp dword`.
- `unsigned short * unsigned short` → `imul word` + `movzx eax`.
- `*(p + x + j)` (les/add bx,di/add bx,si) vs `p[x + j]` (erst ax = x + j).
- Lokale Strukturkopie aus DS: `struct s a = g;` → `mov si / lea di / push ss / pop es / rep movsw`.
- Stack: Aggregate (lokale Objekte, Arrays) liegen hinter allen Skalaren, Parameterkopien und -Og-Zwischenwerten; danach Block-Locals.
- Lokales Objekt mitten in der Funktion: Konstruktor an der Deklarationsstelle, Destruktor (&o,2) am Funktionsende rückwärts.
- Verschachtelte inline-Aufrufe: jede Expansion eigener Slot. inline-Setter mit int-Rückgabe, Ergebnis verworfen → loses `cbw`.
- register-Variable im Block ohne Aufrufe → CX, wenn SI/DI belegt.
- long mit int: `l *= a` → movsx eax / imul edx,eax; long an int-Parameter → `push word [l]`.
- switch auf signed char mit Fällen 0..5 → Sprungtabelle im Code.
- Feldtabellen: Elementzahl prüfen (falsche Zahl verschiebt alles danach).

## Nachträge (Welle L)
- Inline-Assembler: In .C ohne `-B`/`#pragma inline` nimmt BCC seinen eingebauten Assembler; der kodiert Register-Register-Befehle mit Richtungsbit 0 (`add si,di` = 01 FE), TASM die andere Form → oft ohne `db` bytegleich.
- `unsigned short b:1`-Bitfeld in 1-Byte-struct: Test `shr ax / and ax,1 / or ax,ax`; `!(signed char)x.b0` → `and ax,1 / cbw / or ax,ax` (auch in C).
- inline-Elementfunktion: `this` wird nicht kopiert, wenn das Objekt ein Parameter ist; `(x == -99 ? 0 : 1) && ...` → `mov al,0 / mov al,1 / or al,al`.
- signed-char-Wahrheitstest in &&-Kette: nur `!!p->f` ergibt `mov al / cbw / or ax,ax`.
- switch: Sprungtabellen hinter der Funktion (die des zweiten switch zuerst); fehlender case = default-Eintrag.
- `if (..) return 0; return 1;` → ein toter Epilog, mit else zwei; Ende eines if-Blocks bleibt jmp near.
- Variable im inneren Block ohne Aufruf → CX, auch wenn die Funktion sonst Aufrufe hat.
- unsigned-char-Parameter mit Konstante → `push 0cbh`; signed char → `push byte -35h`.
- `(signed short)(signed char)c < 0` → cbw/or; `(unsigned char)r` → mov al/mov ah,0; `r & 0xff` → and ax,0ffh.
- huge: `p += 0x10 - FP_OFF(p)` → PADA@; 3-Byte-struct aus DS-Array in Stack-Local → SCOPY@.
- Variable ohne register bekommt automatisch DI, wenn der register-Parameter SI hat.
- ACHTUNG Prüfung: ccmp.py prüft seit Welle L auch Verweise aufs eigene Segment (Sprungtabellen!). Vorher waren sie ausgeblendet; bei gleichen case-Rümpfen verrät nur die Sprungtabelle die Reihenfolge der Rümpfe (O196: case 7 steht vor case 6).

## Nachträge (Welle N)
- Methodenzeiger 8 Bytes: `class X;` vor `typedef int (X::*PMF)();` (unvollständige Klasse → allgemeinste Darstellung); `if (pm)` → or über 4 Worte, `(this->*pm)()` → call dword ptr es:[bx+..], Zuweisung aus tab[i] → SCOPY@ cx=8.
- Funktionszeiger in ein anderes Overlay: einfach den Funktionsnamen (TLINK lenkt Offset auf den Stub-Thunk 0x20+5n).
- `(signed char)(a ? 5 : 2) == 1` → mov al,imm / cmp al,1.
- Funktion ohne Aufrufe: nur ret register → ret in CX, i in DI; beide/keine register → ret DI, i CX.
- Dritte register-Variable bleibt auf ihrem Stack-Platz; Local hinter den Zwischenwerten = innerer Block.
- `R07_0002(n * sizeof(T))` → movzx eax; `n * 7` → movsx. Vergleich zweier far-Zeiger mit != → mov eax / cmp eax,mem.

## Nachträge (Welle O)
- -Oi: `memcpy(far, far, 64000U)` → push ds / mov ds,ax / rep movsw; Strukturzuweisung far→far → SCOPY@.
- 1-Zeichen-Literal bei strcat (Länge 2 als Konstante): ohne eigenes _DATA nur per asm (OVL238, OVL111).
- Kleine Strukturkopie aus DS (9 Bytes): einzelne mov-Paare statt rep movsw.
- `p = 0` bei far-Zeiger → zwei mov word; long 0 → mov dword.
- Im Original selbst als asm geschriebene Stellen erkennt man an TASM-Kodierung (`31 db` xor bx,bx; `89 d8` mov ax,bx) – dann asm mit -E TASM ist richtig (O004 MSCDEX). asm-Funktion mit Ergebnis in AX ohne return schreiben; `asm jc label` auf C-Marke geht.
- `(unsigned long)p[i]` → movzx; ohne Cast → mov dl / mov dh,0 / movsx edx,dx.
- char-Rückgabe: `f() == 0` → or al,al; `!f()` → cbw / or ax,ax.
- Abschließendes `return x;` hinterlässt auch ohne -Og einen toten Epilog. `return a == 0 ? 1 : 0;` → mov ax,1 / jmp / xor ax,ax.
- `&tr[t - 1].start` (7-Byte-Elemente) → t*7 + tr - 5; `&tr[t].start` lädt tr zuerst.
- Ein falscher Ausdruck kann Sprünge weit davor verschieben (short → near): bei Längenabweichung auch die Zweiglängen prüfen.

## Nachträge (Welle M)
- Große Locals (Arrays/Objekte ab 12 Bytes; 8 und 10 bleiben) legt BCC ans Ende ihres Blocks, hinter -Og-Zwischenwerte, inline-Kopien und innere Blöcke; untereinander Deklarationsreihenfolge.
- inline-Kopien liegen in Auswertungsreihenfolge (letztes Argument zuerst); ein Block mit eigener Deklaration verschiebt seine inline-Kopien hinter die übrigen.
- In C++ ist `atoi` inline (eigene Parameterkopie je Aufruf), in C ein Makro.
- `v = c ? a+b : ..;` rechnet in ax mit gemeinsamem Store; if/else rechnet in dx und speichert je Zweig.
- `cmp / jz short / jmp near Ende` = `if (!g) { ganzer Rumpf }`; `if (g) return;` mit -Og → Epilogkopie.
- `(signed long)(signed short)c` → cbw / movsx eax,ax; `(signed long)c` → movsx eax,byte.
- Rückgabetyp unsigned char → `test al,1`; int → `test ax,1`.
- R07_0140 (free) liefert NULL zurück → `p = R07_0140(p)`.
- Code-Adresse mit literalem Offset: `MK_FP(FP_SEG(R03_0006), 0x278)`.
- switch auf long → Vergleichstabelle im Code (mov cx,n / loop / jmp cs:[bx+4]).
- Gleiche Stringliterale werden nicht zusammengelegt (zwei "" auf benachbarten Adressen).

## Nachträge (Welle P)
- `return ausdruck;` am Funktionsende → toter Epilog auch ohne -Og; `break;` als letzte Anweisung des letzten case am Funktionsende → toter Epilog, ohne break keiner.
- Struct an variadische Funktion als Wert → Kopier-Helfer (lea / mov cx,1ch); Original übergibt meist die Felder einzeln.
- inline-Setter in reiner Datenstruktur (C++) → mov al / mov [bp-x],al / les bx / mov al,[bp-x] / mov es:[..],al.
- unsigned-char-Bitfeld: `h->b == 0 ? a : b` → and ax,1 / or al,al / jnz; `!h->b ? a : b` → mov ah,0 / or ax,ax.
- Ohne Aufrufe kann eine register-Schleifenvariable in AX landen; `register signed char`-Parameter in DL/CL.
- `y + tab[d].dy < 0` → add ax,[..] / jl (Flags der Addition).
- signed-char-Local als signed short zurückgegeben → cbw am Ende.
- Konstruktor mit this=0 / Destruktor mit Flag 3 auch in C als freie Funktion schreibbar.
- cobjfix „mehrere Adressen“ → meist veraltete OBJ mit falschen Optionen, neu bauen.

## Nachträge (Welle R)
- `new K(..)` mit nicht-inline Konstruktor → ctor(0, ..); `delete p` → dtor(p, 3) ohne Nulltest.
- Inline-Konstruktor mit erstem Member-Objekt: new → operator new, dann `if (p) member_ctor(p)`; delete → `if (p) { member_dtor(p,2); operator delete(p); }`; bei Basisklasse dtor(p,0).
- Verschachtelte inline-Funktionen erzeugen neg/sbb/inc + mov ax,1/xor ax,ax/or ax,ax (direkt geschrieben nur cmp/jnz).
- Initialisierung in Deklarationsreihenfolge, inkl. lokaler Objekt-Konstruktoren.
- Destruktor-flags zählen als register-Parameter (SI) → Schleifenzähler ohne register (DI).
- Strukturkopien aus DS mit 6/8 Bytes wortweise; initialisiertes Aggregat liegt oben ([bp-6]), nicht initialisiertes hinter den Skalaren.
- `if (tab[i])` (far-Zeiger) → mov ax,[bx] / or ax,[bx+2]; `tab[i] = 0` → zwei Wort-Stores, Segment zuerst.
- `x + i*step` (register i) lädt x zuerst (push/pop um imul); mit Member `x + cur*step` imul zuerst.

## Nachträge (Welle T)
- Mehrere char-Locals: erstes bei [bp-1], zweites bei [bp-2]; Reihenfolge der Zuweisungen ggf. über Initialisierung bei der Deklaration steuern.
- -Og-Zwischenwert für Offset p+i konkurriert um SI/DI: Local ohne register → DI, Zwischenwert SI; mit register vertauscht.
- `*(p->f + n + 1)` → les/add bx/[bx+2]; `p->f[n + 1]` → inc ax/shl bzw. mov ax,cse/add ax,f.
- Parameterkopien von inline-Elementfunktionen können in einem freien Register landen; konstante Argumente werden nicht kopiert.
- Lokales Objekt in innerem Block → Destruktor am Ende dieses Blocks.
- Aufrufe in Argumenten: Argumente des äußeren Aufrufs können vor dem inneren Aufruf gepusht werden; Zuordnung über die add-sp-Werte.
- `half = (x7 = lvl * 7) / 2;` vermeidet das Nachladen.
- Hinweis: O209 ist die Reisekarte (nicht Heldenerschaffung).

## Nachträge (Welle S)
- -Og-Zwischenwerte vor allen Block-Locals; Werte, die nach Speicherzugriffen neu gerechnet und gespeichert werden, sind Makros (z.B. `X = (g_9492 + dx)`), keine Variablen.
- `n = f(p); if (g != n ...)` vs eingebettet `g != (n = f(p))` (lädt g vor dem Aufruf, push/pop).
- inline-Getter mit long-Rückgabe → mov eax,[..] / mov si,ax.
- add-sp-Wert passt nicht zur Zahl der Pushes → das zuerst gepushte Argument gehört zum äußeren Aufruf.
- struct-Rückgabe: Funktion mit explizitem Zielzeiger deklarieren, sonst totes mov/add.
- Schleife ohne Sprung zur Bedingung am Anfang = do { } while.
- Sprungtabelle mit Rumpfreihenfolge 3, 2, 1, 4 = Durchfallen 3 → 2 → 1.
- unsigned char: `!(c = x)` → mov ah,0/or ax,ax; `(c = x) == 0` → or al,al.
- memcpy mit MK_FP(seg, longoffset) und -Oi → mov esi,edx (kein asm).
- register-Variable im Block ohne Aufrufe → CX; für SI in den umgebenden Block mit Aufrufen. Eine register-Variable kann für zwei Zwecke wiederverwendet werden.
- `*(signed long far *)&h[6] = 0` → mov dword.

## Nachträge (Welle Q)
- String-Intrinsic (strlen usw. mit -Oi) blockiert DI für register-Variablen der Funktionsebene → im inneren Block deklarieren.
- `volatile` auf Speicher-Locals verhindert das Teilen durch -Og (wenn das Original einen Ausdruck jedes Mal neu rechnet).
- Letzter case vor `default: break;`/`default: return;` → default springt auf Abschluss-jmp bzw. Epilogkopie.
- `f()->getname()` (inline) legt eine this-Kopie als Local an.
- Wert vor dem les des Ziels geladen und zweimal gespeichert → Ausdruck steht zweimal im Original.
- `while (buf[++j].f0 != 0xff)` (Pre-Increment in Bedingung) vs j++ im Rumpf (-Og legt &buf[j] ab).
- `&g[f - 1]` → imul, Basis, −Größe; `g + f - 1` rechnet anders. `_rotl(d & 0xf000, 4)` mit -Oi → rol ax.
- Ketten/Zuweisungen mitten in Argumenten möglich: `d = f(g, m = d = 0, 1)`, `b.put(dst, x = tab[n], y)`.
- cobjfix bei struct-Externs: Feldreihenfolge muss stimmen, sonst „mehrere Adressen“. Werkzeug tools/fsd.py: eine Funktion Seite an Seite.

## Nachträge (Welle U)
- Strukturzuweisung far→far: kleine Größen (z. B. 0x49) inline (mov ax,seg / mov si / les di / push ds / rep movsw / movsb), große (0x5ef) über SCOPY@. memcpy wäre falsch (lädt les di zuerst).
- -Og-Zwischenwert statt eigener Variable: mehrfach im Quelltext stehender Ausdruck (`g_c842[g_5974]`, `h->f78 + h->fbb`) → [bp-n] vor den Aggregaten; eine eigene Variable landet anders/länger. Erkennung: `mov [bp-2],dx / … / cmp dx,ax`.
- Bitfelder: 24 × `unsigned short b:1` belegen 3 Bytes, danach Füllbyte nötig.
- `if (..) return A; else if (..) {..} else return B;` erzeugt toten Epilog nach dem ersten return; ohne else nicht.
- Kette auf Feld +0: `a->x16 = a->x1b = p->x` → `les bx,a / push es / les si,p / mov al,es:[si] / pop es`.
- Locals innerer Blöcke liegen hinter -Og-Zwischenwerten; benachbarte Blöcke teilen Stackplätze.
- `g + (x+y)*10` → `mov dx,[g] / add dx,ax`; `(..)*10 + g` → `add ax,[g]`.
- Leerer Schleifenrumpf `for (j = 6; tab[--j] > v; ) ;` → jmp short auf den direkt folgenden Test.
- `if (!h) break; else if (A && B) ; else {..}` → toter jmp near hinter dem break.
- `!!x` auch als alleinige while-Bedingung.
- Werkzeuge: tools/sd.py (mehrzeilige ndisasm-Befehle behoben), tools/sdf.py NNN STARTS k (Funktion Seite an Seite trotz Verschiebung).

## Nachträge (Welle V)
- `register` oft wirkungslos: BCC (-r) verteilt SI/DI selbst; in allen fünf Modulen gleiche Bytes ohne register. Nur initialisiert+zurückgegeben bleibt [bp-2]. Ohne Aufrufe auch DX/BX.
- **-Oe** (globale Registervergabe) kann der Schlüssel sein (O269: Parameter in DX, Zeichen in DL/CL). Zeichen: Parameter nur bis zum ersten Aufruf benutzt, landet in einem Register. In anderen Modulen zerstört -Oe den Code → je Modul probieren.
- switch mit Sprungtabelle: ja-Sprung auf eigenes `jmp short` vor dem Ende nur, wenn letzter case ohne break in `default: break;` fällt; break + default → zwei jmp; ohne default direkt aufs Ende.
- inline-Konstruktor bei new: Parameterkopien in Parameterreihenfolge ab höchster Adresse, Stores in Rumpfreihenfolge.
- Long in Zeigerfeld: `(void far *)(long)` → shld + zwei Stores; mit Feld `unsigned long` und `(unsigned long)p` → `mov eax`.
- `*pp = 0` → `mov dword ptr es:[bx],0`; `*pp = &ds_obj` → `mov [bx+2],ds`. `f() >= 0` (long) → `or ax,dx / jl`. `b - 2` als Argument → `add ax,0fffeh`.
- Inline-asm mit TASM-Kodierung in C++-Elementfunktionen geht mit C-Marken; BCC ergänzt push si/di selbst.
- Sprungtabellen mehrerer switch am Funktionsende in umgekehrter Reihenfolge.

## Nachträge (Welle W)
- Index-Reihenfolge bei `p[n++]`/`p[n-1]` (Member): unsigned n → Index zuerst, dann les p; signed n → p zuerst, Index über es:si, -1 in die Verschiebung gefaltet. Entscheidet auch, ob beim strcat das Ziel vor dem Quellaufruf steht.
- `extern char g[2]` reicht nicht für strcat-Intrinsic mit konstanter Länge (mov cx,2) → asm (wie O238/O111/O260).
- `#pragma option -Og-` / `-Od` schalten -Og mitten in der Datei nicht ab.
- Zwei `new K` (inline-Ktor) in verschiedenen case-Zweigen → eigene Zwischenwert-Plätze; in eigenen Blöcken geteilt.
- Variable vorher als Zwischenspeicher benutzt (`i = t[0]; x = i % 5;` dann Schleifenzähler) → `mov si,ax` statt -Og-Stackwert. Dritte register-Variable ohne Aufrufe → CX.
- `return a || b ? 0 : 1;` → jnz/jz, xor ax,ax/mov ax,1; `return !(a || b);` setzt mov ax,1 zuerst.
- Bitfeld: `(signed char)h->b` im if → `or al,al`; `!(signed char)h->b` → `cbw / or ax,ax`.
- Fehlt der tote Epilog einer „void“-Funktion, gibt sie in Wahrheit etwas zurück (`return R04_0803(..)`).
- `f((void far *)l)` mit long l → zwei push word statt push dword (kann short→near-Sprung davor ändern).
- `cost * pct / 100` → movsx/push eax/mov eax,cost/pop edx/imul; `(long)pct * cost` → imul eax,mem.
- `if (!x) { …; return; } B;` legt B hinter die Epilogkopie des then-Zweigs.
- Zuweisungsreihenfolge im Konstruktor = Code, nicht Parameterliste; überladene Ktoren über verschiedene Klassentypen unterscheidbar.
- Werkzeug tools/sd2.py: difflib-Vergleich, Fixups je OBJ maskiert, Sprungziele/Datenadressen normalisiert (gut bei Längenverschiebung).

## Nachträge (Welle X)
- Byte, das im selben if/else zweimal gelesen wird (Flagbits von `g_b84a[w]`), wird -Og-Zwischenwert auf ungerader Adresse ([bp-9]); eigenes Local ergibt das nicht.
- `s->tmp[n++] = struct27;` → Zieladresse doppelt gepusht, nach SCOPY@ `pop ax / pop dx`; getrennt geschrieben anders.
- inline-Getter `signed long l41() { return (signed short)f41; }` → `cbw / movsx eax,ax` auch bei Byte-Ziel. `== 0` als `!(signed long)(signed short)x`.
- `tab[..] == (unsigned char)(dist - 1)` → `add al,0ffh`; mit (signed char)/(char) → `dec al`.
- `&g[e - 10]` (unsigned char e) → imul / add ax,[off] / add ax,−730; `g + e − 10` über dx.
- `step * k` (long × unsigned) → movzx/push eax/mov eax,step/pop edx/imul; `k * step` → imul eax,[step].
- Sprungtabelle mit Lücke: fehlender case zeigt auf default (nur über die Tabelle erkennbar).
- `case 4: x += 3; default: break;` legt den default-Sprung an eine eigene Stelle; äußeres `case 1: break;` hinter case 3 steht ganz am Ende.
- strcpy-Intrinsic mit globaler Quelle: Quelle zweimal geladen, Ziel über push dx/ax – entsteht von selbst.
- `f(..) == -1` → `cmp ax,0ffffh`. Globaler far-Zeiger `= 0` → mov dword (Member: zwei Wort-Stores).
- Schachbretttabellen: `*(tab + y*25 + x)`.

## Nachträge (Welle Y)
- Funktion ohne Aufrufe: mehrere `register signed char` in DL/BL/CL nach Deklarationsreihenfolge (erste DL, zweite BL, dritte CL).
- Registervergabe folgt der Deklarationsreihenfolge (j vor k, sonst bekommt k DI); ohne register und ohne Aufrufe: erste SI, zweite DI, Block-Local CX.
- `!merker` mehrfach im Block → (int)merker in SI/DI gecacht, bei vollen Registern auf dem Stack (`cmp word [bp-24h],0`).
- `n > i` → `cmp [bp-x],si / jg`; `i < n` → `cmp si,[bp-x] / jl`.
- Far-Zeigerdifferenz als long: `p - g + 4` → xor dx / sub / sbb / idiv ebx / shld; `g + (short)p - 4` → `add bx,0e844h`.
- `if (f()) ; else {..}` → jz / jmp short auf Epilogkopie.
- Mehrere Bits desselben Flag-Bytes: Byte in eigenem Slot (`mov [bp-3],al`), Tabellenoffset in weiterem.
- `x * 3` → mov ax,si / shl ax,1 / add ax,si.
- Auch der letzte case mit break erzeugt jmp short auf das direkt folgende Ende. Dichte aufsteigende switch lassen sich aus der Sprungtabelle generieren (tools/gen/gen211.py).

## Werkzeugfehler cobjfix (18.10., nach Welle V/W)
- ASM `push seg F_R04_000C / push 055dh` (Segment über irgendein Symbol des Moduls, Offset als Zahl) → C hat `callback(R04_055d)` mit seg- UND offset-Fixup. cobjfix nahm den Namen der Segment-Fixup (F_R04_000C) → Offset 0x000C statt 0x055D. ccmp maskiert Fixup-Felder und sieht das NICHT, nur der Komplettlink. Behoben: Offset-Fixups ohne ASM-Fixup werden gegen den Namen geprüft und ggf. auf das Public an der richtigen Stelle umgebogen (Abbruch, wenn dort keins ist).

## Nachträge (Welle Z)
- Inline-Elementfunktion mit zwei einfachen Argumenten: Kopien in Argumentreihenfolge (erstes am höheren Slot) – widerspricht teils dem Welle-M-Hinweis, je Fall prüfen.
- `case 1: …; case 2: break;` (case 1 ohne break) ist der einzige Weg, dass case 2 auf den echten Epilog springt; mit break → toter Epilog.
- -Oi: `*local = *global` (far) → push ds / mov ds,ax / rep movsw / movsb, Quelle zuerst; memcpy lädt Ziel zuerst; `*global = *local` → SCOPY@.
- `volatile short k` (z. B. `used[k = i]`) erzwingt Stackplatz statt CX.
- `left -= n = chunk; …; total += n = chunk;` → zwei movzx-Stores in denselben Slot.
- `from++; if (from == 4)` → cmp dx,4; `++from == 4` → mov ax,dx / cmp ax,4.
- `s->w * s->h` (short*uchar) → h zuerst, push/pop, imul dx; `s->h * s->w` → imul word [mem]. `l * n` (long*int) → movsx eax / mov edx,l / imul edx,eax; `n * l` → imul eax,[l].
- Byte in einem unbenutzten Feldplatz (+0x15 in pics[0]) → anonyme union.
- Epilogkopien mit -Og: manche returns brauchen else-Form; toter jmp short am Ende eines äußeren if-Blocks ← `} else ;`.
- Member namens open/close → `::open`/`::close`.

## Nachträge (Welle AA)
- Ohne `register`: SI/DI bekommt die zuerst deklarierte geeignete Variable, nicht die meistbenutzte → Deklarationsreihenfolge anpassen.
- Funktionszeiger in ein Overlay: ASM `push seg F_Oxxx / push 20h` = Thunk im Stubsegment (0x20 + 5n: CD 3F, dw Offset). In C einfach den Funktionsnamen nehmen; cobjfix liest die Zielfunktion aus dem Thunk der Original-EXE.
- Globaler far-Zeiger = 0 mit zwei Wort-Stores (Segment zuerst) → als zwei int-Zuweisungen schreiben.
- `_vector_new_` im statischen Initialisierer als normaler Aufruf `_vector_new_(g, 0x49, 6UL, 5, Ktor)` mit Deklaration `(void far *, unsigned, unsigned long, unsigned, ...)`.
- `void main(register int argc, char **argv)` → `_main`. Klasse ohne Destruktor: delete direkt über R07_046C ohne Nulltest.
- O032 ohne -Oi: strcpy/strcat sind dort echte Aufrufe.
- Inline-asm in .CPP: `xor cx,cx` → 31 C9, `cmp bx,0ffffh` → 81 FB FF FF; `return _CX;` → mov ax,cx + toter Epilog.
- `set_x(v) { f28 = f26; f26 = v; }` mit `set_x(p->f26)` → f26 als -Og-Zwischenwert in SI, Parameterkopie [bp-2].
- `f164 += (signed char)2` → add byte es:[..],2; `+= 2` → mov al / add al / mov.
- switch mit nur einem case: mov ax / cmp ax / jz short / jmp near. `default: break;` direkt hinter case ohne break → default auf Hauptepilog + toter Epilog; `break; default: break;` → zusätzlicher Epilog.
- `if (a || b) { } else {...}` → jmp near hinter dem then-Zweig.
- `r = f(); if (r == 0) do {...} while (r == 0);` → Test oben und unten.
- Alle Locals in einem else-Block → -Og-Zwischenwerte bei [bp-2…] vor den Block-Skalaren.

## Werkzeug (18.10., nach Welle AA)
- build.sh bricht jetzt ab, wenn cobjfix scheitert (vorher wurde still die ASM-Fassung gelinkt – so waren O069/O226 kurz unbemerkt ASM).
- cobjfix: Overlay-Thunks werden über das Stubsegment der Original-EXE aufgelöst.

## Nachträge (Welle AB)
- **-rd** (nur register-Variablen in Register) kann der Schlüssel sein (O140: sonst bekommt ein Block-Local DI und die register-Variable fällt auf den Stack). Bei anderen falsch → je Modul testen.
- `(char far *)p + (unsigned short)(h * w)`: nur mit unsigned-Cast Produkt vor dem les; ohne Cast push bx/es ums imul.
- 3-Byte-Struktur als Wertparameter: `p->f56 = v;` → ein les, dann mov ax,[bp+0ah] / mov al,[bp+0ch].
- `switch (p->f4c)` + später `g = p->f4c` → gemeinsamer cbw-Zwischenwert (mov [bp-2],ax / dec ax / mov bx,ax).
- switch, jeder Fall endet mit return; → `default: return;` für den toten Schluss-Epilog.
- Vergleichsketten-switch: letzter case ohne break fällt in `default: break;` → Nicht-Treffer per jmp near zum Epilog.
- Variadische Elementfunktionen: va_start → lea / mov [bp-2],ss / mov [bp-4],ax; va_arg(ap, char far *) → add word [ap],4 / es:[bx-4]; va_list um Aufrufe gesichert (save = ap; …; ap = save;).
- Abgeleiteter Konstruktor: zuerst Basis-Ktor (far call, wenn weiter unten), dann Member-Objekte (R42_0008), dann Rumpf.
- Verworfener Rückgabewert: fehlendes mov si,ax vor gemeinsamem mov ax,si.
- sar ohne cwd = `>> 1`, nicht `/ 2`. `f13 += (signed short)v` (long v) → mov ax,[v] / add es:[..],ax.
- Werkzeug tools/fd.sh NNN STARTS: sd2 je Funktion.

## Nachträge (Welle AC)
- Initialisiertes lokales Feld mit nicht-konstanten Werten (`char far *n[7] = { txt[16], … }`): BCC legt statische Kopie im DS an, füllt sie, kopiert per rep movsw ins Local (z. B. ds:60c0 in O103). Ohne eigene Daten: extern struct, Elementzuweisungen, Strukturkopie.
- Far-Zeiger aus es:[bx+…] (über les) → mov ax/mov dx-Paar statt mov eax.
- Großes Objekt in innerem Block liegt hinter dem tiefsten Platz aller Geschwisterblöcke, teilt nicht mit kleinen Block-Locals.
- Intrinsic oder Strukturkopie auf Funktionsebene → dort bekommt keine register-Variable ein Register (auch nicht SI); im inneren Block mit Aufrufen klappt SI/DI.
- Ohne Aufrufe: register-Parameter + Local ohne register → Parameter DX, Local CX; mit register auf beiden vertauscht.
- `g->setlvl(g->f22 = 0)` → Wert ins Feld und in die Parameterkopie.
- `if (x <= 40) ;` → cmp / jg auf den direkt folgenden Befehl (verirrtes Semikolon im Original!).
- `p->txt[r + 60]` → Konstante in die Verschiebung ([bx+0f0h]).
- `h = f(); if (h == 0)` (far) → cmp dword [bp-x],0; `(h = f()) == 0` → or ax,dx.

## Nachträge (Welle AD)
- inline-Getter `int getdir() { return dir; }` auf char-Feld: mit -Og Byte-Zwischenwert [bp-x] + int-Zwischenwert in DI, Vergleiche als int (cbw / cmp ax,k); direkter Feldzugriff vergleicht als Byte.
- inline-Getter mit unsigned-char-Rückgabe lädt bei Zuweisung den Wert VOR der Zieladresse (`tab[j].id = e->getid()` → mov al / lea dx).
- `x += (signed char)-1` → add byte 0ffh nur bei unsigned char; bei signed char immer dec byte.
- `ap += (signed long)(unsigned long)c` → movzx eax / add mem,eax; nur (unsigned long) → mov edx / add / mov.
- Objekt direkt im switch-Bereich ohne Block (`case 1..4: C166 o; … break;`) erlaubt, wenn nur default folgt → eigener Stackplatz; mit Block geteilt.
- Große Locals (ab 12 Bytes) hinter dem tiefsten kleinen Local aller Blöcke, je Block überlappend. Unbenutzte Locals belegen keinen Platz.
- Funktionszeiger ins eigene Overlay (qsort-Vergleich) läuft auch über den Thunk – Funktionsname genügt.
- `g[i] = g[0]` (far-Zeiger-Feld) → mov eax,[+2] / mov dx,[+0] von selbst.
- `g->set20(g->set20(0))`: inneres Ergebnis wird Parameterkopie des äußeren.
- **UNGELÖST (O030):** switch mit Sprungtabelle auf Registerparameter n: Original `mov ax,si / dec ax / mov bx,ax`, BCC erzeugt `mov bx,si / dec bx`. Erfolglos: alle Optionen, Casts, switch (n-k), enum/unsigned/volatile/char-Parameter, inline-Identität, C statt C++. (Vgl. Welle AB: das Muster entsteht, wenn der switch-Wert ein -Og-Zwischenwert ist, der später nochmal gebraucht wird.)

## Nachträge (Welle AE)
- `((signed char far *)h)[k + 0x14f]` → Index zuerst, dann mov es,[p+2] / add bx,[p]; `h->spells[k]` lädt zuerst les bx,h. (unsigned)-Index rechnet in ax; volatile verhindert den -Og-Zwischenwert.
- Bitfeld-Getter mit unsigned-char-Rückgabe auf Aufrufergebnis → this-Kopie als Local, `and ax,1 / or al,al`.
- `add byte es:[..],imm` auf Member eines Member-Zeigers kommt von inline-Elementfunktion `add24(5)` (konstante Argumente nicht kopiert); `e->f24 += 5` direkt ergibt mov al / add / les / les / mov.
- register kann doch wirken (O161): nur mit register auf blocked/nx2 bleibt ny2 auf dem Stack – alternativ beide ganz vorn deklarieren.
- Parameter `signed short` → push word [bp+0ch]; `signed char` → mov al / push ax.
- Freie Funktion mit gleichnamigem Local/Member: `::cost(..)`.
- Auch `x++; else ;` erzeugt den toten jmp short.
- `f1a++; if (f1a == 0)` statt `if (++f1a == 0)`, sonst fehlt erneutes les/cmp.
- `!(unsigned char)p[i]` → mov al / mov ah,0 / or ax,ax.
- `a->x1f0 = a->x1f1 = a->x1f2 = g_c688 = 0` → mov al,0, Stores c688, +1f2, +1f1, +1f0.

## Nachträge (Welle AI)
- Statischer Initialisierer mit Ausdrücken → normale Funktion mit Zuweisungen; -Og legt Mehrfachausdrücke (`c[k].x + 17`) als Stack-Zwischenwerte ab, auch über Stores in andere Externs hinweg (O008: 32 Plätze).
- Statische Locals mit Wächterbyte ohne eigene Daten: `if (g_0506 == 0) { g_0506++; g_0502 = g_cb74; }`.
- `h->f14e > 0` (unsigned char) → cmp byte,0 / ja.
- `int ok() { return bc0 || bc2 ? 0 : 1; }` → xor/mov ax,1/or ax,ax; Bit vorher im if → -Og cacht Bitbyte und Bitwert.
- -Og hält `&h->tal[t]` in DI, Segment per mov es,[bp+8] nachgeladen.
- Initialisierte 6-Byte-Strukturen hinter den Skalaren ([bp-12h]) – Welle-R-Hinweis „oben“ gilt nicht allgemein; 12-Byte-Feld ans Ende; Initialisierung in Deklarationsreihenfolge.
- `tab[(c ? a : b) + x]` → x zuerst gepusht, add ax,dx; `x + (?:)` → add dx,ax.
- `if (cur != t) … other = cur;` (far-Member) → 4-Byte-Zwischenwert.
- `f() != h` (far) → cmp dx,[seg] / cmp ax,[off] mit near-Sprüngen.
- `set1a6(f() - 1)` mit char-Parameter → dec al.

## Nachträge (Welle AH)
- Verschachteltes inline `g->set1e(g->set1e(8))` mit `signed short set1e(signed short v) { if (!(f1e & 0x80)) f1f = f1e; else ; return f1e = v; }` erklärt doppelten Test, Parameterkopie (cbw) und loses cbw.
- `c = g->getmoney()` (signed long Getter) → mov eax / mov [bp-2],al; direkt an char nur mov al.
- `(unsigned long)o->fca * 1000` → movzx eax,byte / imul eax,eax,3e8h; `o->fca * 1000L` → mov al / mov ah,0 / movsx eax,ax.
- `k++; } while (k < 5);` → inc word [bp-x] / cmp word [bp-x],5; `while (++k < 5)` → inc / mov ax / cmp ax.
- Signed-char-Wahrheitstest auch in ||-Ketten: nur mit `!!` cbw / or ax,ax.
- `if (n == 0) return 0; else { …; return x; }` → zwei tote Epiloge; ohne else fehlt der erste.
- switch am Funktionsende: breaks werden mit -Og von selbst zu Epilogkopien.
- Unbenutzte lokale Variable kann trotzdem Stack reservieren (O114 f_12c3; vgl. Welle AD – je nach Fall).
- Initialisiertes Local-Array ohne eigene Moduldaten: `struct sel7 sel = g_00a8;` (großes Local am Ende).
- Werkzeug: selfoff.py nimmt Funktionszeiger auf eigene Overlay-Thunks (ASM 0x20+5n, C = Funktionsanfang) aus – vorher Fehlalarm bei O002.

## Nachträge (Welle AF)
- auto-Aggregat mit nicht-konstanten Initialisierern: Werte in DS-Vorlage, dann rep movsw auf den Stack. Ohne eigene Daten: `g_400c.e[i].s = txt[..]; … struct tab16 arr = g_400c;`.
- `(c > 1 ? 1 : 0) ? 4 : 0` → mov al; `((c > 1) != 0) ? 4 : 0` → mov ax,1 / xor ax,ax / or ax,ax.
- inline-Elementfunktion mit zwei Argumenten kopiert auch register-Parameter; mit einem nimmt BCC SI direkt.
- `default: break;` hinter dem letzten case (auch wenn der ein break hat) → default auf echten Epilog, cases bekommen Epilogkopien.
- `if (..) { ..; return 1; }` ohne else → kein toter jmp hinter der Epilogkopie.
- Locals in Reihenfolge des ersten Auftretens (auch mitten im Rumpf deklarierte Objekte und inline-Kopien).
- `volatile` + eingebettete Zuweisung (`e = &tab[typ = ..]`) → Stackplatz ohne Nachladen.
- Zwei Locals ohne register in Funktion mit switch: Ergebnis CX, anderes SI; mit register vertauscht.
- `running() & 1` (unsigned char) → test al,1; ohne & 1 → or al,al.
- **UNGELÖST (O014, auch O046 ds:4c25, O097 +0e):** `mov dx,0bbdh / add dx,0ch / add ax,dx / mov bx,ax / push [bx]` – Basis und Feldversatz getrennt im Register; BCC faltet immer zu [bx+0bc9h] (~100 Schreibweisen, alle -O). In O014 vorerst als asm (Adressrechnung), Aufruf in C mit _BX. Idee für später: Zeigerarithmetik über (char *)-Cast eines near-Zeigers, Funktionsparameter-Kopie, Makro mit Zwischenzeiger-Variable die -Og wegoptimiert.

## Nachträge (Welle AG)
- asm-strcat gilt für jedes DS-Literal bekannter Länge (`repne scasb / dec di / mov cx,n / rep movsb`), nicht nur 1 Zeichen (O068 "^/^" cx=4, O106 ".GAM" cx=5). → Vermutung: das Original hatte echte Literale im Modul; mit „Daten zurück in die Module“ lösbar.
- switch ohne default am Funktionsende mit `case 2: return;` als letztem Fall: Nicht-Treffer → Epilogkopie hinter den Vergleichen; `default: return;` erzeugt immer einen Zusatzsprung.
- -Oi memset: unsigned Länge (`sizeof(T) * n`) → Ziel zuerst, push es/di/ax … pop; signed Länge (`n * 7`) → Länge zuerst.
- -Oi strcmp(a, b): a in DS:SI, b in ES:DI, repe cmpsb / sbb ax,ax / sbb ax,-1.
- `len = g_28ed - g_27c3` → mov ax,off / xor dx,dx / sub / sbb / shl eax / shrd (cobjfix ordnet Nachbar-Publics zu).
- `if (A != 0 || B != x || n != 16) {lang} else {kurz}` → kurzer Zweig ans Ende; &&-Form vertauscht.
- Local hinter inline-Parameterkopien → im inneren Block deklarieren.
- Ohne Aufrufe: zuerst deklarierte Variable bekommt DI.
- Unbenutztes char-Local hält seinen Stackplatz (vgl. AD/AH: je nach Fall).
- Daten: g_3587 = drei Format-far-Zeiger (Temp-/Spielstandpfad); g_382d/g_38b6 +0x4f = Sound.enabled.
- Werkzeug tools/vv.py: Varianten je Unterordner mit gleichem Dateinamen (Segmentname bleibt), Stapel zu 32, ccmp.
- Werkzeuge sd.py/sd2.py/sdf.py/fsd.py/fd.sh: ohne OBJ-Argument wird `$WOV/OVLNNN.OBJ` genommen (Standard: aktueller Ordner) – keine fest verdrahteten Wellenordner mehr. Aufruf z. B. `WOV=wovxx python3 tools/sd2.py NNN start end`.

## Nachträge (Welle AK)
- if-return-Ketten ohne Locals: Epilogkopien (pop bp / retf) auch ohne -Og; mit else je ein toter Epilog; `mov ax,k / jmp short` zum gemeinsamen Epilog nur mit verschachteltem ?:.
- `dmg = t = f()` → dmg aus AX; `t = f(); dmg = t;` → aus SI.
- `(signed short)a > (signed short)b` (signed char) → mov al / cbw / push ax … cbw / pop dx / cmp dx,ax; ohne Cast cmp al,mem.
- `(p + (signed short)((long)p * (long)pct / 100)) >> 1` → 16-Bit add/sar; ohne short-Cast legt -Og den long auf den Stack.
- `p->txt[a - 1]` mit Nicht-Register-Index → Index zuerst, Zeiger als mov dx,[f+2] / add ax,[f]; mit register-Index zuerst les bx.
- Lokales Bmp, das ein 3-Byte-Feld überlappt → beide in eigenen Geschwister-Blöcken.
- 7-Byte-Kopie aus DS → drei Wort-movs + ein Byte-mov.
- Leere ""-Literale: cobjfix ordnet `extern char g_7718[]` korrekt `_s_7713+5` usw. zu.
- `e->mv_x(e->set_x(0x12))` mit Rechnung im Setter faltet zur Konstante; Ergebnis als Parameterkopie in DI bzw. [bp-8].

## Nachträge (Welle AJ)
- **O030 weiter ungelöst** (Stand: offen/OVL030.CPP). Zusätzlich erfolglos: Parameter ohne register / lokale register-Kopie, `switch (k = n)`, `(n = n)`, `n += 0`, `n & -1`, `n ^ 0`, `n >> 0`, `~~n`, `-(-n)`, `*&n`, long/float/Zeiger-Umwege, enum mit -b-, register volatile/const, _SI/_AX = n, alle inline-Varianten (switch in inline wird nicht expandiert), Optionen -Op -Om -Oa -Ob -Os -O1 -O2 -Ox -O- -Z- -k -N.
  Mechanismus: `dec ax / mov bx,ax` nur, wenn AX einen frisch berechneten Wert hält (cbw, mov ah,0, Ergebnis eines ?:); Kopie einer Variablen → `mov bx,ax / dec bx`. Das Muster kommt im ganzen Spiel nur in O030 vor (4×), 13 andere Stellen haben `mov bx,si / dec bx` → Verdacht: O030 mit anderer Compilerversion/IDE übersetzt.
- `x /= 3` lädt zuerst den Divisor (mov bx,3 vor mov ax,si); `x = x / 3` umgekehrt.
- `abs()` in C++ inline mit Parameterkopie; `__abs__(x)` → mov ax,si / cwd / xor / sub.
- `n * f()` → push ax / mov ax,n / pop dx / imul dx; `f() * n` → imul word [n] (ebenso Member).
- Funktionen ohne Aufrufe: manchmal nur ganz ohne register die Originalregister (CX/DX/SI bzw. SI/CX/DI).
- `if ((h->ae += v) > h->aemax)` eingebettet spart das zweite les. inline mit long-Member: `ap = ap + v` → mov eax / add eax / mov; `ap += v` → add mem,eax. long-Parameter mit int-Argument → movsx-Kopie.
- Zweig hinter die Epilogkopie: `if (!a && !b) { … } else { Zweig }` statt `if (a || b) { Zweig } else …`.
- Byte-Feld über mehrere Setter-Expansionen → gemeinsamer -Og-Byte-Zwischenwert; Setter als Makro wiederverwendbar.

## Nachträge (Welle AL)
- Klemm-Addierer inline `void add_c(i, v) { poi[i].c += v; if (poi[i].c < 0) poi[i].c = 0; else ; }`: konstantes v → inc/dec byte bzw. add byte imm; nur variables v bekommt char-Kopie; je Expansion zwei -Og-Zwischenwerte.
- `(unsigned char)h->bc6 != flag` → and ax,1 / cmp al,[bp-x]; ohne Cast int-Vergleich.
- `!b % 12` → neg ax / sbb ax,ax / inc ax vor cwd / idiv (Präzedenzfehler im Original erkennbar).
- strcmp mit -Oi blockiert wie strcpy die register-Variablen der Funktionsebene.
- -Og hält (unsigned)t eines uchar-Locals in SI und lädt nach jedem Aufruf neu.
- Sprung direkt auf gemeinsames `mov ax,[ret]`: `if (A) {...} else { …; if (ret) {...} } return ret;`.
- `dy = (y = ev->y) - g_949a;` → mov [y],ax / sub ax,[g] / mov si,ax.
- `-(0x100L - first) * 3` → movsx eax,si / mov edx,100h / sub / neg / imul edx,edx,3.
- unsigned-long-Parameter: `n * 3` → movsx eax; `n * 3U` → movzx eax.
- Ein Wort-Slot für zwei Zwecke → eine Variable für beides.
- Heldenstruktur: Attribute ab +0x46 als {Grund, aktuell, Mod} (OVL077); OVL076 nennt dasselbe a[] ab +0x47 – später vereinheitlichen.

## Nachträge (Welle AM/AN) und Werkzeuge
- `#pragma option -rd` wirkt mitten in der Datei (vor einer einzelnen Funktion).
- **ccmp prüft jetzt auch eigene Daten:** Hat die C-OBJ ein eigenes _DATA (z. B. durch `strcat(dst, ".CHR")`), meldet ccmp „EIGENE DATEN“ statt GLEICH (vorher übersehen). cvsasm ebenso.
- strcat-Intrinsic scannt zuerst die Quelle: `strcat(old, o->f4)`. `memcpy(d, s, strlen(s))` erkennt man an strlen mit dec cx; strcpy ohne dec.
- `struct slot item(k) { return inv[k]; }` mit `a[n] = obj->item(j)` → Zielzeiger als Local + Parameterkopie vor SCOPY@.
- strcat irgendwo auf Funktionsebene sperrt register-Variablen der Funktionsebene (strlen/memset nicht unbedingt) → im if-Block deklarieren.
- `t = g->f22 = 9` speichert das Feld zuerst.
- Objektgröße aus `sub sp,..` abzüglich Skalare bestimmen (O104: O269-Objekt 0xa6).
- Generatoren für repetitive Rümpfe aus der ASM lohnen sich (wovam/gen/g067.py, g207.py).
- `if (x == -1) break;` im case springt direkt hinter den äußeren switch; als if-Block landet der Sprung auf dem break-jmp.
- `if (g == 0) ; else if (..) ..` → jnz short Rumpf / jmp short Ende.
- Struktur aus DS ins Local: mov si / lea di / push ss / pop es / rep movsw; memcpy-Intrinsic: push ss / pop es / lea di / mov si / rep movsw.
- `(signed short)l < 100 ? (signed short)l : 100` → cmp word [bp-x],64h auf dem long.
- Mitglieder mit Ktor/Dtor werden nach dem new-Test automatisch konstruiert, am Dtor-Ende rückwärts mit Flag 2 zerstört.
- switch auf char-Parameter im Speicher → mov ax,.. (cbw) / dec ax / mov bx,ax von selbst (zum O030-Registerfall nichts Neues).
- **Hilfsskripte jetzt zentral in tools/:** c.sh, mass.py, var.py, var2.py arbeiten mit `WOV=<Arbeitsordner>` (Umgebungsvariable), nicht mehr kopieren.

## Nachträge (Welle AO)
- Aufrufe machen -Og-Zwischenwerte ungültig: Ausdruck wird nach jedem Aufruf neu geladen und in denselben Slot geschrieben → einfach mehrfach hinschreiben. (Stores in andere Externs machen ihn nicht ungültig.)
- `signed short count() { return f2e; }` (char-Feld): Zuweisung an char-Local → mov al / cbw / mov [local],al; Vergleich → cbw / cmp ax,k.
- char-Local mit `mov al,[r] / cbw / cmp ax,k / jz / jmp near` = switch mit einem case + default, kein if.
- inline-Setter mit zwei Argumenten, Argument mehrfach vorkommend (`g->gox(c->f26, c->f26*0xa8+0x54)`) → -Og-Zwischenwert hinter allen inline-Kopien.
- Bright Eyes seg104 ↔ O090 (skill_cure_disease, has_herb_for_disease, plan_alchemy; Rezepte ds:196a à 0x1c).
- `f(…, (unsigned short)(n * 4), 0)` an long-Parameter → shl ax,2 / movzx eax,ax; long als ein Wort gepusht = (unsigned short)m.
- Klasse mit zwei Member-Bmp: Ktor/Dtor komplett von BCC erzeugt, Dtor-Rumpf leer.

## Nachträge (Welle AP)
- `return (signed long)(v * 100);` → movsx eax,ax / cwd (nicht shld).
- `(signed char)h->fbc.b6 != b6` → Byte-Vergleich; ohne Cast int.
- `(signed short)g->dir == 3` in ||-Kette genügt für int-Zwischenwert (cbw / mov [bp-x],ax), kein Getter nötig.
- `if (--g_c696 < 1)` → dec word / mov ax,[g] / cmp ax,1; getrennt → cmp word [g],1.
- `k = e->w[j = e->cur]` → cbw / mov [bp-x],ax / shl ax,1 ohne Nachladen.
- BCC nutzt außerhalb eines Blocks SI als Hilfsregister (les si,..) → register-Variable in SI in die inneren Blöcke.
- register-Parameter t bekommt SI, wenn zweiter Local r ohne register (DI); mit register r vertauscht.
- `g_8bf5 = (void far *)R02_025d;` speichert Segment, dann Offset; cobjfix biegt seg-Symbol auf das richtige Public.
- long-Local nur eingebettet (`(cost = g_2813) > f2f`) sieht aus wie -Og-Zwischenwert, ist aber eigene Variable.

## Nachträge (Welle AQ)
- `&p[(a - i) * k + 0x135]` → add ax,[p] / add ax,135h; `p + (..)` → mov dx,[p] / add dx,ax.
- Aufruf in einer Summe wird immer zuerst ausgewertet, die geschriebene Reihenfolge entscheidet das Register: `g * 20 + f() - 10` → pop dx / add ax,dx; `f() + g * 20 - 10` → add dx,ax.
- inline-Getter `signed short get()` an signed-char-Parameter → mov al / cbw / push ax; direktes Feld ohne cbw. int-Argumente als mov al / push ax → Prototyp mit char-Parametern.
- `return f == 0x9d ? 0 : 1;` → xor ax,ax zuerst, dann mov ax,1 / or ax,ax.
- Begrenzender Setter `add(signed char v) { f += v; if (f < 0) f = 0; else ; }`; Argument `-x` → neg al.
- default mit if/else am Funktionsende: erst `break;` am Ende des default lässt den then-Zweig auf dessen Epilogkopie springen.
- sd2/fd.sh zeigen Scheinabweichungen bei DS-Adressen ohne Fixup in der Referenz – ccmp/cobjfix sind maßgeblich.

## Nachträge (Welle AR)
- Ausdruck zweimal im selben Aufruf (`sprintf(.., h->name, h->name, ..)`) → -Og-Zwischenwert, danach bei jedem h->name neu gerechnet und gespeichert (entsteht von selbst).
- Summe von `unsigned short b:1` in unsigned-char-Ziel → bytebreit (add al,dl); `== 1` bleibt int (and ax,1 / cmp ax,1); mit `unsigned char b:1` → cmp al,1.
- inline-Setter auf Bitfeld mit signed-char-Parameter: nur mit `unsigned short b:1` → mov al / cbw / and ax,1.
- Index-zuerst-Zugriff auch für 3-Byte-Eigenschaften: `((signed char far *)hero)[sel * 3 + 0x5b]`.
- Strukturkopie aus DS ohne eigenes Public: `struct inc12 ta = *(struct inc12 near *)&g_5a72[13];`.
- switch mit gemischter Erzeugung (Vergleiche + Sprungtabelle) entsteht von selbst aus der case-Reihenfolge.
- Hinweis OVL087: Kopfkommentar vertauscht Talente und Zauber (+14f[0x56]/+1a5 = 86 Zauber, +14e = 52 Talente).

## Nachträge (Welle AS)
- memcpy -Oi mit Member-Quelle (`memcpy(dst, this->h, 0x5ef)`) lädt die Quelle zuerst – „Ziel zuerst“ gilt nicht allgemein; `*dst = *h` → SCOPY@.
- `h + (unsigned)(k*14 + c)` → Index in ax, mov dx,[h] / add dx,ax; ohne Cast add ax,[bp+6].
- inline `setname(char far *s) { strcpy(name, s); }` → Parameterkopie (mov eax / mov [bp-x],eax) – erklärt 4-Byte-Locals vor strcpy/memcpy.
- register-Variable als Index an mehrere inline-Setter: erste Kopie DI, weitere Stack (von selbst).
- long-Parameter als memmove-Länge: `len - 0x7c` nur unteres Wort.
- `size - pic - 0x20 - 23 * sizeof(T)` → add ax,0ffe0h / sub ax,142h.
- Kopierte Funktionen können Blöcke an anderer Stelle haben – nur am jmp-Ziel erkennbar.
- `if (..) { Locals; …; return; } { Local }` ohne Funktions-Locals: Epilogkopie, zweite Block-Locals ab [bp-2].
- near-Adresse: `g_x = g_347d` (extern char[]) → mov [seg],ds / mov [off],imm mit Fixup; `(char near *)0x347d` faltet falsch.
- O176_0EDD ist freie Funktion (hero far *, slot).
- tools/vv.py nutzt jetzt auch WOV.

## Nachträge (Welle AU)
- Methodenzeiger direkt aus Tabelle `(this->*g_4d38[k])()` → call dword ptr [bx+4d38h]; über Variable → SCOPY@ (cx=8) + call es:[bx+0ah].
- `#pragma option -r-` vor einer Funktion hält einen -Og-Zwischenwert auf dem Stack; danach `#pragma option -r`.
- Suchschleife mit leerem Rumpf `for (i = 0; i < N && (…); i++) ;` → jmp Test / inc / Test mit Rücksprüngen; mit break im Rumpf Test am Ende.
- `if (x) break;` am Ende eines do-while → cmp / jz short / jmp short (Kennzeichen geschachtelter do-while).
- far-Zeiger an unsigned-long-Parameter → push dword ptr [mem]; Rückgabewerte als push dx / push ax.
- `t >> 0` wird nicht gefaltet → sar ax,0.
- R41-Objekt als Member 0x19 Bytes (nicht 0x1a).
- Member vs. Basisklasse: Member-Dtoren mit Flag 2 nach dem Rumpf (Basis: 0).
- `g_8bf5 = R18_0ac2;` (resident, nur seg-Fixup im ASM) → cobjfix biegt den Namen richtig um.

## Nachträge (Welle AT)
- sub vs. add −k hängt am Vorzeichen: unsigned → sub dx,14h / sub eax,10h; signed → add dx,-14h. `(10 - i - 1) * sizeof(T)` behält dec ax; mit `* 3` wird −1 zu add dx,-3 gefaltet.
- unsigned Parameter bei `rows[b + i]` → ax = b + i, dann add bx,ax; signed → add bx,b und add bx,i einzeln.
- Member-far-Zeiger = 0: einfaches Mitglied → mov dword; Feldelement (`f356[i] = 0`) → zwei Wort-Stores, Segment zuerst.
- Ketten über Feldelemente speichern von rechts nach links (auch far-Zeiger: `*pb = *pc = -1` speichert pc zuerst).
- switch-Fall ohne break erkennt man an Tabellenzielen mitten im Rumpf; Generator tools/gen/o257/g257.py teilt an jedem Tabellenziel.
- Rückgabewert einmal benutzt → [bp-2]; dreifach → SI.
- Generator tools/gen/o206/g206.py: ASM als Stapelmaschine (Pushes → Ausdrücke, Aufruf + add sp → Argumentliste) – allgemein für lange sprintf-/Aufruffolgen.
- **cobjfix (behoben):** `push seg F_R42_0008 / push 03b3h` (Destruktorzeiger für _vector_new_) gab „widersprüchlich F_R42_03B3 / F_R42_0008“. Jetzt entscheiden bei Widerspruch die Offset-/Zeiger-Fixups, reine Segment-Fixups dürfen über ein anderes Symbol laufen. (O187 nutzt noch die Umgehung mit Klasse BmpA – kann später auf Bmp zurück.)

## Nachträge (Welle AV)
- `if (x) goto L;` → jcc short + jmp short L (Sprung über Sprung, obwohl Ziel in Reichweite); &&-Kette direkt → jng/jnz. Totes `if (0) ;` erzeugt ebenfalls Sprungpaare + toten jmp.
- `h->f42 -= h2->get7(src)` (int-Getter auf char-Feld, src Parameter) → mov al / cbw / mov dl,[f42] / sub dl,al / mov [f42],dl.
- inline `ok6(i) { return f6 == -99 ? 0 : 1; }` → int-Form; direkt im if → Byte-Form.
- Parameter in CL unterdrücken: `#pragma option -rd` vor der Funktion oder volatile am Parameter.
- `case 1: …; default: break;` (case 1 ohne break, andere davor mit) → default springt auf den break-jmp von case 1.
- Objekt im default: `default: { … } break;` → jmp short hinter dem Destruktor.
- Ohne Aufrufe: Mischung `a; register b; c; register d;` legt Register exakt fest; min über if/else statt ?: speichert direkt in CX.
- Gruppe (0x49 Bytes, ds:c30f[6]) beginnt mit `struct hero far *h[7]`; inline set26/set2a: `f28 = f26; return f26 = v;`.
- O047_0000 Ktor (p, q, i, w, f); beide O47-Ktoren 7 Wortargumente, nur über Adresse unterscheidbar.
- Bitfelder in Gegenstand, Inventarplatz (+4), Held (+bc) sind `unsigned short b:1`.

## Nachträge (Welle AY)
- O046: O014-Muster weiter ungelöst (3× asm). Zusätzlich erfolglos: inline-Funktion mit konstantem Zeigerargument, inline-Elementfunktion mit this = g_4c25, Getter für &fc, const-Zeiger, register-Zeiger mit -Op (toter Store bleibt).
- -Oi memset: Füllwert wie geschrieben (`-1` → mov ax,0ffffh / mov ah,al; `0xff` → mov ax,0ffh / mov ah,al).
- `char += char` → mov al,[src] / add [dst],al; `c = c + d` und `char += int-Local` → mov al,[dst] / add al,.. / mov.
- `if (--o->cnt <= 0)` → dec word es:[..] / jg (Flags des dec); `< 1` → cmp.
- Stack: Parameterkopien verschachtelter inline-Setter sind Funktions-Temps (auch im inneren Zweig), dann this-Kopie eines Getters auf Aufrufergebnis, dann -Og-Byte, dann Geschwisterblöcke.
- `if (A && B && ..) ; else f();` ≠ `if (!A || !B ..) f();` (letzteres im Original).
- register nur auf einer Variable: `register short x; short y;` → x SI, y DI; beide/keine → vertauscht.
- `g[y+1][x+2]` hinter far-Zeiger faltet zu y*25 / add bx,si / [bx+30h]; in &&-Ketten wird nur der erste Operand als -Og-Zwischenwert wiederverwendet.
- `default: return;` am Funktionsende auch bei Fällen mit break → Sprung auf echten Epilog + toter Epilog.
- `a = (t = tab[i-1][j]) == -1 ? 2 : g_cabb[t];` → mov [bp-2],ax / cmp ax,0ffffh.
- Daten: ds:9870/9878 Bytes +2/+3 überlappen long +0 (union); ds:c4db = XMS-Move-Struktur; Kampfstruktur +0x3e0 Objektliste 255 × 11, +0xfad Zeiger aufs Kampffeld (+0x14 Art, +0x15 Gitter [24][25]).

## Nachträge (Welle AZ/BA)
- Kopien, die wie ein äußeres inline aussehen, können eigene Locals sein (-Og würde sonst dpos*27 zusammenfassen): `dc = k; dt = st;`.
- `setitem(k, struct inv s) { inv[k] = s; }`: Temporäre aus item() → keine Kopie; benannte Variable → zusätzliche rep-movsw-Kopie. Tauschvariable auf Funktionsebene.
- `w = h->inv[3].id; switch (w)` → mov [bp-x],ax / mov ax,[bp-x].
- `cmp di,k` direkt = if-Kette; switch macht immer mov ax,di / cmp ax,k.
- `!!x` auch im einfachen if (signed-char-Feld) → mov al / cbw / or ax,ax.
- `else if (..) { ..; return; } else {..}` → toter Epilog; geschachtelt `else { if (..) { ..; return; } .. }` → keiner.
- Methode einer fremden Klasse (O166_0000 = Town aus O183) einfach als Methodendeklaration in Town.
- Far-Zeiger-Store über doppelte Dereferenzierung (`o->x0d->fcd = 0`) → zwei Wort-Stores, Segment zuerst; über lokale Zeigervariable → mov dword.
- inline-Parameterkopien entstehen auch ohne -Og.
- `loc.e[i - 1].snd` (lokale Struktur, -Og) → −1 in lea-Basis, Feldversatz als add ax,4.
- `g_c1c5 * 1000 - g->get33()` (unsigned-long-Getter) → mov edx,es:[bx+33h] / sub ax,dx.
- neg/sbb/inc-Muster auch mit freien inline-Funktionen.
- `k = w; for (k += 1; ..)` → mov al,[w] / mov [k],al / mov al,[k] / inc al / mov [k],al.
- Daten: ds:6c46 Formatstring + vier Leerstrings (echte Modul-Literale von O155); ds:b6c1 = 16 far-Zeiger (an O190_01C6 einzeln); ds:1eaa Zauberbeschreibungen (12 Bytes: +5 Einsatz, +7 Zielart, +8 Reichweite).

## Nachträge (Welle BB)
- **Inline-asm, das SI nennt, sperrt SI für register-Variablen der ganzen Funktion.** Lösung: `mov si, offset g_x + k` als `asm db 0beh` / `asm dw offset g_5878 + 6` schreiben (Fixup bleibt). Betrifft evtl. andere asm-strcat-Module.
- `m = 0; m -= (signed short)(l / 5400);` → xor si,si vor idiv, danach sub si,ax; `m = 0 - l / 5400` rechnet in DX.
- Große Objekte aus Geschwisterblöcken enden bei derselben Adresse unter dem tiefsten kleinen Local; this-Kopie in Block mit eigenem Objekt liegt im Blockbereich, sonst vorn auf Funktionsebene.
- Zeiger als eigene Variable vor dem Konstruktor (`h2 = &g_b760[v2]; new O47(h, h2, …)`) → eigenes Local; direkt als Argument nur gepusht.
- Daten: O071 gather_herbs_table ds:5826 (12 × {id, Chance, max, Schwierigkeit}), Modifikatoren ds:585a–5872, Format ds:5878 "%d^%s", dahinter ",@"; Namensfelder ds:2888[3][16] / ds:28b8[3][16]; ds:2472/247a long.
- Camp::run hat signed-char-Parameter (offen/OVL030.CPP deklariert signed short – anpassen).

## Nachträge (Welle AX)
- Bestätigt: asm, das SI/DI nennt, schaltet register-Variablen der ganzen Funktion ab → asm-strcat komplett als `asm db …` + `asm dw offset g_6202` schreiben (bytegleich, cobjfix kommt mit dem dw-offset-Fixup zurecht).
- `dst->x = *(chg + (unsigned)k)` (Byte) → Quelle vor der Zieladresse (push ax / imul / pop ax); signed Index → Ziel zuerst, Quelle über push es / les si. Wortzuweisungen: Ziel zuerst.
- `f() - 2`: signed Rückgabe → add ax,0fffeh; unsigned short → sub ax,2.
- signed long += unsigned long → mov edx / add / mov; mit (signed long)-Casts → add mem,eax.
- Namen: O055 = DSA1 load_ani (seg027); Teilstruktur sub (0x92): Name[5] +0, fl[10] +0b, n +15, pics[25] +16, nfr[10] +7a, fr +8e. O269_043B = Pic::dims(id, &w, &h); O110_0017/1140 = Edit::run/settext; O111_0000/0205 = Edit-Ktor/Dtor.

## O014-Muster GELÖST (Welle AW)
`mov dx,BASIS / add dx,FELD / add ax,dx / mov bx,ax` (Basis und Feldversatz getrennt) entsteht mit
```c
p = *(struct rec far * near *)(typ * 18 + (unsigned)&((struct ent near *)(unsigned)g_4c25)->fe);
R04_0760(g_382d, *(signed short near *)(typ * 18 + (unsigned)&((struct ent near *)(unsigned)g_0bbd)->fc));
```
- Adresse nach unsigned und zurück in near-Zeiger → nicht faltbare Adresskonstante; `typ * 18 + …` mit Literal 18 in dieser Reihenfolge (umgekehrt oder sizeof → faltet bzw. push/pop); ohne inneren (unsigned)-Cast → wieder [bx+…].
- O014, O046, O097 damit asm-frei. Zwischenstufe -Op mit t = g_4c25 lässt toten Store zurück.

## Nachträge (Welle AW)
- inline mit lokaler Zeigerkopie: `readw(short far *p) { unsigned char far *q = (unsigned char far *)p; return readb(q++) + (readb(q) << 8); }` erklärt Temp-/mov eax-/mov edx-Kopien. Parameter einer inline-Funktion als Argument → ohne Kopie; echtes Local → Kopie.
- `MK_FP(FP_SEG(buf), FP_OFF(buf) + cnt)` pusht Segment vor der Addition; `buf + cnt` rechnet zuerst.
- `if (A && (B || C)) {} else if (D)` vs. verschachtelt `if (A) { if (B || C) {} } else if (D)`: nur 1 Byte Sprungziel anders.
- Wenn BCC auf Funktionsebene SI braucht (les si für `f->x = *px`), kommt ein Zähler nur als register-Variable im inneren Block in SI.
- Byte +17 im long +14 (Farbwert) → anonyme union; `(signed short)p->col >> 8` → mov ax / sar ax,8.
- `(signed long)(void far *)&local` → mov ax,ss / lea dx / shl edx / shrd edx; (unsigned long) → zwei Wort-Stores.
- `g_x++; switch (g_x)` → inc byte / mov al / cbw; `switch (++g_x)` → mov al / inc al / mov.
- -Og-Wortwert (`e - 10` zweimal) sieht aus wie Variable; echte Variable rechnet die Bedingung vor dem push.
- Original-asm erkennbar an TASM-Kodierung: `mov al,ah` als 88 E0 (BCC-C ergäbe 8A C4).
- Werkzeuge: tools/pseudo.py (Kurzlisting mit Aufrufargumenten), tools/collapse.py (wiederholte inline-Expansionen zusammenfassen).

## Nachträge (Welle BD)
- Kontrollfluss entscheidet die Registerwahl: if/else-if-Kette am Funktionsende vs. `if (..) { ..; return; } if (..) { ..; return; }` → gleiche Bytes, aber SI/DI vertauscht.
- -rd: register-Parameter bleiben auf dem Stack (cmp word [bp+0ah],..).
- Zwei gox-Varianten im selben Modul (einfach wie O064, doppelt `setx(setx(v))` wie O014) → beide in der Gruppen-Klasse.
- Truhe als reines Block-Objekt `{ Chest c(0x2e, 1); }`; Geschwisterblöcke teilen [bp-68h].
- `set22(set22(0))` mit char-Rückgabe → -Og-Bytes auf ungeraden Adressen.
- `struct ptr7 lst = g_6628;` auf Funktionsebene sperrt alle register-Variablen (push si/di nur wegen rep movsw).

## Nachträge (Welle BC)
- `memmove(&b[i], &b[i + 1], n)`: nur so wird i*0x24 -Og-Zwischenwert; `&b[i] + 1` / `b + i + 1` speichern den Offset von &b[i].
- `*out++ = s` (far-Zeiger-Zeiger-Parameter) → mov ax/mov dx + zwei Wort-Stores; getrennt → mov eax.
- Zähler in CX bei register-Parametern in SI/DI: nur als register in eigenem innerem Block.
- `__rotl__` aus stdlib.h → rol ax auch ohne -Oi.
- `#pragma option -Od` vor einer Funktion hat in O034 f_2559 einen -Og-Byte-Zwischenwert entfernt (widerspricht Welle W – Einzelbeobachtung).
- `return g_c4fb;` (globaler far-Zeiger) am Funktionsende → kein toter Epilog.
- switch-Baum auf unsigned Wert → ja statt jg.
- Ohne Aufrufe: Ergebnis CX, Zähler BX (von selbst).

## Nachträge (Welle BE)
- Distributiv-Regel: `scr + (oy + 48) * 320 + ox + 16` → imul / add ax,[scr] / push bx / push es (BCC zieht die Konstante heraus, Kennzeichen krumme Konstante wie 0x3c10 = 48·320+16); `scr + oy*320 + ox + 0x3c10` lädt den Zeiger zuerst; (unsigned)-Cast → mov dx,[scr] / add dx,ax. Index als Summe → AX-Form, reines Produkt → Zeiger zuerst.
- `vis[(unsigned)(zoom*9 + vy - 1)]` → ganze Zeile -Og-Zwischenwert, Index zuerst; ohne Cast −1 in die Verschiebung.
- `new Bmp(n, n = 16 / zoom)` → mov di,ax / push ax / push di.
- `tidx[i] = k++` lädt k vor der Zieladresse, inc nach dem Byte-Store.
- `memset(tidx, 1, ntiles = n)` → mov cx,ax. `if ((vw = zoom * 18) > w)` vergleicht AX ohne Nachladen.
- Klasse Map (0x28d, ds:be36) für O052/O053; O053_0000 ist Methode (OVL053.CPP deklariert sie frei – für cobjfix egal).

## O030 – Stand nach drittem Anlauf (19.10.)
- Weiter 8 Bytes offen. ~200 weitere Varianten erfolglos (Liste im Bericht; u. a. alle ?:-/Komma-/Arithmetik-/long-/Zeiger-/enum-Umwege, Pseudoregister, Parametertypen, alle Optionen einzeln/kombiniert, IDE BC.EXE statt BCC → identische OBJ, IDE-Vermutung widerlegt).
- Mechanismus: Sprungtabellen-switch lädt jeden einfachen Operanden direkt nach BX; nur Knoten mit festem AX-Ergebnis (cbw, mov ah,0, Zusammenführung eines ?:, Gleitkomma, cwd) geben `dec ax / mov bx,ax`; `mov ax,si` nur beim Vergleichs-switch.
- Teilerfolg: `#pragma option -O` + `switch (0 && n ? 0 : n)` ergibt genau `mov ax,si / dec ax / mov bx,ax` – aber -O entfernt Sprünge auf den Folgebefehl und den toten Epilog, die das Original hat (offen/OVL030_mit_O.CPP).
- Gleichartiges O046_1743 ergibt im Original `mov bx,si / dec bx`. Beste Hypothese: OVL030.OBJ stammt von einer älteren Compilerversion (BC++ 3.0) und wurde nie neu übersetzt. O030 bleibt im Build ASM; C-Stand: offen/OVL030.CPP (Camp::run jetzt signed char).
- PRJ-Optionseinträge (PRJ2MAK): 0xfd = CPU (3 = -3), 0xfa1 = -Og, 0x110 = -G, 0x111 = -O, 0x10e = -v, 0x120 = 1 schaltet -vi- ab, 0xfa2..0xfb2 = -Oc/-Oe/-Om/-Ov/-Ol/-Ob/-Oa/-Op/-Oi, 0x115 = -r-/-rd, 0x113 = -p/-pr.

## SEG005 (19.10.)
- C++ mit leerer Klasse Init (new(1)); Ktor F_R05_01A5, Dtor F_R05_0588 (Muster this == 0 → new, flags & 1 → delete). -3 -Ot.
- F_R05_0012 = CD-/Kopierschutzprüfung: bleibt laut Vereinbarung Byteblock. Technik „Byteblock mit C-Hülle“: `char b[0x32]; (void)b;` (sub sp + leave), `_SI = 0; _DI = 0;` (push si/di + xor), Rumpf als `asm db`, Fernaufrufe als `asm call far ptr name` (Fixups), DS-Adressen als Zahlen, `#pragma warn -rvl`. Generator tools/genblk.py.
- `if (a != b) ; else x = 1;` → jz / jmp short. Expliziter Aufruf im Dtor braucht `::operator delete(p)` + Deklaration.
- ds:b670 = g_text_output_buf (MAP).

## Erkenntnisse aus Schicksalsklinge (Q005, ABSCHLUSSBERICHT.md, Nachtrag 18.10.)
- **Cross-Jumping:** Borland legt gleich endende Codestücke zusammen; steht das Stück direkt vor dem Ziel, springen frühere vorwärts dorthin, sonst spätere rückwärts zum ersten. **Eine Sprungmarke unterbricht das – auch unbenutzt, ohne ein Byte.** (DSA1 seg106 pass_item: zwei unbenutzte Marken.)
- **goto in C vs. C++:** C++ erzeugt bei goto einen toten Zusatzsprung, C nicht. In DSA1 war Attics Code C; in DSA2 sind viele Module nachweislich C++ (Klassen, new, _vector_new_), andere C – bei toten Zusatzsprüngen an C-Fassung denken. Ersatz in C++: `case n: …; if (0) { case m: … }`.
- **Aufrufe ohne Prototyp:** Attic rief Funktionen teils ohne (passenden) Prototyp auf → Argument geht als int statt long/char. In C++ kein zweiter Prototyp möglich (anderer Linkname) → Aufruf über gecasteten Funktionszeiger.
- **C/C++-Unterschied bei Bitfeld-Adressierung** (DSA1 seg098): im C-Modus ohne Cast der Originalcode.
- **Aufrufziele vergleichen, nicht nur Bytes** (bei uns: cobjfix + Komplettlink). PUBDEF-Reihenfolge bestimmt Overlay-Einsprünge (bei uns in cobjfix gelöst).
- Attic hat in der IDE gelinkt (bei uns bereits so).

## Daten-Stufe D1 (19.10.): echte Modul-Literale
- tools/cobjfix_d.py (umhüllt cobjfix.py): nimmt eigenes _DATA/_BSS einer C-OBJ an, bestimmt die DS-Lage aus den Codeverweisen (ASM-Fassung hat dort feste Zahlen), prüft die Bytes gegen das Original-DS, biegt die Fixups auf DGROUP-Publics um und leert das eigene _DATA. Bricht mit „EIGENE DATEN NICHT AUFLOESBAR“ ab, wenn Bytes/Lage nicht passen. build.sh benutzt es; tools/ccmp.py meldet jetzt „GLEICH (+n Bytes eigene Daten geprueft)“ (alte Fassung: ccmp_alt.py).
- **Alle asm-strcat-Stellen sind weg** (O068, O071, O078, O106, O107, O111, O162, O207, O216, O228, O238, O260): einfach `strcat(dst, "…")` schreiben. O078 braucht die Zuweisung im Argument (`strcat(buf + (off = i * 40), " ")`). O207-Literal ist ".TXT", nicht ".CHR".
- Neue Module dürfen also String-Literale direkt benutzen, solange sie dem Original-DS entsprechen.
- Hinweise dsana: ds:5a0b..5a0d gehört O078, ds:6c64..6c66 gehört O162.
- tools/fixvgl_adr.py: wie fixvgl, Datenverweise als DS-Adressen verglichen.

## Welle KS (19.10.): Kopierschutz-Segmente SEG001/SEG003/SEG023 und R05 cd_check als C
Die Regel „Kopierschutz bleibt erhalten“ ist aufgehoben; alle vier Stellen sind jetzt C/C++ (Prüfungen unverändert wirksam).
- Einordnung: SEG001 = Spielschleife (do_travel_mode, main_loop, game_idle, savemenu_save), kein CD-Audio wie DSA1 seg001.
  SEG003 = Handbuchabfrage als Klasse CopyProt (PROTECT.LTX/PASSWORD.DAT; query in der CD-Fassung unbenutzt) + cleanup_game
  + cpu_v86_active. SEG023 = Zeitgeber/Maus (timer_isr INT 1Ch, view_timer_cb, fight_timer_cb mit Integritätsprüfung,
  mouse_isr, long_sqrt). R05_0012 = cd_check (PATH.CFG, MSCDEX, Datenträgername DSA2_CD1/2).
- Integritätsprüfung (fight_timer_cb, alle 250 Takte): Summe der Codebytes zwischen zwei leeren Marken-Funktionen
  (copyprot_code_begin/_end) gegen g_copyprot_sum, dazu Suche nach dem Muster `EB 04 87 23 31 56` + `9A` + Ziel zwischen
  den leeren Marken um game_idle bzw. savemenu_save. Im C: Marken als leere Funktionen in der Originalreihenfolge,
  Bereichsgrenzen `(unsigned char far *)marke` (Segment- und Offset-Fixup; cobjfix biegt den Offset auf das Public an der
  richtigen Stelle), das Muster als `asm db 0ebh, 04h, 87h, 23h, 31h, 56h` direkt vor dem C-Aufruf.
- Far-Zeiger-Vergleiche `p < p_end`/`q > q_end` vergleichen nur die Offsets (jc/jna) – so im Original.
- Rückrufe/ISRs mit `push eax … push esi` direkt nach dem Prolog: das ist Inline-asm als erste Anweisung (pop am Ende),
  kein Compilerverhalten. In .C braucht das `#pragma inline` (BASM kennt keine 32-Bit-Register); die Bytes bleiben gleich.
  SEG023 lief im Original über TASM (long_sqrt `sub ax,ax` = 2B C0; BASM gäbe 29 C0).
- **Relocation-Reihenfolge (neu geprüft nur im Komplettlink, ccmp/cvsasm sehen sie nicht):** TLINK übernimmt die Folge
  der FIXUPP-Einträge in die MZ-Tabelle. BCC-eigene OBJs (C ohne asm) schreiben die Fixups je Satz absteigend, TASM
  aufsteigend, setzt aber die GRP-Fixup eines Interrupt-Prologs (`mov bp,DGROUP`) an den Satzanfang; im Original steht
  sie hinter den ersten Fixups (vermutlich andere TASM-Version). cobjfix.py setzt seit Welle KS jede GRP-Fixup hinter
  ihren Vorgänger aus der ASM-Fassung (ohne Wirkung, wo die Lage schon stimmt). Eine BCC-eigene Fassung (32-Bit-Befehle
  als `asm db`) ist dagegen falsch (absteigende Folge).
- Werkzeugfalle: In der ASM-Fassung waren die markierten Aufrufe als Datenbytes + `dw seg F_R03_0006` zerlegt; die
  C-Fassung hat dort eine Far-Pointer-Fixup (typ 3) an anderer Stelle, cobjfix hielt sie für einen Datenverweis
  (Ziel DATASEG@ → Segment 0x24EB im Aufruf). l01/SEG001.ASM schreibt die beiden Aufrufe jetzt als `call far ptr`.
- `void interrupt f(void)` mit -3: Prolog push ax..bp / mov bp,DGROUP / mov ds,bp / mov bp,sp / sub sp,n kommt vom Compiler.
- Doppelter Test `mov al,..; cbw; mov [bp-x],ax; or ax,ax; jz; cmp word [bp-x],0` = `(lt = g->loctype) != 0 && lt != 0`
  mit lt als Local im inneren Block (Funktionsebene läge vor den 3-Byte-Aggregaten).
- asm-Funktion mit long-Ergebnis in DX:AX und totem Epilog: `return;` am Ende (mit `#pragma warn -rvl`), `return _AX`
  erzeugt movzx/shld.
- `fd = _open(..); if (fd > 0)` → `mov di,ax / or di,di`; eingebettet `(fd = _open(..)) > 0` → `or ax,ax`.
- `delete (CD far *)g_cd;` → CD_dtor(g_cd, 3) ohne Nulltest (Klasse mit nicht-inline Destruktor).
- cd_check: die Literale ergeben genau den Literalpool ds:445b..4537 (Reihenfolge des ersten Auftretens, auch die zwei
  leeren ""), Sprung auf PSP:0000 als Inline-asm (`mov bx,_psp / xor ax,ax / push bx / push ax / retf`, TASM 31 C0).
- Optionen: SEG001 -3 -Ot (mit -Og ebenfalls gleich), SEG003 -3 -Ot -Og (compare: this + i als -Og-Zwischenwert in SI),
  SEG023 -3 -Ot (#pragma inline), SEG005 unverändert -3 -Ot (Spalte D; die Literale von cd_check bilden den
  Literalpool ds:445b..4537 hinter g_ltx_count_init, das bisherige Feld g_445b entfällt).
- Bauen in einem Arbeitsbaum ohne die festen Pfade zu ändern: `unshare -m bash -c 'mount --make-rprivate / && mount
  --bind /root/<baum> /root/sternenschweif && cd /root/sternenschweif && bash tools/build.sh'`. ACHTUNG: Prozesse nie per
  `pkill -f tools/build.sh` beenden (trifft auch Builds in anderen Bäumen), nur per PID.

## Phase B (19.10.): Daten zurück in die Module – Muster für die schweren Fälle
Alle C-Module mit Daten definieren sie jetzt selbst (c_module.tsv Spalte D, Prüfung tools/dscheck.py), Ausnahme der
BSS-Rest DRO131 (s. u.). Muster:
- **Auto-Aggregat mit Initialisierer** (auch mit nicht konstanten Werten, z. B. Textzeigern): BCC legt eine unbenannte
  Vorlage im eigenen `_DATA` an (Definitionsreihenfolge, zwischen den globalen Daten) und kopiert sie per `rep movsw`
  auf den Stack. Bytes, die „mitten in den Daten“ ohne Namen stehen und nur per `mov si,offset` gelesen werden, sind
  fast immer solche Vorlagen (O041 Zeilentabellen, O053 dirs/q, O084 gear/by_type, O103 names).
- **1-basierte Tabellen:** Liegt der MAP-Name ein Element vor dem ersten echten Eintrag (Zugriff `[bx + name]` mit
  Index ab 1), ist der Originalcode `tab[i - 1]` – BCC faltet `- 1 * size` in die Adresse. Echtes Feld unter neuem Namen
  anlegen, der alte MAP-Name bleibt in s_data.h als Alias („= tab - 1“), das Public setzt cobjfix_d (eigen_rewrite) aus
  der MAP. So: g_dng_view_areas, g_town_view_areas, g_lvl_attrib_order, g_type_money_ranges, g_walk_dng,
  g_blaster_*_drivers/_names, g_town_area_descs, g_forbidden_item_lists. Gleiches Muster mit größerem Versatz:
  g_fight_defs_ab[f->id - 0xab] (O140; gefaltete Basis ds:3851 liegt mitten in den Daten von O032 – kein Lesen über
  ein Tabellenende). Faltet BCC nicht (`mov dx,ofs / add dx,7`), hilft ein Cast auf `signed short` beim Index (O086).
- **Methodenzeiger-/Funktionszeigertabellen:** `&Klasse::methode` (8 Bytes je PMF) bzw. Funktionsnamen direkt in die
  Tabelle. Ziele im eigenen Overlay werden segmentrelative Fixups, TLINK setzt den Stub-Einsprung wie im Original;
  fremde Ziele werden EXTDEF, cobjfix_d benennt sie auf F_Oxxx_yyyy um (O048, O078, O108, O258, O032 Zaubertabellen).
- **Globales Objekt mit Konstruktor / Feld mit dynamischem Initialisierer:** BCC erzeugt eine lokale Funktion
  (LPUBDEF @_STCON_) und einen `_INIT_`-Eintrag (fern, Priorität 32). Das Modul liefert ihn selbst: Spalte `DI`
  (tools/dsinfo.py INIT_FN/EXIT_FN, dscheck prüft Art/Priorität/Ziel). O224 `struct saver g_saver;` mit saver::saver(),
  O008 g_inv_areas_knapsack/_all mit `{x, y, x + 17, y + 17}` aus g_inv_slot_xy. cobjfix_d lässt dabei das gleichnamige
  ASM-Public des Init-Ziels weg (sonst zweiter Stub-Einsprung).
- **Objekte von Klassen mit Konstruktor in O032** (Bilder, Gruppen …, konstruiert von globals_ctor): nur Speicher
  `char g_xxxx[n]` unter dem Adressnamen, Zugriff über den typisierten Namen aus s_data.h.
- **Doppelte Publics** (zwei Module definieren denselben Namen) lassen BC.EXE /b stumm abbrechen; tools/pubdup.py
  prüft vor dem Link (build.sh, dsb.sh); zur Diagnose TLINK direkt in lnk/ aufrufen
  (`..\bc\BIN\TLINK.EXE /c /s /m @FULL2.RES`, die Meldungen stehen dann in SCHW_CD.MAP).
- dsconv: `DSCONV_HAND="adr-adr:datei;…"` für Handblöcke, `extern`-Zeilen vor den #include ordnen das BSS im C-Modus.
- Kopierschutz/Integrität (Welle KS) mit eigenen Daten unverändert: g_copyprot_sum (ds:35c2, DATA) und g_tamper_count
  (ds:c6c6) sowie das Objekt g_copyprot mit den Merkern ok/ok2 (ds:c6ca) liegen im Bereich von O032 (Module liefern ihre
  Daten zusammenhängend), SEG003 besitzt nur g_copyprot_passed (ds:3e6e), SEG023 nur seinen eigenen Block ab ds:cb6e.
- **O030 (switch-Registerwahl aus nicht initialisiertem Heap):** Der Vorlauf OVL198 hängt von den Kopfdateien ab, die
  OVL198 einliest – jede Änderung an names/inc kann O030 kippen (cobjfix: „_g_c4c5: mehrere Adressen“). Jetzt
  unabhängig davon: tools/frisch.txt `OVL030 @0115:2e8=8b,2f8=17` lädt per tools/fill/FILL.COM ein 1-KB-Speicherbild
  (tools/heapbild.py, entspricht x030/t/Z0.BIN) in XMS-KB 0x115 vor dem Übersetzen.

## Nachträge (Abschlusswelle C: handgeschriebenes TASM in SEG024/SEG025)
- **Vorwärtssprünge:** TASM 3.1 (ein Durchlauf, `.386p`) reserviert für einen bedingten Vorwärtssprung 4 Bytes, für `jmp` 3, und füllt kurze mit NOPs auf: `73 08 90 90` bzw. `EB 04 90` ist ein gewöhnlicher Sprung im Quelltext. Marken umbenennen ändert nichts, solange Richtung und Ziel bleiben.
- **`jecxz` mit fünf NOPs** (`67 E3 xx 90 90 90 90 90`) = Quelle mit `JUMPS` (Platz für jecxz/jmp short/jmp near).
- **far-Aufruf auf eine Prozedur im selben Segment:** TASM erzeugt selbst `push cs / call near` (4 Bytes). `90 0E E8 ..` (5 Bytes) macht TLINK aus einem echten `call far` mit Fixup → als `db 9Ah / dd ziel` nachbauen.
- **`add si,offset x`** (Fixup) ergibt die Langform `81 C6 iw`; mit einer Konstanten wählt TASM `83 C6 ib` → Langform als `db 81h,0C6h / dw x`.
- **`align` und `dup` erzeugen LIDATA** (tools/objorder.py bricht ab; `align` im byte-Segment warnt zusätzlich): Füllbefehle per Makro (`if (($ - base) and 3) eq 2 / xchg bx,bx ...`), Nullbereiche per `rept n / dw 0 / endm`. Auffüllung `87 DB` (+ `90`) vor abgerollten Schleifen ist vermutlich `align 4` im Original.
- **`ret` in einer `proc far` wird RETF.** Sprünge per `push adresse / ret` brauchen `retn`.
- **Makroargumente trennt TASM auch an Leerzeichen:** Ausdrücke in spitze Klammern (`ZEROS <CLIP_MAX * FACE_SIZE>`).
- **Variablen im Codesegment** als Konstanten `name = $ - base` (absolut, kein Fixup, Kodierung wie `cs:[0ah]`). Tabellen hinter dem Code werden vor ihrer Definition benutzt: Lage als `equ` und an der Tabelle `if ($ - base) ne X / .err / endif` prüfen. Segment mit Versatz (SEG025 beginnt bei CS:000A): `$ - base + 0ah`.
- **Abgerollte Schleifen:** `n = 0 / rept N / step pfx,%n / n = n + 1 / endm`; im Schrittmakro `local` für Marken und `pfx&k = $ - base` für die Sprungtabelle (`dw pfx&k` per eigenem Makro, `%n` geht nur als Makroargument). Displacements wählt TASM selbst (`[di+0]` → ohne, bis 127 Byte, sonst Wort).
- **Text-equates** `name equ <[bp+6]>` für Parameter und Lokale ändern keinen Code; `@@`-Marken innerhalb `proc`/`endp` mit `locals`, auch aus Makros heraus.

## Abschlusswelle A (19.10.): Namen, Byte-Wüsten, Konstanten
- **Gefaltete Tabellenbasis über Modulgrenzen** geht auch für fremde Tabellen: O257 liest die Themen-Dateitabelle von
  O254 als `g_conv_topic_tab[t - TOPIC_SCRIPT_FIRST]` (BCC faltet `- 7 * 2` in die Adresse ds:7710 = MAP-Name g_7710;
  cobjfix ordnet zu). Das Wort, das zufällig an der gefalteten Basis liegt (ds:7710, O250), ist eigenes Datum des
  Nachbarmoduls - nicht mit der Tabelle verwechseln.
- **„unbenutzt“ aus dsconv ist nicht verlässlich**, wo Module später nach C kamen (SEG001/SEG023, Welle KS) oder wo
  Objekte/Tabellen länger sind als ihr C-Typ (g_god_favor[9..10], Teile von g_font, g_screen_pic, g_tmap, Knopftabelle,
  Artikel-/Pronomentexte von O056, Tempellisten von O044, Segmentwort g_menu_texts[15]). Prüfen über alle Verweise der
  linkfertigen OBJ (cbau/*.FIX und lnk/asm1/*.OBJ, `dsrefs.refs`), auch Verweise mit Versatz in den Bereich.
- Echt unbenutzte Bytes heißen `g_unused_<adr>`; Nullfelder als `{ 0 }`, Texte als Literal mit fester Größe
  (`char a[16] = "..\\DIARY.TXT\0\r\n";` - die Endnull fällt in die Größe). Eine typisierte Definition statt eines
  Bytefelds ist bytegleich, wenn Größe und Werte stimmen (`unsigned long g_save_count = 1;` statt `{ 1, 0, 0, 0 }`;
  auch im BSS von O032, C++: Definitionsreihenfolge).
- Ein in zwei Stücke zerlegter Text (O029 `"t." + "$$$"`) wird ein Feld; die MAP-Publics im Inneren erzeugt
  cobjfix_d. Benannte Texte, die nur im eigenen Modul in Reihenfolge des Auftretens benutzt werden, können Literale
  werden (O029: drei `"t.$$$"`, ohne -Oi bleibt strcpy ein Aufruf) - der Name entfällt (namen.tsv Quelle `LIT`).
- Ein Modul darf nur BSS liefern (O128, Spalte D, dsana/korrekturen.tsv): Kampf-Globale ds:c948..cab8 als
  C++-Definitionen in Adressreihenfolge.
- Konstanten in Tabellen (MONTH_*, HOLIDAY_*, TOPIC_*, FINALE_OP_*) ändern keine Bytes.
