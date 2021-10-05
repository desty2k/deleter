import atexit

from deleter.deleters import *
from deleter.util import get_script_path

__all__ = ["register", "unregister"]


def _delete():
    for method in [BatchStartMethod, BatchGotoMethod, SubprocessMethod, OSRemoveMethod]:
        method = method()
        if method.is_platform_compatible():
            method.run(get_script_path())


def register():
    atexit.register(_delete)


def unregister():
    atexit.unregister(_delete)
