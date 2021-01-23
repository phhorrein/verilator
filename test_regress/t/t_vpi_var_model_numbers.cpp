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

#include "Vt_vpi_var_model_numbers.h"
#include "verilated.h"
#include "svdpi.h"

#include "Vt_vpi_var_model_numbers__Dpi.h"

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
#define FILENM "t_vpi_var_model_numbers.cpp"

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

    TestVpiHandle vhint = VPI_HANDLE("int_var");
    CHECK_RESULT_NZ(vhint);
    {
        p = vpi_get_str(vpiType, vhint);
        CHECK_RESULT_CSTR(p, "vpiIntVar");
        props->reset();
        props->m_size = 32;
        props->m_scalar = true;
        CHECK_PROPERTIES(vhint, props);
    }

    TestVpiHandle vhinteger = VPI_HANDLE("integer_var");
    CHECK_RESULT_NZ(vhinteger);
    {
        p = vpi_get_str(vpiType, vhinteger);
        CHECK_RESULT_CSTR(p, "vpiIntegerVar");
        props->reset();
        props->m_size = 32;
        props->m_scalar = true;
        CHECK_PROPERTIES(vhinteger, props);
    }

    TestVpiHandle vhshortint = VPI_HANDLE("shortint_var");
    CHECK_RESULT_NZ(vhshortint);
    {
        p = vpi_get_str(vpiType, vhshortint);
        CHECK_RESULT_CSTR(p, "vpiShortIntVar");
        props->reset();
        props->m_size = 16;
        props->m_scalar = true;
        CHECK_PROPERTIES(vhshortint, props);
    }

    TestVpiHandle vhbyte = VPI_HANDLE("byte_var");
    CHECK_RESULT_NZ(vhbyte);
    {
        p = vpi_get_str(vpiType, vhbyte);
        CHECK_RESULT_CSTR(p, "vpiByteVar");
        props->reset();
        props->m_size = 8;
        props->m_scalar = true;
        CHECK_PROPERTIES(vhbyte, props);
    }

    TestVpiHandle vhreal = VPI_HANDLE("real_var");
    CHECK_RESULT_NZ(vhreal);
    {
        p = vpi_get_str(vpiType, vhreal);
        CHECK_RESULT_CSTR(p, "vpiRealVar");
        props->reset();
        props->m_size = 64;
        props->m_scalar = true;
        CHECK_PROPERTIES(vhreal, props);
    }

    TestVpiHandle vhlongint = VPI_HANDLE("longint_var");
    CHECK_RESULT_NZ(vhlongint);
    {
        p = vpi_get_str(vpiType, vhlongint);
        CHECK_RESULT_CSTR(p, "vpiLongIntVar");
        props->reset();
        props->m_size = 64;
        props->m_scalar = true;
        CHECK_PROPERTIES(vhlongint, props);
    }

    TestVpiHandle vhstring = VPI_HANDLE("string_var");
    CHECK_RESULT_NZ(vhstring);
    {
        p = vpi_get_str(vpiType, vhstring);
        CHECK_RESULT_CSTR(p, "vpiStringVar");
        props->reset();
        props->m_size = 13;
        CHECK_PROPERTIES(vhstring, props);
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
