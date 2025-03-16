#!/usr/bin/env python3
import sys
from pathlib import Path
import subprocess
import logging
from datetime import datetime

BASE_DIR = Path(__file__).parent.absolute()

# Create necessary directories
(BASE_DIR / "data").mkdir(exist_ok=True)
(BASE_DIR / "logs").mkdir(exist_ok=True)

# Define script chain
scripts = [
    {
        "path": "process_numbers.py",
        "args": {
            "input-file": str(BASE_DIR / "data" / "dummy.txt"),
            "output-file": str(BASE_DIR / "data" / "numbers.json"),
            "d": "",  # Empty string for boolean flag
        },
    },
    {
        "path": "calculate_stats.py",
        "args": {
            "input-file": str(BASE_DIR / "data" / "numbers.json"),
            "output-file": str(BASE_DIR / "data" / "stats.json"),
            "d": "",
        },
    },
    {
        "path": "generate_report.py",
        "args": {
            "input-file": str(BASE_DIR / "data" / "stats.json"),
            "output-file": str(BASE_DIR / "data" / "final_report.txt"),
            "d": "",
        },
    },
]

# Set up logging
logger = logging.getLogger("chain_runner")
logger.setLevel(logging.INFO)
fh = logging.FileHandler(
    BASE_DIR / "logs" / f"chain_{datetime.now():%Y%m%d_%H%M%S}.log"
)
ch = logging.StreamHandler()
formatter = logging.Formatter("%(asctime)s - %(name)s - %(levelname)s - %(message)s")
fh.setFormatter(formatter)
ch.setFormatter(formatter)
logger.addHandler(fh)
logger.addHandler(ch)


def run_chain():
    logger.info("Starting script chain execution")

    for script in scripts:
        cmd = [sys.executable, str(BASE_DIR / script["path"])]
        for arg, value in script["args"].items():
            if value:  # If value is not empty
                cmd.extend([f"--{arg}", value])
            else:  # For boolean flags
                cmd.append(f"-{arg}")

        logger.info(f"Executing: {' '.join(cmd)}")
        try:
            subprocess.run(cmd, check=True)
            logger.info(f"Successfully completed {script['path']}")
        except subprocess.CalledProcessError as e:
            logger.error(f"Failed executing {script['path']}: {str(e)}")
            sys.exit(1)

    logger.info("Chain execution completed successfully")


if __name__ == "__main__":
    try:
        run_chain()
        print(f"\nCheck the report at: {BASE_DIR / 'data' / 'final_report.txt'}")
    except Exception as e:
        print(f"\nFatal error in chain execution: {str(e)}")
        sys.exit(1)
