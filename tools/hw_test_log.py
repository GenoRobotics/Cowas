#!/usr/bin/env python3
"""Old name of tools/session_log.py (the FW2 on-device test logger), kept so that

    python tools/hw_test_log.py --port COM6 --tag s1-1

and `import hw_test_log` (tools/compare_tune.py, the tests) keep working. The logic is in session_log.py (2026-10-21).
"""

import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from session_log import TEST_CSV_COLUMNS as CSV_COLUMNS  # noqa: F401  (re-exported for the old imports)
from session_log import RESULTS, append_csv, log_paths, main, parse_kv, parse_test_line  # noqa: F401

if __name__ == "__main__":
    sys.exit(main())
