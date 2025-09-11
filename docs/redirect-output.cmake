#
# SPDX-FileCopyrightText: Copyright 2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
# SPDX-License-Identifier: Apache-2.0
#
execute_process(
  COMMAND "${cmd}" ${args}
  WORKING_DIRECTORY ${wd}
  OUTPUT_VARIABLE stdout
  OUTPUT_STRIP_TRAILING_WHITESPACE
  COMMAND_ERROR_IS_FATAL ANY
)

file(WRITE ${out} ${stdout})
