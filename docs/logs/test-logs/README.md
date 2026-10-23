# On-device test logs

Written by `tools/session_log.py` (the console with its log). One folder per run:

```
<YYYY-MM-DD>/<NN>_<tag>/
    session.log    everything on the console, with the PC time. `<` from the Due, `>` typed, `!` from the tool
    tests.csv      one row per TEST line
    samples/       one JSON per sample or purge, and samples.csv
```

- `NN` counts the runs of the day: 01, 02, ...
- A restart with the same tag on the same day writes into the same folder.
- At `:quit` the tool checks that every saved sample is on disk. If not, it writes `SAMPLES CHECK` in `session.log`.

**If `samples/` is lost**, rebuild it from `session.log` (no port needed):

```
tools\session_log.cmd --rebuild docs\logs\test-logs\<date>\<NN>_<tag>
```

If that folder already has samples, the rebuild goes into the next numbered folder. Nothing is overwritten.

Runs up to 2026-10-19 are in [the archive](../../archive/test-logs-2026-10/README.md).

Commit the logs of each hardware check: [LOG.md](../LOG.md) points to them as evidence.
