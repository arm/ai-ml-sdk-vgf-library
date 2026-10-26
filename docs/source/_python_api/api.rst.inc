.. Generated from the native Python API; do not edit.

.. begin-api: [Encoder]

.. py:class:: Encoder
   :module: vgfpy

   Construct and serialize a VGF file.


   .. py:method:: Encoder.AddBindingSlot(self: vgfpy.Encoder, binding: typing.SupportsInt | typing.SupportsIndex, resource: vgfpy.ResourceRef) -> vgfpy.BindingSlotRef
      :module: vgfpy

      Add a binding slot associated with a model resource.


   .. py:method:: Encoder.AddConstant(self: vgfpy.Encoder, resourceRef: vgfpy.ResourceRef, buffer: collections.abc.Buffer, sparsityDimension: typing.SupportsInt | typing.SupportsIndex = -1) -> vgfpy.ConstantRef
      :module: vgfpy

      Add buffer-protocol data for a constant model resource.


   .. py:method:: Encoder.AddConstantResource(self: vgfpy.Encoder, vkFormat: typing.SupportsInt | typing.SupportsIndex, shape: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], strides: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex]) -> vgfpy.ResourceRef
      :module: vgfpy

      Add a constant resource to the model resource table.


   .. py:method:: Encoder.AddDescriptorSetInfo(self: vgfpy.Encoder, bindings: collections.abc.Sequence[vgfpy.BindingSlotRef] = [], setIndex: typing.SupportsInt | typing.SupportsIndex = 4294967295) -> vgfpy.DescriptorSetInfoRef
      :module: vgfpy

      Add descriptor-set information for binding slots.


   .. py:method:: Encoder.AddInputResource(self: vgfpy.Encoder, vkDescriptorType: typing.SupportsInt | typing.SupportsIndex, vkFormat: typing.SupportsInt | typing.SupportsIndex, shape: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], strides: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], aliasGroupId: typing.SupportsInt | typing.SupportsIndex | None = None) -> vgfpy.ResourceRef
      :module: vgfpy

      Add an input to the model resource table.


   .. py:method:: Encoder.AddIntermediateResource(self: vgfpy.Encoder, vkDescriptorType: typing.SupportsInt | typing.SupportsIndex, vkFormat: typing.SupportsInt | typing.SupportsIndex, shape: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], strides: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], aliasGroupId: typing.SupportsInt | typing.SupportsIndex | None = None) -> vgfpy.ResourceRef
      :module: vgfpy

      Add an intermediate value to the model resource table.


   .. py:method:: Encoder.AddModelSequenceInputsOutputs(self: vgfpy.Encoder, inputs: collections.abc.Sequence[vgfpy.BindingSlotRef] = [], inputNames: collections.abc.Sequence[str] = [], outputs: collections.abc.Sequence[vgfpy.BindingSlotRef] = [], outputNames: collections.abc.Sequence[str] = []) -> None
      :module: vgfpy

      Set the model-level input and output binding slots and optional names.


   .. py:method:: Encoder.AddModule
      :module: vgfpy

      Overloaded function.

      ``1. AddModule(self: vgfpy.Encoder, type: vgfpy.ModuleType, name: str, entryPoint: str, code: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex] = []) -> vgfpy.ModuleRef``

      Add a SPIR-V module and return its reference.

      ``2. AddModule(self: vgfpy.Encoder, type: vgfpy.ModuleType, name: str, entryPoint: str, shaderType: vgfpy.ShaderType, code: str = '') -> vgfpy.ModuleRef``

      Add a source-language shader module and return its reference.


   .. py:method:: Encoder.AddOutputResource(self: vgfpy.Encoder, vkDescriptorType: typing.SupportsInt | typing.SupportsIndex, vkFormat: typing.SupportsInt | typing.SupportsIndex, shape: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], strides: collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], aliasGroupId: typing.SupportsInt | typing.SupportsIndex | None = None) -> vgfpy.ResourceRef
      :module: vgfpy

      Add an output to the model resource table.


   .. py:method:: Encoder.AddPlaceholderModule(self: vgfpy.Encoder, type: vgfpy.ModuleType, name: str, entryPoint: str) -> vgfpy.ModuleRef
      :module: vgfpy

      Deprecated: add a module without code. Use :meth:`AddModule`.


   .. py:method:: Encoder.AddPushConstRange(self: vgfpy.Encoder, stageFlags: typing.SupportsInt | typing.SupportsIndex, offset: typing.SupportsInt | typing.SupportsIndex, size: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.PushConstRangeRef
      :module: vgfpy

      Add a push-constant range to a segment.


   .. py:method:: Encoder.AddSamplerConfig(self: vgfpy.Encoder, resource: vgfpy.ResourceRef, samplerMinFilter: typing.SupportsInt | typing.SupportsIndex, samplerMagFilter: typing.SupportsInt | typing.SupportsIndex, samplerAddressModeU: typing.SupportsInt | typing.SupportsIndex, samplerAddressModeV: typing.SupportsInt | typing.SupportsIndex, samplerBorderColor: typing.SupportsInt | typing.SupportsIndex) -> None
      :module: vgfpy

      Set sampler metadata for a model resource.


   .. py:method:: Encoder.AddSegmentInfo
      :module: vgfpy

      Overloaded function.

      ``1. AddSegmentInfo(self: vgfpy.Encoder, module: vgfpy.ModuleRef, name: str, descriptors: collections.abc.Sequence[vgfpy.DescriptorSetInfoRef] = [], inputs: collections.abc.Sequence[vgfpy.BindingSlotRef] = [], outputs: collections.abc.Sequence[vgfpy.BindingSlotRef] = [], constants: collections.abc.Sequence[vgfpy.ConstantRef] = [], dispatchShape: typing.Annotated[collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], "FixedSize(3)"] = [0, 0, 0], pushConstRanges: collections.abc.Sequence[vgfpy.PushConstRangeRef] = []) -> vgfpy.SegmentInfoRef``

      Add a segment using legacy constant references.

      ``2. AddSegmentInfo(self: vgfpy.Encoder, module: vgfpy.ModuleRef, name: str, descriptors: collections.abc.Sequence[vgfpy.DescriptorSetInfoRef], inputs: collections.abc.Sequence[vgfpy.BindingSlotRef], outputs: collections.abc.Sequence[vgfpy.BindingSlotRef], constantBindings: collections.abc.Sequence[vgfpy.GraphConstantBindingRef], dispatchShape: typing.Annotated[collections.abc.Sequence[typing.SupportsInt | typing.SupportsIndex], "FixedSize(3)"] = [0, 0, 0], pushConstRanges: collections.abc.Sequence[vgfpy.PushConstRangeRef] = []) -> vgfpy.SegmentInfoRef``

      Add a segment using explicit graph constant bindings.


   .. py:method:: Encoder.Finish(self: vgfpy.Encoder) -> None
      :module: vgfpy

      Finish encoding the VGF file before writing it.


   .. py:method:: Encoder.SetAliasGroup(self: vgfpy.Encoder, resource: vgfpy.ResourceRef, aliasGroupId: typing.SupportsInt | typing.SupportsIndex) -> None
      :module: vgfpy

      Assign an alias group to a non-constant model resource.


   .. py:method:: Encoder.WriteTo(self: vgfpy.Encoder, output: object) -> bool
      :module: vgfpy

      Write the finished VGF file to a binary Python IO stream.

.. end-api: [Encoder]

.. begin-api: [ModuleRef]

.. py:class:: ModuleRef(arg0: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Reference to an encoded module.


   .. py:property:: ModuleRef.reference
      :module: vgfpy

      :type: int


      Module-table index.


      Read-only.

.. end-api: [ModuleRef]

.. begin-api: [ResourceRef]

.. py:class:: ResourceRef(arg0: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Reference to an encoded model resource.


   .. py:property:: ResourceRef.reference
      :module: vgfpy

      :type: int


      Model-resource-table index.


      Read-only.

.. end-api: [ResourceRef]

.. begin-api: [ConstantRef]

.. py:class:: ConstantRef(arg0: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Reference to encoded constant data.


   .. py:property:: ConstantRef.reference
      :module: vgfpy

      :type: int


      Constant-table index.


      Read-only.

.. end-api: [ConstantRef]

.. begin-api: [GraphConstantBindingRef]

.. py:class:: GraphConstantBindingRef(arg0: typing.SupportsInt | typing.SupportsIndex, arg1: vgfpy.ConstantRef)
   :module: vgfpy

   Map a graph constant ID to encoded constant data.


   .. py:property:: GraphConstantBindingRef.constant
      :module: vgfpy

      :type: vgfpy.ConstantRef


      Encoded constant reference.


      Read-only.


   .. py:property:: GraphConstantBindingRef.graphConstantId
      :module: vgfpy

      :type: int


      SPIR-V graph constant ID.


      Writable.

.. end-api: [GraphConstantBindingRef]

.. begin-api: [BindingSlotRef]

.. py:class:: BindingSlotRef(arg0: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Reference to an encoded binding slot.


   .. py:property:: BindingSlotRef.reference
      :module: vgfpy

      :type: int


      Binding-slot index.


      Read-only.

.. end-api: [BindingSlotRef]

.. begin-api: [DescriptorSetInfoRef]

.. py:class:: DescriptorSetInfoRef(arg0: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Reference to encoded descriptor-set information.


   .. py:property:: DescriptorSetInfoRef.reference
      :module: vgfpy

      :type: int


      Descriptor-set information index.


      Read-only.

.. end-api: [DescriptorSetInfoRef]

.. begin-api: [SegmentInfoRef]

.. py:class:: SegmentInfoRef(arg0: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Reference to an encoded model segment.


   .. py:property:: SegmentInfoRef.reference
      :module: vgfpy

      :type: int


      Model-segment index.


      Read-only.

.. end-api: [SegmentInfoRef]

.. begin-api: [PushConstRangeRef]

.. py:class:: PushConstRangeRef(arg0: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Reference to an encoded push-constant range.


   .. py:property:: PushConstRangeRef.reference
      :module: vgfpy

      :type: int


      Push-constant-range index.


      Read-only.

.. end-api: [PushConstRangeRef]

.. begin-api: [FormatVersion]

.. py:class:: FormatVersion()
   :module: vgfpy

   Semantic version stored in a VGF header.


   .. py:property:: FormatVersion.major
      :module: vgfpy

      :type: int


      Major version.


      Writable.


   .. py:property:: FormatVersion.minor
      :module: vgfpy

      :type: int


      Minor version.


      Writable.


   .. py:property:: FormatVersion.patch
      :module: vgfpy

      :type: int


      Patch version.


      Writable.

.. end-api: [FormatVersion]

.. begin-api: [HeaderDecoder]

.. py:class:: HeaderDecoder
   :module: vgfpy

   Inspect and validate a VGF header.


   .. py:method:: HeaderDecoder.CheckVersion(self: vgfpy.HeaderDecoder) -> bool
      :module: vgfpy

      Return whether the VGF format version is supported.


   .. py:method:: HeaderDecoder.GetConstantsOffset(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the constant-section byte offset.


   .. py:method:: HeaderDecoder.GetConstantsSize(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the constant-section size in bytes.


   .. py:method:: HeaderDecoder.GetEncoderVulkanHeadersVersion(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the Vulkan header version used by the encoder.


   .. py:method:: HeaderDecoder.GetMajor(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the VGF major version.


   .. py:method:: HeaderDecoder.GetMinor(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the VGF minor version.


   .. py:method:: HeaderDecoder.GetModelResourceTableOffset(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the model-resource-table byte offset.


   .. py:method:: HeaderDecoder.GetModelResourceTableSize(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the model-resource-table size in bytes.


   .. py:method:: HeaderDecoder.GetModelSequenceTableOffset(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the model-sequence-table byte offset.


   .. py:method:: HeaderDecoder.GetModelSequenceTableSize(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the model-sequence-table size in bytes.


   .. py:method:: HeaderDecoder.GetModuleTableOffset(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the module-table byte offset.


   .. py:method:: HeaderDecoder.GetModuleTableSize(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the module-table size in bytes.


   .. py:method:: HeaderDecoder.GetPatch(self: vgfpy.HeaderDecoder) -> int
      :module: vgfpy

      Return the VGF patch version.


   .. py:method:: HeaderDecoder.GetVersion(self: vgfpy.HeaderDecoder) -> vgfpy.FormatVersion
      :module: vgfpy

      Return the VGF format version.


   .. py:method:: HeaderDecoder.IsLatestVersion(self: vgfpy.HeaderDecoder) -> bool
      :module: vgfpy

      Return whether the file uses the latest VGF version.


   .. py:method:: HeaderDecoder.IsValid(self: vgfpy.HeaderDecoder) -> bool
      :module: vgfpy

      Return whether the VGF header magic value is valid.

.. end-api: [HeaderDecoder]

.. begin-api: [ModuleTableDecoder]

.. py:class:: ModuleTableDecoder
   :module: vgfpy

   Inspect modules in a VGF module table.


   .. py:method:: ModuleTableDecoder.getGLSLModuleCode(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> str
      :module: vgfpy

      Return GLSL source, or an empty string.


   .. py:method:: ModuleTableDecoder.getHLSLModuleCode(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> str
      :module: vgfpy

      Return HLSL source, or an empty string.


   .. py:method:: ModuleTableDecoder.getModuleCode(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> object
      :module: vgfpy

      Deprecated: return SPIR-V code as a memoryview. Use :meth:`getSPIRVModuleCode`.


   .. py:method:: ModuleTableDecoder.getModuleEntryPoint(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> str
      :module: vgfpy

      Return the module entry point.


   .. py:method:: ModuleTableDecoder.getModuleName(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> str
      :module: vgfpy

      Return the module name.


   .. py:method:: ModuleTableDecoder.getModuleType(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.ModuleType
      :module: vgfpy

      Return the module type.


   .. py:method:: ModuleTableDecoder.getSPIRVModuleCode(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> object
      :module: vgfpy

      Return SPIR-V code as a memoryview, or None when absent.


   .. py:method:: ModuleTableDecoder.hasGLSLCode(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> bool
      :module: vgfpy

      Return whether GLSL code is present.


   .. py:method:: ModuleTableDecoder.hasHLSLCode(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> bool
      :module: vgfpy

      Return whether HLSL code is present.


   .. py:method:: ModuleTableDecoder.hasSPIRV(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> bool
      :module: vgfpy

      Deprecated: return whether the module contains SPIR-V. Use :meth:`isSPIRV`.


   .. py:method:: ModuleTableDecoder.hasSPIRVCode(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> bool
      :module: vgfpy

      Return whether SPIR-V code is present.


   .. py:method:: ModuleTableDecoder.isGLSL(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> bool
      :module: vgfpy

      Return whether the module uses GLSL.


   .. py:method:: ModuleTableDecoder.isHLSL(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> bool
      :module: vgfpy

      Return whether the module uses HLSL.


   .. py:method:: ModuleTableDecoder.isSPIRV(self: vgfpy.ModuleTableDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> bool
      :module: vgfpy

      Return whether the module uses SPIR-V.


   .. py:method:: ModuleTableDecoder.size(self: vgfpy.ModuleTableDecoder) -> int
      :module: vgfpy

      Return the number of modules.

.. end-api: [ModuleTableDecoder]

.. begin-api: [GraphConstantBinding]

.. py:class:: GraphConstantBinding()
   :module: vgfpy

   Decoded mapping from a graph constant ID to a constant-table index.


   .. py:property:: GraphConstantBinding.constantIndex
      :module: vgfpy

      :type: int


      Constant-table index.


      Writable.


   .. py:property:: GraphConstantBinding.graphConstantId
      :module: vgfpy

      :type: int


      SPIR-V graph constant ID.


      Writable.

.. end-api: [GraphConstantBinding]

.. begin-api: [ModelSequenceTableDecoder]

.. py:class:: ModelSequenceTableDecoder
   :module: vgfpy

   Inspect segments and bindings in a VGF model-sequence table.


   .. py:method:: ModelSequenceTableDecoder.getBindingSlotBinding(self: vgfpy.ModelSequenceTableDecoder, handle: vgfpy.BindingSlotArrayHandle_s, slotIdx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the Vulkan binding number for a binding slot.


   .. py:method:: ModelSequenceTableDecoder.getBindingSlotMrtIndex(self: vgfpy.ModelSequenceTableDecoder, handle: vgfpy.BindingSlotArrayHandle_s, slotIdx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the model-resource-table index for a binding slot.


   .. py:method:: ModelSequenceTableDecoder.getBindingsSize(self: vgfpy.ModelSequenceTableDecoder, handle: vgfpy.BindingSlotArrayHandle_s) -> int
      :module: vgfpy

      Return the number of binding slots in an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getDescriptorBindingSlotsHandle(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex, descIdx: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.BindingSlotArrayHandle_s
      :module: vgfpy

      Return the binding slots for one descriptor set as an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getModelSequenceInputBindingSlotsHandle(self: vgfpy.ModelSequenceTableDecoder) -> vgfpy.BindingSlotArrayHandle_s
      :module: vgfpy

      Return model input binding slots as an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getModelSequenceInputNamesHandle(self: vgfpy.ModelSequenceTableDecoder) -> vgfpy.NameArrayHandle_s
      :module: vgfpy

      Return model input names as an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getModelSequenceOutputBindingSlotsHandle(self: vgfpy.ModelSequenceTableDecoder) -> vgfpy.BindingSlotArrayHandle_s
      :module: vgfpy

      Return model output binding slots as an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getModelSequenceOutputNamesHandle(self: vgfpy.ModelSequenceTableDecoder) -> vgfpy.NameArrayHandle_s
      :module: vgfpy

      Return model output names as an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getName(self: vgfpy.ModelSequenceTableDecoder, handle: vgfpy.NameArrayHandle_s, nameIdx: typing.SupportsInt | typing.SupportsIndex) -> str
      :module: vgfpy

      Return a name from an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getNamesSize(self: vgfpy.ModelSequenceTableDecoder, handle: vgfpy.NameArrayHandle_s) -> int
      :module: vgfpy

      Return the number of names in an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getPushConstRangeOffset(self: vgfpy.ModelSequenceTableDecoder, handle: vgfpy.PushConstantRangeHandle_s, rangeIdx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the byte offset of a push-constant range.


   .. py:method:: ModelSequenceTableDecoder.getPushConstRangeSize(self: vgfpy.ModelSequenceTableDecoder, handle: vgfpy.PushConstantRangeHandle_s, rangeIdx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the byte size of a push-constant range.


   .. py:method:: ModelSequenceTableDecoder.getPushConstRangeStageFlags(self: vgfpy.ModelSequenceTableDecoder, handle: vgfpy.PushConstantRangeHandle_s, rangeIdx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the Vulkan stage flags for a push-constant range.


   .. py:method:: ModelSequenceTableDecoder.getPushConstRangesSize(self: vgfpy.ModelSequenceTableDecoder, handle: vgfpy.PushConstantRangeHandle_s) -> int
      :module: vgfpy

      Return the number of push-constant ranges in an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getSegmentConstantBindings(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> list
      :module: vgfpy

      Return graph constant bindings for a segment.


   .. py:method:: ModelSequenceTableDecoder.getSegmentConstantIndexes(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> object
      :module: vgfpy

      Return legacy constant-table indexes as a memoryview.


   .. py:method:: ModelSequenceTableDecoder.getSegmentDescriptorSetIndex(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex, descIdx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the explicit descriptor-set index.


   .. py:method:: ModelSequenceTableDecoder.getSegmentDescriptorSetInfosSize(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the number of descriptor sets for a segment.


   .. py:method:: ModelSequenceTableDecoder.getSegmentDispatchShape(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> object
      :module: vgfpy

      Return the three-dimensional dispatch shape as a memoryview.


   .. py:method:: ModelSequenceTableDecoder.getSegmentInputBindingSlotsHandle(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.BindingSlotArrayHandle_s
      :module: vgfpy

      Return a segment's input binding slots as an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getSegmentModuleIndex(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the segment's module-table index.


   .. py:method:: ModelSequenceTableDecoder.getSegmentName(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> str
      :module: vgfpy

      Return the segment name.


   .. py:method:: ModelSequenceTableDecoder.getSegmentOutputBindingSlotsHandle(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.BindingSlotArrayHandle_s
      :module: vgfpy

      Return a segment's output binding slots as an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getSegmentPushConstRange(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.PushConstantRangeHandle_s
      :module: vgfpy

      Return a segment's push-constant ranges as an opaque handle.


   .. py:method:: ModelSequenceTableDecoder.getSegmentType(self: vgfpy.ModelSequenceTableDecoder, segmentIdx: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.ModuleType
      :module: vgfpy

      Return the segment module type.


   .. py:method:: ModelSequenceTableDecoder.modelSequenceTableSize(self: vgfpy.ModelSequenceTableDecoder) -> int
      :module: vgfpy

      Return the number of model segments.

.. end-api: [ModelSequenceTableDecoder]

.. begin-api: [BindingSlotArrayHandle_s]

.. py:class:: BindingSlotArrayHandle_s()
   :module: vgfpy

   Opaque binding-slot array handle.

.. end-api: [BindingSlotArrayHandle_s]

.. begin-api: [NameArrayHandle_s]

.. py:class:: NameArrayHandle_s()
   :module: vgfpy

   Opaque name array handle.

.. end-api: [NameArrayHandle_s]

.. begin-api: [PushConstantRangeHandle_s]

.. py:class:: PushConstantRangeHandle_s()
   :module: vgfpy

   Opaque push-constant-range array handle.

.. end-api: [PushConstantRangeHandle_s]

.. begin-api: [ModelResourceTableDecoder]

.. py:class:: ModelResourceTableDecoder
   :module: vgfpy

   Inspect resources and tensor metadata in a VGF model-resource table.


   .. py:method:: ModelResourceTableDecoder.getAliasGroupId(self: vgfpy.ModelResourceTableDecoder, id: typing.SupportsInt | typing.SupportsIndex) -> int | None
      :module: vgfpy

      Return the alias group ID, or None when absent.


   .. py:method:: ModelResourceTableDecoder.getCategory(self: vgfpy.ModelResourceTableDecoder, id: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.ResourceCategory
      :module: vgfpy

      Return the resource usage category.


   .. py:method:: ModelResourceTableDecoder.getDescriptorType(self: vgfpy.ModelResourceTableDecoder, id: typing.SupportsInt | typing.SupportsIndex) -> int | None
      :module: vgfpy

      Return the Vulkan descriptor type, or None when absent.


   .. py:method:: ModelResourceTableDecoder.getSamplerConfigAddressModeU(self: vgfpy.ModelResourceTableDecoder, handle: vgfpy.SamplerConfigHandle_s) -> int
      :module: vgfpy

      Return the Vulkan U-axis address mode.


   .. py:method:: ModelResourceTableDecoder.getSamplerConfigAddressModeV(self: vgfpy.ModelResourceTableDecoder, handle: vgfpy.SamplerConfigHandle_s) -> int
      :module: vgfpy

      Return the Vulkan V-axis address mode.


   .. py:method:: ModelResourceTableDecoder.getSamplerConfigBorderColor(self: vgfpy.ModelResourceTableDecoder, handle: vgfpy.SamplerConfigHandle_s) -> int
      :module: vgfpy

      Return the Vulkan border color value.


   .. py:method:: ModelResourceTableDecoder.getSamplerConfigHandle(self: vgfpy.ModelResourceTableDecoder, id: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.SamplerConfigHandle_s
      :module: vgfpy

      Return sampler configuration as an opaque handle, or None when absent.


   .. py:method:: ModelResourceTableDecoder.getSamplerConfigMagFilter(self: vgfpy.ModelResourceTableDecoder, handle: vgfpy.SamplerConfigHandle_s) -> int
      :module: vgfpy

      Return the Vulkan magnification filter value.


   .. py:method:: ModelResourceTableDecoder.getSamplerConfigMinFilter(self: vgfpy.ModelResourceTableDecoder, handle: vgfpy.SamplerConfigHandle_s) -> int
      :module: vgfpy

      Return the Vulkan minimum filter value.


   .. py:method:: ModelResourceTableDecoder.getTensorShape(self: vgfpy.ModelResourceTableDecoder, id: typing.SupportsInt | typing.SupportsIndex) -> object
      :module: vgfpy

      Return the tensor shape as a memoryview, or None when absent.


   .. py:method:: ModelResourceTableDecoder.getTensorStride(self: vgfpy.ModelResourceTableDecoder, id: typing.SupportsInt | typing.SupportsIndex) -> object
      :module: vgfpy

      Return the tensor strides as a memoryview, or None when absent.


   .. py:method:: ModelResourceTableDecoder.getVkFormat(self: vgfpy.ModelResourceTableDecoder, id: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the Vulkan format.


   .. py:method:: ModelResourceTableDecoder.size(self: vgfpy.ModelResourceTableDecoder) -> int
      :module: vgfpy

      Return the number of model resources.

.. end-api: [ModelResourceTableDecoder]

.. begin-api: [SamplerConfigHandle_s]

.. py:class:: SamplerConfigHandle_s
   :module: vgfpy

   Opaque sampler configuration handle.

.. end-api: [SamplerConfigHandle_s]

.. begin-api: [ConstantDecoder]

.. py:class:: ConstantDecoder
   :module: vgfpy

   Inspect constant metadata and data in a VGF constant section.


   .. py:method:: ConstantDecoder.getConstant(self: vgfpy.ConstantDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> object
      :module: vgfpy

      Return constant bytes as a memoryview, or None when absent.


   .. py:method:: ConstantDecoder.getConstantMrtIndex(self: vgfpy.ConstantDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the model-resource-table index associated with a constant.


   .. py:method:: ConstantDecoder.getConstantSparsityDimension(self: vgfpy.ConstantDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> int
      :module: vgfpy

      Return the sparse dimension, or -1 for a non-sparse constant.


   .. py:method:: ConstantDecoder.isSparseConstant(self: vgfpy.ConstantDecoder, idx: typing.SupportsInt | typing.SupportsIndex) -> bool
      :module: vgfpy

      Return whether a constant is sparse.


   .. py:method:: ConstantDecoder.size(self: vgfpy.ConstantDecoder) -> int
      :module: vgfpy

      Return the number of constants.

.. end-api: [ConstantDecoder]

.. begin-api: [FourCCValue]

.. py:class:: FourCCValue(arg0: str, arg1: str, arg2: str, arg3: str)
   :module: vgfpy

   A four-character code value.


   .. py:property:: FourCCValue.a
      :module: vgfpy

      :type: str


      First character.


      Writable.


   .. py:property:: FourCCValue.b
      :module: vgfpy

      :type: str


      Second character.


      Writable.


   .. py:property:: FourCCValue.c
      :module: vgfpy

      :type: str


      Third character.


      Writable.


   .. py:property:: FourCCValue.d
      :module: vgfpy

      :type: str


      Fourth character.


      Writable.

.. end-api: [FourCCValue]

.. begin-api: [CreateEncoder]

.. py:function:: CreateEncoder(vkHeaderVersion: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.Encoder
   :module: vgfpy

   Create an encoder using the supplied Vulkan header version.

.. end-api: [CreateEncoder]

.. begin-api: [HeaderSize]

.. py:function:: HeaderSize() -> int
   :module: vgfpy

   Return the encoded VGF header size in bytes.

.. end-api: [HeaderSize]

.. begin-api: [HeaderDecoderSize]

.. py:function:: HeaderDecoderSize() -> int
   :module: vgfpy

   Return the native header decoder size in bytes.

.. end-api: [HeaderDecoderSize]

.. begin-api: [CreateHeaderDecoder]

.. py:function:: CreateHeaderDecoder(data: collections.abc.Buffer, headerSize: typing.SupportsInt | typing.SupportsIndex, fileSize: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.HeaderDecoder
   :module: vgfpy

   Create a header decoder over a buffer containing VGF file data.

.. end-api: [CreateHeaderDecoder]

.. begin-api: [ModuleTableDecoderSize]

.. py:function:: ModuleTableDecoderSize() -> int
   :module: vgfpy

   Return the native module-table decoder size in bytes.

.. end-api: [ModuleTableDecoderSize]

.. begin-api: [CreateModuleTableDecoder]

.. py:function:: CreateModuleTableDecoder(data: collections.abc.Buffer, size: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.ModuleTableDecoder
   :module: vgfpy

   Create a module-table decoder over an encoded section.

.. end-api: [CreateModuleTableDecoder]

.. begin-api: [ModelSequenceTableDecoderSize]

.. py:function:: ModelSequenceTableDecoderSize() -> int
   :module: vgfpy

   Return the native model-sequence-table decoder size in bytes.

.. end-api: [ModelSequenceTableDecoderSize]

.. begin-api: [CreateModelSequenceTableDecoder]

.. py:function:: CreateModelSequenceTableDecoder(data: collections.abc.Buffer, size: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.ModelSequenceTableDecoder
   :module: vgfpy

   Create a model-sequence-table decoder over an encoded section.

.. end-api: [CreateModelSequenceTableDecoder]

.. begin-api: [ModelResourceTableDecoderSize]

.. py:function:: ModelResourceTableDecoderSize() -> int
   :module: vgfpy

   Return the native model-resource-table decoder size in bytes.

.. end-api: [ModelResourceTableDecoderSize]

.. begin-api: [CreateModelResourceTableDecoder]

.. py:function:: CreateModelResourceTableDecoder(data: collections.abc.Buffer, size: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.ModelResourceTableDecoder
   :module: vgfpy

   Create a model-resource-table decoder over an encoded section.

.. end-api: [CreateModelResourceTableDecoder]

.. begin-api: [ConstantDecoderSize]

.. py:function:: ConstantDecoderSize() -> int
   :module: vgfpy

   Return the native constant decoder size in bytes.

.. end-api: [ConstantDecoderSize]

.. begin-api: [CreateConstantDecoder]

.. py:function:: CreateConstantDecoder(data: collections.abc.Buffer, size: typing.SupportsInt | typing.SupportsIndex) -> vgfpy.ConstantDecoder
   :module: vgfpy

   Create a constant decoder over an encoded section.

.. end-api: [CreateConstantDecoder]

.. begin-api: [FourCC]

.. py:function:: FourCC(a: str, b: str, c: str, d: str) -> vgfpy.FourCCValue
   :module: vgfpy

   Create a four-character code value.

.. end-api: [FourCC]

.. begin-api: [UndefinedFormat]

.. py:function:: UndefinedFormat() -> int
   :module: vgfpy

   Return the value representing an undefined Vulkan format.

.. end-api: [UndefinedFormat]

.. begin-api: [enums]

.. py:class:: ModuleType(value: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Type of module stored in a VGF file.

   Members:

     Compute

     Graph


   .. py:attribute:: ModuleType.Compute
      :module: vgfpy
      :value: <ModuleType.Compute: 0>


   .. py:attribute:: ModuleType.Graph
      :module: vgfpy
      :value: <ModuleType.Graph: 1>


   .. py:property:: ModuleType.name
      :module: vgfpy

      :type: str



      Read-only.


   .. py:property:: ModuleType.value
      :module: vgfpy

      :type: int



      Read-only.


.. py:class:: ShaderType(value: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Source language used by a shader module.

   Members:

     Glsl

     Hlsl


   .. py:attribute:: ShaderType.Glsl
      :module: vgfpy
      :value: <ShaderType.Glsl: 0>


   .. py:attribute:: ShaderType.Hlsl
      :module: vgfpy
      :value: <ShaderType.Hlsl: 1>


   .. py:property:: ShaderType.name
      :module: vgfpy

      :type: str



      Read-only.


   .. py:property:: ShaderType.value
      :module: vgfpy

      :type: int



      Read-only.


.. py:class:: ResourceCategory(value: typing.SupportsInt | typing.SupportsIndex)
   :module: vgfpy

   Usage category of a model resource.

   Members:

     Input

     Output

     Intermediate

     Constant


   .. py:attribute:: ResourceCategory.Constant
      :module: vgfpy
      :value: <ResourceCategory.Constant: 3>


   .. py:attribute:: ResourceCategory.Input
      :module: vgfpy
      :value: <ResourceCategory.Input: 0>


   .. py:attribute:: ResourceCategory.Intermediate
      :module: vgfpy
      :value: <ResourceCategory.Intermediate: 2>


   .. py:attribute:: ResourceCategory.Output
      :module: vgfpy
      :value: <ResourceCategory.Output: 1>


   .. py:property:: ResourceCategory.name
      :module: vgfpy

      :type: str



      Read-only.


   .. py:property:: ResourceCategory.value
      :module: vgfpy

      :type: int



      Read-only.

.. end-api: [enums]

.. begin-api: [constants]

.. py:data:: HEADER_MAGIC_VALUE_OLD
   :module: vgfpy
   :annotation: = 4041331395


.. py:data:: HEADER_MAGIC_VALUE
   :module: vgfpy
   :annotation: = vgfpy.FourCCValue('V', 'G', 'F', '1')


.. py:data:: HEADER_MAGIC_OFFSET
   :module: vgfpy
   :annotation: = 0


.. py:data:: HEADER_VK_HEADER_VERSION_OFFSET
   :module: vgfpy
   :annotation: = 4


.. py:data:: HEADER_VERSION_OFFSET
   :module: vgfpy
   :annotation: = 8


.. py:data:: HEADER_HEADER_SIZE_VALUE
   :module: vgfpy
   :annotation: = 128


.. py:data:: HEADER_FIRST_SECTION_OFFSET
   :module: vgfpy
   :annotation: = 16


.. py:data:: HEADER_SECOND_SECTION_OFFSET
   :module: vgfpy
   :annotation: = 32


.. py:data:: HEADER_THIRD_SECTION_OFFSET
   :module: vgfpy
   :annotation: = 48


.. py:data:: HEADER_FOURTH_SECTION_OFFSET
   :module: vgfpy
   :annotation: = 64


.. py:data:: HEADER_MODULE_SECTION_OFFSET
   :module: vgfpy
   :annotation: = 16


.. py:data:: HEADER_MODULE_SECTION_OFFSET_OFFSET
   :module: vgfpy
   :annotation: = 16


.. py:data:: HEADER_MODULE_SECTION_SIZE_OFFSET
   :module: vgfpy
   :annotation: = 24


.. py:data:: HEADER_MODEL_SEQUENCE_SECTION_OFFSET
   :module: vgfpy
   :annotation: = 32


.. py:data:: HEADER_MODEL_SEQUENCE_SECTION_OFFSET_OFFSET
   :module: vgfpy
   :annotation: = 32


.. py:data:: HEADER_MODEL_SEQUENCE_SECTION_SIZE_OFFSET
   :module: vgfpy
   :annotation: = 40


.. py:data:: HEADER_MODEL_RESOURCE_SECTION_OFFSET
   :module: vgfpy
   :annotation: = 48


.. py:data:: HEADER_MODEL_RESOURCE_SECTION_OFFSET_OFFSET
   :module: vgfpy
   :annotation: = 48


.. py:data:: HEADER_MODEL_RESOURCE_SECTION_SIZE_OFFSET
   :module: vgfpy
   :annotation: = 56


.. py:data:: HEADER_CONSTANT_SECTION_OFFSET
   :module: vgfpy
   :annotation: = 64


.. py:data:: HEADER_CONSTANT_SECTION_OFFSET_OFFSET
   :module: vgfpy
   :annotation: = 64


.. py:data:: HEADER_CONSTANT_SECTION_SIZE_OFFSET
   :module: vgfpy
   :annotation: = 72


.. py:data:: HEADER_MAJOR_VERSION_VALUE
   :module: vgfpy
   :annotation: = 0


.. py:data:: HEADER_MINOR_VERSION_VALUE
   :module: vgfpy
   :annotation: = 5


.. py:data:: HEADER_PATCH_VERSION_VALUE
   :module: vgfpy
   :annotation: = 0

.. end-api: [constants]
