//
// Created by development on 13.04.19.
//

#ifndef STM32_DISPLAY_ICONV_H
#define STM32_DISPLAY_ICONV_H
/* UTF-8 to ISO-8859-1/ISO-8859-15 mapper.
 * Return 0..255 for valid ISO-8859-15 code points, 256 otherwise.
*/
static inline unsigned int to_latin9(const unsigned int code)
{
    /* Code points 0 to U+00FF are the same in both. */
    if (code < 256U)
        return code;
    switch (code) {
        case 0x0152U: return 188U; /* U+0152 = 0xBC: OE ligature */
        case 0x0153U: return 189U; /* U+0153 = 0xBD: oe ligature */
        case 0x0160U: return 166U; /* U+0160 = 0xA6: S with caron */
        case 0x0161U: return 168U; /* U+0161 = 0xA8: s with caron */
        case 0x0178U: return 190U; /* U+0178 = 0xBE: Y with diaresis */
        case 0x017DU: return 180U; /* U+017D = 0xB4: Z with caron */
        case 0x017EU: return 184U; /* U+017E = 0xB8: z with caron */
        case 0x20ACU: return 164U; /* U+20AC = 0xA4: Euro */
        default:      return 256U;
    }
}

size_t utf8_to_latin9(char * output, char * input,  size_t length);
#endif //STM32_DISPLAY_ICONV_H
