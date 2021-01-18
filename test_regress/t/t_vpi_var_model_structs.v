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

   typedef struct packed {
       logic       s_field;
       logic[2:0]  p_field;
   } struct_test;

   typedef struct packed {
       struct_test field0;
       logic [2:0] p_field;
       struct_test field1;
   } outer_struct;

   typedef union packed {
       logic[3:0] field0;
       struct_test field1;
   } union_test;

   struct_test             s  /*verilator public_flat_rw @(posedge clk) */;
   outer_struct            s_emb  /*verilator public_flat_rw @(posedge clk) */;
   struct_test  [1:0]      s_p1  /*verilator public_flat_rw @(posedge clk) */;
   struct_test  [1:0][2:0] s_p12  /*verilator public_flat_rw @(posedge clk) */;
   struct_test             s_u2 [2:0]  /*verilator public_flat_rw @(posedge clk) */;
   struct_test [3:0]       s_p3u1 [1:0]  /*verilator public_flat_rw @(posedge clk) */;

   union_test              u;

   integer 	  status;

   // Test loop
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
         $write("%%Error: t_vpi_var_model_structs.cpp:%0d: C Test failed\n", status);
         $stop;
      end
      $write("*-* All Finished *-*\n");
      $finish;
   end
endmodule : t
