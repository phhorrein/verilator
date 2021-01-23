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

#include "Vt_vpi_var_model_logic.h"
#include "verilated.h"
#include "svdpi.h"

#include "Vt_vpi_var_model_logic__Dpi.h"

#include "verilated_vpi.h"
#include "verilated_vcd_c.h"

#endif

#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <iostream>

// __FILE__ is too long
#define FILENM "t_vpi_var_model_logic.cpp"

#include "TestSimulator.h"
#include "TestVpi.h"

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

int mon_check() {

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
    VpiVarProps* props = new VpiVarProps();
    VpiVarAcc* acc = new VpiVarAcc();
    // Single logic/reg
    TestVpiHandle vh_onereg = VPI_HANDLE("onereg");
    CHECK_RESULT_NZ(vh_onereg);
    {
        p = vpi_get_str(vpiType, vh_onereg);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // Single bit: vector is false, scalar is true, size is 1
        props->reset();
        props->m_scalar = true;
        props->m_name = "onereg";
        props->m_fullname = "top.t.onereg";
        CHECK_PROPERTIES(vh_onereg, props);
        // a is initialized to 0
        TestVpiHandle iter = vpi_handle(vpiBit, vh_onereg);  // 37.17, detail 12
        CHECK_RESULT(iter, 0);  // According to standard, it should give !NULL, but MS gives NULL
                                // (and it sounds reasonable)
        TestVpiHandle i = vpi_handle(vpiExpr, vh_onereg);  // 37.17, detail 8
        CHECK_RESULT(i, 0);  // According to standard, it should give !NULL, but MS gives NULL (and
                             // it is a bug)
    }
    // Single logic/reg
    TestVpiHandle vh_onebit = VPI_HANDLE("onebit");
    CHECK_RESULT_NZ(vh_onebit);
    {
        p = vpi_get_str(vpiType, vh_onebit);
        CHECK_RESULT_CSTR(p, "vpiBitVar");
        // Single bit: vector is false, scalar is true, size is 1
        props->reset();
        props->m_scalar = true;
        props->m_name = "onebit";
        props->m_fullname = "top.t.onebit";
        CHECK_PROPERTIES(vh_onebit, props);
    }
    // One-dimensional packed array
    TestVpiHandle vha_p0 = VPI_HANDLE("a_p0");
    CHECK_RESULT_NZ(vha_p0);
    {
        // FIXME: add vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vha_p0);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // Single bit, but packed dimension: vector is true, scalar is false, size is 1
        props->reset();
        props->m_vector = true;
        props->m_name = "a_p0";
        props->m_fullname = "top.t.a_p0";
        CHECK_PROPERTIES(vha_p0, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        acc->m_typespec = vpiLogicTypespec;
        CHECK_ACCESSORS(vha_p0, acc);
        v.format = vpiIntVal;
        v.value.integer = 1;
        vpi_put_value(vha_p0, &v, &t, vpiNoDelay);
    }
    TestVpiHandle vha_p1 = VPI_HANDLE("a_p1");
    CHECK_RESULT_NZ(vha_p1);
    {
        // FIXME: add vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vha_p1);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // 2 bits packed: vector is true, scalar is false, size is 1
        props->reset();
        props->m_vector = true;
        props->m_size = 2;
        props->m_name = "a_p1";
        props->m_fullname = "top.t.a_p1";
        CHECK_PROPERTIES(vha_p1, props);
        TestVpiHandle vha_p1_idx1 = vpi_handle_by_index(vha_p1, 1);
        CHECK_RESULT_NZ(vha_p1_idx1);
    }
    TestVpiHandle vha_sp1 = VPI_HANDLE("a_sp1");
    CHECK_RESULT_NZ(vha_sp1);
    {
        // FIXME: add vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vha_sp1);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // 2 bits packed: vector is true, scalar is false, size is 1
        props->reset();
        props->m_vector = true;
        props->m_signedVar = true;
        props->m_size = 2;
        props->m_name = "a_sp1";
        props->m_fullname = "top.t.a_sp1";
        CHECK_PROPERTIES(vha_sp1, props);
        TestVpiHandle vha_sp1_idx1 = vpi_handle_by_index(vha_sp1, 1);
        CHECK_RESULT_NZ(vha_sp1_idx1);
    }
    TestVpiHandle vha_p21 = VPI_HANDLE("a_p21");
    CHECK_RESULT_NZ(vha_p21);
    {
        p = vpi_get_str(vpiType, vha_p21);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        // 2 bits packed: vector is true, scalar is false, size is 1
        props->reset();
        props->m_vector = true;
        props->m_size = 6;
        props->m_name = "a_p21";
        props->m_fullname = "top.t.a_p21";
        CHECK_PROPERTIES(vha_p21, props);
        TestVpiHandle vha_p21_idx1 = vpi_handle_by_index(vha_p21, 1);
        CHECK_RESULT_NZ(vha_p21_idx1);
        p = vpi_get_str(vpiType, vha_p21_idx1);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        props->reset();
        props->m_vector = true;
        props->m_size = 2;
        props->m_name = "a_p21[1]";
        props->m_fullname = "top.t.a_p21[1]";
        CHECK_PROPERTIES(vha_p21_idx1, props);
        TestVpiHandle vha_p21_idx1_idx0 = vpi_handle_by_index(vha_p21_idx1, 0);
        CHECK_RESULT_NZ(vha_p21_idx1_idx0);
        p = vpi_get_str(vpiType, vha_p21_idx1_idx0);
        CHECK_RESULT_CSTR(p, "vpiRegBit");
        props->reset();
        props->m_scalar = true;
        props->m_name = "a_p21[1][0]";
        props->m_fullname = "top.t.a_p21[1][0]";
        CHECK_PROPERTIES(vha_p21_idx1_idx0, props);
    }
    TestVpiHandle vha_u0 = VPI_HANDLE("a_u0");
    CHECK_RESULT_NZ(vha_u0);
    {
        // FIXME: add vpiReg/vpiParent access
        p = vpi_get_str(vpiType, vha_u0);
        CHECK_RESULT_CSTR(p, "vpiArrayVar");
        props->reset();
        props->m_scalar = true;
        props->m_array = true;
        props->m_name = "a_u0";
        props->m_fullname = "top.t.a_u0";
        props->m_arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vha_u0, props);
        TestVpiHandle vha_u0_idx0 = vpi_handle_by_index(vha_u0, 0);
        CHECK_RESULT_NZ(vha_u0_idx0);
        p = vpi_get_str(vpiType, vha_u0_idx0);
        CHECK_RESULT_CSTR(p, "vpiReg");
        props->reset();
        props->m_scalar = true;
        props->m_arrayMember = true;
        props->m_name = "a_u0[0]";
        props->m_fullname = "top.t.a_u0[0]";
        CHECK_PROPERTIES(vha_u0_idx0, props);
        v.format = vpiIntVal;
        v.value.integer = 1;
        vpi_put_value(vha_u0_idx0, &v, &t, vpiNoDelay);
    }
    TestVpiHandle vha_u2 = VPI_HANDLE("a_u2");
    CHECK_RESULT_NZ(vha_u0);
    {
        // FIXME: add vpiReg/vpiParent access
        p = vpi_get_str(vpiType, vha_u2);
        CHECK_RESULT_CSTR(p, "vpiArrayVar");
        props->reset();
        props->m_scalar = true;
        props->m_size = 3;
        props->m_array = true;
        props->m_name = "a_u2";
        props->m_fullname = "top.t.a_u2";
        props->m_arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vha_u2, props);
        TestVpiHandle vha_u2_idx2 = vpi_handle_by_index(vha_u2, 2);
        CHECK_RESULT_NZ(vha_u2_idx2);
        p = vpi_get_str(vpiType, vha_u2_idx2);
        CHECK_RESULT_CSTR(p, "vpiReg");
        props->reset();
        props->m_scalar = true;
        props->m_arrayMember = true;
        props->m_name = "a_u2[2]";
        props->m_fullname = "top.t.a_u2[2]";
        CHECK_PROPERTIES(vha_u2_idx2, props);
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
    TestVpiHandle vha_p1u1 = VPI_HANDLE("a_p1u1");
    CHECK_RESULT_NZ(vha_p1u1);
    {
        // FIXME: add vpiReg/vpiParent access and vpiBit/vpiParent
        p = vpi_get_str(vpiType, vha_p1u1);
        CHECK_RESULT_CSTR(p, "vpiArrayVar");
        props->reset();
        props->m_vector = true;
        props->m_size = 2;
        props->m_array = true;
        props->m_name = "a_p1u1";
        props->m_fullname = "top.t.a_p1u1";
        props->m_arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vha_p1u1, props);
        TestVpiHandle vha_p1u1_idx0 = vpi_handle_by_index(vha_p1u1, 0);
        CHECK_RESULT_NZ(vha_p1u1_idx0);
        p = vpi_get_str(vpiType, vha_p1u1_idx0);
        CHECK_RESULT_CSTR(p, "vpiReg");
        props->reset();
        props->m_vector = true;
        props->m_size = 2;
        props->m_arrayMember = true;
        props->m_name = "a_p1u1[0]";
        props->m_fullname = "top.t.a_p1u1[0]";
        CHECK_PROPERTIES(vha_p1u1_idx0, props);
    }
    TestVpiHandle vha_p0u12 = VPI_HANDLE("a_p0u12");
    CHECK_RESULT_NZ(vha_p0u12);
    {
        // FIXME: add vpiReg/vpiParent access and vpiBit/vpiParent
        p = vpi_get_str(vpiType, vha_p0u12);
        CHECK_RESULT_CSTR2(p, "vpiArrayVar", "vpiRegArray");
        props->reset();
        props->m_vector = false;
        props->m_size = 6;
        props->m_array = true;
        props->m_name = "a_p0u12";
        props->m_fullname = "top.t.a_p0u12";
        props->m_arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vha_p0u12, props);
        // XXX: disagree with ModelSim
        if (!TestSimulator::is_mti()) {
            // ModelSim is not able to find this...
            TestVpiHandle vha_p0u12_idx0 = vpi_handle_by_index(vha_p0u12, 0);
            CHECK_RESULT_NZ(vha_p0u12_idx0);
            p = vpi_get_str(vpiType, vha_p0u12_idx0);
            CHECK_RESULT_CSTR2(p, "vpiArrayVar", "vpiRegArray");
            props->reset();
            props->m_vector = true;
            props->m_size = 3;
            props->m_arrayMember = true;
            props->m_name = "a_p0u12[0]";
            props->m_fullname = "top.t.a_p0u12[0]";
            CHECK_PROPERTIES(vha_p0u12_idx0, props);
        }
    }

#ifndef IS_VPI
    VerilatedVpi::selfTest();
#endif

    return 0;
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
