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

   // Top level variable and parameters
   localparam TOP_PARAM = 2;
   logic top_var = 1'b0;
   logic top_array[1:0] = '{default: 1'b0};
   integer status;
   wire logic top_net;

   assign top_net = top_var;

   generate
   // This one should not be found...
   begin
      // But this one should and it is not found by ModelSim...
      begin: named_scope_1
         logic ns1_var;
         logic ns1_array[1:0];
         assign ns1_var = 1'b1;
      end
   end
   endgenerate

   always_ff @(posedge clk) begin
      top_var <= !top_var;
   end
   
   genvar i;
   generate
      logic gen_var;
      logic gen_array_clk[TOP_PARAM:0];
      logic gen_array_cont[TOP_PARAM-1:0];
      for (i = 0; i < TOP_PARAM; i++) begin: named_for
         logic gen_for_var;
         if (i == 0) begin:useless_init
            assign gen_var = gen_array_cont[i];
         end else begin: normal_assign
            assign gen_array_cont[i] = top_array[i];
            always_ff @(posedge clk) begin
               gen_array_clk[i] <= gen_array_cont[i];
            end 
         end
         assign gen_for_var = gen_array_cont[i];
      end
   endgenerate

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
         $write("%%Error: t_vpi_var_model.cpp:%0d: C Test failed\n", status);
         $stop;
      end
      $write("*-* All Finished *-*\n");
      $finish;
   end
endmodule : t
