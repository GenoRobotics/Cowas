# On-device test logs

Written by `tools/hw_test_log.py` (FW2): `<YYYY-MM-DD>_<tag>.csv` holds one row per `TEST|<name>|<id>|<PASS/FAIL/SKIP>|<k=v;...>` line (columns: host_time, tag, name, id, result, data, raw), and `<YYYY-MM-DD>_<tag>.log` the full console transcript.

Commit the logs of every hardware checklist run: docs/TESTS_PENDING.md and docs/FIRMWARE_LOG.md point to them as evidence.
