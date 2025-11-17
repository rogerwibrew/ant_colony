# Ant Colony Optimization - TSP Solver

A C++ implementation of Ant Colony Optimization algorithm for solving the Travelling Salesman Problem.

## Project Structure

```
ant_colony/
├── include/          # Header files (.h)
├── src/             # Source files (.cpp)
├── tests/           # Google Test files
├── CMakeLists.txt   # CMake configuration
└── Claude.md        # Class specifications and design
```

## Build Instructions

### Prerequisites
- CMake 3.14 or higher
- C++17 compatible compiler (GCC, Clang, or MSVC)
- Internet connection (for downloading googletest)

### Building the Project

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build
cmake --build .

# Or using make directly
make
```

The `compile_commands.json` file will be automatically generated in the build directory for clang tooling (clangd, clang-tidy, etc.).

### Linking compile_commands.json

For better IDE/editor support, create a symlink in the project root:

```bash
ln -s build/compile_commands.json compile_commands.json
```

## Running

```bash
# Run the main program
./build/bin/ant_colony_tsp

# Run tests
./build/bin/ant_colony_tests

# Or using CTest
cd build
ctest --output-on-failure
```

## Testing

This project uses Google Test for unit testing. Tests are located in the `tests/` directory.

To run tests:
```bash
cd build
ctest --verbose
```

## Development

- Place header files in `include/`
- Place implementation files in `src/`
- Place test files in `tests/`
- Update `Claude.md` with class specifications

## License

[Add your license here]
