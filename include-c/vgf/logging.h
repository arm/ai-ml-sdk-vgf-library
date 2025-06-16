/*
 * SPDX-FileCopyrightText: Copyright 2024-2025 Arm Limited and/or its affiliates <open-source-office@arm.com>
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ML_SDK_VGF_LIB_LOGGING_API_H
#define ML_SDK_VGF_LIB_LOGGING_API_H

#ifdef __cplusplus
extern "C" {
#endif
#ifdef __GNUC__ // GCC/CLANG
#    define MLSDK_EXPORT __attribute__((visibility("default")))
#    define MLSDK_IMPORT
#else
#    ifdef _MSC_VER
#        define MLSDK_EXPORT __declspec(dllexport)
#        define MLSDK_IMPORT __declspec(dllimport)
#    else
#        define MLSDK_EXPORT
#        define MLSDK_IMPORT
#        pragma warning Undefined dynamic library export / import semantic for detected toolchain.
#    endif
#endif
#ifdef MLSDK_DYNAMIC_LIB    // Building for a dynamic lib
#    ifdef MLSDK_EXPORT_API // Building the library for export
#        define MLSDKAPI MLSDK_EXPORT
#    else
#        define MLSDKAPI MLSDK_IMPORT
#    endif
#else
#    define MLSDKAPI
#endif

/**
 * \defgroup VGFLOGGINGCAPI Logging C API
 * @{
 */

/**
 * @brief Enum for the log levels
 *
 */
typedef enum {
    mlsdk_logging_log_level_info,
    mlsdk_logging_log_level_warning,
    mlsdk_logging_log_level_debug,
    mlsdk_logging_log_level_error,
} mlsdk_logging_log_level;

/**
 * @brief Type definition for the logging callback
 *
 * @param logLevel Log level of the message
 * @param message Log message
 */
typedef void (*mlsdk_logging_callback)(mlsdk_logging_log_level logLevel, const char *message);

/**
 * @brief Enables logging functionality
 *
 * @param callback Callback that should be used for processing the log messages
 */
MLSDKAPI void mlsdk_logging_enable(mlsdk_logging_callback callback);

/**
 * @brief Disables logging functionality
 *
 */
MLSDKAPI void mlsdk_logging_disable();

/**@}*/

#ifdef __cplusplus
}
#endif

#endif
