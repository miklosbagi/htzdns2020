# Release notes
## 1.0 (not yet released)

### Compatibility

| OS                      | Compatibility        | Notes                               |
| :---------------------- | :--------------------| :-----------------------------------|
| DDWRT on ARMv7          | ✅ Full support      |                                     |
| DDWRT on MIPS           | ⚠️ Not tested        |                                     |
| OpenWRT on MIPS         | ⚠️ Not tested        |                                     |
| Ubuntu 16 on ARMv7      | ⚠️ Not tested        | Should be all OK though.            |
| Ubuntu 16 on RaspPi 3b+ | ⚠️ Not tested        | Should be all OK though.            |
| Ubuntu 16 on RaspPi 4   | ⚠️ Not tested        | Should be all OK though.            |
| Gentoo Linux            | ✅ Full support      |                                     |
| MacOS 10.15 Catalina    | 💥 Not supported     | Bash 3.x compat require some rework |
| Windows 10              | 💥 Not supported     | Haven't been tested at all          |

### Features

Be friendly with the API:
- Don't hit the API all the time.
- Keep a local record of the latest IP updated

Be friendly with the running host:
- Silent run for cron-friendliness

Safety:
- Limit to A records
- Update individual records instead of generating full zone files.

Configuration:
- Interactive configuration builder, so user does not have to dig out zone and record IDs just to config

Lightweight:
- Bash, so we can embed into router firmware if needed
- Silent cron integration 
- Leverage already existing bits (e.g. whatismyip in vendor)

Scale:
- Per host configuration, to allow different boxes making updates to the same zone
- Allow unlimited zones, and unlimited records to be added.
- Update in bulk, not individually (may have its yet to be seen limits with the API).

Tests:
- Light testing skeleton
- Test coverage report

### TODO

Remaining:
- Fix MacOS Support
- Add notification support


