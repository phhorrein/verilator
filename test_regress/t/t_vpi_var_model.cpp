// -*- mode: C++; c-file-style: "cc-mode" -*-
//*************************************************************************
//
// Copyright 2010-2021 by Wilson Snyder. This program is free software; you can
// redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License
// Version 2.0.
// SPDX-License-Identifier: LGPL-3.0-only OR Artistic-2.0
//
//*************************************************************************
//
// This test checks all relations of variables for VPI.
//
// It follows IEEE 1800-2017, section 37.17 (p991-993)
//
// It does not check:
//    - randomization (vpiIsRandomized, vpiRandType)
//    - allocation scheme (vpiAllocScheme)
//    - short real var (vpiShortRealVar)
//    - loads and drivers (FIXME: should probably be done?)
//    - var select (FIXME: should probably be done?)

#ifdef IS_VPI

#include "sv_vpi_user.h"

#else

#include "Vt_vpi_var_model.h"
#include "verilated.h"
#include "svdpi.h"

#include "Vt_vpi_var_model__Dpi.h"

#include "verilated_vpi.h"
#include "verilated_vcd_c.h"

#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>

#include "TestSimulator.h"
#include "TestVpi.h"

// __FILE__ is too long
#define FILENM "t_vpi_var_model.cpp"

#define TEST_MSG \
    if (0) printf

unsigned int main_time = 0;
unsigned int callback_count = 0;
unsigned int callback_count_half = 0;
unsigned int callback_count_quad = 0;
unsigned int callback_count_strs = 0;
unsigned int callback_count_strs_max = 500;

//======================================================================

#ifdef TEST_VERBOSE
bool verbose = true;
#else
bool verbose = false;
#endif

#define CHECK_RESULT_VH(got, exp) \
    if ((got) != (exp)) { \
        printf("%%Error: %s:%d: GOT = %p   EXP = %p\n", FILENM, __LINE__, (got), (exp)); \
        return __LINE__; \
    }

#define CHECK_RESULT_NZ(got) \
    if (!(got)) { \
        printf("%%Error: %s:%d: GOT = NULL  EXP = !NULL\n", FILENM, __LINE__); \
        return __LINE__; \
    }

// Use cout to avoid issues with %d/%lx etc
#define CHECK_RESULT(got, exp) \
    if ((got) != (exp)) { \
        std::cout << std::dec << "%Error: " << FILENM << ":" << __LINE__ << ": GOT = " << (got) \
                  << "   EXP = " << (exp) << std::endl; \
        return __LINE__; \
    }

#define CHECK_RESULT_BOOL(got, exp) \
    do { \
        PLI_INT32 iexp = exp ? 1 : 0; \
        if ((iexp) != (exp)) { \
            std::cout << std::dec << "%Error: " << FILENM << ":" << __LINE__ << ": GOT = " << (got) \
                      << "(" << (((got) == 0) ? false : true) << ")" \
                      << "   EXP = " << (iexp) << "(" << (exp) << ")" << std::endl; \
            return __LINE__; \
        } \
    } while (0)

#define CHECK_RESULT_HEX(got, exp) \
    if ((got) != (exp)) { \
        std::cout << std::dec << "%Error: " << FILENM << ":" << __LINE__ << std::hex \
                  << ": GOT = " << (got) << "   EXP = " << (exp) << std::endl; \
        return __LINE__; \
    }

#define CHECK_RESULT_CSTR(got, exp) \
    if (strcmp((got), (exp))) { \
        printf("%%Error: %s:%d: GOT = '%s'   EXP = '%s'\n", FILENM, __LINE__, \
               ((got) != NULL) ? (got) : "<null>", ((exp) != NULL) ? (exp) : "<null>"); \
        return __LINE__; \
    }

#define CHECK_RESULT_CSTR2(got, exp1, exp2) \
    if (strcmp((got), (exp1)) && strcmp((got), (exp2))) { \
        printf("%%Error: %s:%d: GOT = '%s'   EXP = '%s' or '%s'\n", FILENM, __LINE__, \
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
       int propChecks = check_properties((h), (props)); \
       if (propChecks != 0) { \
          return __LINE__; \
       } \
    } while (0)

#define CHECK_ACCESSORS(h, acc) \
    do { \
       int propChecks = check_accessors((h), (acc)); \
       if (propChecks != 0) { \
          return __LINE__; \
       } \
    } while (0)

typedef struct {
    const char *module;
    const char *scope;
    PLI_INT32 typespec;
    const char *typespecName;
} var_acc_t;

void init_acc(var_acc_t *acc) {
    acc->module = nullptr;
    acc->scope = nullptr;
    acc->typespec = vpiLogicTypespec;
    acc->typespecName = nullptr;
}

int check_accessors(vpiHandle vh, var_acc_t *acc) {
    PLI_INT32 d;
    const char* p;
    if (acc->module) {
        TestVpiHandle mod = vpi_handle(vpiModule, vh);
        CHECK_RESULT_NZ(mod);
        p = vpi_get_str(vpiName, mod);
        CHECK_RESULT_CSTR(p, acc->module);
    }
    if (acc->scope) {
        TestVpiHandle scope = vpi_handle(vpiScope, vh);
        CHECK_RESULT_NZ(scope);
        p = vpi_get_str(vpiName, scope);
        CHECK_RESULT_CSTR(p, acc->scope);
    }
    TestVpiHandle typespec = vpi_handle(vpiTypespec, vh);
    CHECK_RESULT_NZ(typespec);
    d = vpi_get(vpiType, typespec);
    CHECK_RESULT(d, acc->typespec);
    if (acc->typespecName && !TestSimulator::is_mti()) {
        // XXX: ModelSim is not able to read the typespec name
        p = vpi_get_str(vpiName, typespec);
        CHECK_RESULT_CSTR(p, acc->typespecName);
    }
    return 0;
}

// Use a struct to define all properties of a variable which could be checked
typedef struct {
    bool scalar;
    bool vector;
    bool array; // DEPRECATED (but we can still test it...)
    bool structMember;
    bool arrayMember;
    bool packedArrayMember;
    PLI_INT32 size;
    const char *name;
    const char *fullname;
    bool signedVar;
    bool automatic;
    bool constant;
    PLI_INT32 visibility;
    PLI_INT32 arrayType;
} var_props_t;

void init_properties(var_props_t *props) {
    props->scalar = false;
    props->vector = false;
    props->array = false;
    props->structMember = false;
    props->arrayMember = false;
    props->packedArrayMember = false;
    props->signedVar = false;
    props->automatic = false;
    props->constant = false;
    props->arrayType = -1;
    props->visibility = vpiPublicVis;
    props->size = 1;
    props->name = nullptr;
    props->fullname = nullptr;
}

int check_properties(vpiHandle vh, var_props_t *props) {
    PLI_INT32 d;
    const char* p;
    d = vpi_get(vpiVector, vh);
    CHECK_RESULT_BOOL(d, props->vector);
    d = vpi_get(vpiArray, vh);
    CHECK_RESULT_BOOL(d, props->array);
    d = vpi_get(vpiScalar, vh);
    CHECK_RESULT_BOOL(d, props->scalar);
    d = vpi_get(vpiStructUnionMember, vh);
    CHECK_RESULT_BOOL(d, props->structMember);
    d = vpi_get(vpiArrayMember, vh);
    CHECK_RESULT_BOOL(d, props->arrayMember);
    d = vpi_get(vpiPackedArrayMember, vh);
    CHECK_RESULT_BOOL(d, props->packedArrayMember);
    d = vpi_get(vpiSigned, vh);
    CHECK_RESULT_BOOL(d, props->signedVar);
    d = vpi_get(vpiAutomatic, vh);
    CHECK_RESULT_BOOL(d, props->automatic);
    d = vpi_get(vpiConstant, vh);
    CHECK_RESULT_BOOL(d, props->constant);
    d = vpi_get(vpiSize, vh);
    CHECK_RESULT(d, props->size);
    d = vpi_get(vpiArrayType, vh);
    CHECK_RESULT(d, props->arrayType);
    // XXX: disagree with ModelSim
    if (!TestSimulator::is_mti()) {
        d = vpi_get(vpiVisibility, vh);
        CHECK_RESULT(d, props->visibility);
    }
    if (props->name) {
        p = vpi_get_str(vpiName, vh);
        CHECK_RESULT_CSTR(p, props->name);
    }
    if (props->fullname) {
        p = vpi_get_str(vpiFullName, vh);
        CHECK_RESULT_CSTR(p, props->fullname);
    }
    return 0;
}


int _mon_check_arr() {

    const char* p;
    t_vpi_value tmpValue;
    s_vpi_value v;
    t_vpi_vecval vv[1];
    bzero(&vv, sizeof(vv));

    s_vpi_time t;
    t.type = vpiSimTime;
    t.high = 0;
    t.low = 0;

    PLI_INT32 d;
    var_props_t props;
    var_acc_t acc;
    // Single logic/reg
    TestVpiHandle vh_onereg  = VPI_HANDLE("onereg");
    CHECK_RESULT_NZ(vh_onereg);
    {
        p = vpi_get_str(vpiType, vh_onereg);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // Single bit: vector is false, scalar is true, size is 1
        init_properties(&props);
        props.scalar = true;
        props.name = "onereg";
        props.fullname = "top.t.onereg";
        CHECK_PROPERTIES(vh_onereg, &props);
        // a is initialized to 0
        TestVpiHandle iter = vpi_handle(vpiBit, vh_onereg); // 37.17, detail 12
        CHECK_RESULT(iter, 0);  // According to standard, it should give !NULL, but MS gives NULL (and it sounds reasonable)
        TestVpiHandle i = vpi_handle(vpiExpr, vh_onereg); // 37.17, detail 8
        CHECK_RESULT(i, 0);  // According to standard, it should give !NULL, but MS gives NULL (and it is a bug)
    }
    // Single logic/reg
    TestVpiHandle vh_onebit  = VPI_HANDLE("onebit");
    CHECK_RESULT_NZ(vh_onebit);
    {
        p = vpi_get_str(vpiType, vh_onebit);
        CHECK_RESULT_CSTR(p, "vpiBitVar");
        // Single bit: vector is false, scalar is true, size is 1
        init_properties(&props);
        props.scalar = true;
        props.name = "onebit";
        props.fullname = "top.t.onebit";
        CHECK_PROPERTIES(vh_onebit, &props);
    }
    // One-dimensional packed array
    TestVpiHandle vha_p0  = VPI_HANDLE("a_p0");
    CHECK_RESULT_NZ(vha_p0);
    {
        // FIXME: add vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vha_p0);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // Single bit, but packed dimension: vector is true, scalar is false, size is 1
        init_properties(&props);
        props.vector = true;
        props.name = "a_p0";
        props.fullname = "top.t.a_p0";
        CHECK_PROPERTIES(vha_p0, &props);
        init_acc(&acc);
        acc.module = "t";
        acc.scope = "t";
        acc.typespec = vpiLogicTypespec;
        CHECK_ACCESSORS(vha_p0, &acc);
        v.format = vpiIntVal;
        v.value.integer = 1;
        vpi_put_value(vha_p0, &v, &t, vpiNoDelay);
    }
    TestVpiHandle vha_p1  = VPI_HANDLE("a_p1");
    CHECK_RESULT_NZ(vha_p1);
    {
        // FIXME: add vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vha_p1);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // 2 bits packed: vector is true, scalar is false, size is 1
        init_properties(&props);
        props.vector = true;
        props.size = 2;
        props.name = "a_p1";
        props.fullname = "top.t.a_p1";
        CHECK_PROPERTIES(vha_p1, &props);
        TestVpiHandle vha_p1_idx1 = vpi_handle_by_index(vha_p1, 1);
        CHECK_RESULT_NZ(vha_p1_idx1);
    }
    TestVpiHandle vha_sp1  = VPI_HANDLE("a_sp1");
    CHECK_RESULT_NZ(vha_sp1);
    {
        // FIXME: add vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vha_sp1);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // 2 bits packed: vector is true, scalar is false, size is 1
        init_properties(&props);
        props.vector = true;
        props.signedVar = true;
        props.size = 2;
        props.name = "a_sp1";
        props.fullname = "top.t.a_sp1";
        CHECK_PROPERTIES(vha_sp1, &props);
        TestVpiHandle vha_sp1_idx1 = vpi_handle_by_index(vha_sp1, 1);
        CHECK_RESULT_NZ(vha_sp1_idx1);
    }
    TestVpiHandle vha_p21  = VPI_HANDLE("a_p21");
    CHECK_RESULT_NZ(vha_p21);
    {
        p = vpi_get_str(vpiType, vha_p21);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // 2 bits packed: vector is true, scalar is false, size is 1
        init_properties(&props);
        props.vector = true;
        props.size = 6;
        props.name = "a_p21";
        props.fullname = "top.t.a_p21";
        CHECK_PROPERTIES(vha_p21, &props);
        TestVpiHandle vha_p21_idx1 = vpi_handle_by_index(vha_p21, 1);
        CHECK_RESULT_NZ(vha_p21_idx1);
        p = vpi_get_str(vpiType, vha_p21_idx1);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        init_properties(&props);
        props.vector = true;
        props.size = 2;
        props.name = "a_p21[1]";
        props.fullname = "top.t.a_p21[1]";
        CHECK_PROPERTIES(vha_p21_idx1, &props);
        TestVpiHandle vha_p21_idx1_idx0 = vpi_handle_by_index(vha_p21_idx1, 0);
        CHECK_RESULT_NZ(vha_p21_idx1_idx0);
        p = vpi_get_str(vpiType, vha_p21_idx1_idx0);
        CHECK_RESULT_CSTR(p, "vpiRegBit");
        init_properties(&props);
        props.scalar = true;
        props.name = "a_p21[1][0]";
        props.fullname = "top.t.a_p21[1][0]";
        CHECK_PROPERTIES(vha_p21_idx1_idx0, &props);
    }
    TestVpiHandle vha_u0  = VPI_HANDLE("a_u0");
    CHECK_RESULT_NZ(vha_u0);
    {
        // FIXME: add vpiReg/vpiParent access
        p = vpi_get_str(vpiType, vha_u0);
        CHECK_RESULT_CSTR(p, "vpiArrayVar");
        init_properties(&props);
        props.scalar = true;
        props.array = true;
        props.name = "a_u0";
        props.fullname = "top.t.a_u0";
        props.arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vha_u0, &props);
        TestVpiHandle vha_u0_idx0 = vpi_handle_by_index(vha_u0, 0);
        CHECK_RESULT_NZ(vha_u0_idx0);
        p = vpi_get_str(vpiType, vha_u0_idx0);
        CHECK_RESULT_CSTR(p, "vpiReg");
        init_properties(&props);
        props.scalar = true;
        props.arrayMember = true;
        props.name = "a_u0[0]";
        props.fullname = "top.t.a_u0[0]";
        CHECK_PROPERTIES(vha_u0_idx0, &props);
        v.format = vpiIntVal;
        v.value.integer = 1;
        vpi_put_value(vha_u0_idx0, &v, &t, vpiNoDelay);
    }
    TestVpiHandle vha_u2  = VPI_HANDLE("a_u2");
    CHECK_RESULT_NZ(vha_u0);
    {
        // FIXME: add vpiReg/vpiParent access
        p = vpi_get_str(vpiType, vha_u2);
        CHECK_RESULT_CSTR(p, "vpiArrayVar");
        init_properties(&props);
        props.scalar = true;
        props.size = 3;
        props.array = true;
        props.name = "a_u2";
        props.fullname = "top.t.a_u2";
        props.arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vha_u2, &props);
        TestVpiHandle vha_u2_idx2 = vpi_handle_by_index(vha_u2, 2);
        CHECK_RESULT_NZ(vha_u2_idx2);
        p = vpi_get_str(vpiType, vha_u2_idx2);
        CHECK_RESULT_CSTR(p, "vpiReg");
        init_properties(&props);
        props.scalar = true;
        props.arrayMember = true;
        props.name = "a_u2[2]";
        props.fullname = "top.t.a_u2[2]";
        CHECK_PROPERTIES(vha_u2_idx2, &props);
        TestVpiHandle iter = vpi_iterate(vpiRange, vha_u2);
        CHECK_RESULT_NZ(iter);
        p = vpi_get_str(vpiType, iter);
        CHECK_RESULT_CSTR(p, "vpiIterator");
        TestVpiHandle vha_u2_r0 = vpi_scan(iter);
        CHECK_RESULT_NZ(vha_u2_r0);
        p = vpi_get_str(vpiType, vha_u2_r0);
        CHECK_RESULT_CSTR(p, "vpiRange");
        TestVpiHandle vha_u2_r0_l = vpi_handle(vpiLeftRange, vha_u2_r0);
        CHECK_RESULT_NZ(vha_u2_r0_l);
        TestVpiHandle vha_u2_r0_r = vpi_handle(vpiRightRange, vha_u2_r0);
        CHECK_RESULT_NZ(vha_u2_r0_r);
        vpi_get_value(vha_u2_r0_l, &v);
        CHECK_RESULT(v.value.integer, 2);
        vpi_get_value(vha_u2_r0_r, &v);
        CHECK_RESULT(v.value.integer, 0);
    }
    TestVpiHandle vha_p1u1  = VPI_HANDLE("a_p1u1");
    CHECK_RESULT_NZ(vha_p1u1);
    {
        // FIXME: add vpiReg/vpiParent access and vpiBit/vpiParent
        p = vpi_get_str(vpiType, vha_p1u1);
        CHECK_RESULT_CSTR(p, "vpiArrayVar");
        init_properties(&props);
        props.vector = true;
        props.size = 2;
        props.array = true;
        props.name = "a_p1u1";
        props.fullname = "top.t.a_p1u1";
        props.arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vha_p1u1, &props);
        TestVpiHandle vha_p1u1_idx0 = vpi_handle_by_index(vha_p1u1, 0);
        CHECK_RESULT_NZ(vha_p1u1_idx0);
        p = vpi_get_str(vpiType, vha_p1u1_idx0);
        CHECK_RESULT_CSTR(p, "vpiReg");
        init_properties(&props);
        props.vector = true;
        props.size = 2;
        props.arrayMember = true;
        props.name = "a_p1u1[0]";
        props.fullname = "top.t.a_p1u1[0]";
        CHECK_PROPERTIES(vha_p1u1_idx0, &props);
    }
    TestVpiHandle vha_p0u12  = VPI_HANDLE("a_p0u12");
    CHECK_RESULT_NZ(vha_p0u12);
    {
        // FIXME: add vpiReg/vpiParent access and vpiBit/vpiParent
        p = vpi_get_str(vpiType, vha_p0u12);
        CHECK_RESULT_CSTR2(p, "vpiArrayVar", "vpiRegArray");
        init_properties(&props);
        props.vector = false;
        props.size = 6;
        props.array = true;
        props.name = "a_p0u12";
        props.fullname = "top.t.a_p0u12";
        props.arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vha_p0u12, &props);
        // XXX: disagree with ModelSim
        if (!TestSimulator::is_mti()) {
            // ModelSim is not able to find this...
            TestVpiHandle vha_p0u12_idx0 = vpi_handle_by_index(vha_p0u12, 0);
            CHECK_RESULT_NZ(vha_p0u12_idx0);
            p = vpi_get_str(vpiType, vha_p0u12_idx0);
            CHECK_RESULT_CSTR2(p, "vpiArrayVar", "vpiRegArray");
            init_properties(&props);
            props.vector = true;
            props.size = 3;
            props.arrayMember = true;
            props.name = "a_p0u12[0]";
            props.fullname = "top.t.a_p0u12[0]";
            CHECK_PROPERTIES(vha_p0u12_idx0, &props);
        }
    }

   return 0;
}

int _mon_check_structs() {

    const char* p;
    t_vpi_value tmpValue;
    s_vpi_value v;
    t_vpi_vecval vv[1];
    bzero(&vv, sizeof(vv));

    s_vpi_time t;
    t.type = vpiSimTime;
    t.high = 0;
    t.low = 0;

    PLI_INT32 d;
    var_props_t props;
    var_acc_t acc;

    TestVpiHandle vhs = VPI_HANDLE("s");
    CHECK_RESULT_NZ(vhs);
    {
        // FIXME: add vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vhs);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        init_properties(&props);
        props.name = "s";
        props.fullname = "top.t.s";
        props.size = 4;
        CHECK_PROPERTIES(vhs, &props);
        init_acc(&acc);
        acc.module = "t";
        acc.scope = "t";
        acc.typespec = vpiStructTypespec;
        acc.typespecName = "struct_test";
        CHECK_ACCESSORS(vhs, &acc);

        TestVpiHandle vhs_iter = vpi_iterate(vpiMember, vhs);
        CHECK_RESULT_NZ(vhs_iter);
        p = vpi_get_str(vpiType, vhs_iter);
        CHECK_RESULT_CSTR(p, "vpiIterator");
        
        TestVpiHandle vhs_field0 = vpi_scan(vhs_iter);
        CHECK_RESULT_NZ(vhs_field0);
        p = vpi_get_str(vpiType, vhs_field0);
        CHECK_RESULT_CSTR(p, "vpiReg");
        init_properties(&props);
        props.scalar = true;
        props.name = "s.s_field";
        props.fullname = "top.t.s.s_field";
        props.size = 1;
        props.structMember = true;
        CHECK_PROPERTIES(vhs_field0, &props);
        
        TestVpiHandle vhs_field1 = vpi_scan(vhs_iter);
        CHECK_RESULT_NZ(vhs_field1);
        p = vpi_get_str(vpiType, vhs_field1);
        CHECK_RESULT_CSTR(p, "vpiReg");
        init_properties(&props);
        props.vector = true;
        props.name = "s.p_field";
        props.fullname = "top.t.s.p_field";
        props.size = 3;
        props.structMember = true;
        CHECK_PROPERTIES(vhs_field1, &props);

        TestVpiHandle vhs_field1_idx0 = vpi_handle_by_index(vhs_field1, 0);
        CHECK_RESULT_NZ(vhs_field1_idx0);
        p = vpi_get_str(vpiType, vhs_field1_idx0);
        CHECK_RESULT_CSTR(p, "vpiRegBit");
        init_properties(&props);
        props.scalar = true;
        props.name = "s.p_field[0]";
        props.fullname = "top.t.s.p_field[0]";
        props.size = 1;
        CHECK_PROPERTIES(vhs_field1_idx0, &props);

        // XXX: disagree with ModelSim
        if (!TestSimulator::is_mti()) {
            // ModelSim segfaults here
            TestVpiHandle vhs_done = vpi_scan(vhs_iter);
            CHECK_RESULT(vhs_done, 0);
        }

        TestVpiHandle vhs_f0_par = vpi_handle(vpiParent, vhs_field0);
        CHECK_RESULT_NZ(vhs_f0_par);
        p = vpi_get_str(vpiType, vhs_f0_par);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        init_properties(&props);
        props.name = "s";
        props.fullname = "top.t.s";
        props.size = 4;
        CHECK_PROPERTIES(vhs_f0_par, &props);
    }

    TestVpiHandle vhs_emb = VPI_HANDLE("s_emb");
    CHECK_RESULT_NZ(vhs_emb);
    {
        p = vpi_get_str(vpiType, vhs_emb);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        init_properties(&props);
        props.name = "s_emb";
        props.fullname = "top.t.s_emb";
        props.size = 11;
        CHECK_PROPERTIES(vhs_emb, &props);
        TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_emb);
        CHECK_RESULT_NZ(vh_iter);
        p = vpi_get_str(vpiType, vh_iter);
        CHECK_RESULT_CSTR(p, "vpiIterator");
        TestVpiHandle vhs_emb_field0 = vpi_scan(vh_iter);
        CHECK_RESULT_NZ(vhs_emb_field0);
        p = vpi_get_str(vpiType, vhs_emb_field0);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        init_properties(&props);
        props.name = "s_emb.field0";
        props.fullname = "top.t.s_emb.field0";
        props.size = 4;
        props.structMember = true;
        CHECK_PROPERTIES(vhs_emb_field0, &props);
        TestVpiHandle vh_f0_iter = vpi_iterate(vpiMember, vhs_emb_field0);
        CHECK_RESULT_NZ(vh_f0_iter);
        TestVpiHandle vhs_emb_field0_sfield = vpi_scan(vh_f0_iter);
        CHECK_RESULT_NZ(vhs_emb_field0_sfield);
        p = vpi_get_str(vpiType, vhs_emb_field0_sfield);
        CHECK_RESULT_CSTR(p, "vpiReg");
        init_properties(&props);
        props.scalar = true;
        props.name = "s_emb.field0.s_field";
        props.fullname = "top.t.s_emb.field0.s_field";
        props.size = 1;
        props.structMember = true;
        CHECK_PROPERTIES(vhs_emb_field0_sfield, &props);
    }
    
    TestVpiHandle vhs_p1 = VPI_HANDLE("s_p1");
    CHECK_RESULT_NZ(vhs_p1);
    {
        // FIXME: missing vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vhs_p1);
        CHECK_RESULT_CSTR(p, "vpiPackedArrayVar");
        init_properties(&props);
        props.vector = true;
        props.name = "s_p1";
        props.fullname = "top.t.s_p1";
        props.size = 8;
        CHECK_PROPERTIES(vhs_p1, &props);
        init_acc(&acc);
        acc.module = "t";
        acc.scope = "t";
        acc.typespec = vpiPackedArrayTypespec;
        acc.typespecName = "struct_test";
        CHECK_ACCESSORS(vhs_p1, &acc);

        // XXX: disagree with ModelSim
        if (!TestSimulator::is_mti()) {
            TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_p1);
            CHECK_RESULT_NZ(vh_iter);
            TestVpiHandle vhs_p1_idx0 = vpi_scan(vh_iter);
            CHECK_RESULT_NZ(vhs_p1_idx0);
            p = vpi_get_str(vpiType, vhs_p1_idx0);
            // Does not work with ModelSim, returns vpiReg, see 37.18, detail 3
            CHECK_RESULT_CSTR(p, "vpiStructVar");
            init_properties(&props);
            props.name = "s_p1[0]";
            props.fullname = "top.t.s_p1[0]";
            props.size = 4;
            props.packedArrayMember = true;
            CHECK_PROPERTIES(vhs_p1_idx0, &props);

            TestVpiHandle vh_iter_struct = vpi_iterate(vpiMember, vhs_p1_idx0);
            CHECK_RESULT_NZ(vh_iter_struct);
            p = vpi_get_str(vpiType, vh_iter_struct);
            CHECK_RESULT_CSTR(p, "vpiIterator");

            TestVpiHandle vhs_p1_idx0_field0 = vpi_scan(vh_iter_struct);
            CHECK_RESULT_NZ(vhs_p1_idx0_field0);
            p = vpi_get_str(vpiType, vhs_p1_idx0_field0);
            CHECK_RESULT_CSTR(p, "vpiReg");
            init_properties(&props);
            props.name = "s_p1[0].field0";
            props.fullname = "top.t.s_p1[0].field0";
            props.size = 4;
            props.structMember = true;
            CHECK_PROPERTIES(vhs_p1_idx0_field0, &props);
        }
    }
    
    TestVpiHandle vhs_p12 = VPI_HANDLE("s_p12");
    CHECK_RESULT_NZ(vhs_p12);
    {
        p = vpi_get_str(vpiType, vhs_p12);
        CHECK_RESULT_CSTR(p, "vpiPackedArrayVar");
        init_properties(&props);
        props.vector = true;
        props.name = "s_p12";
        props.fullname = "top.t.s_p12";
        props.size = 24;
        CHECK_PROPERTIES(vhs_p12, &props);

        // XXX: Disagree with ModelSim
        if (!TestSimulator::is_mti()) {
            TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_p12);
            CHECK_RESULT_NZ(vh_iter);
            TestVpiHandle vhs_p12_idx0 = vpi_scan(vh_iter);
            CHECK_RESULT_NZ(vhs_p12_idx0);
            p = vpi_get_str(vpiType, vhs_p12_idx0);
            // Does not work with ModelSim, returns vpiReg, see 37.18, detail 3
            CHECK_RESULT_CSTR(p, "vpiPackedArrayVar");
            init_properties(&props);
            props.name = "s_p12[0]";
            props.fullname = "top.t.s_p12[0]";
            props.size = 12;
            props.packedArrayMember = true;
            CHECK_PROPERTIES(vhs_p12_idx0, &props);

            TestVpiHandle vh_iter_inner = vpi_iterate(vpiMember, vhs_p12_idx0);
            CHECK_RESULT_NZ(vh_iter_inner);

            TestVpiHandle vhs_p12_idx0_idx0 = vpi_scan(vh_iter_inner);
            CHECK_RESULT_NZ(vhs_p12_idx0_idx0);
            p = vpi_get_str(vpiType, vhs_p12_idx0_idx0);
            CHECK_RESULT_CSTR(p, "vpiStructVar");
            init_properties(&props);
            props.name = "s_p12[0][0]";
            props.fullname = "top.t.s_p12[0][0]";
            props.size = 4;
            props.packedArrayMember = true;
            CHECK_PROPERTIES(vhs_p12_idx0_idx0, &props);
        }
    }
    TestVpiHandle vhs_u2 = VPI_HANDLE("s_u2");
    CHECK_RESULT_NZ(vhs_u2);
    {
        p = vpi_get_str(vpiType, vhs_u2);
        CHECK_RESULT_CSTR2(p, "vpiArrayVar", "vpiRegArray");
        init_properties(&props);
        props.name = "s_u2";
        props.fullname = "top.t.s_u2";
        props.size = 3;
        props.array = true;
        props.arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vhs_u2, &props);
        init_acc(&acc);
        acc.module = "t";
        acc.scope = "t";
        acc.typespec = vpiArrayTypespec;
        acc.typespecName = "struct_test";
        CHECK_ACCESSORS(vhs_u2, &acc);

        TestVpiHandle vhs_u2_idx1 = vpi_handle_by_index(vhs_u2, 1);
        CHECK_RESULT_NZ(vhs_u2_idx1);
        p = vpi_get_str(vpiType, vhs_u2_idx1);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        init_properties(&props);
        props.name = "s_u2[1]";
        props.fullname = "top.t.s_u2[1]";
        props.size = 4;
        props.arrayMember = true;
        CHECK_PROPERTIES(vhs_u2_idx1, &props);

        TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_u2_idx1);
        CHECK_RESULT_NZ(vh_iter);
        TestVpiHandle vhs_u2_idx1_f0 = vpi_scan(vh_iter);
        CHECK_RESULT_NZ(vhs_u2_idx1_f0);
        p = vpi_get_str(vpiType, vhs_u2_idx1_f0);
        CHECK_RESULT_CSTR2(p, "vpiReg", "vpiLogicVar");
        init_properties(&props);
        props.name = "s_u2[1].s_field";
        props.fullname = "top.t.s_u2[1].s_field";
        props.size = 1;
        props.structMember = true;
        CHECK_PROPERTIES(vhs_u2_idx1_f0, &props);
    }
    if (!TestSimulator::is_mti()) {
        // XXX: ModelSim things that this is a 8 elements array...
        TestVpiHandle vhs_p3u1 = VPI_HANDLE("s_p3u1");
        CHECK_RESULT_NZ(vhs_p3u1);
        {
            p = vpi_get_str(vpiType, vhs_p3u1);
            CHECK_RESULT_CSTR2(p, "vpiArrayVar", "vpiRegArray");
            init_properties(&props);
            props.name = "s_p3u1";
            props.fullname = "top.t.s_p3u1";
            props.size = 2;
            props.array = true;
            props.arrayType = vpiStaticArray;
            CHECK_PROPERTIES(vhs_p3u1, &props);

            TestVpiHandle vhs_p3u1_idx1 = vpi_handle_by_index(vhs_p3u1, 1);
            CHECK_RESULT_NZ(vhs_p3u1_idx1);
            p = vpi_get_str(vpiType, vhs_p3u1_idx1);
            CHECK_RESULT_CSTR(p, "vpiPackedArrayVar");
            init_properties(&props);
            props.name = "s_p3u1[1]";
            props.fullname = "top.t.s_p3u1[1]";
            props.size = 16;
            props.arrayMember = true;
            CHECK_PROPERTIES(vhs_p3u1_idx1, &props);

            TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_p3u1_idx1);
            CHECK_RESULT_NZ(vh_iter);
            TestVpiHandle vhs_p3u1_idx1_idx0 = vpi_scan(vh_iter);
            CHECK_RESULT_NZ(vhs_p3u1_idx1_idx0);
            p = vpi_get_str(vpiType, vhs_p3u1_idx1_idx0);
            CHECK_RESULT_CSTR(p, "vpiStructVar");
            init_properties(&props);
            props.name = "s_p3u1[1][0]";
            props.fullname = "top.t.s_p3u1[1][0]";
            props.size = 4;
            props.packedArrayMember = true;
            CHECK_PROPERTIES(vhs_p3u1_idx1_idx0, &props);
        }
    }
    TestVpiHandle vhu = VPI_HANDLE("u");
    CHECK_RESULT_NZ(vhu);
    {
        p = vpi_get_str(vpiType, vhu);
        CHECK_RESULT_CSTR(p, "vpiUnionVar");
        init_properties(&props);
        props.name = "u";
        props.fullname = "top.t.u";
        props.size = 4;
        CHECK_PROPERTIES(vhu, &props);

        TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhu);
        CHECK_RESULT_NZ(vh_iter);
        TestVpiHandle vhu_field0 = vpi_scan(vh_iter);
        CHECK_RESULT_NZ(vhu_field0);
        p = vpi_get_str(vpiType, vhu_field0);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        init_properties(&props);
        props.name = "u.field0";
        props.fullname = "top.t.u.field0";
        props.size = 4;
        props.structMember = true;
        CHECK_PROPERTIES(vhu_field0, &props);
        TestVpiHandle vhu_field1 = vpi_scan(vh_iter);
        CHECK_RESULT_NZ(vhu_field1);
        p = vpi_get_str(vpiType, vhu_field1);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        init_properties(&props);
        props.name = "u.field1";
        props.fullname = "top.t.u.field1";
        props.size = 4;
        props.structMember = true;
        CHECK_PROPERTIES(vhu_field1, &props);
    }

    TestVpiHandle vhint = VPI_HANDLE("int_var");
    CHECK_RESULT_NZ(vhint);
    {
        p = vpi_get_str(vpiType, vhint);
        CHECK_RESULT_CSTR(p, "vpiIntVar");
        init_properties(&props);
        props.size = 32;
        props.scalar = true;
        CHECK_PROPERTIES(vhint, &props);
    }

    TestVpiHandle vhinteger = VPI_HANDLE("integer_var");
    CHECK_RESULT_NZ(vhinteger);
    {
        p = vpi_get_str(vpiType, vhinteger);
        CHECK_RESULT_CSTR(p, "vpiIntegerVar");
        init_properties(&props);
        props.size = 32;
        props.scalar = true;
        CHECK_PROPERTIES(vhinteger, &props);
    }

    TestVpiHandle vhshortint = VPI_HANDLE("shortint_var");
    CHECK_RESULT_NZ(vhshortint);
    {
        p = vpi_get_str(vpiType, vhshortint);
        CHECK_RESULT_CSTR(p, "vpiShortIntVar");
        init_properties(&props);
        props.size = 16;
        props.scalar = true;
        CHECK_PROPERTIES(vhshortint, &props);
    }

    TestVpiHandle vhbyte = VPI_HANDLE("byte_var");
    CHECK_RESULT_NZ(vhbyte);
    {
        p = vpi_get_str(vpiType, vhbyte);
        CHECK_RESULT_CSTR(p, "vpiByteVar");
        init_properties(&props);
        props.size = 8;
        props.scalar = true;
        CHECK_PROPERTIES(vhbyte, &props);
    }

    TestVpiHandle vhreal = VPI_HANDLE("real_var");
    CHECK_RESULT_NZ(vhreal);
    {
        p = vpi_get_str(vpiType, vhreal);
        CHECK_RESULT_CSTR(p, "vpiRealVar");
        init_properties(&props);
        props.size = 64;
        props.scalar = true;
        CHECK_PROPERTIES(vhreal, &props);
    }

    TestVpiHandle vhlongint = VPI_HANDLE("longint_var");
    CHECK_RESULT_NZ(vhlongint);
    {
        p = vpi_get_str(vpiType, vhlongint);
        CHECK_RESULT_CSTR(p, "vpiLongIntVar");
        init_properties(&props);
        props.size = 64;
        props.scalar = true;
        CHECK_PROPERTIES(vhlongint, &props);
    }

    TestVpiHandle vhstring = VPI_HANDLE("string_var");
    CHECK_RESULT_NZ(vhstring);
    {
        p = vpi_get_str(vpiType, vhstring);
        CHECK_RESULT_CSTR(p, "vpiStringVar");
        init_properties(&props);
        props.size = 13;
        CHECK_PROPERTIES(vhstring, &props);
    }

    return 0;
}

int mon_check() {
    if (int status = _mon_check_arr()) return status;
    if (int status = _mon_check_structs()) return status;
#ifndef IS_VPI
    VerilatedVpi::selfTest();
#endif
    return 0;  // Ok
}

//======================================================================

#ifdef IS_VPI

static int mon_check_vpi() {
    TestVpiHandle href = vpi_handle(vpiSysTfCall, 0);
    s_vpi_value vpi_value;

    vpi_value.format = vpiIntVal;
    vpi_value.value.integer = mon_check();
    vpi_put_value(href, &vpi_value, NULL, vpiNoDelay);

    return 0;
}

static s_vpi_systf_data vpi_systf_data[] = {{vpiSysFunc, vpiIntFunc, (PLI_BYTE8*)"$mon_check",
                                             (PLI_INT32(*)(PLI_BYTE8*))mon_check_vpi, 0, 0, 0},
                                            0};

// cver entry
void vpi_compat_bootstrap(void) {
    p_vpi_systf_data systf_data_p;
    systf_data_p = &(vpi_systf_data[0]);
    while (systf_data_p->type != 0) vpi_register_systf(systf_data_p++);
}

// icarus entry
void (*vlog_startup_routines[])() = {vpi_compat_bootstrap, 0};

#else

double sc_time_stamp() { return main_time; }
int main(int argc, char** argv, char** env) {
    double sim_time = 1100;
    Verilated::commandArgs(argc, argv);

    VM_PREFIX* topp = new VM_PREFIX("");  // Note null name - we're flattening it out

#ifdef VERILATOR
#ifdef TEST_VERBOSE
    Verilated::scopesDump();
#endif
#endif

#if VM_TRACE
    Verilated::traceEverOn(true);
    VL_PRINTF("Enabling waves...\n");
    VerilatedVcdC* tfp = new VerilatedVcdC;
    topp->trace(tfp, 99);
    tfp->open(STRINGIFY(TEST_OBJ_DIR) "/simx.vcd");
#endif

    topp->eval();
    topp->clk = 0;
    main_time += 10;

    while (sc_time_stamp() < sim_time && !Verilated::gotFinish()) {
        main_time += 1;
        topp->eval();
        VerilatedVpi::callValueCbs();
        topp->clk = !topp->clk;
        // mon_do();
#if VM_TRACE
        if (tfp) tfp->dump(main_time);
#endif
    }
    CHECK_RESULT(callback_count, 501);
    CHECK_RESULT(callback_count_half, 250);
    CHECK_RESULT(callback_count_quad, 2);
    CHECK_RESULT(callback_count_strs, callback_count_strs_max);
    if (!Verilated::gotFinish()) {
        vl_fatal(FILENM, __LINE__, "main", "%Error: Timeout; never got a $finish");
    }
    topp->final();

#if VM_TRACE
    if (tfp) tfp->close();
#endif

    VL_DO_DANGLING(delete topp, topp);
    exit(0L);
}

#endif
