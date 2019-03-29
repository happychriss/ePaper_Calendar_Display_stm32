//
// Created by development on 02.12.18.
//

#ifndef STM32_DISPLAY_PAINTER_H
#define STM32_DISPLAY_PAINTER_H

#include <fontem.h>

void InitDisplayMemory();
void PaintText(const struct font* pFont,uint16_t x_pos, uint16_t y_pos, char *p);
#endif //STM32_DISPLAY_PAINTER_H
