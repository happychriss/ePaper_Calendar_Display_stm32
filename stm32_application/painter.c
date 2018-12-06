//
// Created by development on 02.12.18.
//

#include <stdint-gcc.h>
#include <stdlib.h>
#include "painter.h"
#include "sram.h"
#include "fonts/font-robo_8-8.h"
#include "fonts/font-robo_bold_8-8.h"
#include "fonts/font-robo_12-12.h"
#include "fonts/font-robo_bold_12-12.h"

#define BLACK 0b00
#define DARK_GREY 0b01
#define LIGHT_GREY 0b10
#define WHITE 0b11

#define CANVAS_X 800
#define CANVAS_Y 600

const unsigned char x_bit_mask[4] = {
        0b11000000,
        0b00110000,
        0b00001100,
        0b00000011};



const void *font_table[4] =
        {
                &font_robo_8_8 ,
                &font_robo_12_12,
                &font_robo_bold_8_8,
                &font_robo_bold_12_12
        };


//800 Zeichen pro Zeile,

void InitDisplayMemory() {
    uint16_t white=0b0000000000000000; //16bit = 8 Zeichen
    uint16_t black=0b1111111111111111;
    uint32_t address1=0;
    uint32_t address2=20000;

    FSMC_SRAM_FillBuffer(black,address1,60000); // 600*800=480.000 / 8 = 60.000
    FSMC_SRAM_FillBuffer(white,address2,40000); // 600*800=480.000 / 8 = 60.000
}



const struct glyph *font_get_glyph(const struct font *font, glyph_t glyph)
{
    if (glyph > font->max)
        return NULL;

    size_t first = 0, last = font->count;
    const struct glyph **glyphs = font->glyphs;

    while (first < last) {
        size_t mid = first + (last - first) / 2;
        if (glyph <= glyphs[mid]->glyph)
            last = mid;
        else
            first = mid + 1;
    }

    return (last < font->count && glyphs[last]->glyph == glyph) ? *(glyphs + last) : NULL;
}

int16_t font_get_kerning(const struct font *font, glyph_t left, glyph_t right)
{
    if (font == NULL || left == 0 || right == 0) return 0;
    const struct glyph *g = font_get_glyph(font, right);
    if (g == NULL || g->kerning == NULL) return 0;

    const struct kerning *k;
    for (k = g->kerning; k->left != 0; k++)
        if (k->left == left) return k->offset;

    return 0;
}


int font_draw_glyph_L(const struct font *font,
                     int x, int y, int width, int height,
                     const struct glyph *glyph)
{
    unsigned int rows = glyph->rows, cols = glyph->cols;
    const unsigned char *data = glyph->bitmap;
    unsigned char count = 0, class = 0;

    for (unsigned int row = 0; row < rows; row++) {
        int yofs = row + y + (font->ascender - glyph->top);

        uint8_t pBufferLineBytes[200];  //800 pixel = 100 bytes * 2 for grey colors = 200
        FSMC_SRAM_ReadBuffer((uint16_t *)pBufferLineBytes,row*100,100); //16bit = 8 pixel = 100 unit16

        for (unsigned int col = 0; col < cols; col++) {
            int xofs = col + x + glyph->left;

            uint8_t val;
            if (font->compressed) {
                if (count == 0) {
                    count = (*data & 0x3f) + 1;
                    class = *data >> 6;
                    data++;
                }

                if (class == 0)
                    val = *(data++);
                else if (class == 3)
                    val = 0xff;
                else
                    val = 0;
                count--;
            } else {
                val = data[(row * cols) + col];
            }

            if ((yofs >= 0) && (yofs < height) && (xofs >= 0) && (xofs < width)) {

                uint8_t x_byte_sram_current;
                uint8_t x_bit_sram_current;
                uint8_t color_bits;

                //bit and byte of sram, I want to update
                x_byte_sram_current=(uint8_t )(xofs/8);
                x_bit_sram_current =(uint8_t )(xofs-(8*x_byte_sram_current));

                if (val < 64) color_bits=WHITE;
                else if (val < 128) color_bits=LIGHT_GREY;
                else if (val < 192) color_bits=DARK_GREY;
                else color_bits=BLACK;

                uint8_t buffer=pBufferLineBytes[x_byte_sram_current];

                // value = (value & ~mask) | (newvalue & mask);
                buffer=(buffer & x_bit_mask[x_bit_sram_current])|(color_bits & x_bit_mask[x_bit_sram_current]);

                pBufferLineBytes[x_byte_sram_current]=buffer;

            }
        }

        FSMC_SRAM_WriteBuffer((uint16_t *)pBufferLineBytes,row*100,100); //16bit = 8 pixel = 100 unit16

    }

    return glyph->advance;
}

int font_draw_char_L(const struct font *font,
                     int x, int y, int width, int height,
                     glyph_t glyph, glyph_t prev)
{
    if (font == NULL) return -1;
    const struct glyph *g = font_get_glyph(font, glyph);
    if (g == NULL) return -2;

    int kerning_offset = font_get_kerning(font, prev, glyph);

    return font_draw_glyph_L(font, x + kerning_offset, y, width, height,
                              g) + kerning_offset;
}

void PaintText(struct font* pFont,uint16_t x_pos, uint16_t y_pos, char *p) {

    int x = 0;
    char prev = 0;
    while (*p) {
        x += font_draw_char_L(pFont, x, 0, CANVAS_X, CANVAS_Y,  *p, prev);
        prev = *p;
        p++;
    }

}
