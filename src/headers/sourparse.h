#ifndef SOURPARSE_H
#define SOURPARSE_H

#include "stdint.h"

typedef struct {
    uint16_t left, right;
    int16_t value; // kerning value for the above pair. If it's more than 0, the cahracters will be moved apart. If it's less than 0, the characters will be moved closer
} SP_kerning_pair;

typedef struct {
    int16_t ascender;
    int16_t descender;
    int16_t line_gap;
    uint16_t advance_max_width;
    int16_t min_left_side_bearing;
    int16_t min_right_side_bearing;
    int16_t x_max_extent;
    int16_t carat_slope_rise;
    int16_t carat_slope_run;
    int16_t carat_offset;
    int16_t metric_data_format;
    uint16_t number_of_h_metrics;
} SP_hhea_table;

typedef struct {
    uint16_t advance_width;
    int16_t left_side_bearing;
} SP_long_hor_metric;

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
    int current_byte, number_of_glyphs, number_of_kerning_pairs, *glyph_offsets; 
    uint16_t *unicode_to_glyph_indicies;
    float units_per_em;
    uint8_t *buffer;
    SP_glyph *glyphs;
    int16_t index_to_loca_format;
    SP_long_hor_metric *h_metrics;
    SP_hhea_table hhea_table;
    int16_t *left_side_bearings;
    SP_kerning_pair *kerning_pairs; 
} SP_font;

SP_font* SP_load_font(char *filename);
void SP_free_font(SP_font* font);

#endif