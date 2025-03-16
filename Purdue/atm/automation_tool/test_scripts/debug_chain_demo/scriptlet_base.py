#!/run/current-system/sw/bin/env python3

import argparse
import logging
import sys
import time
import os
import psutil
import tracemalloc
import json
from datetime import datetime
from icecream import ic
from typing import Any, Dict
from pathlib import Path


class ScriptletBase:
    def __init__(self, name: str):
        self.start_time = time.time()
        self.script_name = name
        self.logger = self._setup_logger()
        self.args = self._parse_arguments()
        self._initialize_debugging()
        self._log_startup_info()

    def _setup_logger(self) -> logging.Logger:
        logger = logging.getLogger(self.script_name)
        logger.setLevel(logging.INFO)

        # Create logs directory if it doesn't exist
        Path("logs").mkdir(exist_ok=True)

        formatter = logging.Formatter(
            "%(asctime)s - %(name)s - PID:%(process)d - %(levelname)s - %(message)s"
        )

        # File handler
        fh = logging.FileHandler(f"logs/{self.script_name}.log")
        fh.setFormatter(formatter)
        logger.addHandler(fh)

        # Console handler
        ch = logging.StreamHandler()
        ch.setFormatter(formatter)
        logger.addHandler(ch)

        return logger

    def _parse_arguments(self) -> argparse.Namespace:
        parser = argparse.ArgumentParser(description=f"Scriptlet: {self.script_name}")
        parser.add_argument(
            "-d", "--debug", action="store_true", help="Enable debug mode"
        )
        parser.add_argument("--caller", default="manual", help="Calling script name")
        parser.add_argument("--input-file", required=True, help="Input file path")
        parser.add_argument("--output-file", required=True, help="Output file path")
        return parser.parse_args()

    def _initialize_debugging(self):
        if self.args.debug:
            self.logger.setLevel(logging.DEBUG)
            tracemalloc.start()
            ic.enable()
        else:
            ic.disable()

    def _log_startup_info(self):
        self.logger.info(f"Starting {self.script_name}")
        self.logger.info(f"Launch time: {datetime.now()}")
        self.logger.info(f"Called by: {self.args.caller}")
        self.logger.info(f"Arguments: {vars(self.args)}")
        self.logger.info(f"PID: {os.getpid()}")

    def _log_completion_info(self):
        execution_time = time.time() - self.start_time
        self.logger.info(f"Execution time: {execution_time:.2f} seconds")

        if self.args.debug:
            process = psutil.Process(os.getpid())
            self.logger.debug(
                f"Memory usage: {process.memory_info().rss / 1024 / 1024:.2f} MB"
            )
            current, peak = tracemalloc.get_traced_memory()
            self.logger.debug(f"Current memory allocation: {current / 1024:.2f} KB")
            self.logger.debug(f"Peak memory allocation: {peak / 1024:.2f} KB")
            tracemalloc.stop()

    def run(self) -> Dict[str, Any]:
        try:
            result = self._execute()
            if self.args.debug:
                ic(result)
            return result
        except Exception as e:
            self.logger.error(f"Error occurred: {str(e)}")
            self.logger.debug("Exception details:", exc_info=True)
            raise
        finally:
            self._log_completion_info()

    def _execute(self) -> Dict[str, Any]:
        raise NotImplementedError
