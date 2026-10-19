import csv
import importlib.util
import pathlib
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]
spec = importlib.util.spec_from_file_location('build_progression_bis', ROOT/'tools/build_progression_bis.py')
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class CatalogTests(unittest.TestCase):
    def test_faction_alternatives_and_phase_union(self):
        with (ROOT/'data/progression-bis/catalog.tsv').open() as f:
            rows = list(csv.DictReader(f, delimiter='\t'))
        self.assertTrue(any(r['selection_type'] == 'published-alternative' and "Khadgar" in r['name'] for r in rows))
        self.assertEqual({'classic', 'tbc', 'wrath'}, {r['era'] for r in rows})
        self.assertEqual(set(range(19)), {int(s) for r in rows for s in r['progression_states'].split(',')})
        self.assertTrue(any(r['name'] == 'Lionheart Helm' for r in rows))
        self.assertTrue(all(r['source'].startswith('https://wowtbc.gg/') for r in rows))
        self.assertEqual(len(rows), len({tuple(r.items()) for r in rows}))

    def test_ambiguous_identity_is_not_guessed(self):
        by = module.collections.defaultdict(list)
        base = dict(name='Test', RequiredLevel=60, InventoryType=1, ItemLevel=60, Flags=0)
        by['test'] = [{**base, 'entry': 1, 'Quality': 2}, {**base, 'entry': 2, 'Quality': 4}]
        result, reason = module.resolve('Test', {}, 'classic', 'warrior', by, set())
        self.assertEqual([], result)
        self.assertTrue(reason.startswith('ambiguous:'))

    def test_heroic_and_acquisition_disambiguation(self):
        by = module.collections.defaultdict(list)
        base = dict(name='Test', RequiredLevel=80, InventoryType=1, ItemLevel=277)
        by['test'] = [{**base, 'entry': 1, 'Flags': 0}, {**base, 'entry': 2, 'Flags': 8}]
        result, _ = module.resolve('Test (H)', {}, 'wrath', 'mage', by, {1, 2})
        self.assertEqual([2], [r['entry'] for r in result])


if __name__ == '__main__':
    unittest.main()
