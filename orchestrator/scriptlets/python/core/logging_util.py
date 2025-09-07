### AUTOGEN(rebase_workspace.sh)
import logging, sys
def get_logger(name: str):
    logger = logging.getLogger(name)
    if not logger.handlers:
        h = logging.StreamHandler(sys.stderr)
        fmt = logging.Formatter("[%(levelname)s] %(name)s %(message)s")
        h.setFormatter(fmt)
        logger.addHandler(h)
        logger.setLevel(logging.INFO)
    return logger
