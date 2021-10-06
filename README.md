# deleter

[![Build](https://github.com/desty2k/deleter/actions/workflows/build.yml/badge.svg)](https://github.com/desty2k/deleter/actions/workflows/build.yml)
[![Version](https://img.shields.io/pypi/v/deleter)](https://pypi.org/project/deleter/)
[![Version](https://img.shields.io/pypi/dm/deleter)](https://pypi.org/project/deleter/)

Automatically remove python scripts from disk after execution.

## Installation

From PyPI

```shell
pip install deleter -U
```

From sources

```shell
git clone https://github.com/desty2k/deleter.git
cd deleter
pip install .
```

## Usage

```python
import deleter

deleter.register()

# your code
```
