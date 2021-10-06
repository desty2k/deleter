import os
import time
import subprocess

FILENAME = "deleter_test.py"
TEST_SCRIPT = """
import deleter
deleter.register()
"""


def test_deleter():
    with open(FILENAME, "w+") as f:
        f.write(TEST_SCRIPT)
    subprocess.run("python {}".format(FILENAME))
    time.sleep(3)
    if os.path.isfile(FILENAME):
        raise Exception("deleter did not work")
