"""Central logging utility.

Provides get_logger which standardizes formatting for the automation framework.
"""
from __future__ import annotations
import logging
import sys

_LOGGER_CACHE = {}

FMT = "%(asctime)s | %(levelname)s | %(name)s | %(message)s"
DATEFMT = "%Y-%m-%dT%H:%M:%S"

def get_logger(name: str) -> logging.Logger:
    if name in _LOGGER_CACHE:
        return _LOGGER_CACHE[name]
    logger = logging.getLogger(name)
    if not logger.handlers:
        handler = logging.StreamHandler(sys.stderr)
        handler.setFormatter(logging.Formatter(FMT, DATEFMT))
        logger.addHandler(handler)
        logger.setLevel(logging.INFO)
        logger.propagate = False
    _LOGGER_CACHE[name] = logger
    return logger
