import os
import sys


def is_frozen():
    return getattr(sys, 'frozen', False) and hasattr(sys, '_MEIPASS')


def get_script_path():
    return os.path.realpath(sys.argv[0])


def is_nt():
    return os.name.startswith("nt")


def is_posix():
    return os.name.startswith("posix")
