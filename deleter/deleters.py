import os


__all__ = ["SubprocessMethod", "OSRemoveMethod", "BatchGotoMethod", "BatchStartMethod"]


class DeleteMethod:
    """Base class for all delete methods."""
    platforms = []

    def __init__(self):
        super(DeleteMethod, self).__init__()

    def run(self, script_path):
        pass

    def is_platform_compatible(self):
        return os.name in self.platforms


class SubprocessMethod(DeleteMethod):
    """Spawn new Python process and remove."""
    platforms = ["posix"]

    def __init__(self):
        super(SubprocessMethod, self).__init__()

    def run(self, script_path):
        import sys
        import subprocess
        subprocess.Popen("python -c \"import os, time; time.sleep(1); os.remove('{}');\"".format(script_path),
                         shell=True)
        sys.exit(0)


class OSRemoveMethod(DeleteMethod):
    """Delete script by calling `os.remove` and exit."""
    platforms = ["posix"]

    def __init__(self):
        super(OSRemoveMethod, self).__init__()

    def run(self, script_path):
        import os
        import sys
        os.remove(script_path)
        sys.exit(0)


class BatchStartMethod(DeleteMethod):
    """Creates batch file which kills Python process and then deletes itself."""
    platforms = ["nt"]

    def __init__(self):
        super(BatchStartMethod, self).__init__()

    def run(self, script_path):
        with open("deleter.bat", "w") as f:
            f.write("""
            TASKKILL /PID {} /F
            DEL "{}"
            start /b "" cmd /c del "%~f0"&exit /b
            """.format(os.getpid(), script_path))
        os.startfile(f.name)


class BatchGotoMethod(DeleteMethod):
    """Similar to batch start method. Uses `goto` instead of starting new process in batch file."""
    platforms = ["nt"]

    def run(self, script_path):
        with open("deleter.bat", "w") as f:
            f.write("""
            TASKKILL /PID {} /F
            DEL "{}"
            (goto) 2>nul & del "%~f0"
            """.format(os.getpid(), script_path))
        os.startfile(f.name)
