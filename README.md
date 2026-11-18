# AccountsManager

A local Linux companion for tracking Clan War, Clan War League and Ranked attacks across Clash of Clans accounts.

## Quick start

Install Clang with C++26 support, CMake 4.4.5+, GNU Make, Qt 6.8+ and vcpkg.
Create a Supercell API key for your IP address and put it in an untracked `.env` in the project directory.

```dotenv
API_KEY=your_key
```

```sh
make run release
```

Open the accounts tab and add a public player tag. [Developer Portal](https://developer.clashofclans.com/)

[Usage and dependencies](docs/usage.md) · [API contract](docs/api_contract.md) · [Specification](docs/technical_specification.md) · [Third-party notices](docs/third_party_notices.md)

This material is unofficial and is not endorsed by Supercell. [Fan Content Policy](https://supercell.com/en/fan-content-policy/)
