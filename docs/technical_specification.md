# AccountsManager technical specification

**Version 1.0 · November 3, 2026**
Preliminary specification for an independently developed, unofficial Clash of Clans companion.

## 1. Purpose and motivation

AccountsManager brings several Clash of Clans accounts into one interface. A player can see which accounts have attacks left in regular Clan Wars, Clan War Leagues (CWL) and individual Ranked Battles, and which clan they need to visit next.

The game API supplies activity data automatically. Manual input is appropriate for a player's own plans, such as a future clan transfer. The project also provides hands-on experience with C++, asynchronous APIs, storage and interface development. The author makes the technical decisions and uses AI for guidance.

## 2. Goals

1. Show activity across accounts on one screen.
2. Calculate used and remaining regular-war and CWL attacks from available game data.
3. Track individual Ranked attacks separately from CWL and unlimited farming battles.
4. Help players plan clan transfers during wars and CWL.
5. Present the information clearly and consistently.
6. Start with a useful local application while keeping future mobile access and synchronization feasible.

## 3. Audience and platforms

The first user is the author, who plays on several accounts. The application may later be published as a free, open-source tool for players with similar needs. Linux, including Omarchy and Hyprland, is the initial platform. Android and iOS are longer-term possibilities subject to feasibility. The local version operates without a server or cross-device synchronization.

## 4. User scenarios

### UC-01. Review all accounts

At launch the player sees each account's name, tag, current clan where known, status and remaining attacks in regular war, current CWL round and individual Ranked. Cards show planned transfers and the last successful update. The attention filter brings time-sensitive accounts to the top.

### UC-02. Check a regular war

Identify whether the account participates, how many attacks it has used and how many remain. Distinguish matchmaking, preparation, active war, completed war, absence of war and unavailable data.

### UC-03. Check CWL

Show seasonal eligibility, current round and individual attack status using the clan entered for CWL. The player's current clan can differ.

### UC-04. Check individual Ranked

Show the player's league, applicable tournament period, available allowance, usage and remainder when the source supports them. This is a required product scenario. When an exact value cannot be derived, explain the limitation.

### UC-05. Plan a clan transfer

The player sets a CWL clan and optionally plans a transfer to another clan for a regular war. The application shows where to return to attack and can send a reminder. The player performs all transfers in the game.

### UC-06. Refresh activity

Request a refresh on startup, at the configured interval or on user action according to the refresh settings. A request failure for one account must allow the others to remain usable.

## 5. Functional requirements

### 5.1. Account management

Add players by public tag, retrieve names and basic profile details, set optional aliases, remove accounts and reorder them. Persist the list between sessions. Users supply no passwords or Supercell ID credentials.

### 5.2. Dashboard

Give each activity its own status. Distinguish remaining attacks, completion, non-participation, waiting, unavailable data and insufficient data. Highlight accounts needing attention. The default list follows the user's order. The filtered list sorts by nearest actionable deadline and places undated actions last. Retain the last successful snapshot after a failure and mark its age. Missing information must never create a false zero.

### 5.3. Regular Clan Wars

Fetch the tracked clan's current war. Match the account to the lineup by player tag and use the limit for that war. Show the phase, used and remaining attacks and end time when available.

### 5.4. Clan War Leagues

Store the clan binding for the current CWL season. Fetch the group and relevant round, identify participation in that round and determine whether the player has attacked. Show the round's status and deadline where available. A temporary change of home clan must preserve CWL tracking.

### 5.5. Individual Ranked

Use the current league and tournament period. Derive the exact usage and remainder only from confirmed data. Support different allowances, period rollover and league changes. Where official data is insufficient, evaluate the coverage, delay and terms of alternative sources before adopting one. Display insufficient data while the answer is unknown.

### 5.6. Clans and reminders

Track the current clan, CWL clan and pinned regular-war clan separately. Accept a manually scheduled transfer. Show actions such as returning to an eligible war clan. Optional local reminders cover unused attacks during the final hour of a war or CWL round and scheduled transfers when due. A timed plan can trigger offline. Game reminders require fresh API data.

### 5.7. Settings and offline use

Provide manual refresh and an adjustable interval within API limits. Persist accounts, settings and transfer plans locally. Show labeled cached snapshots offline. Support light and dark Omarchy palettes and update the interface when the desktop theme changes. Users can enable reminders across graphical logins and close the window while reminders run.

## 6. Interface requirements

The main screen should answer “Which account needs me now?” Show the action and remaining attacks directly on the card. Keep all three modes visually distinct. Provide understandable loading, empty and error states. Use legible text labels as well as color. The desktop layout should allow later adaptation to smaller screens.

## 7. Data sources and integrations

Prefer the official read-only [Supercell developer API](https://developer.clashofclans.com/).

| Resource | Endpoint |
| --- | --- |
| Player profile | `GET /players/{playerTag}` |
| Clan profile | `GET /clans/{clanTag}` |
| Current regular war | `GET /clans/{clanTag}/currentwar` |
| CWL group | `GET /clans/{clanTag}/currentwar/leaguegroup` |
| CWL war | `GET /clanwarleagues/wars/{warTag}` |
| Ranked group | `GET /leaguegroup/{leagueGroupTag}/{leagueSeasonId}` |
| Recent battles | `GET /players/{playerTag}/battlelog` |

The presence of a battle log alone does not establish that it contains every attack in the period or the player's personal allowance. Third-party histories can have gaps and polling delays. Any additional provider must be identified and used in accordance with its terms. ClashKing is a candidate for research. [ClashKing API](https://api.clashk.ing/docs)

Farming, Ranked, regular war and CWL follow distinct rules. Ranked allowances and league structure changed in 2026 and may change again. [Supercell release notes](https://supercell.com/en/games/clashofclans/blog/release-notes/the-sound-of-clash-update/)

## 8. Technical requirements

Use Clang-supported C++26 features, CMake, Qt 6 Quick and QML, Qt Network and SQLite through Qt SQL. Separate the API client, domain decisions, storage and presentation. Use a maintainable local architecture that the author can explain. Introduce a backend only for a demonstrated requirement.

### Security and API constraints

- Use public game data and avoid collecting account passwords or automating game actions.
- Exclude API credentials from the repository, logs and distributed binaries. A local developer configuration is acceptable.
- Handle IP-bound keys, rate limits, authorization errors and outages. A future mobile or publicly distributed client may need a separately designed secure intermediary.
- Show when data was obtained and distinguish an actual absence of attacks from a failed request.

## 9. Quality requirements

A failed request must preserve other accounts' data and leave the UI responsive. Show the source, timestamp and uncertainty. Isolate platform-specific integration from domain logic. Keep components small and document build requirements and important decisions. Protect secrets in storage, logs and error messages. Prefer local operation without recurring costs and justify any additional infrastructure.

## 10. MVP scope

The Linux MVP includes multiple saved accounts, automatic API access, a unified dashboard, regular-war and CWL attack tracking, individual Ranked status, an independent CWL clan binding, transfer plans and handling of stale or unavailable information. The intended complete product shows the remaining attacks in all three modes without manual bookkeeping. An unavailable exact Ranked counter remains an explicit limitation.

Cloud accounts, synchronization, mobile builds, advanced analytics, in-game actions, mandatory push notifications and multiple speculative providers fall outside the MVP.

## 11. Development stages

1. Establish available API data for regular wars, CWL and Ranked.
2. Fetch one profile with a minimal C++ client and handle failures.
3. Model multiple accounts, clans, wars, CWL, Ranked and attack status.
4. Persist accounts, clan bindings, plans, timestamps and cache.
5. Provide a QML dashboard and account settings.
6. Refine multi-account operation, unavailable-data handling and Linux documentation.
7. Extend the desktop workflow with opt-in reminders. Consider mobile UI and secure synchronization when actual demand justifies them.

The educational goal is for the author to understand the implementation and adopt each technology when the next feature needs it.

## 12. Acceptance criteria

- Users can add multiple accounts by tag and see their names, tags and known current clans.
- Regular wars and CWL are tracked separately. Attack counts reflect participation and actual game data.
- CWL tracking survives a temporary clan transfer.
- Ranked excludes farming and clan wars. The UI shows a remainder when supported and explains any limitation.
- The overview identifies accounts that need action. A transfer plan is saved locally and the player performs it in the game.
- Network failures preserve labeled snapshots. Updating accounts leaves the UI responsive and persists local settings.
- API keys stay out of the repository and distributed binaries.
- The application runs on Linux.

## 13. Risks and open questions

| Risk | Response |
| --- | --- |
| Incomplete Ranked data | Require evidence for both usage and allowance before displaying a count. |
| Changing league rules | Keep the rules outside the UI and support varying limits. |
| CWL played in another clan | Store the CWL binding independently. |
| A private or inaccessible war | Display an unavailable status. |
| Keys restricted by IP on future mobile releases | Evaluate a secure backend at that stage. |
| Premature complexity | Begin with the local client and expand on demand. |
| Existing tools such as War Report and ClashKing | Prioritize the author's workflow and engineering practice. |

## 14. Publication and legal considerations

AccountsManager is an unofficial fan application. A public release must respect Supercell's terms for trademarks, game materials, permitted behavior and attribution. Review the current policy and dependency licenses before distribution. Fan content is generally intended for noncommercial use unless separately authorized by Supercell. [Fan Content Policy](https://supercell.com/en/fan-content-policy/)

> This material is unofficial and is not endorsed by Supercell. For more information see Supercell's Fan Content Policy: www.supercell.com/fan-content-policy.

## 15. Development references

- [Official Clash of Clans Developer Portal](https://developer.clashofclans.com/)
- [Unofficial Markdown mirror of the API reference](https://github.com/MasiaAntoine/clash-of-clan-api-doc-official)
- [ClashKing API](https://api.clashk.ing/docs)
- [Ranked rule changes, April 27, 2026](https://supercell.com/en/games/clashofclans/blog/release-notes/the-sound-of-clash-update/)
- [Supercell Fan Content Policy](https://supercell.com/en/fan-content-policy/)
- [Qt 6 documentation](https://doc.qt.io/qt-6/)

The Linux implementation uses Qt Quick, Qt Network and SQLite. Regular wars follow the current or pinned clan, and the CWL clan remains independent. The application derives game activity from the API. Current limitations are described in the [API contract](api_contract.md).
