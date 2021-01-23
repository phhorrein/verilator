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

#include "Vt_vpi_scope.h"
#include "verilated.h"
#include "svdpi.h"

#include "Vt_vpi_scope__Dpi.h"

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
#define FILENM "t_vpi_scope.cpp"

#define TEST_MSG \
    if (0) printf

//======================================================================

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
    // First, let's iterate and find module "t"
    vpiHandle mod_iter = vpi_iterate(vpiModule, NULL);
    CHECK_RESULT_NZ(mod_iter);
    TestVpiHandle vh_top = vpi_scan(mod_iter);
    CHECK_RESULT_NZ(vh_top);
    p = vpi_get_str(vpiName, vh_top);
    CHECK_RESULT_CSTR(p, "top");
    TestVpiHandle top_iter = vpi_iterate(vpiModule, vh_top);
    CHECK_RESULT_NZ(top_iter);
    TestVpiHandle vh_t = vpi_scan(top_iter);
    CHECK_RESULT_NZ(vh_t);
    p = vpi_get_str(vpiName, vh_t);
    CHECK_RESULT_CSTR(p, "t");

    TestVpiHandle t_ports = vpi_iterate(vpiPort, vh_t);
    CHECK_RESULT_NZ(t_ports);
    TestVpiHandle vh_clk = vpi_scan(t_ports);
    CHECK_RESULT_NZ(vh_clk);
    p = vpi_get_str(vpiName, vh_clk);
    CHECK_RESULT_CSTR(p, "clk");

    {
        TestVpiHandle t_vars = vpi_iterate(vpiVariables, vh_t);
        CHECK_RESULT_NZ(t_vars);
        TestVpiHandle vh_top_var = vpi_scan(t_vars);
        CHECK_RESULT_NZ(vh_top_var);
        props->reset();
        props->m_name = "top_var";
        props->m_fullname = "top.t.top_var";
        props->m_scalar = true;
        CHECK_PROPERTIES(vh_top_var, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        CHECK_ACCESSORS(vh_top_var, acc);
        // gen_for_var is not in this scope
        TestVpiHandle vh_unreachable = vpi_handle_by_name((PLI_BYTE8 *)"gen_for_var", vh_t);
        CHECK_RESULT(vh_unreachable, 0);
    }

    TestVpiHandle t_scopes = vpi_iterate(vpiInternalScope, vh_t);
    CHECK_RESULT_NZ(t_scopes);
    TestVpiHandle vh_for_1 = vpi_scan(t_scopes);
    CHECK_RESULT_NZ(vh_for_1);
    p = vpi_get_str(vpiName, vh_for_1);
    CHECK_RESULT_CSTR(p, "named_for[1]");
    p = vpi_get_str(vpiType, vh_for_1);
    CHECK_RESULT_CSTR(p, "vpiGenScope");
    {
        TestVpiHandle t_vars = vpi_iterate(vpiVariables, vh_for_1);
        CHECK_RESULT_NZ(t_vars);
        TestVpiHandle vh_for_var = vpi_scan(t_vars);
        CHECK_RESULT_NZ(vh_for_var);
        props->reset();
        props->m_name = "gen_for_var";
        props->m_fullname = "top.t.named_for[1].gen_for_var";
        props->m_scalar = true;
        CHECK_PROPERTIES(vh_for_var, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "named_for[1]";
        CHECK_ACCESSORS(vh_for_var, acc);
        TestVpiHandle vh_direct_for_var = vpi_handle_by_name((PLI_BYTE8 *)"gen_for_var", vh_for_1);
        CHECK_RESULT_NZ(vh_direct_for_var);
        CHECK_PROPERTIES(vh_direct_for_var, props);
        CHECK_ACCESSORS(vh_direct_for_var, acc);

        TestVpiHandle t_inner_scope = vpi_iterate(vpiInternalScope, vh_for_1);
        CHECK_RESULT_NZ(t_inner_scope);
        TestVpiHandle vh_else = vpi_scan(t_inner_scope);
        CHECK_RESULT_NZ(vh_else);
        p = vpi_get_str(vpiName, vh_else);
        CHECK_RESULT_CSTR(p, "normal_assign");
        p = vpi_get_str(vpiFullName, vh_else);
        CHECK_RESULT_CSTR(p, "top.t.named_for[1].normal_assign");
        TestVpiHandle mod = vpi_handle(vpiModule, vh_else);
        CHECK_RESULT_NZ(mod);
        p = vpi_get_str(vpiName, mod);
        CHECK_RESULT_CSTR(p, "t");
        TestVpiHandle scope = vpi_handle(vpiScope, vh_else);
        CHECK_RESULT_NZ(scope);
        p = vpi_get_str(vpiName, scope);
        CHECK_RESULT_CSTR(p, "named_for[1]");
    }
    TestVpiHandle vh_for_0 = vpi_scan(t_scopes);
    CHECK_RESULT_NZ(vh_for_0);
    p = vpi_get_str(vpiName, vh_for_0);
    CHECK_RESULT_CSTR(p, "named_for[0]");
    {
        TestVpiHandle t_vars = vpi_iterate(vpiVariables, vh_for_0);
        CHECK_RESULT_NZ(t_vars);
        TestVpiHandle vh_for_var = vpi_scan(t_vars);
        CHECK_RESULT_NZ(vh_for_var);
        props->reset();
        props->m_name = "gen_for_var";
        props->m_fullname = "top.t.named_for[0].gen_for_var";
        props->m_scalar = true;
        CHECK_PROPERTIES(vh_for_var, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "named_for[0]";
        CHECK_ACCESSORS(vh_for_var, acc);
        TestVpiHandle vh_direct_for_var = vpi_handle_by_name((PLI_BYTE8 *)"gen_for_var", vh_for_0);
        CHECK_RESULT_NZ(vh_direct_for_var);
        CHECK_PROPERTIES(vh_direct_for_var, props);
        CHECK_ACCESSORS(vh_direct_for_var, acc);
        TestVpiHandle t_inner_scope = vpi_iterate(vpiInternalScope, vh_for_0);
        CHECK_RESULT_NZ(t_inner_scope);
        TestVpiHandle vh_if = vpi_scan(t_inner_scope);
        CHECK_RESULT_NZ(vh_if);
        p = vpi_get_str(vpiName, vh_if);
        CHECK_RESULT_CSTR(p, "useless_init");
        p = vpi_get_str(vpiFullName, vh_if);
        CHECK_RESULT_CSTR(p, "top.t.named_for[0].useless_init");
        TestVpiHandle mod = vpi_handle(vpiModule, vh_if);
        CHECK_RESULT_NZ(mod);
        p = vpi_get_str(vpiName, mod);
        CHECK_RESULT_CSTR(p, "t");
        TestVpiHandle scope = vpi_handle(vpiScope, vh_if);
        CHECK_RESULT_NZ(scope);
        p = vpi_get_str(vpiName, scope);
        CHECK_RESULT_CSTR(p, "named_for[0]");
    }

    TestVpiHandle vh_iter_net = vpi_iterate(vpiNet, vh_t);
    CHECK_RESULT_NZ(vh_iter_net);
    {
        TestVpiHandle vh_clk = vpi_scan(vh_iter_net);
        CHECK_RESULT_NZ(vh_clk);
        props->reset();
        props->m_name = "clk";
        props->m_fullname = "top.t.clk";
        props->m_scalar = true;
        CHECK_PROPERTIES(vh_clk, props);
        acc->reset();
        acc->m_module = "t";
        CHECK_ACCESSORS(vh_clk, acc);
        TestVpiHandle vh_top_net = vpi_scan(vh_iter_net);
        CHECK_RESULT_NZ(vh_top_net);
        props->reset();
        props->m_name = "top_net";
        props->m_fullname = "top.t.top_net";
        props->m_scalar = true;
        CHECK_PROPERTIES(vh_top_net, props);
        acc->reset();
        acc->m_module = "t";
        CHECK_ACCESSORS(vh_top_net, acc);
        if (!TestSimulator::is_mti()) {
            TestVpiHandle last = vpi_scan(vh_iter_net);
            CHECK_RESULT(last, 0);
        }
    }

    TestVpiHandle vh_iter_reg = vpi_iterate(vpiReg, vh_t);
    CHECK_RESULT_NZ(vh_iter_reg);
    {
        TestVpiHandle vh_top_var = vpi_scan(vh_iter_reg);
        CHECK_RESULT_NZ(vh_top_var);
        props->reset();
        props->m_name = "top_var";
        props->m_fullname = "top.t.top_var";
        props->m_scalar = true;
        CHECK_PROPERTIES(vh_top_var, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        CHECK_ACCESSORS(vh_top_var, acc);
        TestVpiHandle vh_gen_var = vpi_scan(vh_iter_reg);
        CHECK_RESULT_NZ(vh_gen_var);
        props->reset();
        props->m_name = "gen_var";
        props->m_fullname = "top.t.gen_var";
        props->m_scalar = true;
        CHECK_PROPERTIES(vh_gen_var, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        CHECK_ACCESSORS(vh_gen_var, acc);
        if (!TestSimulator::is_mti()) {
            TestVpiHandle last = vpi_scan(vh_iter_reg);
            CHECK_RESULT(last, 0);
        }
    }

    TestVpiHandle vh_iter_reg_array = vpi_iterate(vpiRegArray, vh_t);
    CHECK_RESULT_NZ(vh_iter_reg_array);
    {
        TestVpiHandle vh_top_array = vpi_scan(vh_iter_reg_array);
        CHECK_RESULT_NZ(vh_top_array);
        props->reset();
        props->m_name = "top_array";
        props->m_fullname = "top.t.top_array";
        props->m_array = true;
        props->m_size = 2;
        props->m_arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vh_top_array, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        acc->m_typespec = vpiArrayTypespec;
        CHECK_ACCESSORS(vh_top_array, acc);
    }

    TestVpiHandle vh_iter_memory = vpi_iterate(vpiMemory, vh_t);
    CHECK_RESULT_NZ(vh_iter_memory);
    {
        TestVpiHandle vh_top_array = vpi_scan(vh_iter_memory);
        CHECK_RESULT_NZ(vh_top_array);
        props->reset();
        props->m_name = "top_array";
        props->m_fullname = "top.t.top_array";
        props->m_array = true;
        props->m_size = 2;
        props->m_arrayType = vpiStaticArray;
        CHECK_PROPERTIES(vh_top_array, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        acc->m_typespec = vpiArrayTypespec;
        CHECK_ACCESSORS(vh_top_array, acc);
    }

    TestVpiHandle vh_iter_param = vpi_iterate(vpiParameter, vh_t);
    CHECK_RESULT_NZ(vh_iter_param);
    {
        TestVpiHandle vh_top_param = vpi_scan(vh_iter_param);
        CHECK_RESULT_NZ(vh_top_param);
        props->reset();
        props->m_name = "TOP_PARAM";
        props->m_fullname = "top.t.TOP_PARAM";
        props->m_scalar = true;
        props->m_size = 32;
        CHECK_PROPERTIES(vh_top_param, props);
        acc->reset();
        acc->m_module = "t";
        acc->m_scope = "t";
        acc->m_typespec = 0;
        CHECK_ACCESSORS(vh_top_param, acc);
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
