# Building VGF lib from source

The build system must have:

- CMake 3.22 or later.
- C/C++ 17 compiler: GCC, or optionally Clang on Linux and MSVC on Windows®.
- Python 3.10 or later. Required python libraries for building are listed in
  `tooling-requirements.txt`.
- Flatbuffers flatc compiler.

The following dependencies are also needed:

- [Argument Parser for Modern C++](https://github.com/p-ranav/argparse).
- [JSON for Modern C++](https://github.com/nlohmann/json).
- [pybind11](https://github.com/pybind/pybind11).
- [GoogleTest](https://github.com/google/googletest). Optional, for testing.

For the preferred dependency versions see the manifest file.

## Providing Flatc

There are 3 options for providing the flatc binary.

1.  Installing flatc to the system. To install flatc to the system and make it
    available on the searchable `PATH`, see the
    [flatbuffers documentation](https://flatbuffers.dev/).

2.  Building flatc from repo source. When the repository is initialized using
    the repo manifest, you can find a flatbuffers source checkout in
    `<repo-root>/dependencies/flatbuffers/`. Navigate to this location and run
    the following commands:

```bash
cmake -B build && cmake --build build --parallel $(nproc)
```

```{note}
If you build into the `build` folder, the VGF-lib cmake scripts
automatically find the flatc binary.
```

3.  Providing a custom flatc path. If flatc cannot be searched using `PATH` or
    found in the default `<repo-root>/dependencies/flatbuffers/build` path, you
    can provide a custom binary file path to the build script using the
    `--flatc-path <path_to_flatc>` option, see
    [Building with the script](#building-with-the-script).

<a name="building-with-the-script"></a>

## Building with the script

To make the build configuration options easily discoverable, we provide a python
build script. When you run the script from a git-repo manifest checkout, the
script uses default paths and does not require any additional arguments. If you
do not use the script, you must specify paths to the dependencies.

To build on the current platform, for example on Linux or Windows®, run the
following command:

```bash
python3 $SDK_PATH/sw/vgf-lib/scripts/build.py -j $(nproc)
```

To cross compile for AArch64 architecture, add the following option:

```bash
python3 $SDK_PATH/sw/vgf-lib/scripts/build.py -j $(nproc) --target-platform aarch64
```

To enable and run tests, use the `--test` option. To lint the tests, use the
`--lint` option. To build the documentation, use the `--doc` option. To build
the documentation, you must have `sphinx` and `doxygen` installed on your
machine.

You can install the build artifacts for this project into a specified location.
To install the build artifacts, pass the `--install` option with the required
path.

To create an archive with the build artifacts option, you must add the
`--package` option. The archive is stored in the provided location.

For more command line options, see the help output:

```bash
python3 $SDK_PATH/sw/vgf-lib/scripts/build.py --help
```
