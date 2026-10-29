#!/usr/bin/env python3
# Gegenstand 0x145 ist der Schluesselbund (ITEMS.LTX 325, USE.LTX 101-110), nicht ein Kraeuterbehaelter.
# Benennt Bezeichner und Kommentare um. Idempotent; laeuft auch ueber -p1-Patches.
import re, sys
IDS = [('item_use_herb_pouch', 'item_use_keyring'), ('item_in_herb_pouch', 'item_on_keyring'),
       ('use_herb_pouch', 'use_keyring'), ('in_herb_pouch', 'on_keyring'), ('ITEM_HERB_POUCH', 'ITEM_KEYRING'),
       ('g_herb_ids', 'g_keyring_keys'), ('herb_bits', 'key_bits'), ('group_find_herb', 'group_find_key'),
       ('hero_find_herb', 'hero_find_key'), ('hero_take_herb', 'hero_take_key'),
       ('chest_special_herb', 'chest_special_key'), ('special_herb', 'special_key')]
TXT = [('Kraut auswaehlen und herausnehmen, sonst ein Kraut', 'Schluessel auswaehlen und abnehmen, sonst einen Schluessel'),
       ('Kraut herausnehmen (group_give_item), sonst Kraut', 'Schluessel abnehmen (group_give_item), sonst Schluessel'),
       ('Kraut id', 'Schluessel id'), ('Kraut 0x111', 'Schluessel 0x111'), ('mit diesem Kraut', 'mit diesem Schluessel'),
       ('Kraeuterfaecher', 'Faecher des Schluesselbunds'), ('Kraeuterbehaelters', 'Schluesselbunds'),
       ('Kraeuterbehaelter', 'Schluesselbund'), ('Sammelbehaelters 0x145', 'Schluesselbunds 0x145'),
       ('Kräuterbeutel', 'Schlüsselbund'), ('Kräuterbehälter', 'Schlüsselbund')]
def conv(s):
    for a, b in IDS: s = re.sub(r'\b' + a + r'\b', b, s)
    for a, b in TXT: s = s.replace(a, b)
    return s
for p in sys.argv[1:]:
    raw = open(p, 'rb').read(); enc = 'utf-8'
    try: t = raw.decode('utf-8')
    except UnicodeDecodeError: t = raw.decode('latin-1'); enc = 'latin-1'
    n = conv(t)
    if n != t:
        open(p, 'wb').write(n.encode(enc)); print('geaendert', p)
