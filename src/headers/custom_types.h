#ifndef CUSTOM_TYPES_H
#define CUSTOM_TYPES_H

#include "stdint.h"

typedef struct {
    int *x_coords, *y_coords, *contour_end_indicies, number_of_points, number_of_contours;
    uint8_t *flags;
} SP_glyph;

typedef struct {
    int current_byte, number_of_glyphs, *glyph_offsets; 
    uint16_t *unicode_to_glyph_indicies, units_per_em;
    uint8_t *buffer;
    SP_glyph *glyphs;
    int16_t index_to_loca_format;
} SP_font;

#endif