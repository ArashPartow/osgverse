/**
 * Copyright (c) 2021-2022 Floyd M. Chitalu.
 * All rights reserved.
 *
 * NOTE: This file is licensed under GPL-3.0-or-later (default).
 * A commercial license can be purchased from Floyd M. Chitalu.
 *
 * License details:
 *
 * (A)  GNU General Public License ("GPL"); a copy of which you should have
 *      recieved with this file.
 * 	    - see also: <http://www.gnu.org/licenses/>
 * (B)  Commercial license.
 *      - email: floyd.m.chitalu@gmail.com
 *
 * The commercial license options is for users that wish to use MCUT in
 * their products for comercial purposes but do not wish to release their
 * software products under the GPL license.
 *
 * Author(s)     : Floyd M. Chitalu
 */

#include "mcut/mcut.h"

#include "mcut/internal/utils.h"

#include "mcut/internal/frontend.h"

#include <exception>
#include <stdexcept>

#if defined(MCUT_BUILD_WINDOWS)
#pragma warning(disable : 26812)
#endif

/*
std::invalid_argument: related to the input parameters
std::runtime_error: system runtime error e.g. out of memory
std::logic_error: a bug caught through an assertion failure
std::exception: unknown error source e.g. probably another bug
*/
#define CATCH_POSSIBLE_EXCEPTIONS(logstr)              \
    catch (std::invalid_argument & e0)                 \
    {                                                  \
        logstr = e0.what();                            \
        return_value = McResult::MC_INVALID_VALUE;     \
    }                                                  \
    catch (std::runtime_error & e1)                    \
    {                                                  \
        logstr = e1.what();                            \
        return_value = McResult::MC_INVALID_OPERATION; \
    }                                                  \
    catch (std::logic_error & e2)                      \
    {                                                  \
        logstr = e2.what();                            \
        return_value = McResult::MC_RESULT_MAX_ENUM;   \
    }                                                  \
    catch (std::exception & e3)                        \
    {                                                  \
        logstr = e3.what();                            \
        return_value = McResult::MC_RESULT_MAX_ENUM;   \
    }

thread_local std::string per_thread_api_log_str;

MCAPI_ATTR McResult MCAPI_CALL mcCreateContext(McContext* pOutContext, McFlags contextFlags)
{
    McResult return_value = McResult::MC_NO_ERROR;
    per_thread_api_log_str.clear();

    if (pOutContext == nullptr) {
        per_thread_api_log_str = "context ptr undef (NULL)";
        return_value = McResult::MC_INVALID_VALUE;
    } else {
        try {
            create_context_impl(pOutContext, contextFlags);
        }
        CATCH_POSSIBLE_EXCEPTIONS(per_thread_api_log_str);
    }

    if (return_value != McResult::MC_NO_ERROR) {
        std::fprintf(stderr, "%s(...) -> %s\n", __FUNCTION__, per_thread_api_log_str.c_str());
    }

    return return_value;
}

MCAPI_ATTR McResult MCAPI_CALL mcDebugMessageCallback(McContext pContext, pfn_mcDebugOutput_CALLBACK cb, const void* userParam)
{
    McResult return_value = McResult::MC_NO_ERROR;
    per_thread_api_log_str.clear();

    if (pContext == nullptr) {
        per_thread_api_log_str = "context ptr (param0) undef (NULL)";
    } else if (cb == nullptr) {
        per_thread_api_log_str = "callback function ptr (param1) undef (NULL)";
    } else {
        try {
            debug_message_callback_impl(pContext, cb, userParam);
        }
        CATCH_POSSIBLE_EXCEPTIONS(per_thread_api_log_str);
    }

    if (!per_thread_api_log_str.empty()) {
        std::fprintf(stderr, "%s(...) -> %s\n", __FUNCTION__, per_thread_api_log_str.c_str());

        if (return_value == McResult::MC_NO_ERROR) // i.e. problem with basic local parameter checks
        {
            return_value = McResult::MC_INVALID_VALUE;
        }
    }

    return return_value;
}

MCAPI_ATTR McResult MCAPI_CALL mcDebugMessageControl(McContext pContext, McDebugSource source, McDebugType type, McDebugSeverity severity, bool enabled)
{
    McResult return_value = McResult::MC_NO_ERROR;
    per_thread_api_log_str.clear();

    if (pContext == nullptr) {
        per_thread_api_log_str = "context ptr (param0) undef (NULL)";
    } else if (
        false == (source == McDebugSource::MC_DEBUG_SOURCE_ALL || //
            source == McDebugSource::MC_DEBUG_SOURCE_KERNEL || //
            source == McDebugSource::MC_DEBUG_SOURCE_ALL)) {
        per_thread_api_log_str = "debug source val (param1) invalid";
    } else if (
        false == (type == McDebugType::MC_DEBUG_TYPE_ALL || //
            type == McDebugType::MC_DEBUG_TYPE_DEPRECATED_BEHAVIOR || //
            type == McDebugType::MC_DEBUG_TYPE_ERROR || //
            type == McDebugType::MC_DEBUG_TYPE_OTHER)) {
        per_thread_api_log_str = "debug type val (param2) invalid";
    } else if (
        false == (severity == McDebugSeverity::MC_DEBUG_SEVERITY_HIGH || //
            severity == McDebugSeverity::MC_DEBUG_SEVERITY_MEDIUM || //
            severity == McDebugSeverity::MC_DEBUG_SEVERITY_LOW || //
            severity == McDebugSeverity::MC_DEBUG_SEVERITY_NOTIFICATION || //
            severity == McDebugSeverity::MC_DEBUG_SEVERITY_ALL)) {
        per_thread_api_log_str = "debug severity val (param3) invalid";
    } else {
        try {
            debug_message_control_impl(pContext, source, type, severity, enabled);
        }
        CATCH_POSSIBLE_EXCEPTIONS(per_thread_api_log_str);
    }

    if (!per_thread_api_log_str.empty()) {
        std::fprintf(stderr, "%s(...) -> %s\n", __FUNCTION__, per_thread_api_log_str.c_str());
        if (return_value == McResult::MC_NO_ERROR) // i.e. problem with basic local parameter checks
        {
            return_value = McResult::MC_INVALID_VALUE;
        }
    }

    return return_value;
}

MCAPI_ATTR McResult MCAPI_CALL mcGetInfo(const McContext context, McFlags info, uint64_t bytes, void* pMem, uint64_t* pNumBytes)
{
    McResult return_value = McResult::MC_NO_ERROR;
    per_thread_api_log_str.clear();

    if (context == nullptr) {
        per_thread_api_log_str = "context ptr (param0) undef (NULL)";
    } else if (bytes != 0 && pMem == nullptr) {
        per_thread_api_log_str = "invalid specification (param2 & param3)";
    } else if (false == (info == MC_CONTEXT_FLAGS)) // check all possible values
    {
        per_thread_api_log_str = "invalid info flag val (param1)";
    } else if ((info == MC_CONTEXT_FLAGS) && (pMem != nullptr && bytes != sizeof(McFlags))) {
        per_thread_api_log_str = "invalid byte size (param2)"; // leads to e.g. "out of bounds" memory access during memcpy
    } else {
        try {
            get_info_impl(context, info, bytes, pMem, pNumBytes);
        }
        CATCH_POSSIBLE_EXCEPTIONS(per_thread_api_log_str);
    }

    if (!per_thread_api_log_str.empty()) {
        std::fprintf(stderr, "%s(...) -> %s\n", __FUNCTION__, per_thread_api_log_str.c_str());
        if (return_value == McResult::MC_NO_ERROR) // i.e. problem with basic local parameter checks
        {
            return_value = McResult::MC_INVALID_VALUE;
        }
    }

    return return_value;
}

MCAPI_ATTR McResult MCAPI_CALL mcDispatch(
    const McContext context,
    McFlags dispatchFlags,
    const void* pSrcMeshVertices,
    const uint32_t* pSrcMeshFaceIndices,
    const uint32_t* pSrcMeshFaceSizes,
    uint32_t numSrcMeshVertices,
    uint32_t numSrcMeshFaces,
    const void* pCutMeshVertices,
    const uint32_t* pCutMeshFaceIndices,
    const uint32_t* pCutMeshFaceSizes,
    uint32_t numCutMeshVertices,
    uint32_t numCutMeshFaces)
{
    TIMESTACK_RESET(); // reset tracking vars

    TIMESTACK_PUSH(__FUNCTION__);

    McResult return_value = McResult::MC_NO_ERROR;
    per_thread_api_log_str.clear();

    if (context == nullptr) {
        per_thread_api_log_str = "context ptr (param0) undef (NULL)";
    } else if (dispatchFlags == 0) {
        per_thread_api_log_str = "dispatch flags unspecified";
    } else if ((dispatchFlags & MC_DISPATCH_REQUIRE_THROUGH_CUTS) && //
        (dispatchFlags & MC_DISPATCH_FILTER_FRAGMENT_LOCATION_UNDEFINED)) {
        // The user states that she does not want a partial cut but yet also states that she
        // wants to keep fragments with partial cuts. These two options are mutually exclusive!
        per_thread_api_log_str = "use of mutually-exclusive flags: MC_DISPATCH_REQUIRE_THROUGH_CUTS & MC_DISPATCH_FILTER_FRAGMENT_LOCATION_UNDEFINED";
    } else if ((dispatchFlags & MC_DISPATCH_VERTEX_ARRAY_FLOAT) == 0 && (dispatchFlags & MC_DISPATCH_VERTEX_ARRAY_DOUBLE) == 0) {
        per_thread_api_log_str = "dispatch vertex aray type unspecified";
    } else if (pSrcMeshVertices == nullptr) {
        per_thread_api_log_str = "source-mesh vertex-position array ptr undef (NULL)";
    } else if (numSrcMeshVertices < 3) {
        per_thread_api_log_str = "invalid source-mesh vertex count";
    } else if (pSrcMeshFaceIndices == nullptr) {
        per_thread_api_log_str = "source-mesh face-index array ptr undef (NULL)";
    } /*else if (pSrcMeshFaceSizes == nullptr) {
        per_thread_api_log_str = "source-mesh face-size array ptr undef (NULL)";
    }*/ else if (numSrcMeshFaces < 1) {
        per_thread_api_log_str = "invalid source-mesh vertex count";
    } else if (pCutMeshVertices == nullptr) {
        per_thread_api_log_str = "cut-mesh vertex-position array ptr undef (NULL)";
    } else if (numCutMeshVertices < 3) {
        per_thread_api_log_str = "invalid cut-mesh vertex count";
    } else if (pCutMeshFaceIndices == nullptr) {
        per_thread_api_log_str = "cut-mesh face-index array ptr undef (NULL)";
    } /*else if (pCutMeshFaceSizes == nullptr) {
        per_thread_api_log_str = "cut-mesh face-size array ptr undef (NULL)";
    } */else if (numCutMeshFaces < 1) {
        per_thread_api_log_str = "invalid cut-mesh vertex count";
    } else {
        try {
            dispatch_impl(
                context,
                dispatchFlags,
                pSrcMeshVertices,
                pSrcMeshFaceIndices,
                pSrcMeshFaceSizes,
                numSrcMeshVertices,
                numSrcMeshFaces,
                pCutMeshVertices,
                pCutMeshFaceIndices,
                pCutMeshFaceSizes,
                numCutMeshVertices,
                numCutMeshFaces);
        }
        CATCH_POSSIBLE_EXCEPTIONS(per_thread_api_log_str);
    }

    if (!per_thread_api_log_str.empty()) {

        std::fprintf(stderr, "%s(...) -> %s\n", __FUNCTION__, per_thread_api_log_str.c_str());

        if (return_value == McResult::MC_NO_ERROR) // i.e. problem with basic local parameter checks
        {
            return_value = McResult::MC_INVALID_VALUE;
        }
    }

    TIMESTACK_POP();

    return return_value;
}

MCAPI_ATTR McResult MCAPI_CALL mcGetConnectedComponents(
    const McContext context,
    const McConnectedComponentType connectedComponentType,
    const uint32_t numEntries,
    McConnectedComponent* pConnComps,
    uint32_t* numConnComps)
{
    McResult return_value = McResult::MC_NO_ERROR;
    per_thread_api_log_str.clear();

    if (context == nullptr) {
        per_thread_api_log_str = "context ptr (param0) undef (NULL)";
    } else if (connectedComponentType == 0) {
        per_thread_api_log_str = "invalid type-parameter (param1) (0)";
    } else if (numConnComps == nullptr && pConnComps == nullptr) {
        per_thread_api_log_str = "output parameters undef (param3 & param4)";
    } else {
        try {
            get_connected_components_impl(context, connectedComponentType, numEntries, pConnComps, numConnComps);
        }
        CATCH_POSSIBLE_EXCEPTIONS(per_thread_api_log_str);
    }

    if (!per_thread_api_log_str.empty()) {

        std::fprintf(stderr, "%s(...) -> %s\n", __FUNCTION__, per_thread_api_log_str.c_str());

        if (return_value == McResult::MC_NO_ERROR) // i.e. problem with basic local parameter checks
        {
            return_value = McResult::MC_INVALID_VALUE;
        }
    }

    return return_value;
}

McResult MCAPI_CALL mcGetConnectedComponentData(
    const McContext context,
    const McConnectedComponent connCompId,
    McFlags queryFlags,
    uint64_t bytes,
    void* pMem,
    uint64_t* pNumBytes)
{
    McResult return_value = McResult::MC_NO_ERROR;
    per_thread_api_log_str.clear();
    
    if (context == nullptr) {
        per_thread_api_log_str = "context ptr (param0) undef (NULL)";
    }
    if (connCompId == nullptr) {
        per_thread_api_log_str = "connected component ptr (param1) undef (NULL)";
    } else if (queryFlags == 0) {
        per_thread_api_log_str = "flags (param1) undef (0)";
    } else if (bytes != 0 && pMem == nullptr) {
        per_thread_api_log_str = "null parameter (param3 & param4)";
    } else {
        try {
            get_connected_component_data_impl(context, connCompId, queryFlags, bytes, pMem, pNumBytes);
        }
        CATCH_POSSIBLE_EXCEPTIONS(per_thread_api_log_str);
    }

    if (!per_thread_api_log_str.empty()) {

        std::fprintf(stderr, "%s(...) -> %s\n", __FUNCTION__, per_thread_api_log_str.c_str());

        if (return_value == McResult::MC_NO_ERROR) // i.e. problem with basic local parameter checks
        {
            return_value = McResult::MC_INVALID_VALUE;
        }
    }

    return return_value;
}

McResult MCAPI_CALL mcReleaseConnectedComponents(
    const McContext context,
    uint32_t numConnComps,
    const McConnectedComponent* pConnComps)
{
    McResult return_value = McResult::MC_NO_ERROR;
    per_thread_api_log_str.clear();

    if (context == nullptr) {
        per_thread_api_log_str = "context ptr (param0) undef (NULL)";
    } else if (numConnComps > 0 && pConnComps == NULL) {
        per_thread_api_log_str = "invalid pointer to connected components";
    } else if (numConnComps == 0 && pConnComps != NULL) {
        per_thread_api_log_str = "number of connected components not set";
    } else {
        try {
            release_connected_components_impl(context, numConnComps, pConnComps);
        }
        CATCH_POSSIBLE_EXCEPTIONS(per_thread_api_log_str);
    }

    if (!per_thread_api_log_str.empty()) {

        std::fprintf(stderr, "%s(...) -> %s\n", __FUNCTION__, per_thread_api_log_str.c_str());

        if (return_value == McResult::MC_NO_ERROR) // i.e. problem with basic local parameter checks
        {
            return_value = McResult::MC_INVALID_VALUE;
        }
    }

    return return_value;
}

MCAPI_ATTR McResult MCAPI_CALL mcReleaseContext(const McContext context)
{
    McResult return_value = McResult::MC_NO_ERROR;
    per_thread_api_log_str.clear();

    if (context == nullptr) {
        per_thread_api_log_str = "context ptr (param0) undef (NULL)";
    } else {
        try {
            release_context_impl(context);
        }
        CATCH_POSSIBLE_EXCEPTIONS(per_thread_api_log_str);
    }

    if (!per_thread_api_log_str.empty()) {

        std::fprintf(stderr, "%s(...) -> %s\n", __FUNCTION__, per_thread_api_log_str.c_str());

        if (return_value == McResult::MC_NO_ERROR) // i.e. problem with basic local parameter checks
        {
            return_value = McResult::MC_INVALID_VALUE;
        }
    }

    return return_value;
}
