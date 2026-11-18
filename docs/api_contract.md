# API contract and limitations

AccountsManager reads the official Supercell API at `https://api.clashofclans.com/v1`.

## Regular Clan Wars

`GET /clans/{clanTag}/currentwar` supplies the phase, start and end times, `attacksPerMember` and the members of both sides. Player tags identify participants. An explicit empty `attacks` array counts as zero. For a missing array, the count is zero only when the full lineup is present and the recorded attacks already account for the entire team total. Otherwise it remains unknown. An explicit null also remains unknown. The parser rejects duplicate attack orders, mismatched attacker tags and invalid field types.

The parser accepts camelCase phases and underscore variants. An unknown phase yields insufficient data. The client compares both clan tags and the start time against CWL wars to identify a shared war. If the type remains uncertain, it withholds the regular-war count.

## Clan War Leagues

The configured CWL clan determines the group requested from `GET /clans/{clanTag}/currentwar/leaguegroup`. Published war tags lead to `GET /clanwarleagues/wars/{warTag}`. The placeholder `#0` has no war resource. Group HTTP 404 means that the group could not be found for that clan.

The season roster establishes eligibility. Each round has its own lineup. A player outside the season roster can be classified from the group alone. A uniquely identified, currently active war for the clan provides the attack count independently of missing historical rounds. An active war takes precedence over preparation. If a missing war could determine the current round, the count remains unknown. Expired wars do not trigger attack reminders. A missing or null personal attack array in a CWL war leaves usage unknown.

## Weekly Ranked

The player profile supplies `currentLeagueGroupTag` and `currentLeagueSeasonId` for `GET /leaguegroup/{leagueGroupTag}/{leagueSeasonId}?playerTag=…`. For the matching `members[].playerTag`, attacks used equal `attackWinCount + attackLoseCount`. Both counters must be nonnegative integers. Zero is valid. Missing counters, repeated members, invalid counts and overflow cannot produce an exact remainder. Profile-wide wins, defenses, trophies and battle-log length do not enter the calculation.

The limit comes from `parameters.maxBattles`. When absent, the interface labels the fallback limit from the [Supercell league rules](https://sc-clashofclans.zendesk.com/hc/en-us/articles/49529109982875-Ranked-Leagues).

| League tiers | Weekly attacks |
| --- | ---: |
| 1–6 | 6 |
| 7–12 | 8 |
| 13–18 | 10 |
| 19–27 | 12 |
| 28–30 | 14 |
| 31–33 | 18 |
| Legend III (34) | 24 |
| Legend II (35) | 30 |

The table covers IDs `105000001` through `105000035` and needs updating when the rules change. Unknown tiers have no fallback. The profile period must match the start of the current week at Monday 05:00 UTC. A supplied `parameters.seasonId` must also match. A rollover, missing participant or usage above the limit yields insufficient data. Each count belongs to a single API snapshot.

## Legend I

Legend I (`105000036`) uses league days starting at 05:00 UTC. Late entry can reduce the player's daily allocation. The battle log identifies observed attacks and their timestamps. An exact remainder also requires complete daily coverage and the number of targets actually assigned to the player. The dashboard reports the daily allocation as unknown. [Supercell Legend League guide](https://support.supercell.com/clash-of-clans/en/articles/legend-league-4.html)

ClashKing's daily Legend battle log lists observed attacks and defenses. Its public schema omits the player's assigned target count, so it cannot supply the missing allowance. AccountsManager uses Supercell as its sole game data source. [ClashKing API schema](https://api.clashk.ing/openapi.json)

## Network and cache

Qt Network handles requests asynchronously and validates TLS certificates. The client limits responses to 4 MiB, allows up to four simultaneous requests, sets a ten-second deadline and backs off after HTTP 429. These are client settings. SQLite retains the last successful snapshot after an error. The dashboard shows its age and error state. Counters apply only to their own war or tournament period. [Qt Network](https://doc.qt.io/qt-6/qnetworkaccessmanager.html)
