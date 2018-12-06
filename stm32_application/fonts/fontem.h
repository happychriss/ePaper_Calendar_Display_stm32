//
// Created by development on 05.12.18.
//

#ifndef STM32_DISPLAY_FONTEM_H
#define STM32_DISPLAY_FONTEM_H
/** Glyph character value rype */
typedef uint16_t glyph_t;

/** Description of a glyph; a single character in a font. */
struct glyph {
    glyph_t			glyph;          /** The glyph this entry refers to */

    int16_t			left;           /** Offset of the left edge of the glyph */
    int16_t			top;            /** Offset of the top edge of the glyph */
    int16_t			advance;        /** Horizonal offset when advancing to the next glyph */

    uint16_t		cols;           /** Width of the bitmap */
    uint16_t		rows;           /** Height of the bitmap */
    const uint8_t		*bitmap;        /** Bitmap data */

    const struct kerning	*kerning;       /** Font kerning data */
};

/** Kerning table; for a pair of glyphs, provides the horizontal adjustment. */
struct kerning {
    glyph_t left;   /** The left-glyph */
    int16_t offset; /** The kerning offset for this glyph pair */
};

/** Description of a font. */
struct font {
    char			*name;          /** Name of the font */
    char			*style;         /** Style of the font */

    uint16_t		size;           /** Point size of the font */
    uint16_t		dpi;            /** Resolution of the font */

    int16_t			ascender;       /** Ascender height */
    int16_t			descender;      /** Descender height */
    int16_t			height;         /** Baseline-to-baseline height */

    uint16_t		count;          /** Number of glyphs */
    uint16_t		max;            /** Maximum glyph index */
    const struct glyph	**glyphs;       /** Font glyphs */
    char			compressed;     /** TRUE if glyph bitmaps are RLE compressed */
};

#endif //STM32_DISPLAY_FONTEM_H
