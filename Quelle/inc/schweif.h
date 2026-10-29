/* schweif.h - gemeinsamer Header fuer die C/C++-Rekonstruktion von DSA 2 Sternenschweif (Attic 1994)
 *
 * Einbinden mit  #include "schweif.h"  und  BCC ... -I..\names\inc  (BCC 3.1, -mlarge, C und C++).
 * Der Header erzeugt selbst keinen Code und keine Daten: nur Typen, Konstanten, extern-Deklarationen,
 * Prototypen und (C++) inline-Elementfunktionen, die erst bei Benutzung expandiert werden.
 * Standard-Header (stdio.h, string.h ...) bindet jedes Modul selbst ein - sie entscheiden ueber
 * Intrinsics (-Oi) und inline-Varianten (atoi, abs) und gehoeren daher zum Modul.
 *
 *   s_const.h   Konstanten und enums (Eigenschaften, Talente, Zauber, Krankheiten, Gifte ...)
 *   s_struct.h  Datenstrukturen (hero, group, item_desc, inv_slot, enemy ...) mit inline-Settern
 *   s_class.h   C++-Klassen (Bmp ...)
 *   s_data.h    globale Daten (erzeugt aus names/daten/manuell_data.tsv)
 *   s_proto.h   Prototypen (erzeugt aus names/namen.tsv + Deklarationen der Module)
 *   s_fix.h     Schalter fuer die Behebung der Originalfehler (FIX_BUGS, FIX_EXPLOITS, FIX_nnnn; nur Makros)
 *
 * Konventionen: names/KONVENTIONEN.md. Namen und Herkunft: names/namen.tsv. Abweichungen: konflikte.tsv.
 */
#ifndef SCHWEIF_H
#define SCHWEIF_H

#include "s_fix.h"
#include "s_const.h"
#include "s_struct.h"
#include "s_class.h"
#include "s_data.h"
#include "s_proto.h"

#endif
