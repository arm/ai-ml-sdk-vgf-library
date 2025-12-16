# VGF Library — Release Notes

---

## Version 0.8.0 – *Updater & Tooling Update*

### Highlights

- Added the `vgf_updater` CLI (and accompanying documentations) to upgrade outdated
  VGF payloads safely, with fixes for uncaught exceptions when conversions
  fail.
- Decoder APIs across C++, C, and Python can now report whether an asset
  already matches the latest format revision so build systems can gate
  conversions.
- Expanded logging coverage plus parser/DataView cleanups improve diagnostics
  without impacting the runtime footprint.

### Build, Packaging & Developer Experience

- Modernized the pip package: switched to `pyproject.toml`, added the missing
  metadata, and fixed package naming/installation ordering issues that affected
  `--install`.
- Defaulted the build system to Ninja, refined the CMake packaging flow.
- Introduced `clang-tidy` configuration and streamlined cppcheck
  invocation/CLI integration (including build-script driven execution).

### Platform & Compliance

- Added Darwin targets for AArch64 to the pip packaging matrix.
- Refreshed SBOM data and adopted usage of `REUSE.toml`.

### Supported Platforms

The following platform combinations are supported:

- Linux - AArch64 and x86-64
- Windows® - x86-64
- Darwin - AArch64 (experimental)
- Android™ - AArch64 (experimental)

---

## Version 0.7.0 – *Initial Public Release*

## Purpose

A library for encoding/decoding **VGF files** that package SPIR-V modules,
constants, shaders and metadata.

## Features

### Core APIs

- **C++ Encoder API**: High-level API designed for easy integration into offline
  tooling for building VGF files
- **C++ Decoder API**: Lightweight, high-performance decoder designed for
  integration into performance-critical applications like game engines
- **C Decoder API**: C-compatible decoder interface for broader language
  compatibility
- **Python Bindings**: Python API support

### Memory Management & Performance

- **Zero Dynamic Allocation**: Decoder does not allocate any dynamic memory
  internally - you maintain full control by providing required memory
- **Memory-Mapped File Support**: Format supports mmap read operations to
  minimize copies and reduce peak memory requirements
- **Lightweight Design**: Optimized for high-performing programs with minimal
  overhead

### Tooling & Utilities

- **VGF Dump Tool (`vgf_dump`)**:
  - Converts VGF file contents to human-readable JSON format
  - Generates scenario file templates for use with the Scenario Runner tool

### File Format Support

The file format is versioned; the project aims to maintain backward compatibility with older VGF folder versions. File formats:

- **Version 0.4.0**: Added large model file support
- **Version 0.3.0**: Initial version

## Platform Support

The following platform combinations are supported:

- Linux - X86-64
- Windows® - X86-64

---
