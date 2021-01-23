// DESCRIPTION: Verilator: Verilog Test module
//
// Copyright 2010 by Wilson Snyder. This program is free software; you can
// redistribute it and/or modify it under the terms of either the GNU
// Lesser General Public License Version 3 or the Perl Artistic License
// Version 2.0.
// SPDX-License-Identifier: LGPL-3.0-only OR Artistic-2.0

`ifdef USE_VPI_NOT_DPI
//We call it via $c so we can verify DPI isn't required - see bug572
`else
import "DPI-C" context function integer mon_check();
`endif

module t (/*AUTOARG*/
   // Inputs
   clk
   );

`ifdef VERILATOR
`systemc_header
extern "C" int mon_check();
`verilog
`endif

   input clk;

   wire logic            onereg  /*verilator public_flat_rw @(posedge clk) */ = '0 ;
   // Arrays
   wire logic [0:0]        a_p0  /*verilator public_flat_rw @(posedge clk) */;
   wire logic signed [1:0] a_sp1  /*verilator public_flat_rw @(posedge clk) */;
   wire logic [1:0]        a_p1  /*verilator public_flat_rw @(posedge clk) */;
   // verilator lint_off LITENDIAN
   wire logic [0:1]        a_p1le  /*verilator public_flat_rw @(posedge clk) */;
   // verilator lint_on LITENDIAN
   wire logic [2:0][1:0]   a_p21  /*verilator public_flat_rw @(posedge clk) */;
   wire logic              a_u0 [0:0]  /*verilator public_flat_rw @(posedge clk) */;
   wire logic              a_u2 [2:0]  /*verilator public_flat_rw @(posedge clk) */;
   wire logic [1:0]        a_p1u1 [1:0]  /*verilator public_flat_rw @(posedge clk) */;
   wire logic [0:0]        a_p0u12 [1:0][2:0]  /*verilator public_flat_rw @(posedge clk) */;

   integer 	  status;

   // Test loop
   assign onereg = 1'b0;
   assign a_p0 = '0;
   assign a_p1 = 2'b10;
   assign a_p1le = '0;
   assign a_p21 = '0;
   assign a_u0[0] = '0;
   assign a_u2[0] = '0;
   assign a_p1u1[0] = '0;
   assign a_p0u12[0][0] = '0;
   initial begin
`ifdef VERILATOR
      status = $c32("mon_check()");
`else
      status = $mon_check();
`endif
`ifndef USE_VPI_NOT_DPI
      status = mon_check();
`endif
      if (status != 0) begin
         $write("%%Error: t_vpi_net_model_logic.cpp:%0d: C Test failed\n", status);
         $stop;
      end
      $write("*-* All Finished *-*\n");
      $finish;
   end
endmodule : t
