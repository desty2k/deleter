import os
import subprocess
from abc import abstractmethod, ABC

__all__ = ["SubprocessMethod", "OSRemoveMethod", "BatchGotoMethod", "BatchStartMethod"]


class DeleteMethod(ABC):
    """Base class for all delete methods."""
    platforms = []

    def __init__(self, script_path):
        super(DeleteMethod, self).__init__()
        self.script_path = script_path

    @abstractmethod
    def run(self):
        pass

    def is_compatible(self):
        return os.name in self.platforms


class SubprocessMethod(DeleteMethod):
    """Spawn new Python process and remove."""
    platforms = ["posix"]

    def __init__(self, script_path):
        super(SubprocessMethod, self).__init__(script_path)

    def run(self):
        import sys
        subprocess.run("python -c \"import os, time; time.sleep(1); os.remove('{}');\"".format(self.script_path),
                       shell=True)
        sys.exit(0)

    def is_compatible(self):
        return super().is_compatible() and subprocess.run("python", stdout=subprocess.DEVNULL,
                                                          stderr=subprocess.DEVNULL).returncode == 0


class OSRemoveMethod(DeleteMethod):
    """Delete script by calling `os.remove` and exit."""
    platforms = ["posix"]

    def __init__(self, script_path):
        super(OSRemoveMethod, self).__init__(script_path)

    def run(self):
        import os
        import sys
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
        os.startfile(f.name)

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
        os.startfile(f.name)

    def is_compatible(self):
        try:
            with open(BAT_FILENAME, "w+") as f:
                f.close()
                os.remove(f.name)
            return super().is_compatible()
        except Exception:
            return False
