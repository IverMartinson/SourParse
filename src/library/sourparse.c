#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../headers/sourparse.h"

void skip_8(SP_font *font){
    font->current_byte++;
}

void skip_16(SP_font *font){
    font->current_byte += 2;
}

void skip_32(SP_font *font){
    font->current_byte += 4;
}

uint8_t get_u8(SP_font *font){
    return font->buffer[font->current_byte++];
}

// file is in big endian
// swap bytes
// 0x1234 -> 0x3412
uint16_t get_u16(SP_font *font){
    return (uint16_t)(font->buffer[font->current_byte++] << 8 | font->buffer[font->current_byte++]);
}

uint32_t get_u32(SP_font *font){
    return (uint32_t)(font->buffer[font->current_byte++] << 24 | font->buffer[font->current_byte++] << 16 | font->buffer[font->current_byte++] << 8 | font->buffer[font->current_byte++]);
}

int8_t get_i8(SP_font *font){
    return font->buffer[font->current_byte++];
}

int16_t get_i16(SP_font *font){
    return (int16_t)(font->buffer[font->current_byte++] << 8 | font->buffer[font->current_byte++]);
}

int32_t get_i32(SP_font *font){
    return (int32_t)(font->buffer[font->current_byte++] << 24 | font->buffer[font->current_byte++] << 16 | font->buffer[font->current_byte++] << 8 | font->buffer[font->current_byte++]);
}

void get_tag(SP_font *font, char *tag){
    tag[0] = get_u8(font);
    tag[1] = get_u8(font);
    tag[2] = get_u8(font);
    tag[3] = get_u8(font);
    tag[4] = '\0';
}

float get_F2DOT14(SP_font *font) {
    int16_t signedVal = (int16_t)(font->buffer[font->current_byte++] << 8 | font->buffer[font->current_byte++]);
    return signedVal / 16384.0f;
}

int* read_glyph_coords(SP_font *font, uint8_t *flags, int number_of_points, int is_this_function_reading_the_x_coordinates_hmmmm_well_if_this_vaSPable_is_one_then_it_means_we_are_okay_yayyyyyy){
    int byte_size_bit = is_this_function_reading_the_x_coordinates_hmmmm_well_if_this_vaSPable_is_one_then_it_means_we_are_okay_yayyyyyy ? 1 : 2;
    int sign_slash_repeat_bit = is_this_function_reading_the_x_coordinates_hmmmm_well_if_this_vaSPable_is_one_then_it_means_we_are_okay_yayyyyyy ? 4 : 5;
    int *coords = malloc(sizeof(int) * number_of_points);

    for (int i = 0; i < number_of_points; ++i){
        uint8_t flag = flags[i];
        int on_curve = flag & 1;

        int delta = 0;

        // delta is 8 bits
        if (flag >> byte_size_bit & 1){
            // delta is the current coord.
            // it's called delta because the coords are differences in position, not positions
            // also this part            ------------------------------------------    sets the sign of the delta
            delta = (int)get_u8(font) * (flag >> sign_slash_repeat_bit & 1 ? 1 : -1);
        } // delta is 16 bits 
        else {
            if (!(flag >> sign_slash_repeat_bit & 1)){
                delta = (int)get_i16(font);
            }
        }

        if (i == 0){
            coords[i] = delta;
        }else {
            coords[i] = coords[i - 1] + delta;
        }
    }

    return coords;
}

// composite flags
#define ARG_1_AND_2_ARE_WORDS 0x0001
#define ARGS_ARE_XY_VALUES 0x0002
#define WE_HAVE_A_SCALE 0x0008
#define MORE_COMPONENTS 0x0020
#define WE_HAVE_AN_X_AND_Y_SCALE 0x0040
#define WE_HAVE_A_TWO_BY_TWO 0x0080
#define WE_HAVE_INSTRUCTIONS 0x0100

// simple flags
#define REPEAT_FLAG 0x08

void read_glyph(SP_font *font, int current_glyph){ 
    int glyph_start = font->glyph_offsets[current_glyph];
    int glyph_end   = font->glyph_offsets[current_glyph + 1];
    
    font->current_byte = glyph_start;
    
    SP_glyph *glyph = &font->glyphs[current_glyph];
    
    if (glyph_start == glyph_end){
        glyph->number_of_contours = 0;
        
        return;
    }
    
    glyph->number_of_contours = (int)get_i16(font);

    // do i need these? i dont think so
    glyph->x_min = get_i16(font);
    glyph->y_min = get_i16(font);
    glyph->x_max = get_i16(font);
    glyph->y_max = get_i16(font);

    // glyph is composite :(
    if (glyph->number_of_contours < 0){    
        glyph->is_composite = 1;
        
        uint16_t flags;

        glyph->number_of_components = 0;
        
        int current_allocated_components = 5;

        // assume the glyph has 5 children components
        glyph->components = malloc(sizeof(SP_component) * current_allocated_components);

        int reading_glyphs = 1;
        while (reading_glyphs) {
            glyph->number_of_components++;
            
            // we use number of components as the current component because we're iteratiing it anyways.
            // A better name would be more fitting, but it wouldn't make sense to use another var
            if (glyph->number_of_components >= current_allocated_components){
                current_allocated_components += 5;

                glyph->components = realloc(glyph->components, current_allocated_components);
            }

            SP_component *c_glyph = &glyph->components[glyph->number_of_components];

            flags = get_u16(font);
            c_glyph->glyph_index = get_u16(font);

            c_glyph->scale_x = c_glyph->scale_y = c_glyph->scale01 = c_glyph->scale10 = 1;

            if (flags & ARG_1_AND_2_ARE_WORDS) { // args are words
                if (flags & ARGS_ARE_XY_VALUES) { // args are signed offsets
                    c_glyph->arg1 = get_i16(font);
                    c_glyph->arg2 = get_i16(font);
                } else { // ------------------------ args are point indecies
                    c_glyph->arg1 = get_u16(font);
                    c_glyph->arg2 = get_u16(font);
                }
            } else { // args are bytes
                if (flags & ARGS_ARE_XY_VALUES) {
                    c_glyph->arg1 = get_i8(font);
                    c_glyph->arg2 = get_i8(font);
                } else {
                    c_glyph->arg1 = get_u8(font);
                    c_glyph->arg2 = get_u8(font);
                }
            }

            if (flags & WE_HAVE_A_SCALE) { // we have a scale
                c_glyph->scale_x = c_glyph->scale_y = (int)get_F2DOT14(font);
            } else if (flags & WE_HAVE_AN_X_AND_Y_SCALE) { // seperate x and y scales
                c_glyph->scale_x = (int)get_F2DOT14(font);
                c_glyph->scale_y = (int)get_F2DOT14(font);
            } else if (flags & WE_HAVE_A_TWO_BY_TWO) { // 2x2 transform
                c_glyph->scale_x = (int)get_F2DOT14(font);
                c_glyph->scale01 = (int)get_F2DOT14(font);
                c_glyph->scale10 = (int)get_F2DOT14(font);
                c_glyph->scale_y = (int)get_F2DOT14(font);
            }

            if (!(flags & MORE_COMPONENTS)) { // this is the last c_glyph, stop reading (please&thankyou)
                reading_glyphs = 0;
            }
        }

        if (flags & WE_HAVE_INSTRUCTIONS){ // we have instructions
            uint16_t number_of_instructions = get_u16(font);

            for (int i = 0; i < number_of_instructions; ++i) skip_8(font); // we dont care about instructions
        }

        font->current_byte = glyph_end;
    
        return;
    }

    glyph->is_composite = 0;

    glyph->contour_end_indicies = malloc(sizeof(int) * glyph->number_of_contours);

    for (int i = 0; i < (int)glyph->number_of_contours; ++i){
        glyph->contour_end_indicies[i] = (int)get_u16(font);
    }
    
    // skip instructions
    uint16_t instruction_length = get_u16(font);
    
    if (instruction_length > 0){
        for (int i = 0; i < instruction_length; ++i){
            skip_8(font);
        }
    }

    // we still want to skip the instructions so that we dont get offset.
    // also set point count to 0
    if (glyph->number_of_contours == 0){
        glyph->number_of_points = 0;
        
        return; 
    } 

    // add one because these are 0-indexed indicies.
    // the last one is the highest index
    glyph->number_of_points = glyph->contour_end_indicies[glyph->number_of_contours - 1] + 1;

    glyph->flags = malloc(sizeof(uint8_t) * (glyph->number_of_points));

    for (int i = 0; i < glyph->number_of_points; ++i){ // flags
        glyph->flags[i] = get_u8(font); // flag
        
        if (glyph->flags[i] & REPEAT_FLAG){ // is the flag a repeat flag?
            uint8_t flag = glyph->flags[i];
        
            int times_to_repeat = (int)get_u8(font);

            for (int j = 0; j < times_to_repeat; ++j){ // yes, get the next byte (number of repeats) and repeat this flag however many times
                glyph->flags[++i] = flag;
            }
        }
    }

    glyph->x_coords = read_glyph_coords(font, glyph->flags, glyph->number_of_points, 1);
    glyph->y_coords = read_glyph_coords(font, glyph->flags, glyph->number_of_points, 0);

    font->current_byte = glyph_end;
}

#define KERN_HORIZONTAL   0x0001  // bit 0
#define KERN_MINIMUM      0x0002  // bit 1
#define KERN_CROSS_STREAM 0x0004  // bit 2
#define KERN_OVERRIDE     0x0008  // bit 3
#define KERN_FORMAT       0xFF00  // bits 8–15

SP_font* SP_load_font(char *filename){
    SP_font *font = calloc(1, sizeof(SP_font));

    FILE *fp = fopen(filename, "rb");

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    font->buffer = malloc(size);

    fread(font->buffer, 1, size, fp);
    fclose(fp);

    skip_32(font);

    uint16_t number_of_tables = get_u16(font);

    skip_32(font);
    skip_16(font);

    int glyph_offset = 0;
    int maxp_offset = 0;
    int cmap_offset = 0;
    int loca_offset = 0;
    int head_offset = 0;
    int hhea_offset = 0;
    int hmtx_offset = 0;
    int kern_offset = 0;

    for (int i = 0; i < (int)number_of_tables; ++i){
        char tag[5]; get_tag(font, tag);
        uint32_t checksum = get_u32(font);
        uint32_t offset = get_u32(font);
        uint32_t length = get_u32(font);
    
        if (strcmp(tag, "glyf") == 0){
            glyph_offset = (int)offset;
        }
        
        else if (strcmp(tag, "maxp") == 0){
            maxp_offset = (int)offset;
        }
        
        else if (strcmp(tag, "cmap") == 0){
            cmap_offset = (int)offset;
        }

        else if (strcmp(tag, "loca") == 0){
            loca_offset = (int)offset;
        }

        else if (strcmp(tag, "head") == 0){
            head_offset = (int)offset;
        }

        else if (strcmp(tag, "hhea") == 0){
            hhea_offset = (int)offset;
        }

        else if (strcmp(tag, "hmtx") == 0){
            hmtx_offset = (int)offset;
        }

        else if (strcmp(tag, "kern") == 0){
            kern_offset = (int)offset;
        }
    }

    // head table
    { 
        font->current_byte = head_offset;

        // skip nonsense
        skip_32(font); // version
        skip_32(font); // font revision
        skip_32(font); // checksum adjustment
        skip_32(font); // magic number
        skip_16(font); // flags

        font->units_per_em = (float)get_u16(font);
    }

    // skip gibberjabber
    {
        skip_32(font); // 64 bit longdatetime
        skip_32(font);
        skip_32(font); // 64 bit longdatetime
        skip_32(font); 
        skip_32(font); // mins
        skip_32(font); // maxs
        skip_16(font);
        skip_16(font);
        skip_16(font);

        font->index_to_loca_format = get_i16(font);    
    }

    // get number of glyphs
    {
        font->current_byte = maxp_offset;

        skip_32(font);

        font->number_of_glyphs = (int)get_u16(font);
    }

    // kerning data
    {
        font->current_byte = kern_offset;

        uint16_t kerning_version = get_u16(font);
        uint16_t number_of_kerning_tables = get_u16(font);
        
        for (int i = 0; i < number_of_kerning_tables; ++i){
            uint16_t version = get_u16(font);
            uint16_t length = get_u16(font); // length of subtable in bytes
            uint16_t coverage = get_u16(font); // what info is in this table
            
            int is_horizontal = coverage & KERN_HORIZONTAL;
            int is_minimum = coverage & KERN_MINIMUM; // dont know what a minimum is
            int is_cross_stream = coverage & KERN_CROSS_STREAM; // dunno what this is
            int is_override = coverage & KERN_OVERRIDE;
            int format = (coverage & KERN_FORMAT) >> 8; // these are high bits so we have to shift them down
            
            // only searching format 0 for now. That's what windows uses so it's probably common
            if (format == 0){
                uint16_t number_of_pairs = get_u16(font);
                uint16_t search_range = get_u16(font);
                uint16_t entry_selector = get_u16(font);
                uint16_t range_shift = get_u16(font);

                font->number_of_kerning_pairs = number_of_pairs;
                
                font->kerning_pairs = calloc(number_of_pairs, sizeof(SP_kerning_pair*));

                for (int j = 0; j < number_of_pairs; ++j){
                    font->kerning_pairs[j] = (SP_kerning_pair){get_u16(font), get_u16(font), get_i16(font)};
                }
                
                break;
            }
        }
    }

    // glyph locations
    {
        font->current_byte = loca_offset;

        font->glyph_offsets = malloc(sizeof(int) * (font->number_of_glyphs + 1));

        for (int i = 0; i <= font->number_of_glyphs; ++i){
            if (font->index_to_loca_format){ // long offsets (32 bit)
                font->glyph_offsets[i] = (int)get_u32(font) + glyph_offset;
            }else { // short offsets (16 bit)
                font->glyph_offsets[i] = (int)get_u16(font) * 2 + glyph_offset;
            }
        }
    }

    // character map
    {
        font->current_byte = cmap_offset;

        uint16_t version = get_u16(font);

        uint16_t number_of_cmap_tables = get_u16(font);

        int last_byte_offset;

        for (int i = 0; i < number_of_cmap_tables; ++i){
            uint16_t platform_id = get_u16(font);
            uint16_t encoding_id = get_u16(font);
            uint32_t subtable_offset = get_u32(font);

            last_byte_offset = font->current_byte;
            font->current_byte = cmap_offset + subtable_offset;

            if (platform_id == 3 && encoding_id == 1){
                // i'll add this when i have to use a font that needs it 
            } else if (platform_id == 0 && encoding_id == 3){
                // assuming only format 4 for now because im lazy >w<

                uint16_t format = get_u16(font);
                uint16_t length = get_u16(font);
                uint16_t language = get_u16(font);
                uint16_t seg_count_x2 = get_u16(font); uint16_t seg_count = seg_count_x2 / 2;
                skip_16(font);
                skip_16(font);
                skip_16(font);
            
                uint16_t *end_codes = malloc(sizeof(uint16_t) * seg_count);
                for (int i = 0; i < seg_count; ++i) end_codes[i] = get_u16(font);
                
                skip_16(font);
                
                uint16_t *start_codes = malloc(sizeof(uint16_t) * seg_count);
                for (int i = 0; i < seg_count; ++i) start_codes[i] = get_u16(font);
                
                int16_t *id_deltas = malloc(sizeof(uint16_t) * seg_count);
                for (int i = 0; i < seg_count; ++i) id_deltas[i] = get_u16(font);
                
                int id_range_offsets_size = length - (font->current_byte - (cmap_offset + subtable_offset));
                uint16_t *id_range_offsets = malloc(sizeof(uint16_t) * id_range_offsets_size);
                for (int i = 0; i < id_range_offsets_size; ++i) id_range_offsets[i] = get_u16(font);
                
                int highest_code = end_codes[seg_count - 1];

                font->unicode_to_glyph_indicies = malloc(sizeof(uint16_t) * highest_code + 10);

                // loop over segments.
                // a segment is a range of unicode characters that are all mapped with the same formula.
                // i dont understand this at all
                for (int i = 0; i < seg_count; i++) {
                    for (int j = start_codes[i]; j <= end_codes[i]; j++) {
                        uint16_t glyph_index = 0;
                        if (id_range_offsets[i] == 0) {
                            glyph_index = (j + id_deltas[i]) & 0xFFFF;
                        } else {
                            // Compute address relative to this idRangeOffset word
                            int offset = id_range_offsets[i]/2 + (j - start_codes[i]);
                            int16_t *glyphIdArray = (int16_t *)(&id_range_offsets[i]);
                            glyph_index = glyphIdArray[offset];

                            if (glyph_index != 0) {
                                glyph_index = (glyph_index + id_deltas[i]) & 0xFFFF;
                            }
                        }
                        font->unicode_to_glyph_indicies[j] = glyph_index;
                    }
                }

                free(end_codes);
                free(start_codes);
                free(id_deltas);
                free(id_range_offsets);

                break; // stop because we only need one
            } else { // excuse
                // printf("cant read this becuase dont want to");
            }

            font->current_byte = last_byte_offset;
        }
    }

    // hhea table
    {
        font->current_byte = hhea_offset;
        
        skip_32(font); // skip version

        font->hhea_table.ascender = get_i16(font); // typographic ascent
        font->hhea_table.descender = get_i16(font); // typographic descent
        font->hhea_table.line_gap = get_i16(font); // typographic line gap
        font->hhea_table.advance_max_width = get_u16(font); // max advance width for entry in hmtx table
        font->hhea_table.min_left_side_bearing = get_i16(font); // "Minimum left sidebearing value in 'hmtx' table for glyphs with contours (empty glyphs should be ignored)."
        font->hhea_table.min_right_side_bearing = get_i16(font); // "Minimum right sidebearing value; calculated as min(aw - (lsb + xMax - xMin)) for glyphs with contours (empty glyphs should be ignored)."
        font->hhea_table.x_max_extent = get_i16(font); // "Max(lsb + (xMax - xMin))"
        font->hhea_table.carat_slope_rise = get_i16(font); // used to calculate the slope of the cursor (1 for vertical)
        font->hhea_table.carat_slope_run = get_i16(font); // 0 for vertical
        font->hhea_table.carat_offset = get_i16(font); // "The amount by which a slanted highlight on a glyph needs to be shifted to produce the best appearance. Set to 0 for non-slanted fonts"
        //                                       ^^^^too tired to make this simpler
        skip_32(font); // skip reserved
        skip_32(font); // (why is there reserved in the first place? for extra stuff later on? just add it to the end?)
        font->hhea_table.metric_data_format = get_i16(font); // "0 for current format" (?)
        font->hhea_table.number_of_h_metrics = get_u16(font); // h_metric entry count in hmtx table
    }

    // hmtx table
    {
        font->current_byte = hmtx_offset;

        font->h_metrics = malloc(sizeof(SP_long_hor_metric) * font->number_of_glyphs); // advance width and left side bearings for each glyph
        // font->left_side_bearings = malloc(sizeof(int16_t) * font->number_of_glyphs); // left side bearings for glyphs with IDs >= number_of_h_metrics
        // ^^^^^^^^^^^^^^^^^^^ this is useful for monospaced fonts where most everything has the same left side bearing. it saves on memory
        // ^^^^^^^^^^^^^^^^^^^ when number_of_h_metrics is < number of glyphs, the last advance width applies to all
        // ^^^^^^^^^^^^^^^^^^^ to make this easier on the user -- and because memory really isn't a problem anymore -- I'll only use h_metrics and make it the size of number of glyphs

        for (int i = 0; i < font->number_of_glyphs; ++i){
            if (i < font->hhea_table.number_of_h_metrics) font->h_metrics[i] = (SP_long_hor_metric){get_u16(font), get_i16(font)}; 
            else font->h_metrics[i] = (SP_long_hor_metric){font->h_metrics[font->hhea_table.number_of_h_metrics - 1].advance_width, get_i16(font)}; 
        }
    }

    // read glyphs
    {
        font->current_byte = glyph_offset;

        font->glyphs = malloc(sizeof(SP_glyph) * font->number_of_glyphs);

        for (int i = 0; i < font->number_of_glyphs; ++i){
            read_glyph(font, i);
        }
    }

    return font;
}

void SP_free_font(SP_font *font){
    free(font->buffer);
    free(font->glyph_offsets);
    
    for (int i = 0; i < font->number_of_glyphs; ++i){
        if (!font->glyphs[i].is_composite){
            free(font->glyphs[i].contour_end_indicies);
            free(font->glyphs[i].flags);
            free(font->glyphs[i].x_coords);
            free(font->glyphs[i].y_coords);
        } else {
            free(font->glyphs[i].components);
        }    
    }
    
    free(font->glyphs);
    free(font->unicode_to_glyph_indicies);
    free(font->left_side_bearings);
    free(font->h_metrics);

    free(font->kerning_pairs);

    free(font);
}