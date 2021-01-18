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

#include "Vt_vpi_var_model_structs.h"
#include "verilated.h"
#include "svdpi.h"

#include "Vt_vpi_var_model_structs__Dpi.h"

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
#define FILENM "t_vpi_var_model_structs.cpp"

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
    VpiVarProps *props = new VpiVarProps();
    VpiVarAcc *acc = new VpiVarAcc();

    TestVpiHandle vhs = VPI_HANDLE("s");
    CHECK_RESULT_NZ(vhs);
    {
        // FIXME: add vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vhs);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        props->reset();
        props->m_name = "s";
        props->m_fullname = "top.t.s";
        props->m_size = 4;
        CHECK_PROPERTIES(vhs, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        acc->m_typespec = vpiStructTypespec;
        acc->m_typespecName = "struct_test";
        CHECK_ACCESSORS(vhs, acc);

        TestVpiHandle vhs_iter = vpi_iterate(vpiMember, vhs);
        CHECK_RESULT_NZ(vhs_iter);
        p = vpi_get_str(vpiType, vhs_iter);
        CHECK_RESULT_CSTR(p, "vpiIterator");
        
        TestVpiHandle vhs_field0 = vpi_scan(vhs_iter);
        CHECK_RESULT_NZ(vhs_field0);
        p = vpi_get_str(vpiType, vhs_field0);
        CHECK_RESULT_CSTR(p, "vpiReg");
        props->reset();
        props->m_scalar = true;
        props->m_name = "s.s_field";
        props->m_fullname = "top.t.s.s_field";
        props->m_size = 1;
        props->m_structMember = true;
        CHECK_PROPERTIES(vhs_field0, props);
        
        TestVpiHandle vhs_field1 = vpi_scan(vhs_iter);
        CHECK_RESULT_NZ(vhs_field1);
        p = vpi_get_str(vpiType, vhs_field1);
        CHECK_RESULT_CSTR(p, "vpiReg");
        props->reset();
        props->m_vector = true;
        props->m_name = "s.p_field";
        props->m_fullname = "top.t.s.p_field";
        props->m_size = 3;
        props->m_structMember = true;
        CHECK_PROPERTIES(vhs_field1, props);

        TestVpiHandle vhs_field1_idx0 = vpi_handle_by_index(vhs_field1, 0);
        CHECK_RESULT_NZ(vhs_field1_idx0);
        p = vpi_get_str(vpiType, vhs_field1_idx0);
        CHECK_RESULT_CSTR(p, "vpiRegBit");
        props->reset();
        props->m_scalar = true;
        props->m_name = "s.p_field[0]";
        props->m_fullname = "top.t.s.p_field[0]";
        props->m_size = 1;
        CHECK_PROPERTIES(vhs_field1_idx0, props);

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
        props->reset();
        props->m_name = "s";
        props->m_fullname = "top.t.s";
        props->m_size = 4;
        CHECK_PROPERTIES(vhs_f0_par, props);
    }

    TestVpiHandle vhs_emb = VPI_HANDLE("s_emb");
    CHECK_RESULT_NZ(vhs_emb);
    {
        p = vpi_get_str(vpiType, vhs_emb);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        props->reset();
        props->m_name = "s_emb";
        props->m_fullname = "top.t.s_emb";
        props->m_size = 11;
        CHECK_PROPERTIES(vhs_emb, props);
        TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_emb);
        CHECK_RESULT_NZ(vh_iter);
        p = vpi_get_str(vpiType, vh_iter);
        CHECK_RESULT_CSTR(p, "vpiIterator");
        TestVpiHandle vhs_emb_field0 = vpi_scan(vh_iter);
        CHECK_RESULT_NZ(vhs_emb_field0);
        p = vpi_get_str(vpiType, vhs_emb_field0);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        props->reset();
        props->m_name = "s_emb.field0";
        props->m_fullname = "top.t.s_emb.field0";
        props->m_size = 4;
        props->m_structMember = true;
        CHECK_PROPERTIES(vhs_emb_field0, props);
        TestVpiHandle vh_f0_iter = vpi_iterate(vpiMember, vhs_emb_field0);
        CHECK_RESULT_NZ(vh_f0_iter);
        TestVpiHandle vhs_emb_field0_sfield = vpi_scan(vh_f0_iter);
        CHECK_RESULT_NZ(vhs_emb_field0_sfield);
        p = vpi_get_str(vpiType, vhs_emb_field0_sfield);
        CHECK_RESULT_CSTR(p, "vpiReg");
        props->reset();
        props->m_scalar = true;
        props->m_name = "s_emb.field0.s_field";
        props->m_fullname = "top.t.s_emb.field0.s_field";
        props->m_size = 1;
        props->m_structMember = true;
        CHECK_PROPERTIES(vhs_emb_field0_sfield, props);
    }
    
    TestVpiHandle vhs_p1 = VPI_HANDLE("s_p1");
    CHECK_RESULT_NZ(vhs_p1);
    {
        // FIXME: missing vpiBit/vpiParent access
        p = vpi_get_str(vpiType, vhs_p1);
        CHECK_RESULT_CSTR(p, "vpiPackedArrayVar");
        props->reset();
        props->m_vector = true;
        props->m_name = "s_p1";
        props->m_fullname = "top.t.s_p1";
        props->m_size = 8;
        CHECK_PROPERTIES(vhs_p1, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        acc->m_typespec = vpiPackedArrayTypespec;
        acc->m_typespecName = "struct_test";
        CHECK_ACCESSORS(vhs_p1, acc);

        // XXX: disagree with ModelSim
        if (!TestSimulator::is_mti()) {
            TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_p1);
            CHECK_RESULT_NZ(vh_iter);
            TestVpiHandle vhs_p1_idx0 = vpi_scan(vh_iter);
            CHECK_RESULT_NZ(vhs_p1_idx0);
            p = vpi_get_str(vpiType, vhs_p1_idx0);
            // Does not work with ModelSim, returns vpiReg, see 37.18, detail 3
            CHECK_RESULT_CSTR(p, "vpiStructVar");
            props->reset();
            props->m_name = "s_p1[0]";
            props->m_fullname = "top.t.s_p1[0]";
            props->m_size = 4;
            props->m_packedArrayMember = true;
            CHECK_PROPERTIES(vhs_p1_idx0, props);

            TestVpiHandle vh_iter_struct = vpi_iterate(vpiMember, vhs_p1_idx0);
            CHECK_RESULT_NZ(vh_iter_struct);
            p = vpi_get_str(vpiType, vh_iter_struct);
            CHECK_RESULT_CSTR(p, "vpiIterator");

            TestVpiHandle vhs_p1_idx0_field0 = vpi_scan(vh_iter_struct);
            CHECK_RESULT_NZ(vhs_p1_idx0_field0);
            p = vpi_get_str(vpiType, vhs_p1_idx0_field0);
            CHECK_RESULT_CSTR(p, "vpiReg");
            props->reset();
            props->m_name = "s_p1[0].field0";
            props->m_fullname = "top.t.s_p1[0].field0";
            props->m_size = 4;
            props->m_structMember = true;
            CHECK_PROPERTIES(vhs_p1_idx0_field0, props);
        }
    }
    
    TestVpiHandle vhs_p12 = VPI_HANDLE("s_p12");
    CHECK_RESULT_NZ(vhs_p12);
    {
        p = vpi_get_str(vpiType, vhs_p12);
        CHECK_RESULT_CSTR(p, "vpiPackedArrayVar");
        props->reset();
        props->m_vector = true;
        props->m_name = "s_p12";
        props->m_fullname = "top.t.s_p12";
        props->m_size = 24;
        CHECK_PROPERTIES(vhs_p12, props);

        // XXX: Disagree with ModelSim
        if (!TestSimulator::is_mti()) {
            TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_p12);
            CHECK_RESULT_NZ(vh_iter);
            TestVpiHandle vhs_p12_idx0 = vpi_scan(vh_iter);
            CHECK_RESULT_NZ(vhs_p12_idx0);
            p = vpi_get_str(vpiType, vhs_p12_idx0);
            // Does not work with ModelSim, returns vpiReg, see 37.18, detail 3
            CHECK_RESULT_CSTR(p, "vpiPackedArrayVar");
            props->reset();
            props->m_name = "s_p12[0]";
            props->m_fullname = "top.t.s_p12[0]";
            props->m_size = 12;
            props->m_packedArrayMember = true;
            CHECK_PROPERTIES(vhs_p12_idx0, props);

            TestVpiHandle vh_iter_inner = vpi_iterate(vpiMember, vhs_p12_idx0);
            CHECK_RESULT_NZ(vh_iter_inner);

            TestVpiHandle vhs_p12_idx0_idx0 = vpi_scan(vh_iter_inner);
            CHECK_RESULT_NZ(vhs_p12_idx0_idx0);
            p = vpi_get_str(vpiType, vhs_p12_idx0_idx0);
            CHECK_RESULT_CSTR(p, "vpiStructVar");
            props->reset();
            props->m_name = "s_p12[0][0]";
            props->m_fullname = "top.t.s_p12[0][0]";
            props->m_size = 4;
            props->m_packedArrayMember = true;
            CHECK_PROPERTIES(vhs_p12_idx0_idx0, props);
        }
    }
    TestVpiHandle vhs_u2 = VPI_HANDLE("s_u2");
    CHECK_RESULT_NZ(vhs_u2);
    {
        p = vpi_get_str(vpiType, vhs_u2);
        CHECK_RESULT_CSTR2(p, "vpiArrayVar", "vpiRegArray");
        props->reset();
        props->m_name = "s_u2";
        props->m_fullname = "top.t.s_u2";
        props->m_size = 3;
        props->m_array = true;
        props->m_arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vhs_u2, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        acc->m_typespec = vpiArrayTypespec;
        acc->m_typespecName = "struct_test";
        CHECK_ACCESSORS(vhs_u2, acc);

        TestVpiHandle vhs_u2_idx1 = vpi_handle_by_index(vhs_u2, 1);
        CHECK_RESULT_NZ(vhs_u2_idx1);
        p = vpi_get_str(vpiType, vhs_u2_idx1);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        props->reset();
        props->m_name = "s_u2[1]";
        props->m_fullname = "top.t.s_u2[1]";
        props->m_size = 4;
        props->m_arrayMember = true;
        CHECK_PROPERTIES(vhs_u2_idx1, props);

        TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_u2_idx1);
        CHECK_RESULT_NZ(vh_iter);
        TestVpiHandle vhs_u2_idx1_f0 = vpi_scan(vh_iter);
        CHECK_RESULT_NZ(vhs_u2_idx1_f0);
        p = vpi_get_str(vpiType, vhs_u2_idx1_f0);
        CHECK_RESULT_CSTR2(p, "vpiReg", "vpiLogicVar");
        props->reset();
        props->m_name = "s_u2[1].s_field";
        props->m_fullname = "top.t.s_u2[1].s_field";
        props->m_size = 1;
        props->m_structMember = true;
        CHECK_PROPERTIES(vhs_u2_idx1_f0, props);
    }
    if (!TestSimulator::is_mti()) {
        // XXX: ModelSim things that this is a 8 elements array...
        TestVpiHandle vhs_p3u1 = VPI_HANDLE("s_p3u1");
        CHECK_RESULT_NZ(vhs_p3u1);
        {
            p = vpi_get_str(vpiType, vhs_p3u1);
            CHECK_RESULT_CSTR2(p, "vpiArrayVar", "vpiRegArray");
            props->reset();
            props->m_name = "s_p3u1";
            props->m_fullname = "top.t.s_p3u1";
            props->m_size = 2;
            props->m_array = true;
            props->m_arrayType = vpiStaticArray;
            CHECK_PROPERTIES(vhs_p3u1, props);

            TestVpiHandle vhs_p3u1_idx1 = vpi_handle_by_index(vhs_p3u1, 1);
            CHECK_RESULT_NZ(vhs_p3u1_idx1);
            p = vpi_get_str(vpiType, vhs_p3u1_idx1);
            CHECK_RESULT_CSTR(p, "vpiPackedArrayVar");
            props->reset();
            props->m_name = "s_p3u1[1]";
            props->m_fullname = "top.t.s_p3u1[1]";
            props->m_size = 16;
            props->m_arrayMember = true;
            CHECK_PROPERTIES(vhs_p3u1_idx1, props);

            TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhs_p3u1_idx1);
            CHECK_RESULT_NZ(vh_iter);
            TestVpiHandle vhs_p3u1_idx1_idx0 = vpi_scan(vh_iter);
            CHECK_RESULT_NZ(vhs_p3u1_idx1_idx0);
            p = vpi_get_str(vpiType, vhs_p3u1_idx1_idx0);
            CHECK_RESULT_CSTR(p, "vpiStructVar");
            props->reset();
            props->m_name = "s_p3u1[1][0]";
            props->m_fullname = "top.t.s_p3u1[1][0]";
            props->m_size = 4;
            props->m_packedArrayMember = true;
            CHECK_PROPERTIES(vhs_p3u1_idx1_idx0, props);
        }
    }
    TestVpiHandle vhu = VPI_HANDLE("u");
    CHECK_RESULT_NZ(vhu);
    {
        p = vpi_get_str(vpiType, vhu);
        CHECK_RESULT_CSTR(p, "vpiUnionVar");
        props->reset();
        props->m_name = "u";
        props->m_fullname = "top.t.u";
        props->m_size = 4;
        CHECK_PROPERTIES(vhu, props);

        TestVpiHandle vh_iter = vpi_iterate(vpiMember, vhu);
        CHECK_RESULT_NZ(vh_iter);
        TestVpiHandle vhu_field0 = vpi_scan(vh_iter);
        CHECK_RESULT_NZ(vhu_field0);
        p = vpi_get_str(vpiType, vhu_field0);
        CHECK_RESULT_CSTR2(p, "vpiLogicVar", "vpiReg");
        props->reset();
        props->m_name = "u.field0";
        props->m_fullname = "top.t.u.field0";
        props->m_size = 4;
        props->m_structMember = true;
        CHECK_PROPERTIES(vhu_field0, props);
        TestVpiHandle vhu_field1 = vpi_scan(vh_iter);
        CHECK_RESULT_NZ(vhu_field1);
        p = vpi_get_str(vpiType, vhu_field1);
        CHECK_RESULT_CSTR(p, "vpiStructVar");
        props->reset();
        props->m_name = "u.field1";
        props->m_fullname = "top.t.u.field1";
        props->m_size = 4;
        props->m_structMember = true;
        CHECK_PROPERTIES(vhu_field1, props);
    }

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
