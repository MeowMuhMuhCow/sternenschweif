// daten.js – Spieldaten-Verwaltung (IndexedDB, Ordner-/ZIP-Import, GOG-ISO9660-Leser, SHA-256-Prüfung)
// für DSA 2: Sternenschweif (Browser-Edition)

(function() {
    'use strict';

    var DB_NAME = 'sternenschweif-spieldaten';
    var DB_VERSION = 1;
    var STORE_NAME = 'dateien';
    var META_KEY = '__meta__';

    // Pflichtdateien laut Starter/spiele.json ("sternenschweif")
    var PFLICHT_DATEIEN = ['SCHWEIF.EXE', 'DATA/STAR.DAT'];

    // Bekannte SHA-256-Prüfsumme der unterstützten deutschen CD-Version C1.02 (SCHWEIF.EXE, 1.387.552 Bytes)
    var BEKANNTE_VERSIONEN = {
        '9164a40aa1e39bfc4d6114648a9011851913ec9d871d785791cd18b40d0d03a1': 'DSA 2: Sternenschweif – Deutsche CD-Version C1.02 (GOG / Original-CD)'
    };

    var IGNORIERTE_ENDUNGEN = [
        '.BAT', '.COM', '.DLL', '.DOC', '.TXT', '.PDF', '.HTML', '.HTM',
        '.LNK', '.ICO', '.BMP', '.PNG', '.JPG', '.CONF', '.INI', '.INST', '.INS', '.CUE'
    ];

    function openDB() {
        return new Promise(function(resolve, reject) {
            var req = indexedDB.open(DB_NAME, DB_VERSION);
            req.onupgradeneeded = function(ev) {
                var db = ev.target.result;
                if (!db.objectStoreNames.contains(STORE_NAME)) {
                    db.createObjectStore(STORE_NAME);
                }
            };
            req.onsuccess = function(ev) { resolve(ev.target.result); };
            req.onerror = function(ev) { reject(ev.target.error || new Error('IndexedDB konnte nicht geöffnet werden.')); };
        });
    }

    async function sha256Hex(uint8) {
        if (!window.crypto || !window.crypto.subtle) return '';
        var hashBuf = await window.crypto.subtle.digest('SHA-256', uint8);
        var arr = new Uint8Array(hashBuf);
        var hex = '';
        for (var i = 0; i < arr.length; i++) {
            hex += arr[i].toString(16).padStart(2, '0');
        }
        return hex;
    }

    function normalizeGameRelPath(rawPath) {
        var p = rawPath.replace(/\\/g, '/').replace(/^\/+/, '');
        var parts = p.split('/').filter(Boolean);
        if (parts.length === 0) return '';
        var upParts = parts.map(function(s) { return s.toUpperCase(); });

        // Falls ein übergeordneter Ordner ausgewählt wurde (z. B. "Realms of Arkania 2/..."),
        // ab bekannten Unterordnern oder Wurzeldateien schneiden.
        var knownFolders = ['DATA', 'GAMES', 'MUSIC', 'DRIVERS', 'CD'];
        for (var i = 0; i < upParts.length - 1; i++) {
            if (knownFolders.indexOf(upParts[i]) !== -1) {
                return upParts.slice(i).join('/');
            }
        }
        var base = upParts[upParts.length - 1];
        if (/^TRACK\d+\.OGG$/i.test(base)) {
            return 'MUSIC/' + base;
        }
        var rootFiles = ['SCHWEIF.EXE', 'PATH.CFG', 'SOUND.CFG', 'GAMEA.GOG', 'GAMEB.GOG', 'TRAIL.EXE', 'STARTRAI.EXE', 'SCHICKM.EXE', 'BLADE.EXE', 'RIVA.EXE'];
        if (rootFiles.indexOf(base) !== -1) {
            return base;
        }
        if (upParts.length >= 2) {
            return upParts.slice(-2).join('/');
        }
        return base;
    }

    function shouldImportFile(normPath) {
        if (!normPath) return false;
        var base = normPath.split('/').pop();
        if (!base || base.startsWith('.')) return false;
        if (base === 'GAMEA.GOG' || base === 'GAMEB.GOG') return true;
        if (base.endsWith('.EXE')) {
            return (
                base === 'SCHWEIF.EXE' ||
                base === 'TRAIL.EXE' ||
                base === 'STARTRAI.EXE' ||
                base === 'STARTRAIL.EXE' ||
                base === 'SCHICKM.EXE' ||
                base === 'BLADE.EXE' ||
                base === 'RIVA.EXE'
            );
        }
        for (var i = 0; i < IGNORIERTE_ENDUNGEN.length; i++) {
            if (base.endsWith(IGNORIERTE_ENDUNGEN[i])) return false;
        }
        return true;
    }

    // Mode2/2352 bzw. Mode1/2048 ISO9660-Leser für gamea.gog (extrahiert CD/DATA/* und CD/GAMES/* per Blob.slice)
    async function extractFromGogIsoBlob(blob, onProgress) {
        var extracted = {};
        if (!blob || blob.size < 40000) return extracted;

        async function readSlice(offset, length) {
            var ab = await blob.slice(offset, offset + length).arrayBuffer();
            return new Uint8Array(ab);
        }

        var sectorSize = 2352;
        var dataOffset = 24;
        var pvd = await readSlice(16 * 2352 + 24, 2048);
        var sig = String.fromCharCode(pvd[1], pvd[2], pvd[3], pvd[4], pvd[5]);
        if (sig !== 'CD001') {
            pvd = await readSlice(16 * 2048, 2048);
            sig = String.fromCharCode(pvd[1], pvd[2], pvd[3], pvd[4], pvd[5]);
            if (sig !== 'CD001') return extracted;
            sectorSize = 2048;
            dataOffset = 0;
        }

        async function readIsoBytes(lba, byteLen) {
            var out = new Uint8Array(byteLen);
            if (sectorSize === 2048) {
                var direct = await readSlice(lba * 2048, byteLen);
                out.set(direct);
                return out;
            }
            // In Blöcken zu je 2048 Sektoren (~4.8 MB Rohdaten) lesen und 2048 Nutzbytes je 2352-Byte-Sektor entpacken
            var totalSectors = Math.ceil(byteLen / 2048);
            var chunkSectors = 2048;
            var written = 0;
            for (var s = 0; s < totalSectors; s += chunkSectors) {
                var count = Math.min(chunkSectors, totalSectors - s);
                var raw = await readSlice((lba + s) * 2352, count * 2352);
                for (var k = 0; k < count && written < byteLen; k++) {
                    var take = Math.min(2048, byteLen - written);
                    var start = k * 2352 + dataOffset;
                    out.set(raw.subarray(start, start + take), written);
                    written += take;
                }
            }
            return out;
        }

        var view = new DataView(pvd.buffer, pvd.byteOffset, pvd.byteLength);
        var rootLba = view.getUint32(156 + 2, true);
        var rootLen = view.getUint32(156 + 10, true);

        var entries = [];
        async function scanDir(dirLba, dirLen, prefix) {
            var data = await readIsoBytes(dirLba, dirLen);
            var dv = new DataView(data.buffer, data.byteOffset, data.byteLength);
            var pos = 0;
            while (pos < dirLen) {
                var reclen = data[pos];
                if (reclen === 0) {
                    pos = ((pos >> 11) + 1) << 11;
                    continue;
                }
                var flba = dv.getUint32(pos + 2, true);
                var flen = dv.getUint32(pos + 10, true);
                var flags = data[pos + 25];
                var nlen = data[pos + 32];
                var name = '';
                for (var i = 0; i < nlen; i++) {
                    name += String.fromCharCode(data[pos + 33 + i]);
                }
                var semi = name.indexOf(';');
                if (semi !== -1) name = name.substring(0, semi);
                if (name.endsWith('.')) name = name.slice(0, -1);
                name = name.toUpperCase();

                if (name && name.charCodeAt(0) > 1) {
                    var full = prefix ? (prefix + '/' + name) : name;
                    if (flags & 2) {
                        await scanDir(flba, flen, full);
                    } else {
                        entries.push({ path: full, lba: flba, size: flen });
                    }
                }
                pos += reclen;
            }
        }

        await scanDir(rootLba, rootLen, '');

        // Alle CD-Datenarchive extrahieren (STAR.DAT, STARCD.DAT, SPEECH.DAT, SPEECHCD.DAT, FX.DAT, RAW.DAT sowie GAMES/*)
        var wanted = {
            'DATA/STAR.DAT': 'CD/DATA/STAR.DAT',
            'DATA/STARCD.DAT': 'CD/DATA/STARCD.DAT',
            'DATA/SPEECH.DAT': 'CD/DATA/SPEECH.DAT',
            'DATA/SPEECHCD.DAT': 'CD/DATA/SPEECHCD.DAT',
            'DATA/FX.DAT': 'CD/DATA/FX.DAT',
            'DATA/RAW.DAT': 'CD/DATA/RAW.DAT',
            'GAMES/CHARS1.DAT': 'CD/GAMES/CHARS1.DAT',
            'GAMES/EMPTY': 'CD/GAMES/EMPTY',
            'GAMES/GAMES.TMP': 'CD/GAMES/GAMES.TMP',
            'GAMES/START.GAM': 'CD/GAMES/START.GAM'
        };

        for (var idx = 0; idx < entries.length; idx++) {
            var e = entries[idx];
            var target = wanted[e.path];
            if (!target) continue;
            if (onProgress) onProgress('Entpacke CD-Abbild: ' + e.path + ' (' + (e.size / (1024 * 1024)).toFixed(1) + ' MB)…');
            extracted[target] = await readIsoBytes(e.lba, e.size);
        }
        return extracted;
    }

    async function checkSavedGameData() {
        var db = await openDB();
        return new Promise(function(resolve) {
            var tx = db.transaction(STORE_NAME, 'readonly');
            var store = tx.objectStore(STORE_NAME);
            var req = store.get(META_KEY);
            req.onsuccess = function() {
                db.close();
                var meta = req.result || null;
                if (meta && !meta.hasFullCdSpeech) {
                    resolve(null);
                    return;
                }
                resolve(meta);
            };
            req.onerror = function() {
                db.close();
                resolve(null);
            };
        });
    }

    async function loadSavedGameFiles(onProgress) {
        var db = await openDB();
        return new Promise(function(resolve, reject) {
            var tx = db.transaction(STORE_NAME, 'readonly');
            var store = tx.objectStore(STORE_NAME);
            var files = {};
            var count = 0;
            var cursorReq = store.openCursor();
            cursorReq.onsuccess = function(ev) {
                var cursor = ev.target.result;
                if (cursor) {
                    if (cursor.key !== META_KEY) {
                        files[cursor.key] = cursor.value instanceof Uint8Array ? cursor.value : new Uint8Array(cursor.value);
                        count++;
                        if (onProgress && count % 10 === 0) {
                            onProgress('Lade Spieldaten aus Browser-Speicher (' + count + ' Dateien)…');
                        }
                    }
                    cursor.continue();
                } else {
                    db.close();
                    resolve(files);
                }
            };
            cursorReq.onerror = function(ev) {
                db.close();
                reject(ev.target.error || new Error('Fehler beim Lesen aus IndexedDB.'));
            };
        });
    }

    async function clearSavedGameData() {
        var db = await openDB();
        await new Promise(function(resolve, reject) {
            var tx = db.transaction(STORE_NAME, 'readwrite');
            tx.objectStore(STORE_NAME).clear();
            tx.oncomplete = function() { db.close(); resolve(); };
            tx.onerror = function(ev) { db.close(); reject(ev.target.error); };
        });
    }

    async function validateAndStoreFiles(rawFileMap, onProgress) {
        var keys = Object.keys(rawFileMap);
        var hasStarDat = !!rawFileMap['DATA/STAR.DAT'] || !!rawFileMap['CD/DATA/STAR.DAT'];
        var hasSchweifExe = !!rawFileMap['SCHWEIF.EXE'];
        var hasEnglishExe = !!(rawFileMap['TRAIL.EXE'] || rawFileMap['STARTRAI.EXE'] || rawFileMap['STARTRAIL.EXE']);
        var hasDsa1 = !!(rawFileMap['SCHICKM.EXE'] || rawFileMap['BLADE.EXE'] || rawFileMap['SCHICK.DAT']);
        var hasDsa3 = !!rawFileMap['RIVA.EXE'];

        if (hasEnglishExe || (hasStarDat && !hasSchweifExe)) {
            throw new Error(
                'Englische Version (Star Trail) erkannt. Dieser Port unterstützt ausschließlich die deutsche CD-Version C1.02 von DSA 2: Sternenschweif (SCHWEIF.EXE).'
            );
        }
        if (hasDsa1 && !hasStarDat) {
            throw new Error(
                'DSA 1 (Die Schicksalsklinge) erkannt. Bitte wähle das Spielverzeichnis von DSA 2: Sternenschweif (mit SCHWEIF.EXE und DATA\\STAR.DAT).'
            );
        }
        if (hasDsa3 && !hasStarDat) {
            throw new Error(
                'DSA 3 (Schatten über Riva) erkannt. Bitte wähle das Spielverzeichnis von DSA 2: Sternenschweif (mit SCHWEIF.EXE und DATA\\STAR.DAT).'
            );
        }

        // Vollständige CD-Archive (STAR.DAT, SPEECH.DAT, FX.DAT, RAW.DAT) auch in DATA/* übernehmen,
        // damit sowohl DATA/* als auch CD/DATA/* alle CD-Einträge (inkl. SPEECH.DAT Eintrag 1 für check_speech_file
        // und alle Zwischensequenz-Sprachdateien) enthalten.
        var sharedArchives = ['STAR.DAT', 'SPEECH.DAT', 'FX.DAT', 'RAW.DAT'];
        for (var sa = 0; sa < sharedArchives.length; sa++) {
            var arcName = sharedArchives[sa];
            var hdKey = 'DATA/' + arcName;
            var cdKey = 'CD/DATA/' + arcName;
            if (rawFileMap[cdKey]) {
                if (!rawFileMap[hdKey] || rawFileMap[cdKey].byteLength >= rawFileMap[hdKey].byteLength) {
                    rawFileMap[hdKey] = rawFileMap[cdKey];
                }
            }
        }

        var missing = [];
        for (var i = 0; i < PFLICHT_DATEIEN.length; i++) {
            if (!rawFileMap[PFLICHT_DATEIEN[i]]) {
                missing.push(PFLICHT_DATEIEN[i]);
            }
        }
        if (missing.length > 0) {
            throw new Error(
                'Pflichtdateien fehlen: ' + missing.join(', ') +
                '. Bitte wähle deinen vollständigen Sternenschweif-Ordner (GOG „Realms of Arkania 2“ oder Original-CD-Installation mit SCHWEIF.EXE und DATA\\STAR.DAT).'
            );
        }

        var exeBytes = rawFileMap['SCHWEIF.EXE'];
        var exeHash = await sha256Hex(exeBytes);
        var versionLabel = BEKANNTE_VERSIONEN[exeHash] || ('Deutsche Version (SCHWEIF.EXE, ' + exeBytes.length.toLocaleString('de-DE') + ' Bytes)');
        var unknownVersionWarning = !BEKANNTE_VERSIONEN[exeHash];

        var musicTracks = 0;
        var totalBytes = 0;
        keys = Object.keys(rawFileMap);
        for (var k = 0; k < keys.length; k++) {
            if (/^MUSIC\/TRACK\d+\.OGG$/i.test(keys[k])) musicTracks++;
            totalBytes += rawFileMap[keys[k]].byteLength;
        }

        var hasCdStar = !!(rawFileMap['CD/DATA/STAR.DAT'] || (rawFileMap['DATA/STAR.DAT'] && rawFileMap['DATA/STAR.DAT'].byteLength > 12000000));
        var hasFullCdSpeech = !!(
            rawFileMap['CD/DATA/SPEECHCD.DAT'] &&
            ((rawFileMap['CD/DATA/SPEECH.DAT'] && rawFileMap['CD/DATA/SPEECH.DAT'].byteLength > 7000000) ||
             (rawFileMap['DATA/SPEECH.DAT'] && rawFileMap['DATA/SPEECH.DAT'].byteLength > 7000000))
        );

        if (onProgress) onProgress('Speichere ' + keys.length + ' Dateien in IndexedDB…');

        var meta = {
            importedAt: new Date().toISOString(),
            fileCount: keys.length,
            totalBytes: totalBytes,
            musicTracks: musicTracks,
            hasCdData: hasCdStar,
            hasFullCdSpeech: hasFullCdSpeech,
            exeSha256: exeHash,
            versionLabel: versionLabel,
            unknownVersionWarning: unknownVersionWarning
        };

        var db = await openDB();
        await new Promise(function(resolve, reject) {
            var tx = db.transaction(STORE_NAME, 'readwrite');
            var store = tx.objectStore(STORE_NAME);
            store.clear();
            store.put(meta, META_KEY);
            for (var m = 0; m < keys.length; m++) {
                var p = keys[m];
                store.put(rawFileMap[p], p);
            }
            tx.oncomplete = function() { db.close(); resolve(); };
            tx.onerror = function(ev) { db.close(); reject(ev.target.error || new Error('Fehler beim Speichern in IndexedDB.')); };
        });

        return { meta: meta, files: rawFileMap };
    }

    async function importFromFileList(fileList, onProgress) {
        var rawFileMap = {};
        var arr = Array.from(fileList || []);
        if (arr.length === 0) {
            throw new Error('Keine Dateien ausgewählt.');
        }

        if (arr.length === 1 && arr[0].name.toLowerCase().endsWith('.zip')) {
            return await importFromZipFile(arr[0], onProgress);
        }

        var gogIsoFiles = [];
        for (var i = 0; i < arr.length; i++) {
            var f = arr[i];
            var rel = f.webkitRelativePath || f.fullPath || f.name;
            var norm = normalizeGameRelPath(rel);
            if (!shouldImportFile(norm)) continue;
            if (norm === 'GAMEA.GOG' || norm === 'GAMEB.GOG') {
                gogIsoFiles.push(f);
                continue;
            }
            if (onProgress && i % 5 === 0) {
                onProgress('Lese Datei ' + (i + 1) + ' / ' + arr.length + ' (' + norm + ')…');
            }
            var buf = await f.arrayBuffer();
            rawFileMap[norm] = new Uint8Array(buf);
        }

        for (var g = 0; g < gogIsoFiles.length; g++) {
            var isoExtracted = await extractFromGogIsoBlob(gogIsoFiles[g], onProgress);
            var isoKeys = Object.keys(isoExtracted);
            for (var ik = 0; ik < isoKeys.length; ik++) {
                rawFileMap[isoKeys[ik]] = isoExtracted[isoKeys[ik]];
            }
        }

        return await validateAndStoreFiles(rawFileMap, onProgress);
    }

    async function importFromZipFile(zipFile, onProgress) {
        if (typeof fflate === 'undefined') {
            throw new Error('ZIP-Bibliothek (fflate) nicht geladen.');
        }
        if (onProgress) onProgress('Lese ZIP-Archiv (' + (zipFile.size / (1024 * 1024)).toFixed(1) + ' MB)…');
        var ab = await zipFile.arrayBuffer();
        if (onProgress) onProgress('Entpacke ZIP-Archiv…');
        var unzipped = fflate.unzipSync(new Uint8Array(ab));
        var rawFileMap = {};
        var gogBlobs = [];
        var entryNames = Object.keys(unzipped);
        for (var i = 0; i < entryNames.length; i++) {
            var entry = entryNames[i];
            if (entry.endsWith('/')) continue;
            var data = unzipped[entry];
            if (!data || data.byteLength === 0) continue;
            var norm = normalizeGameRelPath(entry);
            if (!shouldImportFile(norm)) continue;
            if (norm === 'GAMEA.GOG' || norm === 'GAMEB.GOG') {
                gogBlobs.push(new Blob([data]));
                continue;
            }
            rawFileMap[norm] = data;
        }
        for (var g = 0; g < gogBlobs.length; g++) {
            var isoExtracted = await extractFromGogIsoBlob(gogBlobs[g], onProgress);
            var isoKeys = Object.keys(isoExtracted);
            for (var ik = 0; ik < isoKeys.length; ik++) {
                rawFileMap[isoKeys[ik]] = isoExtracted[isoKeys[ik]];
            }
        }
        return await validateAndStoreFiles(rawFileMap, onProgress);
    }

    window.DSADaten = {
        checkSavedGameData: checkSavedGameData,
        loadSavedGameFiles: loadSavedGameFiles,
        clearSavedGameData: clearSavedGameData,
        importFromFileList: importFromFileList,
        importFromZipFile: importFromZipFile
    };
})();
