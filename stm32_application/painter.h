//
// Created by development on 02.12.18.
//

#ifndef STM32_DISPLAY_PAINTER_H
#define STM32_DISPLAY_PAINTER_H

#include <fontem.h>
#include "global.h"

#define CANVAS_X 800
#define CANVAS_Y 600

extern t_grafic_buffer_line* grafic_buffer_lines;
extern uint8_t *ptr_grafic_buffer;

void ClearDisplay();
void PaintText(const struct font* pFont,uint16_t x_pos, uint16_t y_pos, char *p);


#endif //STM32_DISPLAY_PAINTER_H
