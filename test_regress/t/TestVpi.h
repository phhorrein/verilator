// -*- mode: C++; c-file-style: "cc-mode" -*-
//*************************************************************************
//
// Copyright 2013-2017 by Wilson Snyder. This program is free software; you can
// redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License
// Version 2.0.
// SPDX-License-Identifier: LGPL-3.0-only OR Artistic-2.0
//
//*************************************************************************

#include "sv_vpi_user.h"

//======================================================================

#define CHECK_RESULT_VH(got, exp) \
    if ((got) != (exp)) { \
        printf("%%Error: line %d: GOT = %p   EXP = %p\n", __LINE__, (got), (exp)); \
        return __LINE__; \
    }

#define CHECK_RESULT_NZ(got) \
    if (!(got)) { \
        printf("%%Error: line %d: GOT = NULL  EXP = !NULL\n", __LINE__); \
        return __LINE__; \
    }

// Use cout to avoid issues with %d/%lx etc
#define CHECK_RESULT(got, exp) \
    if ((got) != (exp)) { \
        std::cout << std::dec << "%Error: line " << __LINE__ << ": GOT = " << (got) \
                  << "   EXP = " << (exp) << std::endl; \
        return __LINE__; \
    }

#define CHECK_RESULT_BOOL(got, exp) \
    do { \
        PLI_INT32 iexp = exp ? 1 : 0; \
        if ((iexp) != (exp)) { \
            std::cout << std::dec << "%Error: line " << __LINE__ << ": GOT = " << (got) \
                      << "(" << (((got) == 0) ? false : true) << ")" \
                      << "   EXP = " << (iexp) << "(" << (exp) << ")" << std::endl; \
            return __LINE__; \
        } \
    } while (0)

#define CHECK_RESULT_HEX(got, exp) \
    if ((got) != (exp)) { \
        std::cout << std::dec << "%Error: line " << __LINE__ << std::hex \
                  << ": GOT = " << (got) << "   EXP = " << (exp) << std::endl; \
        return __LINE__; \
    }

#define CHECK_RESULT_CSTR(got, exp) \
    if (strcmp((got), (exp))) { \
        printf("%%Error: line %d: GOT = '%s'   EXP = '%s'\n", __LINE__, \
               ((got) != NULL) ? (got) : "<null>", ((exp) != NULL) ? (exp) : "<null>"); \
        return __LINE__; \
    }

#define CHECK_RESULT_CSTR2(got, exp1, exp2) \
    if (strcmp((got), (exp1)) && strcmp((got), (exp2))) { \
        printf("%%Error: line %d: GOT = '%s'   EXP = '%s' or '%s'\n", __LINE__, \
               ((got) != NULL) ? (got) : "<null>", ((exp1) != NULL) ? (exp1) : "<null>", \
               ((exp2) != NULL) ? (exp2) : "<null>"); \
        return __LINE__; \
    }

#define CHECK_RESULT_CSTR_STRIP(got, exp) CHECK_RESULT_CSTR(got + strspn(got, " "), exp)

// We cannot replace those with VL_STRINGIFY, not available when PLI is build
#define STRINGIFY(x) STRINGIFY2(x)
#define STRINGIFY2(x) #x

#define CHECK_PROPERTIES(h, props) \
    do { \
       int propChecks = (h).check_properties((props)); \
       if (propChecks != 0) { \
          return __LINE__; \
       } \
    } while (0)

#define CHECK_ACCESSORS(h, acc) \
    do { \
       int propChecks = (h).check_accessors((acc)); \
       if (propChecks != 0) { \
          return __LINE__; \
       } \
    } while (0)


class VpiVarAcc {
public:
    const char *m_module;
    const char *m_scope;
    PLI_INT32   m_typespec;
    const char *m_typespecName;

    VpiVarAcc() {
        reset();
    }

    void reset() {
        m_module = nullptr;
        m_scope = nullptr;
        m_typespec = vpiLogicTypespec;
        m_typespecName = nullptr;
    }
};

class VpiVarProps {
public:
    bool m_scalar;
    bool m_vector;
    bool m_array; // DEPRECATED (but we can still test it...)
    bool m_structMember;
    bool m_arrayMember;
    bool m_packedArrayMember;
    PLI_INT32 m_size;
    const char *m_name;
    const char *m_fullname;
    bool m_signedVar;
    bool m_automatic;
    bool m_constant;
    PLI_INT32 m_visibility;
    PLI_INT32 m_arrayType;

    VpiVarProps() {
        reset();
    }
    void reset() {
        m_scalar = false;
        m_vector = false;
        m_array = false;
        m_structMember = false;
        m_arrayMember = false;
        m_packedArrayMember = false;
        m_signedVar = false;
        m_automatic = false;
        m_constant = false;
        m_arrayType = -1;
        m_visibility = vpiPublicVis;
        m_size = 1;
        m_name = nullptr;
        m_fullname = nullptr;
    }
};

class TestVpiHandle {
    /// For testing, etc, wrap vpiHandle in an auto-releasing class
    vpiHandle m_handle = NULL;
    bool m_freeit = true;

public:
    TestVpiHandle() {}
    TestVpiHandle(vpiHandle h)
        : m_handle(h) {}
    ~TestVpiHandle() { release(); }
    operator vpiHandle() const { return m_handle; }
    inline TestVpiHandle& operator=(vpiHandle h) {
        release();
        m_handle = h;
        return *this;
    }
    void release() {
        if (m_handle && m_freeit) {
            // Below not VL_DO_DANGLING so is portable
            vpi_release_handle(m_handle);
            m_handle = NULL;
        }
    }
    // Freed by another action e.g. vpi_scan; so empty and don't free again
    void freed() {
        m_handle = NULL;
        m_freeit = false;
    }

    int check_accessors(VpiVarAcc *acc) {
        PLI_INT32 d;
        const char* p;
        if (acc->m_module) {
            TestVpiHandle mod = vpi_handle(vpiModule, m_handle);
            CHECK_RESULT_NZ(mod);
            p = vpi_get_str(vpiName, mod);
            CHECK_RESULT_CSTR(p, acc->m_module);
        }
        if (acc->m_scope) {
            TestVpiHandle scope = vpi_handle(vpiScope, m_handle);
            CHECK_RESULT_NZ(scope);
            p = vpi_get_str(vpiName, scope);
            CHECK_RESULT_CSTR(p, acc->m_scope);
        }
        TestVpiHandle typespec = vpi_handle(vpiTypespec, m_handle);
        CHECK_RESULT_NZ(typespec);
        d = vpi_get(vpiType, typespec);
        CHECK_RESULT(d, acc->m_typespec);
        if (acc->m_typespecName && !TestSimulator::is_mti()) {
            // XXX: ModelSim is not able to read the typespec name
            p = vpi_get_str(vpiName, typespec);
            CHECK_RESULT_CSTR(p, acc->m_typespecName);
        }
        return 0;
    }

    int check_properties(VpiVarProps *props) {
        PLI_INT32 d;
        const char* p;
        d = vpi_get(vpiVector, m_handle);
        CHECK_RESULT_BOOL(d, props->m_vector);
        d = vpi_get(vpiArray, m_handle);
        CHECK_RESULT_BOOL(d, props->m_array);
        d = vpi_get(vpiScalar, m_handle);
        CHECK_RESULT_BOOL(d, props->m_scalar);
        d = vpi_get(vpiStructUnionMember, m_handle);
        CHECK_RESULT_BOOL(d, props->m_structMember);
        d = vpi_get(vpiArrayMember, m_handle);
        CHECK_RESULT_BOOL(d, props->m_arrayMember);
        d = vpi_get(vpiPackedArrayMember, m_handle);
        CHECK_RESULT_BOOL(d, props->m_packedArrayMember);
        d = vpi_get(vpiSigned, m_handle);
        CHECK_RESULT_BOOL(d, props->m_signedVar);
        d = vpi_get(vpiAutomatic, m_handle);
        CHECK_RESULT_BOOL(d, props->m_automatic);
        d = vpi_get(vpiConstant, m_handle);
        CHECK_RESULT_BOOL(d, props->m_constant);
        d = vpi_get(vpiSize, m_handle);
        CHECK_RESULT(d, props->m_size);
        d = vpi_get(vpiArrayType, m_handle);
        CHECK_RESULT(d, props->m_arrayType);
        // XXX: disagree with ModelSim
        if (!TestSimulator::is_mti()) {
            d = vpi_get(vpiVisibility, m_handle);
            CHECK_RESULT(d, props->m_visibility);
        }
        if (props->m_name) {
            p = vpi_get_str(vpiName, m_handle);
            CHECK_RESULT_CSTR(p, props->m_name);
        }
        if (props->m_fullname) {
            p = vpi_get_str(vpiFullName, m_handle);
            CHECK_RESULT_CSTR(p, props->m_fullname);
        }
        return 0;
    }
};


