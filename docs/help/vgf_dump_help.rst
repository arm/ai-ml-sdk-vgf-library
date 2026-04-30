Usage: ./vgf_dump [--help] [--version] --input VAR --output VAR
                  [[--dump-spirv VAR]|[--dump-glsl VAR]|[--dump-hlsl VAR]|
                  [--dump-constant VAR]|[--scenario-template]]
                  [--scenario-template-add-boundaries]

Optional arguments:
  -h, --help                          shows help message and exits
  -v, --version                       prints version information and exits
  -i, --input                         The VGF input file [required]
  -o, --output                        The output file [nargs=0..1] [default: "-"]
  --dump-spirv                        Dump the SPIR-V™ module code at the given index
  --dump-glsl                         Dump the GLSL module code at the given index
  --dump-hlsl                         Dump the HLSL module code at the given index
  --dump-constant                     Dump the constant at the given index. Outputs NumPy file if output file is .npy, otherwise dumps as raw binary.
  --scenario-template                 Create a scenario template based on the VGF
  --scenario-template-add-boundaries  If creating a scenario template, add frame boundaries before and after
