#!/usr/bin/env python
# -*- coding: utf-8 -*-

from setuptools import setup, find_packages

# Package imports
from deleter import __version__

with open("README.md", "r", encoding="utf-8") as f:
    long_desc = f.read()

setup(
    name='deleter',
    version=__version__,
    packages=find_packages(),
    url='https://github.com/desty2k/deleter',
    license='MIT',
    author='Wojciech Wentland',
    author_email='wojciech.wentland@int.pl',
    description='Delete Python scripts at exit',
    long_description_content_type='text/markdown',
    python_requires='>=3.6',
    zip_safe=False,  # don't use eggs
    long_description=long_desc,

    entry_points={
        'console_scripts': [
            'deleter=deleter.__main__:main_entry',
        ],
    },

    classifiers=[
        'Development Status :: 4 - Beta',

        'License :: OSI Approved :: MIT License',

        'Programming Language :: Python',
        'Programming Language :: Python :: 3',
        'Programming Language :: Python :: 3 :: Only',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',

        'Programming Language :: Python :: Implementation :: CPython',

        'Intended Audience :: Developers',
        'Intended Audience :: Other Audience',
        'Intended Audience :: System Administrators',
        'Intended Audience :: Information Technology',

        'Topic :: Software Development',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Utilities',

    ],
    keywords=['delete', 'remove', 'autoremove', 'autodelete', 'exit', 'atexit'],
)
