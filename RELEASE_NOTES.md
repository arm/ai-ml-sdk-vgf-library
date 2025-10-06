# VGF Library — Release Notes

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

---
