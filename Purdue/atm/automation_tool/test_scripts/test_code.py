#!/usr/bin/env python3

import subprocess
import sys
import logging
import json
import time
from datetime import datetime
from typing import List, Dict, Optional
from pathlib import Path


class ChainExecutionError(Exception):
    def __init__(self, script_name: str, error_details: str, exit_code: int):
        self.script_name = script_name
        self.error_details = error_details
        self.exit_code = exit_code
        super().__init__(f"Failed executing {script_name}: {error_details}")


class ScriptChain:
    def __init__(
        self, scripts: List[Dict[str, any]], chain_name: str = "default_chain"
    ):
        self.scripts = scripts
        self.chain_name = chain_name
        self.current_script = None
        self.logger = self._setup_chain_logger()
        self.execution_summary = {
            "start_time": None,
            "end_time": None,
            "total_duration": 0,
            "successful_scripts": [],
            "failed_script": None,
            "error_details": None,
        }

    def _setup_chain_logger(self) -> logging.Logger:
        logger = logging.getLogger(f"chain_{self.chain_name}")
        logger.setLevel(logging.INFO)

        # Create logs directory if it doesn't exist
        Path("logs").mkdir(exist_ok=True)

        # Create formatters for different handlers
        file_formatter = logging.Formatter(
            "%(asctime)s - %(name)s - %(levelname)s - %(message)s"
        )
        console_formatter = logging.Formatter(
            "%(message)s"  # Simplified format for console
        )

        # File handler
        fh = logging.FileHandler(
            f'logs/chain_{self.chain_name}_{datetime.now().strftime("%Y%m%d_%H%M%S")}.log'
        )
        fh.setFormatter(file_formatter)
        logger.addHandler(fh)

        # Console handler
        ch = logging.StreamHandler()
        ch.setFormatter(console_formatter)
        logger.addHandler(ch)

        return logger

    def _execute_script(self, script: Dict[str, any]) -> subprocess.CompletedProcess:
        script_path = script["path"]
        cmd = [
            "python3",
            script_path,
            "--caller",
            self.current_script or "chain_executor",
        ]

        # Add any additional arguments
        for arg, value in script.get("args", {}).items():
            cmd.extend([f"--{arg}", str(value)])

        self.logger.info(f"\n{'='*80}")
        self.logger.info(f"Executing: {' '.join(cmd)}")

        try:
            start_time = time.time()
            process = subprocess.run(cmd, check=True, capture_output=True, text=True)
            duration = time.time() - start_time

            self.logger.info(f"✓ Successfully completed in {duration:.2f} seconds")
            return process

        except subprocess.CalledProcessError as e:
            duration = time.time() - start_time
            error_msg = (
                f"\n❌ Script failed after {duration:.2f} seconds\n"
                f"Exit code: {e.returncode}\n"
                f"Error output:\n{e.stderr}\n"
                f"Standard output:\n{e.stdout}\n"
            )
            raise ChainExecutionError(script_path, error_msg, e.returncode)

    def execute_chain(self, continue_on_error: bool = False) -> Dict:
        self.execution_summary["start_time"] = datetime.now().isoformat()
        total_scripts = len(self.scripts)

        self.logger.info(f"\nStarting execution chain: {self.chain_name}")
        self.logger.info(f"Total scripts to execute: {total_scripts}")

        for index, script in enumerate(self.scripts, 1):
            script_path = script["path"]
            try:
                self.logger.info(
                    f"\nExecuting script {index}/{total_scripts}: {script_path}"
                )
                process = self._execute_script(script)
                self.execution_summary["successful_scripts"].append(
                    {
                        "script": script_path,
                        "args": script.get("args", {}),
                        "output": process.stdout.strip(),
                    }
                )
                self.current_script = script_path

            except ChainExecutionError as e:
                self.execution_summary["failed_script"] = script_path
                self.execution_summary["error_details"] = e.error_details

                self.logger.error(
                    f"\nChain execution failed at script {index}/{total_scripts}"
                )
                self.logger.error(e.error_details)

                if not continue_on_error:
                    break
                else:
                    self.logger.warning(
                        "Continuing execution due to continue_on_error=True"
                    )

        self.execution_summary["end_time"] = datetime.now().isoformat()
        self._log_final_summary()
        return self.execution_summary

    def _log_final_summary(self):
        start = datetime.fromisoformat(self.execution_summary["start_time"])
        end = datetime.fromisoformat(self.execution_summary["end_time"])
        duration = (end - start).total_seconds()
        self.execution_summary["total_duration"] = duration

        self.logger.info("\n" + "=" * 80)
        self.logger.info("EXECUTION CHAIN SUMMARY")
        self.logger.info("=" * 80)
        self.logger.info(f"Chain Name: {self.chain_name}")
        self.logger.info(f"Total Duration: {duration:.2f} seconds")
        self.logger.info(
            f"Successful Scripts: {len(self.execution_summary['successful_scripts'])}/{len(self.scripts)}"
        )

        if self.execution_summary["failed_script"]:
            self.logger.error(
                f"Chain Failed at: {self.execution_summary['failed_script']}"
            )
        else:
            self.logger.info("Chain Completed Successfully")

        # Save summary to JSON file
        summary_file = f"logs/chain_summary_{self.chain_name}_{datetime.now().strftime('%Y%m%d_%H%M%S')}.json"
        with open(summary_file, "w") as f:
            json.dump(self.execution_summary, f, indent=2)
        self.logger.info(f"\nDetailed summary saved to: {summary_file}")


# ----------------------------------------------------Scriplets are chained and call------------------------------------
# Example usage with error handling
scripts = [
    {
        "path": "script1.py",
        "args": {"input-file": "data.txt", "output-file": "output1.txt"},
    },
    {
        "path": "script2.py",
        "args": {"input-file": "output1.txt", "output-file": "final.txt"},
    },
]

try:
    chain = ScriptChain(scripts, chain_name="data_processing")
    summary = chain.execute_chain(continue_on_error=False)

except Exception as e:
    print(f"\nFatal error in chain execution: {str(e)}")
    sys.exit(1)
