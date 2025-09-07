from __future__ import annotations
import logging
import sys

_LOGGER_CACHE = {}


def get_logger(name: str) -> logging.Logger:
    if name in _LOGGER_CACHE:
        return _LOGGER_CACHE[name]
    logger = logging.getLogger(name)
    if not logger.handlers:
        handler = logging.StreamHandler(sys.stderr)
        formatter = logging.Formatter('[%(asctime)s] %(levelname)s %(name)s: %(message)s')
        handler.setFormatter(formatter)
        logger.addHandler(handler)
        logger.setLevel(logging.INFO)
    _LOGGER_CACHE[name] = logger
    return logger
