VGF Python API Reference
========================

The Python API is provided by the ``vgfpy`` extension module. It supports
encoding VGF files and decoding their header, module, model-sequence,
model-resource, and constant sections. Array data is accepted through the
Python buffer protocol and decoded array data is returned as a ``memoryview``.

Build and import the bindings
-----------------------------

Build the bindings from a VGF Library source checkout:

.. code-block:: shell

   python3 scripts/build.py --build-pylib

The extension is written to ``build/src`` by default. Add that directory to
``PYTHONPATH`` when running a Python application:

.. code-block:: shell

   export PYTHONPATH="$PWD/build/src${PYTHONPATH:+:$PYTHONPATH}"

Then import the module:

.. code-block:: python

   import vgfpy as vgf

Python example
--------------

Create a minimal VGF file in memory, then inspect its header and module table:

.. code-block:: python

   import io

   import vgfpy as vgf

   encoder = vgf.CreateEncoder(vkHeaderVersion=0)
   encoder.AddModule(vgf.ModuleType.Graph, "graph", "main")
   encoder.Finish()

   output = io.BytesIO()
   encoder.WriteTo(output)
   data = output.getbuffer()

   header = vgf.CreateHeaderDecoder(data, vgf.HeaderSize(), len(data))
   modules = vgf.CreateModuleTableDecoder(
       data[header.GetModuleTableOffset() :], header.GetModuleTableSize()
   )
   print(modules.getModuleName(0))

Python entry points
-------------------

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - API
     - Purpose
   * - :py:func:`~vgfpy.CreateEncoder`
     - Create an encoder for constructing a VGF file.
   * - :py:func:`~vgfpy.CreateHeaderDecoder`
     - Validate a VGF header and inspect its version and sections.
   * - :py:func:`~vgfpy.CreateModuleTableDecoder`
     - Inspect the modules stored in a VGF file.
   * - :py:func:`~vgfpy.CreateModelSequenceTableDecoder`
     - Inspect segments, bindings, and model inputs and outputs.
   * - :py:func:`~vgfpy.CreateModelResourceTableDecoder`
     - Inspect model resources, tensor metadata, and sampler configuration.
   * - :py:func:`~vgfpy.CreateConstantDecoder`
     - Inspect constant metadata and data.

.. contents:: API catalogue
   :local:
   :depth: 1

Encoder
-------

.. autofunction:: vgfpy.CreateEncoder

.. autoclass:: vgfpy.Encoder
   :members:

Encoder references
------------------

Encoder methods return reference objects that identify entries while the VGF
file is being constructed.

.. autoclass:: vgfpy.ModuleRef
   :members:

.. autoclass:: vgfpy.ResourceRef
   :members:

.. autoclass:: vgfpy.ConstantRef
   :members:

.. autoclass:: vgfpy.GraphConstantBindingRef
   :members:

.. autoclass:: vgfpy.BindingSlotRef
   :members:

.. autoclass:: vgfpy.DescriptorSetInfoRef
   :members:

.. autoclass:: vgfpy.SegmentInfoRef
   :members:

.. autoclass:: vgfpy.PushConstRangeRef
   :members:

Header decoder
--------------

Decoder factory functions retain a reference to their input buffer. Keep the
buffer unchanged for the lifetime of the returned decoder.

.. autofunction:: vgfpy.HeaderSize

.. autofunction:: vgfpy.HeaderDecoderSize

.. autofunction:: vgfpy.CreateHeaderDecoder

.. autoclass:: vgfpy.FormatVersion
   :members:

.. autoclass:: vgfpy.HeaderDecoder
   :members:

Module-table decoder
--------------------

.. autofunction:: vgfpy.ModuleTableDecoderSize

.. autofunction:: vgfpy.CreateModuleTableDecoder

.. autoclass:: vgfpy.ModuleTableDecoder
   :members:

Model-sequence-table decoder
----------------------------

.. autofunction:: vgfpy.ModelSequenceTableDecoderSize

.. autofunction:: vgfpy.CreateModelSequenceTableDecoder

.. autoclass:: vgfpy.GraphConstantBinding
   :members:

.. autoclass:: vgfpy.ModelSequenceTableDecoder
   :members:

The binding-slot, name, and push-constant-range handle objects are opaque.
Pass them unchanged to the corresponding methods on
:py:class:`~vgfpy.ModelSequenceTableDecoder`.

.. autoclass:: vgfpy.BindingSlotArrayHandle_s

.. autoclass:: vgfpy.NameArrayHandle_s

.. autoclass:: vgfpy.PushConstantRangeHandle_s

Model-resource-table decoder
----------------------------

.. autofunction:: vgfpy.ModelResourceTableDecoderSize

.. autofunction:: vgfpy.CreateModelResourceTableDecoder

.. autoclass:: vgfpy.ModelResourceTableDecoder
   :members:

Sampler configuration handles are opaque. Pass them unchanged to the sampler
configuration accessors on :py:class:`~vgfpy.ModelResourceTableDecoder`.

.. autoclass:: vgfpy.SamplerConfigHandle_s

Constant decoder
----------------

.. autofunction:: vgfpy.ConstantDecoderSize

.. autofunction:: vgfpy.CreateConstantDecoder

.. autoclass:: vgfpy.ConstantDecoder
   :members:

Common types and functions
--------------------------

.. autofunction:: vgfpy.FourCC

.. autoclass:: vgfpy.FourCCValue
   :members:

.. autofunction:: vgfpy.UndefinedFormat

Enumerations
------------

The following table is generated from the exported Python enumerations.

.. py-enum-table::

Header constants
----------------

The module exposes the following constants for inspecting the encoded header
layout. Section constants are provided for the module, model-sequence,
model-resource, and constant sections.

.. list-table::
   :header-rows: 1
   :widths: 35 65

   * - Category
     - Constants
   * - Magic and version
     - ``HEADER_MAGIC_VALUE``, ``HEADER_MAGIC_VALUE_OLD``,
       ``HEADER_MAJOR_VERSION_VALUE``, ``HEADER_MINOR_VERSION_VALUE``,
       ``HEADER_PATCH_VERSION_VALUE``
   * - Header layout
     - ``HEADER_MAGIC_OFFSET``, ``HEADER_VK_HEADER_VERSION_OFFSET``,
       ``HEADER_VERSION_OFFSET``, ``HEADER_HEADER_SIZE_VALUE``,
       ``HEADER_FIRST_SECTION_OFFSET``, ``HEADER_SECOND_SECTION_OFFSET``,
       ``HEADER_THIRD_SECTION_OFFSET``, ``HEADER_FOURTH_SECTION_OFFSET``
   * - Module section
     - ``HEADER_MODULE_SECTION_OFFSET``,
       ``HEADER_MODULE_SECTION_OFFSET_OFFSET``,
       ``HEADER_MODULE_SECTION_SIZE_OFFSET``
   * - Model-sequence section
     - ``HEADER_MODEL_SEQUENCE_SECTION_OFFSET``,
       ``HEADER_MODEL_SEQUENCE_SECTION_OFFSET_OFFSET``,
       ``HEADER_MODEL_SEQUENCE_SECTION_SIZE_OFFSET``
   * - Model-resource section
     - ``HEADER_MODEL_RESOURCE_SECTION_OFFSET``,
       ``HEADER_MODEL_RESOURCE_SECTION_OFFSET_OFFSET``,
       ``HEADER_MODEL_RESOURCE_SECTION_SIZE_OFFSET``
   * - Constant section
     - ``HEADER_CONSTANT_SECTION_OFFSET``,
       ``HEADER_CONSTANT_SECTION_OFFSET_OFFSET``,
       ``HEADER_CONSTANT_SECTION_SIZE_OFFSET``
