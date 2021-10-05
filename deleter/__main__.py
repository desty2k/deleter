import sys
import argparse

from deleter import __version__


def main(argv):
    parser = argparse.ArgumentParser(
        prog='deleter',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        description="Delete Python script at exit",
    )
    parser.add_argument('-V', '--version', action='version', version='v{}'.format(__version__),
                        help='print version and exit')
    parser.parse_args(argv)


def main_entry():
    main(sys.argv[1:])


if __name__ == '__main__':
    main_entry()
