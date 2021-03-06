REGEX = r"^(?P<major>0|[1-9]\d*)\.(?P<minor>0|[1-9]\d*)\.(?P<patch>0|[1-9]\d*)(?:-(?P<prerelease>(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*)(?:\.(?:0|[1-9]\d*|\d*[a-zA-Z-][0-9a-zA-Z-]*))*))?(?:\+(?P<buildmetadata>[0-9a-zA-Z-]+(?:\.[0-9a-zA-Z-]+)*))?$"


def test_print_version():
    import re
    from subprocess import run, PIPE
    version = "".join(run(['python', '-m', 'deleter', '-V'], stdout=PIPE, stderr=PIPE).stdout.decode().splitlines()).replace("v", "")
    assert re.match(REGEX, version) is not None
