import os
import sys
import shlex
import subprocess
from abc import abstractmethod, ABC

__all__ = ["SubprocessMethod", "OSRemoveMethod", "BatchGotoMethod", "BatchStartMethod"]


STARTUPINFO = None
if os.name == 'nt':
    STARTUPINFO = subprocess.STARTUPINFO()
    STARTUPINFO.dwFlags |= 0x08000000  # CREATE_NO_WINDOW
    STARTUPINFO.dwFlags |= 0x00000008  # DETACHED_PROCESS
    STARTUPINFO.dwFlags |= 0x00000200  # CREATE_NEW_PROCESS_GROUP


class DeleteMethod(ABC):
    """Base class for all delete methods."""
    platforms = []

    def __init__(self, script_path):
        super(DeleteMethod, self).__init__()
        self.script_path = os.path.normpath(script_path)

    @abstractmethod
    def run(self):
        pass

    def is_compatible(self):
        return os.name in self.platforms


class SubprocessMethod(DeleteMethod):
    """Spawn new Python process and call os.remove."""
    platforms = ["nt", "posix"]

    def __init__(self, script_path):
        super(SubprocessMethod, self).__init__(script_path)

    def run(self):
        subprocess.run(shlex.split("python -c \"import os, time; time.sleep(1); os.remove(r'{}');\""
                                   .format(self.script_path)), startupinfo=STARTUPINFO)
        sys.exit(0)

    def is_compatible(self):
        return super().is_compatible() and subprocess.run(["python", "-V"], stdout=subprocess.DEVNULL,
                                                          stderr=subprocess.DEVNULL).returncode == 0


class OSRemoveMethod(DeleteMethod):
    """Delete script by calling `os.remove` and exit."""
    platforms = ["posix"]

    def __init__(self, script_path):
        super(OSRemoveMethod, self).__init__(script_path)

    def run(self):
        os.remove(self.script_path)
        sys.exit(0)

    def is_compatible(self):
        return super().is_compatible() and os.access(self.script_path, os.W_OK)


BAT_FILENAME = "deleter.bat"


class BatchStartMethod(DeleteMethod):
    """Creates batch file which kills Python process and then deletes itself."""
    platforms = ["nt"]

    def __init__(self, script_path):
        super(BatchStartMethod, self).__init__(script_path)

    def run(self):
        with open(BAT_FILENAME, "w") as f:
            f.write("""
            TASKKILL /PID {} /F
            DEL "{}"
            start /b "" cmd /c del "%~f0"&exit /b
            """.format(os.getpid(), self.script_path))
        subprocess.run(f.name, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, startupinfo=STARTUPINFO)

    def is_compatible(self):
        try:
            with open(BAT_FILENAME, "w+") as f:
                f.close()
                os.remove(f.name)
            return super().is_compatible()
        except Exception:
            return False


class BatchGotoMethod(DeleteMethod):
    """Similar to batch start method. Uses `goto` instead of starting new process in batch file."""
    platforms = ["nt"]

    def __init__(self, script_path):
        super(BatchGotoMethod, self).__init__(script_path)

    def run(self):
        with open(BAT_FILENAME, "w") as f:
            f.write("""
            TASKKILL /PID {} /F
            DEL "{}"
            (goto) 2>nul & del "%~f0"
            """.format(os.getpid(), self.script_path))
        subprocess.run(f.name, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, startupinfo=STARTUPINFO)

    def is_compatible(self):
        try:
            with open(BAT_FILENAME, "w+") as f:
                f.close()
                os.remove(f.name)
            return super().is_compatible()
        except Exception:
            return False
