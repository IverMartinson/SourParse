#ifndef SOURPARSE_H
#define SOURPARSE_H

#include "stdint.h"

typedef struct {
    int scale_x, scale01, scale10, scale_y;
    int glyph_index;
    int16_t arg1, arg2;
} SP_component;

typedef struct {
    int *x_coords, *y_coords, *contour_end_indicies; 
    int number_of_points, number_of_contours, number_of_components;
    int advance_width;
    uint8_t *flags;
    int16_t x_min, y_min, x_max, y_max;
    SP_component *components;
    int is_composite;
} SP_glyph;

typedef struct {
    int current_byte, number_of_glyphs, *glyph_offsets; 
    uint16_t *unicode_to_glyph_indicies, units_per_em;
    uint8_t *buffer;
    SP_glyph *glyphs;
    int16_t index_to_loca_format;
} SP_font;

SP_font* SP_load_font(char *filename);
void SP_free_font(SP_font* font);

#endif