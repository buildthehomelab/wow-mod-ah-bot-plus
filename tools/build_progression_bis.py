#!/usr/bin/env python3
"""Normalize factual published BiS selections; never infer a ranking or silently guess an ID."""
import argparse
import collections
import csv
import hashlib
import json
import pathlib
import re

PHASES = {
    'classic': {'pre-bis': [0], 'p1': [0], 'p2': [0], 'p3': [1, 2], 'p4': [3], 'p5': [4, 5], 'p6': [6, 7]},
    'tbc': {'pre-bis': [8], 't4': [8], 't5': [9], 't6': [10, 11], 'za': [12], 'swp': [12]},
    'wrath': {'pre-bis': [13], 't7': [13], 't8': [14], 't9': [15], 't10': [16], 't10.5': [17, 18]},
}
CLASS = {'warrior': 1, 'paladin': 2, 'hunter': 3, 'rogue': 4, 'priest': 5,
         'death knight': 6, 'shaman': 7, 'mage': 8, 'warlock': 9, 'druid': 11}
PRIMARY = {'agility': 3, 'strength': 4, 'intellect': 5, 'spirit': 6, 'stamina': 7}
ALIASES = {'Lizardscale Eyepatch': "Foror's Eyepatch", 'Gloves of Bitte Reprisal (H)': 'Gloves of Bitter Reprisal (H)'}
# Publisher labels distinguish these same-name quest trinkets by their effect.
EXPLICIT = {'Mark of the Champion-v2': [23206], 'Mark of the Champion (SP)': [23207]}


def normal(name):
    return name.replace('\u2019', "'").strip().casefold()


def resolve(name, gear, era, cls, by_name, obtainable):
    if name in EXPLICIT:
        return [i for values in by_name.values() for i in values if i['entry'] in EXPLICIT[name]], 'explicit-effect-variant'
    base = ALIASES.get(name, name).replace("VanCleef's'", "VanCleef's").replace("Windrunner's (A)", "Windrunner's")
    base = re.sub(r'[- ]v[0-9]+$', '', base)
    base = re.sub(r' \((?:H|N|OH|A|SP|Druid|Rogue|Priest|Hunter)\)$', '', base)
    candidates = [i for i in by_name[normal(base)] if i['InventoryType'] and i['RequiredLevel'] <= {'classic': 60, 'tbc': 70, 'wrath': 80}[era]]
    if '(OH)' in name:
        candidates = [i for i in candidates if i['InventoryType'] == 22]
    elif base == 'Warglaive of Azzinoth':
        candidates = [i for i in candidates if i['InventoryType'] == 21]
    if base == "Royal Seal of Eldre'Thalas":
        candidates = [i for i in candidates if i['AllowableClass'] & (1 << (CLASS[cls] - 1))]
    if name.endswith(' (H)'):
        candidates = [i for i in candidates if i['Flags'] & 8]
    level = gear.get('ilvl')
    if level and any(i['ItemLevel'] == level for i in candidates):
        candidates = [i for i in candidates if i['ItemLevel'] == level]
    if len(candidates) > 1 and any(i['entry'] in obtainable for i in candidates):
        candidates = [i for i in candidates if i['entry'] in obtainable]
    # Only narrow by a stat fingerprint when it actually matches. Classic/Wrath
    # publishers and 3.3.5 may have different historic itemization (notably Ulduar).
    for stat, code in PRIMARY.items():
        val = gear.get('primary_stats', {}).get(stat)
        if not isinstance(val, (float, int)):
            continue
        matching = [i for i in candidates if sum(i.get('stat_value'+str(k), 0) for k in range(1, 11) if i.get('stat_type'+str(k)) == code) == val]
        if matching:
            candidates = matching
    spell_crit = gear.get('secondary_stats', {}).get('spell_crit')
    if isinstance(spell_crit, (float, int)):
        matching = [i for i in candidates if sum(i.get('stat_value'+str(k), 0) for k in range(1, 11) if i.get('stat_type'+str(k)) == 21) == spell_crit]
        if matching:
            candidates = matching
    if len(candidates) <= 1:
        return candidates, 'unique-name/metadata'
    keys = ['ItemLevel', 'InventoryType', 'class', 'subclass', 'Quality', 'spellid_1', 'spellid_2'] + [f'{field}{k}' for k in range(1, 11) for field in ['stat_type', 'stat_value']]
    fingerprints = {tuple(i.get(k, 0) for k in keys) for i in candidates}
    if len(fingerprints) == 1:
        return candidates, 'equivalent-variants'
    return [], 'ambiguous:' + ','.join(str(i['entry']) for i in candidates)


def build(sources, cache, items, obtainable, output):
    by_name = collections.defaultdict(list)
    for item in items:
        by_name[normal(item['name'])].append(item)
    rows, unresolved, provenance = [], [], []
    used = set()
    for source in sources:
        if not source['ok']:
            continue
        raw = (cache / pathlib.Path(source['file']).name).read_bytes()
        if hashlib.sha256(raw).hexdigest() != source['sha256']:
            raise ValueError('Source hash mismatch: ' + source['page'])
        ctx = json.loads(raw)['result']['pageContext']
        gear = {g['name']: g for g in ctx['gearData'] if isinstance(g.get('name'), str)}
        provenance.append({k: source[k] for k in ['era', 'cls', 'spec', 'slug', 'page', 'url', 'sha256']})
        for selection in ctx['bisList']:
            names = [(selection.get('name'), 'main')]
            if isinstance(selection.get('alternative'), str):
                names.append((selection['alternative'], 'published-alternative'))
            for name, selection_type in names:
                if not isinstance(name, str):
                    continue
                metadata = gear.get(name, gear.get(selection.get('name'), {}))
                for phase, states in PHASES[source['era']].items():
                    if selection.get(phase, {}).get('bis') is not True:
                        continue
                    matches, resolution = resolve(name, metadata, source['era'], source['cls'], by_name, obtainable)
                    if not matches:
                        unresolved.append({'era': source['era'], 'class': source['cls'], 'spec': source['spec'], 'phase': phase, 'name': name, 'reason': resolution, 'source': source['page']})
                    for item in matches:
                        used.add(item['entry'])
                        rows.append(dict(item_id=item['entry'], name=item['name'], era=source['era'], phase=phase,
                                         progression_states=','.join(map(str, states)), class_name=source['cls'], spec=source['spec'],
                                         slot=selection.get('slot', ''), published_name=name, source_ilvl=metadata.get('ilvl', ''),
                                         realm_ilvl=item['ItemLevel'], binding=item['bonding'], selection_type=selection_type,
                                         resolution=resolution, source=source['page']))
    rows = list({tuple(row.items()): row for row in rows}.values())
    output.mkdir(parents=True, exist_ok=True)
    rows.sort(key=lambda r: (r['era'], r['phase'], r['class_name'], r['spec'], r['slot'], r['item_id']))
    with (output / 'catalog.tsv').open('w') as f:
        writer = csv.DictWriter(f, fieldnames=list(rows[0]), delimiter='\t', lineterminator='\n')
        writer.writeheader(); writer.writerows(rows)
    (output / 'sources.json').write_text(json.dumps(provenance, indent=2) + '\n')
    (output / 'unresolved.json').write_text(json.dumps(unresolved, indent=2) + '\n')
    report = {'rows': len(rows), 'distinct_items': len(used), 'pages': len(provenance), 'unresolved_rows': len(unresolved),
              'unavailable_sources': [{k: s[k] for k in ['era', 'cls', 'spec', 'url']} for s in sources if not s['ok']],
              'by_era': {era: {'rows': sum(r['era'] == era for r in rows), 'items': len({r['item_id'] for r in rows if r['era'] == era})} for era in PHASES}}
    (output / 'coverage.json').write_text(json.dumps(report, indent=2) + '\n')
    return report


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--cache', type=pathlib.Path, required=True)
    parser.add_argument('--items', type=pathlib.Path, required=True)
    parser.add_argument('--obtainable', type=pathlib.Path, required=True)
    parser.add_argument('--output', type=pathlib.Path, required=True)
    args = parser.parse_args()
    print(json.dumps(build(json.loads((args.cache/'sources.json').read_text()), args.cache,
                           json.loads(args.items.read_text()), set(json.loads(args.obtainable.read_text())), args.output), indent=2))
