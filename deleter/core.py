import atexit

from deleter.deleters import *
from deleter.util import get_script_path

__all__ = ["register", "unregister", "run"]


def run():
    path = get_script_path()
    for method in [BatchStartMethod, BatchGotoMethod, OSRemoveMethod, SubprocessMethod]:
        deleter = method(path)
        if deleter.is_compatible():
            deleter.run()


def register():
    atexit.register(run)


def unregister():
    atexit.unregister(run)
