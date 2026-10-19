# Published phase-BiS reference

Snapshot: October 19, 2026. `catalog.tsv` contains factual item IDs, class/spec/slot,
published phase, advisory Individual Progression state tags and the original source
page. These are WOWTBC.GG's **explicit BiS selections and explicit alternatives**, kept
as published; no new rankings or stat weights were invented. `sources.json` records
80 page-data URLs and SHA-256 receipts. `coverage.json` counts the data and records
three unavailable Classic source variants. `unresolved.json` is empty for the imported
pages: every selected label resolves to verified realm item-template IDs.

The catalog currently contains 9,013 rows and 2,327 distinct item IDs (some appear in
multiple expansions/specs/phases). Of these, 130 have template binding that permits
ordinary AH trade; listing filters still apply. The full catalog intentionally retains
BoP recommendations for reference without changing listing eligibility.

The lists are available at these publisher entry points; each row links its exact page:

- [Classic example](https://wowtbc.gg/classic/bis-list/fury-warrior/)
- [TBC example](https://wowtbc.gg/bis-list/fury-warrior/)
- [Wrath example](https://wowtbc.gg/wotlk/bis-list/fury-warrior/)

The publisher excludes some PvP/world-boss choices and pre-raid world-drop BoE epics.
Three requested Classic variants were unavailable at snapshot time: two-handed Fury,
Protection mitigation and Protection Paladin. The separately published Fury/Protection
Warrior tank list is included. The catalog is complete for the retrieved publisher
pages, **not a claim of exhaustive BiS coverage for every possible realm build**.
Published lists can disagree and Classic itemization (notably Ulduar levels) differs
from original 3.3.5. Item identity is preserved while both published and realm item
levels are retained for inspection. No item stats are changed by this catalog.

## State mapping

Individual Progression states describe content already completed. These advisory tags
follow its existing access checks/default ZG/ZA settings, not literal Classic phase
numbers. Prices use the union because different characters occupy different states.

| Published list phase | Individual Progression states |
| --- | --- |
| Classic pre-raid, P1, P2 | 0 |
| Classic P3 (BWL) | 1, 2 |
| Classic P4 (ZG) | 3 |
| Classic P5 (AQ) | 4, 5 |
| Classic P6 (Naxx40) | 6, 7 |
| TBC pre-raid, T4 | 8 |
| TBC T5 | 9 |
| TBC T6 | 10, 11 |
| TBC ZA, Sunwell | 12 |
| Wrath pre-raid, T7 | 13 |
| Wrath T8 | 14 |
| Wrath T9 | 15 |
| Wrath T10 | 16 |
| Wrath T10.5 / Ruby Sanctum | 17, 18 |

State 11 is commented/deprecated in the current progression enum and retained only
as a compatibility tag. ZA and Sunwell share state 12 under the default gates. Custom
gate settings may differ. These tags are not new availability checks and the source's
published recommendation phase is always retained verbatim.

## Rebuild and identity rules

`tools/build_progression_bis.py` consumes cached publisher page-data plus an exported
world item-template identity reference and loot/vendor item IDs. It never executes
publisher code or writes to the database. It imports only `bis: true` entries and
explicit `alternative` labels for those entries. Names resolve using era level bounds,
item level when it matches, weapon hand, class restriction, heroic flag, acquisition
records and matching primary-stat fingerprints. Stat filters only narrow a positive
match because Classic and 3.3.5 can differ. Equal identity/stat variants are retained
(e.g. faction counterparts); otherwise ambiguous IDs fail into `unresolved.json`.
Small publisher typos/labels are normalized explicitly in the importer. Mark of the
Champion's AP/SP labels have explicit IDs. Rankings and phase selections are untouched.

Example with a source cache and read-only reference exports:

```sh
python3 tools/build_progression_bis.py --cache /path/to/cache \
  --items /path/to/item-reference.json --obtainable /path/to/obtainable-item-ids.json \
  --output data/progression-bis
python3 tools/build_bis_header.py
```

The cache's `sources.json` must retain each source URL, class/spec/era, raw filename
and receipt hash. Regeneration refuses changed raw content. Source cache and full live
item exports are audit inputs rather than committed publisher page copies. The runtime
header is generated solely from the committed TSV; `--check` validates that union
without network or database access. Future source refreshes should update receipts,
review identity changes/coverage and regenerate the header through a new reviewed PR.
