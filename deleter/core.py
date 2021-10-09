import atexit

from deleter.deleters import *
from deleter.util import get_script_path

__all__ = ["register", "unregister"]


def _delete():
    path = get_script_path()
    for method in [BatchStartMethod, BatchGotoMethod, SubprocessMethod, OSRemoveMethod]:
        deleter = method(path)
        if deleter.is_compatible():
            deleter.run()


def register():
    atexit.register(_delete)


def unregister():
    atexit.unregister(_delete)
