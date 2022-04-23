import pybind11, sys
from os import path
from glob import glob
from setuptools import setup
from pybind11.setup_helpers import Pybind11Extension, build_ext, ParallelCompile, naive_recompile

__version__ = "0.0.2"

ParallelCompile("NPY_NUM_BUILD_JOBS", needs_recompile=naive_recompile).install()

# read the contents of your README file
this_directory = path.abspath(path.dirname(__file__))
with open(path.join(this_directory, 'README.md'), encoding='utf-8') as f:
    long_description = f.read()
        
ext_modules = [
    Pybind11Extension(
        'external_proc._external_proc',
        sorted(glob("src/external_proc/*.cpp")),
        define_macros = [('VERSION_INFO', __version__)],
        include_dirs=[pybind11.get_include()],
        language='c++',
        extra_compile_args=['/std:c++17'],
        extra_link_args=['imagehlp.lib', 'Advapi32.lib']
    ),
]

setup(
    name='external_proc',  # имя библиотеки собранной pybind11
    version=__version__,
    author='bananasss00',
    author_email='bananasss00@yandex.com',
    description='pybind11 extension',
    long_description=long_description,
    long_description_content_type='text/markdown',
    ext_modules=ext_modules,
    requires=['pybind11'],  # Указываем зависимость от pybind11
    # package_dir = {'': 'lib'},
    packages=['external_proc'],
    python_requires='>=3.6,<4.0',
    classifiers=[
        'Operating System :: Microsoft :: Windows :: Windows 10',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10'
    ],
    package_data={ 
        'external_proc': ["__init__.py", "_external_proc.pyi", "external_proc.py", "get_proc_x32.exe", "get_proc_x64.exe", ],
    },
    # cmdclass={"build_ext": build_ext}, # Pybind11Extension::extra_compile_args=['/std:c++17'],
    zip_safe=False,
)