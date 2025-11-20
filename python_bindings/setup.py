"""
Setup script for building the ACO Solver Python bindings using pybind11.

Usage:
    pip install pybind11
    python setup.py build_ext --inplace
"""

from setuptools import setup, Extension
from pybind11.setup_helpers import Pybind11Extension, build_ext
import sys

# Get C++ source files (all implementation files except main.cpp)
cpp_sources = [
    'bindings.cpp',
    '../src/City.cpp',
    '../src/Graph.cpp',
    '../src/Tour.cpp',
    '../src/TSPLoader.cpp',
    '../src/PheromoneMatrix.cpp',
    '../src/Ant.cpp',
    '../src/AntColony.cpp',
]

# Compiler flags
extra_compile_args = ['-O3']
if sys.platform == 'darwin':  # macOS
    extra_compile_args.append('-std=c++17')
    extra_compile_args.append('-stdlib=libc++')
elif sys.platform == 'linux':  # Linux
    extra_compile_args.append('-std=c++17')

# Define the extension module
ext_modules = [
    Pybind11Extension(
        "aco_solver",
        cpp_sources,
        include_dirs=['../include'],
        cxx_std=17,
        extra_compile_args=extra_compile_args,
        define_macros=[('VERSION_INFO', '1.0.0')],
    ),
]

setup(
    name="aco_solver",
    version="1.0.0",
    author="ACO TSP Solver Team",
    description="Ant Colony Optimization TSP Solver - Python Bindings",
    long_description="""
        Python bindings for a high-performance C++ Ant Colony Optimization
        solver for the Travelling Salesman Problem. Supports TSPLIB format
        and provides real-time progress callbacks for visualization.
    """,
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.7",
    install_requires=[
        "pybind11>=2.6.0",
    ],
)
