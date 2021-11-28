# Changelog

- 0.1.1:
  - Add info abount cancelling deletion to readme
  - Fix bandit and radon checks
  - Bump development status to beta

- 0.1.0:
  - Add NT support to `SubprocessMethod`
  - Do not show console window when running shell commands
  - Run tests on 3 platforms and on 2 Python versions
  - Change delete methods order
  - Use `shlex.split` to split commands into lists of strings
  - Remove `shell` and `capture_output` kwargs from all subprocess calls
  - Run Python with `-m` argument in version test
  - Update PyPI tags
  - Change development status to alpha

- 0.0.3:
  - Export `run` function
  - Make `DeleteMethod` subclass `ABC`, mark `run` as abstract method
  - Add `is_compatible` methods
  - Add missing init to goto method
  - Pass script path in constructor, not as `run` argument
  - Remove `is_platform_compatible` method

- 0.0.2:
  - Add badegs to readme
  - Add tests
  - Update build workflow

- 0.0.1:
  - Create repo
  - Add first working version
  - Add deploy workflows
