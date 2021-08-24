import os
from os import listdir
from os.path import join, isdir


def poetry_env_use(exe):
    os.system(f'poetry env use {exe}')
def poetry_install():
    os.system(f'poetry install')
def poetry_update():
    os.system(f'poetry update')
def poetry_run(cmd):
    os.system(f'poetry run {cmd}')


PYTHON_VERSIONS = r"c:\pythons\.pyenv\pyenv-win\versions"
VERSIONS = [f for f in listdir(PYTHON_VERSIONS) if isdir(join(PYTHON_VERSIONS, f))]
VERSIONS.sort(key=lambda x: 'win32' in x)  # x64 versions first
# VERSIONS = [v for v in VERSIONS if 'win32' in v]  # only win32 versions

os.system('chcp 65001')
for v in VERSIONS:
    p_exe = f'{PYTHON_VERSIONS}/{v}/python.exe'
    print(f'=== TEST PYTHON {v} ===')
    os.system(f'{p_exe} main.py')




