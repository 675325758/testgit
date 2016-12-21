#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mp4_fmt.h"


enum AVColorSpace{
    AVCOL_SPC_RGB         = 0,
    AVCOL_SPC_BT709       = 1, ///< also ITU-R BT1361 / IEC 61966-2-4 xvYCC709 / SMPTE RP177 Annex B
    AVCOL_SPC_UNSPECIFIED = 2,
    AVCOL_SPC_FCC         = 4,
    AVCOL_SPC_BT470BG     = 5, ///< also ITU-R BT601-6 625 / ITU-R BT1358 625 / ITU-R BT1700 625 PAL & SECAM / IEC 61966-2-4 xvYCC601
    AVCOL_SPC_SMPTE170M   = 6, ///< also ITU-R BT601-6 525 / ITU-R BT1358 525 / ITU-R BT1700 NTSC / functionally identical to above
    AVCOL_SPC_SMPTE240M   = 7,
    AVCOL_SPC_YCOCG       = 8, ///< Used by Dirac / VC-2 and H.264 FRext, see ITU-T SG16
    AVCOL_SPC_NB             , ///< Not part of ABI
};

/**
 * Sequence parameter set
 */
typedef struct SPS {
    int profile_idc;
    int level_idc;
    int chroma_format_idc;
    int transform_bypass;              ///< qpprime_y_zero_transform_bypass_flag
    int log2_max_frame_num;            ///< log2_max_frame_num_minus4 + 4
    int poc_type;                      ///< pic_order_cnt_type
    int log2_max_poc_lsb;              ///< log2_max_pic_order_cnt_lsb_minus4
    int delta_pic_order_always_zero_flag;
    int offset_for_non_ref_pic;
    int offset_for_top_to_bottom_field;
    int poc_cycle_length;              ///< num_ref_frames_in_pic_order_cnt_cycle
    int ref_frame_count;               ///< num_ref_frames
    int gaps_in_frame_num_allowed_flag;
    int mb_width;                      ///< pic_width_in_mbs_minus1 + 1
    int mb_height;                     ///< pic_height_in_map_units_minus1 + 1
    int frame_mbs_only_flag;
    int mb_aff;                        ///< mb_adaptive_frame_field_flag
    int direct_8x8_inference_flag;
    int crop;                          ///< frame_cropping_flag
    unsigned int crop_left;            ///< frame_cropping_rect_left_offset
    unsigned int crop_right;           ///< frame_cropping_rect_right_offset
    unsigned int crop_top;             ///< frame_cropping_rect_top_offset
    unsigned int crop_bottom;          ///< frame_cropping_rect_bottom_offset
    int vui_parameters_present_flag;
    //AVRational sar;
    int video_signal_type_present_flag;
    int full_range;
    int colour_description_present_flag;
    //enum AVColorPrimaries color_primaries;
    //enum AVColorTransferCharacteristic color_trc;
    enum AVColorSpace colorspace;
    int timing_info_present_flag;
    uint32_t num_units_in_tick;
    uint32_t time_scale;
    int fixed_frame_rate_flag;
    short offset_for_ref_frame[256]; // FIXME dyn aloc?
    int bitstream_restriction_flag;
    int num_reorder_frames;
    int scaling_matrix_present;
    uint8_t scaling_matrix4[6][16];
    uint8_t scaling_matrix8[6][64];
    int nal_hrd_parameters_present_flag;
    int vcl_hrd_parameters_present_flag;
    int pic_struct_present_flag;
    int time_offset_length;
    int cpb_cnt;                          ///< See H.264 E.1.2
    int initial_cpb_removal_delay_length; ///< initial_cpb_removal_delay_length_minus1 + 1
    int cpb_removal_delay_length;         ///< cpb_removal_delay_length_minus1 + 1
    int dpb_output_delay_length;          ///< dpb_output_delay_length_minus1 + 1
    int bit_depth_luma;                   ///< bit_depth_luma_minus8 + 8
    int bit_depth_chroma;                 ///< bit_depth_chroma_minus8 + 8
    int residual_color_transform_flag;    ///< residual_colour_transform_flag
    int constraint_set_flags;             ///< constraint_set[0-3]_flag
} SPS;

typedef struct GetBitContext {
    const uint8_t *buffer, *buffer_end;
    int index;
    int size_in_bits;
    int size_in_bits_plus8;
} GetBitContext;

/****************************************************************************/

#undef	BITSTREAM_READER_LE 
#define	UNCHECKED_BITSTREAM_READER 1

#if defined(__GNUC__)
#    define av_unused __attribute__((unused))
#else
#    define av_unused
#endif


#define OPEN_READER(name, gb)                   \
    unsigned int name##_index = (gb)->index;    \
    av_unused unsigned int name##_cache

#define HAVE_BITS_REMAINING(name, gb) 1

#   define AV_RB64(x)                                   \
    (((uint64_t)((const uint8_t*)(x))[0] << 56) |       \
     ((uint64_t)((const uint8_t*)(x))[1] << 48) |       \
     ((uint64_t)((const uint8_t*)(x))[2] << 40) |       \
     ((uint64_t)((const uint8_t*)(x))[3] << 32) |       \
     ((uint64_t)((const uint8_t*)(x))[4] << 24) |       \
     ((uint64_t)((const uint8_t*)(x))[5] << 16) |       \
     ((uint64_t)((const uint8_t*)(x))[6] <<  8) |       \
      (uint64_t)((const uint8_t*)(x))[7])

#   define AV_RL64(x)                                   \
    (((uint64_t)((const uint8_t*)(x))[7] << 56) |       \
     ((uint64_t)((const uint8_t*)(x))[6] << 48) |       \
     ((uint64_t)((const uint8_t*)(x))[5] << 40) |       \
     ((uint64_t)((const uint8_t*)(x))[4] << 32) |       \
     ((uint64_t)((const uint8_t*)(x))[3] << 24) |       \
     ((uint64_t)((const uint8_t*)(x))[2] << 16) |       \
     ((uint64_t)((const uint8_t*)(x))[1] <<  8) |       \
      (uint64_t)((const uint8_t*)(x))[0])


#   define AV_RB32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[0] << 24) |    \
               (((const uint8_t*)(x))[1] << 16) |    \
               (((const uint8_t*)(x))[2] <<  8) |    \
                ((const uint8_t*)(x))[3])


#   define AV_RL32(x)                                \
    (((uint32_t)((const uint8_t*)(x))[3] << 24) |    \
               (((const uint8_t*)(x))[2] << 16) |    \
               (((const uint8_t*)(x))[1] <<  8) |    \
                ((const uint8_t*)(x))[0])



////////////////////////////////


#ifdef BITSTREAM_READER_LE

# ifdef LONG_BITSTREAM_READER
#   define UPDATE_CACHE(name, gb) name##_cache = \
        AV_RL64((gb)->buffer + (name##_index >> 3)) >> (name##_index & 7)
# else
#   define UPDATE_CACHE(name, gb) name##_cache = \
        AV_RL32((gb)->buffer + (name##_index >> 3)) >> (name##_index & 7)
# endif

# define SKIP_CACHE(name, gb, num) name##_cache >>= (num)

#else

# ifdef LONG_BITSTREAM_READER
#   define UPDATE_CACHE(name, gb) name##_cache = \
        AV_RB64((gb)->buffer + (name##_index >> 3)) >> (32 - (name##_index & 7))
# else
#   define UPDATE_CACHE(name, gb) name##_cache = \
        AV_RB32((gb)->buffer + (name##_index >> 3)) << (name##_index & 7)
# endif

# define SKIP_CACHE(name, gb, num) name##_cache <<= (num)

#endif

#ifndef NEG_SSR32
#   define NEG_SSR32(a,s) ((( int32_t)(a))>>(32-(s)))
#endif

#ifndef NEG_USR32
#   define NEG_USR32(a,s) (((uint32_t)(a))>>(32-(s)))
#endif

#ifdef BITSTREAM_READER_LE
#   define SHOW_UBITS(name, gb, num) zero_extend(name##_cache, num)
#   define SHOW_SBITS(name, gb, num) sign_extend(name##_cache, num)
#else
#   define SHOW_UBITS(name, gb, num) NEG_USR32(name##_cache, num)
#   define SHOW_SBITS(name, gb, num) NEG_SSR32(name##_cache, num)
#endif

#if UNCHECKED_BITSTREAM_READER
#   define SKIP_COUNTER(name, gb, num) name##_index += (num)
#else
#   define SKIP_COUNTER(name, gb, num) \
    name##_index = FFMIN(name##_size_plus8, name##_index + (num))
#endif


#define GET_CACHE(name, gb) ((uint32_t)name##_cache)



#define LAST_SKIP_BITS(name, gb, num) SKIP_COUNTER(name, gb, num)

#define CLOSE_READER(name, gb) (gb)->index = name##_index

static inline unsigned zero_extend(unsigned val, unsigned bits)
{
    return (val << ((8 * sizeof(int)) - bits)) >> ((8 * sizeof(int)) - bits);
}

static inline  int sign_extend(int val, unsigned bits)
{
    unsigned shift = 8 * sizeof(int) - bits;
    union { unsigned u; int s; } v = { (unsigned) val << shift };
    return v.s >> shift;
}

/****************************************************************************/

/* ff_h264_decode_seq_parameter_set */

/**
 * Read 1-25 bits.
 */
static unsigned int get_bits(GetBitContext *s, int n)
{
    register int tmp;
    OPEN_READER(re, s);
    //av_assert2(n>0 && n<=25);
    UPDATE_CACHE(re, s);
    tmp = SHOW_UBITS(re, s, n);
    LAST_SKIP_BITS(re, s, n);
    CLOSE_READER(re, s);
    return tmp;
}

static unsigned int get_bits1(GetBitContext *s)
{
    unsigned int index = s->index;
    uint8_t result = s->buffer[index>>3];
#ifdef BITSTREAM_READER_LE
    result >>= index & 7;
    result &= 1;
#else
    result <<= index & 7;
    result >>= 8 - 1;
#endif
#if !UNCHECKED_BITSTREAM_READER
    if (s->index < s->size_in_bits_plus8)
#endif
        index++;
    s->index = index;

    return result;
}

const uint8_t my_ff_golomb_vlc_len[512]={
19,17,15,15,13,13,13,13,11,11,11,11,11,11,11,11,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
};

const uint8_t my_ff_ue_golomb_vlc_code[512]={
32,32,32,32,32,32,32,32,31,32,32,32,32,32,32,32,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,
 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 9,10,10,10,10,11,11,11,11,12,12,12,12,13,13,13,13,14,14,14,14,
 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4,
 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

const int8_t my_ff_se_golomb_vlc_code[512]={
 17, 17, 17, 17, 17, 17, 17, 17, 16, 17, 17, 17, 17, 17, 17, 17,  8, -8,  9, -9, 10,-10, 11,-11, 12,-12, 13,-13, 14,-14, 15,-15,
  4,  4,  4,  4, -4, -4, -4, -4,  5,  5,  5,  5, -5, -5, -5, -5,  6,  6,  6,  6, -6, -6, -6, -6,  7,  7,  7,  7, -7, -7, -7, -7,
  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3,  3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3, -3,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
 -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
};

	
 /**
 * read unsigned exp golomb code, constraint to a max of 31.
 * the return value is undefined if the stored value exceeds 31.
 */
static int get_ue_golomb_31(GetBitContext *gb){
    unsigned int buf;

    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);
    buf=GET_CACHE(re, gb);

    buf >>= 32 - 9;
    LAST_SKIP_BITS(re, gb, my_ff_golomb_vlc_len[buf]);
    CLOSE_READER(re, gb);

    return my_ff_ue_golomb_vlc_code[buf];
}

#define av_log2       av_log2_c

const uint8_t mp4_ff_log2_tab[256]={
        0,0,1,1,2,2,2,2,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
        5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
        7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7
};

static int av_log2_c(unsigned int v)
{
    int n = 0;
    if (v & 0xffff0000) {
        v >>= 16;
        n += 16;
    }
    if (v & 0xff00) {
        v >>= 8;
        n += 8;
    }
    n += mp4_ff_log2_tab[v];

    return n;
}


/****************************************************************************/

static void init_get_bits(GetBitContext *s, const uint8_t *buffer,
                                 int bit_size)
{
    int buffer_size = (bit_size+7)>>3;
    if (buffer_size < 0 || bit_size < 0) {
        buffer_size = bit_size = 0;
        buffer = NULL;
    }

    s->buffer       = buffer;
    s->size_in_bits = bit_size;
    s->size_in_bits_plus8 = bit_size + 8;
    s->buffer_end   = buffer + buffer_size;
    s->index        = 0;
}

 /**
 * read unsigned exp golomb code.
 */
static inline int get_ue_golomb(GetBitContext *gb){
    unsigned int buf;
    int log;

    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);
    buf=GET_CACHE(re, gb);

    if(buf >= (1<<27)){
        buf >>= 32 - 9;
        LAST_SKIP_BITS(re, gb, my_ff_golomb_vlc_len[buf]);
        CLOSE_READER(re, gb);

        return my_ff_ue_golomb_vlc_code[buf];
    }else{
        log= 2*av_log2(buf) - 31;
        buf>>= log;
        buf--;
        LAST_SKIP_BITS(re, gb, 32 - log);
        CLOSE_READER(re, gb);

        return buf;
    }
}

/**
 * read signed exp golomb code.
 */
static inline int get_se_golomb(GetBitContext *gb){
    unsigned int buf;
    int log;

    OPEN_READER(re, gb);
    UPDATE_CACHE(re, gb);
    buf=GET_CACHE(re, gb);

    if(buf >= (1<<27)){
        buf >>= 32 - 9;
        LAST_SKIP_BITS(re, gb, my_ff_golomb_vlc_len[buf]);
        CLOSE_READER(re, gb);

        return my_ff_se_golomb_vlc_code[buf];
    }else{
        log= 2*av_log2(buf) - 31;
        buf>>= log;

        LAST_SKIP_BITS(re, gb, 32 - log);
        CLOSE_READER(re, gb);

        if(buf&1) buf= -(buf>>1);
        else      buf=  (buf>>1);

        return buf;
    }
}

#define MAX_SPS_COUNT	32
#define MAX_PICTURE_COUNT 32
#define FF_ARRAY_ELEMS(a) (sizeof(a) / sizeof((a)[0]))

int h264_decode_sps(u8 *data, int len, int *width, int *height)
{
    SPS *sps;
    int i;
    unsigned int sps_id;
    int profile_idc, level_idc, constraint_set_flags = 0;
	GetBitContext gb;

	init_get_bits(&gb, data, len*8);
	

    profile_idc= get_bits(&gb, 8);
    constraint_set_flags |= get_bits1(&gb) << 0;   //constraint_set0_flag
    constraint_set_flags |= get_bits1(&gb) << 1;   //constraint_set1_flag
    constraint_set_flags |= get_bits1(&gb) << 2;   //constraint_set2_flag
    constraint_set_flags |= get_bits1(&gb) << 3;   //constraint_set3_flag
    constraint_set_flags |= get_bits1(&gb) << 4;   //constraint_set4_flag
    constraint_set_flags |= get_bits1(&gb) << 5;   //constraint_set5_flag
    get_bits(&gb, 2); // reserved
    level_idc= get_bits(&gb, 8);
    sps_id= get_ue_golomb_31(&gb);

    if(sps_id >= MAX_SPS_COUNT) {
        printf("sps_id (%d) out of range\n", sps_id);
        return -1;
    }
	sps = (SPS *)calloc(sizeof(SPS), 1);
    if(sps == NULL)
        return -1;

    sps->time_offset_length = 24;
    sps->profile_idc= profile_idc;
    sps->constraint_set_flags = constraint_set_flags;
    sps->level_idc= level_idc;
    sps->full_range = -1;

    sps->scaling_matrix_present = 0;
    sps->colorspace = 2; //AVCOL_SPC_UNSPECIFIED

    if(sps->profile_idc == 100 || sps->profile_idc == 110 ||
       sps->profile_idc == 122 || sps->profile_idc == 244 || sps->profile_idc ==  44 ||
       sps->profile_idc ==  83 || sps->profile_idc ==  86 || sps->profile_idc == 118 ||
       sps->profile_idc == 128 ) {
        sps->chroma_format_idc= get_ue_golomb_31(&gb);
        if (sps->chroma_format_idc > 3U) {
            printf("chroma_format_idc %d is illegal\n", sps->chroma_format_idc);
            goto fail;
        } else if(sps->chroma_format_idc == 3) {
            sps->residual_color_transform_flag = get_bits1(&gb);
            if(sps->residual_color_transform_flag) {
                printf("separate color planes are not supported\n");
                goto fail;
            }
        }
        sps->bit_depth_luma   = get_ue_golomb(&gb) + 8;
        sps->bit_depth_chroma = get_ue_golomb(&gb) + 8;
        if (sps->bit_depth_luma > 14U || sps->bit_depth_chroma > 14U) {
            printf("illegal bit depth value (%d, %d)\n",
                   sps->bit_depth_luma, sps->bit_depth_chroma);
            goto fail;
        }
        sps->transform_bypass = get_bits1(&gb);
        //decode_scaling_matrices(h, sps, NULL, 1, sps->scaling_matrix4, sps->scaling_matrix8);
    }else{
        sps->chroma_format_idc= 1;
        sps->bit_depth_luma   = 8;
        sps->bit_depth_chroma = 8;
    }

    sps->log2_max_frame_num= get_ue_golomb(&gb) + 4;
    if (sps->log2_max_frame_num < 4 || sps->log2_max_frame_num > 16) {
        printf("illegal log2_max_frame_num %d\n", sps->log2_max_frame_num);
        goto fail;
    }

    sps->poc_type= get_ue_golomb_31(&gb);

    if(sps->poc_type == 0){ //FIXME #define
        unsigned t = get_ue_golomb(&gb);
        if(t>12){
            printf("log2_max_poc_lsb (%d) is out of range\n", t);
            goto fail;
        }
        sps->log2_max_poc_lsb= t + 4;
    } else if(sps->poc_type == 1){//FIXME #define
        sps->delta_pic_order_always_zero_flag= get_bits1(&gb);
        sps->offset_for_non_ref_pic= get_se_golomb(&gb);
        sps->offset_for_top_to_bottom_field= get_se_golomb(&gb);
        sps->poc_cycle_length                = get_ue_golomb(&gb);

        if((unsigned)sps->poc_cycle_length >= FF_ARRAY_ELEMS(sps->offset_for_ref_frame)){
            printf("poc_cycle_length overflow %u\n", sps->poc_cycle_length);
            goto fail;
        }

        for(i=0; i<sps->poc_cycle_length; i++)
            sps->offset_for_ref_frame[i]= get_se_golomb(&gb);
    }else if(sps->poc_type != 2){
        printf("illegal POC type %d\n", sps->poc_type);
        goto fail;
    }

    sps->ref_frame_count= get_ue_golomb_31(&gb);
    if(sps->ref_frame_count > MAX_PICTURE_COUNT-2 || sps->ref_frame_count > 16U){
        printf("too many reference frames\n");
        goto fail;
    }
    sps->gaps_in_frame_num_allowed_flag= get_bits1(&gb);
	// TODO: Get it, HA HA HA
    sps->mb_width = get_ue_golomb(&gb) + 1;
    sps->mb_height= get_ue_golomb(&gb) + 1;

	*width = sps->mb_width*16;
	*height = sps->mb_height*16;
	
    free(sps);
    return 0;
	
fail:
    free(sps);
    return -1;	
}

u8 *h264_get_next_block(u8 *data, int len)
{
	u8 *end = data + len - 4;

	if (len < 4)
		return NULL;
	
	for ( ; data < end; data++) {
//printf("%02X ", *data);		
		/* lookup 00 00 00 01 */
		if (*data != 0x00)
			continue;
		if (*(u32 *)data == htonl(0x00000001))
			return data;
	}
//printf("\n");	

	return NULL;
}

#define	NALU_SPS	7
#define	NALU_PPS	8

int h264_get_pic_size(u8 *data, int len, int *width, int *height)
{
	u8 *end = data + len;
	
	/* parse NAL，一般PSP块在第一块 */
	while (data != NULL && data < end - 4) {
		if ((data = h264_get_next_block(data, (int)(end - data))) == NULL)
			break;
		
		data += 4;
		if (data + 1 < end && (*data & 0x1F) == NALU_SPS) {
			data++;
			return h264_decode_sps(data, (int)(end - data), width, height);
		}
	}

	return -1;
}

#if 0

static u8 sps_test1[] = {
	0x42, 0x40, 0x1F, 0x96, 0x54, 0x0A, 0x0F, 0xD0, 0x0F, 0x39, 0xEA
};

int main(int argc, char **argv)
{
	int ret;
	int w = 0, h = 0;
	
	ret = h264_decode_sps(sps_test1, sizeof(sps_test1), &w, &h);
	printf("ret=%d, w=%d, h=%d\n", ret, w, h);

	return 0;
}

#endif

