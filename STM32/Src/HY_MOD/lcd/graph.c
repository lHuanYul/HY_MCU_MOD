#include "HY_MOD/lcd/graph.h"
#if defined(HY_MOD_STM32_LCD_1INCH47) || defined(HY_MOD_STM32_LCD12864)

static uint8_t graph_num_0_data[] =
{
    0b00011111, 0b10000000,
    0b00110000, 0b11000000,
    0b01100000, 0b01100000,
    0b01100000, 0b01100000,
    0b01100000, 0b01100000,
    0b01100000, 0b01100000,
    0b00110000, 0b11000000,
    0b00011111, 0b10000000,
};
LcdGraphData graph_num_0 =
{
    .data = graph_num_0_data,
    .width = 2,
    .width_bit = 12,
    .height = 8,
};

// ===== 透過 LcdGraphParm 進行統一封裝 =====
LcdGraphParm parm_8x16 = {
    .font_8x16 = &graph_8x16_const,
    .c_graphs = NULL,
};

#endif