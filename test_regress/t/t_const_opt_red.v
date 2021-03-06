// DESCRIPTION: Verilator: Verilog Test module
//
// This file ONLY is placed under the Creative Commons Public Domain, for
// any use, without warranty, 2020 Wilson Snyder.
// SPDX-License-Identifier: CC0-1.0

module t(/*AUTOARG*/
   // Inputs
   clk
   );
   input clk;

   integer cyc=0;
   reg [63:0] crc;
   reg [63:0] sum;

   // Take CRC data and apply to testblock inputs
   wire [31:0] in = crc[31:0];

   /*AUTOWIRE*/
   // Beginning of automatic wires (for undeclared instantiated-module outputs)
   logic                a1;                     // From test of Test.v
   logic                a2;                     // From test of Test.v
   logic                a3;                     // From test of Test.v
   logic                a4;                     // From test of Test.v
   logic                a5;                     // From test of Test.v
   logic                o1;                     // From test of Test.v
   logic                o2;                     // From test of Test.v
   logic                o3;                     // From test of Test.v
   logic                o4;                     // From test of Test.v
   logic                o5;                     // From test of Test.v
   logic                x1;                     // From test of Test.v
   logic                x2;                     // From test of Test.v
   logic                x3;                     // From test of Test.v
   logic                x4;                     // From test of Test.v
   logic                x5;                     // From test of Test.v
   // End of automatics

   wire [31:0]          i = crc[31:0];

   Test test(/*AUTOINST*/
             // Outputs
             .a1                        (a1),
             .a2                        (a2),
             .a3                        (a3),
             .a4                        (a4),
             .a5                        (a5),
             .o1                        (o1),
             .o2                        (o2),
             .o3                        (o3),
             .o4                        (o4),
             .o5                        (o5),
             .x1                        (x1),
             .x2                        (x2),
             .x3                        (x3),
             .x4                        (x4),
             .x5                        (x5),
             // Inputs
             .clk                       (clk),
             .i                         (i[31:0]));

   // Aggregate outputs into a single result vector
   // verilator lint_off WIDTH
   wire [63:0] result = {a1,a2,a3,a4,a5,
                         o1,o2,o3,o4,o5,
                         x1,x2,x3,x4,x5};
   // verilator lint_on WIDTH

   // Test loop
   always @ (posedge clk) begin
`ifdef TEST_VERBOSE
      $write("[%0t] cyc==%0d crc=%x result=%x\n",$time, cyc, crc, result);
      $display("a %b %b %b %b %b", a1, a2, a3, a4, a5);
      $display("o %b %b %b %b %b", o1, o2, o3, o4, o5);
      $display("x %b %b %b %b %b", x1, x2, x3, x4, x5);
`endif
      cyc <= cyc + 1;
      crc <= {crc[62:0], crc[63]^crc[2]^crc[0]};
      sum <= result ^ {sum[62:0],sum[63]^sum[2]^sum[0]};
      if (cyc == 0) begin
         // Setup
         crc <= 64'h5aef0c8d_d70a4497;
         sum <= '0;
      end
      else if (cyc < 10) begin
         sum <= '0;
      end
      else if (cyc < 99) begin
         if (a1 != a2) $stop;
         if (a1 != a3) $stop;
         if (a1 != a4) $stop;
         if (a1 != a5) $stop;
         if (o1 != o2) $stop;
         if (o1 != o3) $stop;
         if (o1 != o4) $stop;
         if (o1 != o5) $stop;
         if (x1 != x2) $stop;
         if (x1 != x3) $stop;
         if (x1 != x4) $stop;
         if (x1 != x5) $stop;
      end
      else begin
         $write("[%0t] cyc==%0d crc=%x sum=%x\n",$time, cyc, crc, sum);
         if (crc !== 64'hc77bb9b3784ea091) $stop;
         // What checksum will we end up with (above print should match)
`define EXPECTED_SUM 64'hd7bd9c247dc7243c
         if (sum !== `EXPECTED_SUM) $stop;
         $write("*-* All Finished *-*\n");
         $finish;
      end
   end

endmodule

module Test(/*AUTOARG*/
   // Outputs
   a1, a2, a3, a4, a5, o1, o2, o3, o4, o5, x1, x2, x3, x4, x5,
   // Inputs
   clk, i
   );

   input clk;
   input [31:0] i;

   output logic a1, a2, a3, a4, a5;
   output logic o1, o2, o3, o4, o5;
   output logic x1, x2, x3, x4, x5;

   always_ff @(posedge clk) begin
      a1 <= (i[5] & ~i[3] & i[1]);
      a2 <= (i[5]==1 & i[3]==0 & i[1]==1);
      a3 <= &{i[5], ~i[3], i[1]};
      a4 <= ((i & 32'b101010) == 32'b100010);
      a5 <= ((i & 32'b001010) == 32'b000010) & i[5];
      //
      o1 <= (~i[5] | i[3] | ~i[1]);
      o2 <= (i[5]!=1 | i[3]!=0 | i[1]!=1);
      o3 <= |{~i[5], i[3], ~i[1]};
      o4 <= ((i & 32'b101010) != 32'b100010);
      o5 <= ((i & 32'b001010) != 32'b000010) | !i[5];
      //
      x1 <= (i[5] ^ ~i[3] ^ i[1]);
      x2 <= (i[5]==1 ^ i[3]==0 ^ i[1]==1);
      x3 <= ^{i[5], ~i[3], i[1]};
      x4 <= ^((i & 32'b101010) ^ 32'b001000);
      x5 <= ^((i & 32'b001010) ^ 32'b001000) ^ i[5];
   end

endmodule
