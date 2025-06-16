# ML SDK VGF Lib

The VGF Lib is a library that encodes and decodes VGF files. The VGF format
covers the entire model use-cases. These use-cases can be a combination of
SPIR-V™ module binary data, their constants, and compute shaders. The library
provides a suite of APIs and tools for working with VGF files. The library
includes the following:

- C++ encoder API
- C++ decoder API
- C decoder API
- VGF dumping tool: `vgf_dump`

The encoder provides a simple, high-level API for building up VGF files. It is
designed for easy integration into offline tooling.

The decoder is designed to be lightweight and to be included in high-performing
programs, for example game engines. Therefore, the decoder does not allocate any
dynamic memory on its own. To maintain full control, you must allocate the
memory required and provide it to this library. To minimize copies and peak
memory requirements, the format also supports mmap read operations on the
contents.

VGF lib also provides a dumping tool which is called vgf_dump. vgf_dump prints
the content of a VGF file into human-readable JSON format. You can also use
vgf_dump to generate scenario file templates. You can use the scenario file
templates with the scenario runner tool.

## How to build

[Build](BUILD.md)

## License

[Apache-2.0](LICENSES/Apache-2.0.txt)

## Security

If you have found a security issue, see the [Security Section](SECURITY.md)

## Trademark notice

Arm® is a registered trademarks of Arm Limited (or its subsidiaries) in the US
and/or elsewhere.

Khronos®, Vulkan® and SPIR-V™ are registered trademarks of the
[Khronos® Group](https://www.khronos.org/legal/trademarks).
