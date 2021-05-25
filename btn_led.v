`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2020/06/16 18:48:53
// Design Name: 
// Module Name: btn_led
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////

`timescale 1ps / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2020/06/16 18:48:53
// Design Name: 
// Module Name: btn_led
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module btn_led(
    input sys_clk,
    input btnc,
    input btnu,
    input btnd,
    input btnl,
    input btnr,
    output [7:0]led,
    output [7:0]ja
    );
    
    reg [25:0] count = 0;
    reg [7:0]  a = 0;
    reg [7:0]  b = 0;
    reg [4:0] btn = 0;

    always@(posedge sys_clk) begin
        if (btnu == 1) begin
            btn = 5'b10000;
        end else if (btnd == 1) begin
            btn = 5'b01000;
        end else if (btnl == 1) begin
            btn = 5'b00100;
        end else if (btnr == 1) begin
            btn = 5'b00010;
        end else if (btnc == 1) begin
            btn = 5'b00001;
        end
        count <= count + 1;
        if (btn == 5'b00001) begin
            if (count < 25'h020_0000 || ( count > 25'h040_0000 && count < 25'h050_0000) 
            || ( count > 25'h070_0000 && count < 25'h090_0000) || ( count > 25'h0b0_0000 && count < 25'h0d0_0000)
            || ( count > 25'h0e0_0000 && count < 25'h0e4_0000)
            || ( count > 25'h0fc_0000 && count < 25'h100_0000)) begin // *4 
                a[0] = count[0];
            end else begin
                a[0] = 1'b0;
            end
            if (count < 25'h004_0000 || ( count > 25'h01c_0000 && count < 25'h020_0000) 
            || ( count > 25'h040_0000 && count < 25'h050_0000) || ( count > 25'h070_0000 && count < 25'h078_0000)
            || ( count > 25'h0b0_0000 && count < 25'h0b8_0000) || ( count > 25'h0e0_0000 && count < 25'h0e4_0000)
            || ( count > 25'h0fc_0000 && count < 25'h100_0000)) begin // *4 
                a[1] = count[1];
                a[2] = count[2];
            end else begin
                a[1] = 1'b0;
                a[2] = 1'b0;
            end
            
            if (count < 25'h020_0000 
            || ( count > 25'h040_0000 && count < 25'h050_0000) || ( count > 25'h070_0000 && count < 25'h078_0000)
            || ( count > 25'h0b0_0000 && count < 25'h0b8_0000) || ( count > 25'h0e4_0000 && count < 25'h0e8_0000)
            || ( count > 25'h0f8_0000 && count < 25'h0fc_0000)) begin // *4 
                a[3] = count[3];
            end else begin
                a[3] = 1'b0;
            end
            
            if ( count < 25'h008_0000 
            || ( count > 25'h040_0000 && count < 25'h050_0000) || ( count > 25'h070_0000 && count < 25'h090_0000)
            || ( count > 25'h0b0_0000 && count < 25'h0b8_0000) || ( count > 25'h0e4_0000 && count < 25'h0e8_0000)
            || ( count > 25'h0f8_0000 && count < 25'h0fc_0000)) begin // *4 
                a[4] = count[4];
            end else begin
                a[4] = 1'b0;
            end
            
            if ( count < 25'h010_0000 
            || ( count > 25'h040_0000 && count < 25'h050_0000) || ( count > 25'h088_0000 && count < 25'h090_0000)
            || ( count > 25'h0b0_0000 && count < 25'h0b8_0000) || ( count > 25'h0e8_0000 && count < 25'h0ec_0000)
            || ( count > 25'h0f4_0000 && count < 25'h0f8_0000)) begin // *4 
                a[5] = count[5];
            end else begin
                a[5] = 1'b0;
            end
            
            if ( count < 25'h008_0000 || ( count > 25'h010_0000 && count < 25'h018_0000)
            || ( count > 25'h040_0000 && count < 25'h050_0000) || ( count > 25'h088_0000 && count < 25'h090_0000)
            || ( count > 25'h0b0_0000 && count < 25'h0b8_0000) || ( count > 25'h0e8_0000 && count < 25'h0ec_0000)
            || ( count > 25'h0f4_0000 && count < 25'h0f8_0000)) begin // *4 
                a[6] = count[6];
            end else begin
                a[6] = 1'b0;
            end
            
            if ( count < 25'h008_0000 || ( count > 25'h018_0000 && count < 25'h020_0000)
            || ( count > 25'h040_0000 && count < 25'h050_0000) || ( count > 25'h070_0000 && count < 25'h090_0000)
            || ( count > 25'h0b0_0000 && count < 25'h0d0_0000) 
            || ( count > 25'h0ec_0000 && count < 25'h0f4_0000)) begin // *4 
                a[7] = count[7];
            end else begin
                a[7] = 1'b0;
            end
            b = a;
        end
        else if (btn == 5'b10000) begin
            b = 8'b11111111;
        end
        else if (btn == 5'b01000) begin
            b[7] = count[24];
            b[6] = count[24];
            b[5] = count[24];
            b[4] = count[24];
            b[3] = count[24];
            b[2] = count[24];
            b[1] = count[24];
            b[0] = count[24];
        end
        else if (btn == 5'b00100) begin
            b[7] = count[25];
            b[6] = count[25];
            b[5] = count[25];
            b[4] = count[25];
            b[3] = count[25];
            b[2] = count[25];
            b[1] = count[25];
            b[0] = count[25];
        end
        else if (btn == 5'b00010) begin
            b[7] = count[23];
            b[6] = count[23];
            b[5] = count[23];
            b[4] = count[23];
            b[3] = count[23];
            b[2] = count[23];
            b[1] = count[23];
            b[0] = count[23];
        end
    end
    assign led[7:0] = b;
    assign ja[7:0] = b;
endmodule

/*module btn_led(
    input sys_clk,
    input btnc,
    input btnu,
    input btnd,
    input btnl,
    input btnr,
    output [7:0]led
    );
    
    reg [29:0] count = 0;
    reg flag = 0;
    reg f = 1;
    reg [7:0]  b = 0;
    integer i = 14;             // frequency       22 <= i <= 26
    integer j = 1;                    // position j-1 / j / j+1       1 <= j <= 6

    always@(posedge sys_clk) begin

        count <= count + 1;
        if ((btnu == 1 || btnd == 1 || btnl == 1 || btnr == 1 || btnc == 1) && (f == 1)) begin
            if (btnu == 1) begin
                flag = 0;
                if (i > 12) begin
                    i = i-1;
                end
            end else if (btnd == 1) begin
                flag = 0;
                if (i < 16) begin
                    i = i+1;
                end
            end else if (btnl == 1) begin
                flag = 0;
                if (j < 6) begin
                    j = j+1;
                end
            end else if (btnr == 1) begin
                flag = 0;
                if (j > 1) begin
                    j = j-1;
                end
            end else if (btnc == 1) begin
                flag = 1;
            end
            f = 0;
        end else if (btnu == 0 && btnd == 0 && btnl == 0 && btnr == 0 && btnc == 0) begin
            f = 1;
            b = 0;
        end
        if (flag)begin
            b = 8'b11111111;
        end else begin
            b[j-1] = count[i];
            b[j] = count[i];
            b[j+1] = count[i];
        end
        #period;
    end
    parameter period = 500;
    assign led[7:0] = b;
endmodule*/
