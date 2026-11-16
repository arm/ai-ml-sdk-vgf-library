VGF Library Architecture
========================

VGF Library packages the code, resource descriptions, execution order, and
constants needed to deploy a workload as a VGF file. It provides an encoder for
creating files and decoders for tools and runtime consumers. The library
describes the workload; the consumer implements its execution.

Responsibilities and boundaries
-------------------------------

A VGF file contains an indexed module table, an ordered sequence of segments,
a model resource table, and constants. Segment metadata connects a module to
descriptor bindings, input and output resources, graph constants, dispatch
dimensions, and push constant ranges. These records let a consumer reconstruct
the relationships needed to prepare and execute the workload. The
:doc:`file_format` specifies the serialized representation.

Modules are classified as graph or compute and can store SPIR-V™ binaries,
GLSL source, or HLSL source. Modules can also omit their code so that a consumer
can supply it later. VGF Library stores these representations without compiling
source shaders or executing modules.

A consumer that executes source modules must arrange their compilation.
Scenario Runner and Workload Library support source modules for compute
workloads when the corresponding compilation support is enabled. Applications
using the decoder directly provide their own compilation integration.

Model Converter uses the encoder to produce VGF files. Scenario Runner and
Workload Library decode VGF files through their separate runtime implementations;
an application can also use the decoder directly. The runtime consumer interprets
the decoded metadata, creates the Vulkan® objects, supplies input data, allocates
and binds resources, and records and submits execution commands. VGF Library does
not create a Vulkan® device or execute Vulkan® commands. Storing Vulkan® enum
values and dispatch metadata does not introduce a dependency on the Vulkan®
runtime.

Creating and consuming a VGF file
---------------------------------

Producing an asset
~~~~~~~~~~~~~~~~~~

A producer starts with module implementations or placeholders, resource
descriptions and constant payloads, then connects them into a workload. The
independent declarations can be made in different valid orders; references must
exist before calls that use them. The encoder copies constant bytes supplied to
``AddConstant``. Bindings, ordered segments and model inputs/outputs describe how
the declared records relate; they do not execute the workload.

.. uml:: puml/vgf-producer-workflow.puml
   :align: center
   :caption: Build and connect records, finalize metadata, then write a VGF asset.
   :alt: A producer creates an encoder, declares modules, resources and copied constants in a valid order, connects metadata and ordered segments, calls Finish, then WriteTo to produce a VGF artifact.

Call ``Finish`` after completing the metadata, then ``WriteTo`` with the output
stream. Check its result and handle stream errors. Writing consumes the encoder's
constant buffers; do not rely on repeated writes or retrying a failed write with
the same encoder. See :doc:`encoder_api` for the detailed construction sequence
and :doc:`encoder_c_api` or :doc:`python_api` for language-specific interfaces.

Consuming an asset
~~~~~~~~~~~~~~~~~~

A consumer owns, reads or maps the encoded bytes and selects the sections needed
for its task. Header creation checks compatibility, declared section bounds and
applicable alignment; it does not validate all section contents or relationships.
Create the required section decoders and check the references and execution
requirements needed by the consumer before using the records.

.. uml:: puml/vgf-consumer-workflow.puml
   :align: center
   :caption: Checked decoding leads to inspection or an explicit runtime handoff.
   :alt: A consumer retains VGF backing, creates a header decoder, decodes required sections and checks references, then chooses inspection and extraction or hands data to a runtime or application that records Vulkan® work.

If a decoder cannot be created or a required check fails, the caller handles the
error and stops that route. The inspection and execution choices above apply to
successfully decoded and checked input; they are not automatic error fallbacks.
A runtime consumer must resolve missing module implementations, compile supported
textual code when needed, and manage Vulkan® resources, recording and execution.
The VGF core only returns representation and borrowed views.

Keep encoded backing alive and unchanged through the last decoder or borrowed-view
use, including raw Python views that escape a decoder's lifetime. Retaining the
backing independently can suffice after the decoder is released; copy bytes when
independent ownership is needed. See :doc:`decoder_api`, :doc:`decoder_c_api` and
:doc:`python_api` for validation, storage and lifetime details.

Worked record relationships
---------------------------

This synthetic VGF asset illustrates independent identifier spaces. It is not
an example of exact Model Converter output. Two ordered GRAPH segments, A and B,
reuse module-table entry 0, with compatible input/output resource descriptions.

.. uml:: puml/vgf-record-example.puml
   :align: center
   :caption: Two segment uses share a module and a graph constant.
   :alt: Ordered GRAPH segments A and B both reference module 0. A reads MRT 0 and writes MRT 1; B reads MRT 1 and writes MRT 2. Both bind graph constant ID 5 to constant-table entry 0, whose resource is MRT 3.

.. list-table:: Example references
   :header-rows: 1
   :widths: 25 75

   * - Record
     - References
   * - Model interface
     - Input binding 40 → MRT 0; output binding 90 → MRT 2
   * - Segment A (module 0)
     - Input binding 40 → MRT 0; output binding 90 → MRT 1
   * - Segment B (module 0)
     - Input binding 40 → MRT 1; output binding 90 → MRT 2
   * - A descriptors
     - Set 2: binding 7 → MRT 0; binding 9 → MRT 1
   * - B descriptors
     - Set 2: binding 7 → MRT 1; binding 9 → MRT 2
   * - Both graph constant bindings
     - Graph constant ID 5 → constant-table entry 0 → MRT 3

Binding numbers 40 and 90 identify logical interfaces. Descriptor locations
(set 2, bindings 7 and 9) identify shader-visible locations; neither is an MRT
index. Reusing logical numbers across segments does not make their resources
identical: A's output and B's input share MRT 1 explicitly. Module index 0
identifies the shared implementation, not a position in the execution sequence.

Constant-table entry 0 owns the encoded payload metadata and refers to constant
resource MRT 3. Each segment explicitly maps graph constant ID 5 to that same
constant entry; graph constant IDs are not constant-table or MRT indices.
The encoder's explicit ``GraphConstantBindingRef`` form expresses this mapping.
The consumer prepares these resources and synchronizes A's output before B reads
it; the record relationships do not execute commands or insert barriers.

Public interfaces and implementation
------------------------------------

The C++ implementation is shared by the C and Python interfaces. The diagram
shows the library's API calls and serialization dependencies.

.. uml:: puml/vgf-components.puml
   :align: center
   :caption: VGF Library interfaces and serialization core.
   :alt: C and Python interfaces call the C++ encoder and decoders, which use FlatBuffers metadata and fixed header and constant layouts.

The C++ API lives in the ``mlsdk::vgflib`` namespace. Its encoder constructs
records and returns references used to connect them. ``Finish`` finalizes the
metadata tables, and ``WriteTo`` writes the complete VGF file to a stream. Decoding
is split into a header decoder and separate decoders for the module, model
sequence, model resource, and constant sections. The header exposes the section
locations so the caller can pass each section's bytes to its decoder. C++ factory
functions support both allocated decoder objects and construction in
caller-provided storage. See :doc:`encoder_api` and :doc:`decoder_api` for the
construction sequence and API contracts.

The C API wraps the same encoder and decoder implementation. The encoder uses an
opaque handle that owns a C++ encoder, while the decoder API exposes opaque
decoder pointers constructed in caller-provided storage. These wrappers translate
C records and references into the C++ operations; they are part of the ``vgf``
library rather than a second serialization implementation. See
:doc:`encoder_c_api`, :doc:`decoder_c_api`, :doc:`logging_api`, and
:doc:`logging_c_api` for the interface details.

The optional ``vgfpy`` extension uses pybind11 to expose the C++ encoder,
decoders, and supporting types to Python. It links to ``vgf`` directly. Python
buffer objects provide input bytes, and decoded array data is exposed through
memory views. The :doc:`python_api` documents the Python interface and its
buffer-lifetime requirements.

Serialization core
~~~~~~~~~~~~~~~~~~

The module, model sequence, and model resource tables use FlatBuffers. Their
shared schema is ``schema/vgf.fbs``; the encoder and decoders use its generated
C++ definitions internally. The public APIs expose library types and references,
so callers do not need to build or navigate FlatBuffers tables themselves.

The file header and current constants section use dedicated binary layouts
defined in ``src/header.hpp`` and ``src/constant.hpp``. Constants are written as
metadata records and raw bytes, separately from the three FlatBuffers tables.
The constant decoder also accepts the older FlatBuffers constant representation.
This division keeps the container layout and section interpretation inside the
core while allowing producers and consumers to use the same public interfaces.
See :doc:`file_format` for layout, alignment, and compatibility details.

Tools and shared utilities
~~~~~~~~~~~~~~~~~~~~~~~~~~

VGF Library provides two command-line tools: VGF Dump (``vgf_dump``) for
inspection and extraction, and VGF Updater (``vgf_updater``) for format updates.
Both tools use ``vgf-utils`` for file handling, memory mapping, and assembling
decoded records into a workload representation. The utilities library also
provides NumPy file I/O and temporary-folder helpers. It is installed and
exported for reuse by other SDK components, including Scenario Runner and
Workload Library. These helpers sit outside the encoder and decoder core.

VGF Dump
^^^^^^^^

``vgf_dump`` uses the decoders to inspect a VGF file, extract module or constant
data, and generate a Scenario Runner JSON template. To explore execution,
review and edit the generated template, supply its referenced resource, shader or
binary files as needed, then run it with Scenario Runner. Dump itself does not
execute the scenario.

.. uml:: puml/vgf-dump-components.puml
   :align: center
   :caption: VGF Dump and shared VGF libraries.
   :alt: VGF Dump uses vgf-utils for file handling and decoded records, and VGF Library for decoding.

See :doc:`vgf_dump` for usage and supported options.

VGF Updater
^^^^^^^^^^^

``vgf_updater`` decodes a supported older VGF file and re-encodes its supported
records into a new output file in the current format. If the input is already at
the exact latest version, it exits without writing output.

.. uml:: puml/vgf-updater-components.puml
   :align: center
   :caption: VGF Updater and shared VGF libraries.
   :alt: VGF Updater uses vgf-utils for file handling and decoded records, and VGF Library for decoding and encoding.

See :doc:`vgf_updater` for usage and supported options.
