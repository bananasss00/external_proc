CD /D "%~dp0"
poetry run python setup.py build --build-lib=.
REM --debug
pause