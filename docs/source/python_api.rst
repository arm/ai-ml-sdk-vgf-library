VGF Python API Reference
========================

The Python API is provided by the ``vgfpy`` extension module. It supports
encoding VGF files and decoding their header, module, model-sequence,
model-resource, and constant sections. Array data is accepted through the
Python buffer protocol and decoded array data is returned as a ``memoryview``.

.. py:module:: vgfpy

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

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [CreateEncoder]
   :end-before: .. end-api: [CreateEncoder]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [Encoder]
   :end-before: .. end-api: [Encoder]

Encoder references
------------------

Encoder methods return reference objects that identify entries while the VGF
file is being constructed.

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ModuleRef]
   :end-before: .. end-api: [ModuleRef]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ResourceRef]
   :end-before: .. end-api: [ResourceRef]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ConstantRef]
   :end-before: .. end-api: [ConstantRef]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [GraphConstantBindingRef]
   :end-before: .. end-api: [GraphConstantBindingRef]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [BindingSlotRef]
   :end-before: .. end-api: [BindingSlotRef]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [DescriptorSetInfoRef]
   :end-before: .. end-api: [DescriptorSetInfoRef]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [SegmentInfoRef]
   :end-before: .. end-api: [SegmentInfoRef]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [PushConstRangeRef]
   :end-before: .. end-api: [PushConstRangeRef]

Header decoder
--------------

Decoder factory functions retain a reference to their input buffer. Keep the
buffer unchanged for the lifetime of the returned decoder.

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [HeaderSize]
   :end-before: .. end-api: [HeaderSize]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [HeaderDecoderSize]
   :end-before: .. end-api: [HeaderDecoderSize]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [CreateHeaderDecoder]
   :end-before: .. end-api: [CreateHeaderDecoder]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [FormatVersion]
   :end-before: .. end-api: [FormatVersion]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [HeaderDecoder]
   :end-before: .. end-api: [HeaderDecoder]

Module-table decoder
--------------------

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ModuleTableDecoderSize]
   :end-before: .. end-api: [ModuleTableDecoderSize]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [CreateModuleTableDecoder]
   :end-before: .. end-api: [CreateModuleTableDecoder]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ModuleTableDecoder]
   :end-before: .. end-api: [ModuleTableDecoder]

Model-sequence-table decoder
----------------------------

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ModelSequenceTableDecoderSize]
   :end-before: .. end-api: [ModelSequenceTableDecoderSize]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [CreateModelSequenceTableDecoder]
   :end-before: .. end-api: [CreateModelSequenceTableDecoder]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [GraphConstantBinding]
   :end-before: .. end-api: [GraphConstantBinding]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ModelSequenceTableDecoder]
   :end-before: .. end-api: [ModelSequenceTableDecoder]

The binding-slot, name, and push-constant-range handle objects are opaque.
Pass them unchanged to the corresponding methods on
:py:class:`~vgfpy.ModelSequenceTableDecoder`.

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [BindingSlotArrayHandle_s]
   :end-before: .. end-api: [BindingSlotArrayHandle_s]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [NameArrayHandle_s]
   :end-before: .. end-api: [NameArrayHandle_s]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [PushConstantRangeHandle_s]
   :end-before: .. end-api: [PushConstantRangeHandle_s]

Model-resource-table decoder
----------------------------

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ModelResourceTableDecoderSize]
   :end-before: .. end-api: [ModelResourceTableDecoderSize]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [CreateModelResourceTableDecoder]
   :end-before: .. end-api: [CreateModelResourceTableDecoder]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ModelResourceTableDecoder]
   :end-before: .. end-api: [ModelResourceTableDecoder]

Sampler configuration handles are opaque. Pass them unchanged to the sampler
configuration accessors on :py:class:`~vgfpy.ModelResourceTableDecoder`.

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [SamplerConfigHandle_s]
   :end-before: .. end-api: [SamplerConfigHandle_s]

Constant decoder
----------------

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ConstantDecoderSize]
   :end-before: .. end-api: [ConstantDecoderSize]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [CreateConstantDecoder]
   :end-before: .. end-api: [CreateConstantDecoder]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [ConstantDecoder]
   :end-before: .. end-api: [ConstantDecoder]

Common types and functions
--------------------------

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [FourCC]
   :end-before: .. end-api: [FourCC]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [FourCCValue]
   :end-before: .. end-api: [FourCCValue]

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [UndefinedFormat]
   :end-before: .. end-api: [UndefinedFormat]

Enumerations
------------

The following reference is generated from the exported Python enumerations.

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [enums]
   :end-before: .. end-api: [enums]

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

.. include:: _python_api/api.rst.inc
   :start-after: .. begin-api: [constants]
   :end-before: .. end-api: [constants]
