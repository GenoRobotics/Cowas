import sys
from pathlib import Path

# tools/ is a folder of scripts, not a package: make analyze_la3 importable
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
