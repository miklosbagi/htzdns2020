# Release notes
## 1.0 (not yet released)
Completed features:

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
---
Remaining:
- Test with MacOS, Gentoo Linux and Raspberry Pi.
- Add notification support
- 
