/* gzip -- demo program for gzip/gunzip

   Copyright (C) 2010 Gilles Buloz

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "gzip.h"

#define DYN_ALLOC /* use dynamic allocation of memory instead of static arrays (saves some memory) */
//#define MAXSEG_64K /* for systems with 64K segment limit */
#define MESSAGES /* print error/warning messages */

#define ZIP_OUTBUFSIZ 512  /* output buffer size. Must be at least 2 */
/* This is also the blocksize used when writing to output */
//#define ZIP_OUTMINBLKSIZE 4 /* minimum blocksize suported by output device/file. MUST BE A POWER OF TWO */
/* Not used when output is to memory */
/* Used to pad the last write to the minimum suported size */
/* If commented out, the minimum blocksize is 1 */
//#define ZIP_VERY_SMALL_MEM_USAGE /* to have a very small memory usage, but with a little bit less compression ratio */

#define UNZIP_INBUFSIZ 512 /* input buffer size. MUST BE A POWER OF TWO. */
/* Not used when input is from memory */
/* Used as blocksize when input if from a device */
/* Must be at least UNZIP_ONTHEFLY_MIN_LOOKAHEAD for on-the-fly input (see below) */
/* For on the fly processing, the input buffer is used as a circular buffer */
#define UNZIP_OUTBUFSIZ 512 /* output buffer size. MUST BE A POWER OF TWO */
/* Not used when output is to memory */
/* The size of this buffer is also the blocksize used when writing to the output file/device (unless forced below) */
/* To no use any buffer (blocksize = 1), comment out the #define above */
/* To check CRC of the gziped data before doing the real gunzip, set this value to at least the window size (WSIZE) */
/* used when the data was gziped; default is 32768. A lower value can be used if the data has been gziped with */
/* gziplite using the option ZIP_VERY_SMALL_MEM_USAGE */
/* This buffer is mandatory if the output device does not support lseek(). In this can set its size to at least */
/* the window size (WSIZE) used when the data was gziped */
//#define UNZIP_OUTMAXBLKSIZE 64 /* maximum blocksize suported by output device/file. MUST BE A DIVISOR OF UNZIP_OUTBUFSIZ */
/* Not used when output is to memory */
/* If commented out, the maximum blocksize is UNZIP_OUTBUFSIZ */
//#define UNZIP_OUTMINBLKSIZE 4 /* minimum blocksize suported by output device/file. MUST BE A POWER OF TWO */
/* Not used when output is to memory */
/* Used to pad the last write to the minimum suported size */
/* If commented out, the minimum blocksize is 1 */
#define UNZIP_ONTHEFLY_MIN_LOOKAHEAD 256 /* minimum amount of data required in input buffer for on-the-fly-unzip */
/* When unzip_add_data() is called, the input buffer is filled with the supplied data */
/* each time the input buffer is full, it is processed till the minimum lookahead is reached */
/* then the input buffer is filled again until all the data supplied has been added */
/* If this value is too low, we get input underrun errors */
#define UNZIP_CODES_DATA_OVERLAY /* overlay data for fixed and dynamic codes */
/* This saves some memory but fixed data must be inited each time a fixed block is processed, */
/* so unzip may be a little bit slower */

#define OF(args)  args

#define FD_MEMORY     (-1) /* set ifd or ofd to this value for input from / output to memory */
#define FD_ON_THE_FLY (-2) /* set ifd to this value for on the fly compression */

#define DEMO_MEMORY_BUFFER_SIZE (1UL << 20) /* used for input or output to memory */

/*
 * NOTE for indatasize and outmaxsize for unzip_init() / zip_init() below
 *
 * indatasize : Mandatory when ziping from memory
 *              For gunzip from memory or gzip/gunzip from a file, it is used to
 *              limit the size of data read. An input_error is set if more
 *              data is required to complete. Not used when input is from on-the-fly
 *              Use value -1 for "no limit"
 *
 * outmaxsize : Maximum size allowed to write to output, or -1 for "no limit"
 *              If more data to write is needed, an output_error is set
 */

typedef void *voidp;
typedef int file_t; /* Do not use stdio */

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#ifdef MESSAGES
#define msg_print(param,...) //_log_debug(__VA_ARGS__)
#else
#define msg_print(param)
#endif

#ifdef MAXSEG_64K
#  define fcalloc(items,size) calloc((items),(size))
#else
#  define fcalloc(items,size) malloc((size_t)(items)*(size_t)(size))
#endif
#define fcfree(ptr) free(ptr)

#define memzero(s, n) memset ((voidp)(s), 0, (n))

#define OS_CODE  0x03  /* assume Unix */
#define near
#define	GZIP_MAGIC "\037\213" /* Magic header for gzip files, 1F 8B */

/* Compression methods (see algorithm.doc) */
#define STORED      0
#define COMPRESSED  1
#define PACKED      2
#define LZHED       3
/* methods 4 to 7 reserved */
#define DEFLATED    8

#define EXTERN(type, array)  extern type * near array = NULL
#define DECLARE(type, array, size)  type * near array = NULL
#ifdef DYN_ALLOC
#  define ALLOC(type, array, size, rc) {	       		      \
	if (!array) {\
		array = (type*)fcalloc((size_t)(((size)+1L)/2), 2*sizeof(type)); \
		if (!array) {msg_print(stderr, "ERROR: allocating memory\n"); rc |= -1;} \
	}\
   }
#  define FREE(array) {if (array != NULL) fcfree(array), array=NULL;}
#else
#  define ALLOC(type, array, size, rc) { \
     static type near _array[size]; \
     array = _array; \
     }
#  define FREE(array)
#endif

/* Diagnostic functions */
#ifdef DEBUG
extern int verbose;
#  define Assert(cond,msg) {if (!(cond)) fprintf(stderr, "ERROR: %s\n", (msg));}
#  define Trace(x) fprintf x
#  define Tracev(x) {if (verbose) fprintf x ;}
#  define Tracevv(x) {if (verbose>1) fprintf x ;}
#  define Tracec(c,x) {if (verbose && (c)) fprintf x ;}
#  define Tracecv(c,x) {if (verbose>1 && (c)) fprintf x ;}
#else
#  define Assert(cond,msg)
#  define Trace(x)
#  define Tracev(x)
#  define Tracevv(x)
#  define Tracec(c,x)
#  define Tracecv(c,x)
#endif

//#define FROM_MEMORY /* read data from a buffer in memory rather than from a file descriptor */
//#define FROM_ON_THE_FLY /* data to be ziped/unziped is supplied "on the fly" by calling zip/unzip_add_data() */
//#define TO_MEMORY /* write data to a buffer in memory rather than to a file descriptor */


#define SMALL_MEM /* for gzip to have a little bit less memory usage */

/* For compression, input is done in window[]. For decompression, output */
/* is done in window */

#define OUTBUFSIZ ZIP_OUTBUFSIZ
#ifndef ZIP_OUTMINBLKSIZE
#  define ZIP_OUTMINBLKSIZE 1
#endif

/* standard gzip values */
#define DIST_BUFSIZE 0x8000  /* buffer for distances, see trees.c */
#define HASH_BITS    15      /* Number of bits used to hash strings (for 16 bit machines, do not use values above 15) */
#define BITS         16      /* >= HASH_BITS+1 */
#define WSIZE        0x8000  /* >= 2^(BITS-1). window size. must be a power of two (32K is used in standard gzip) */

#ifdef SMALL_MEMc
#  undef DIST_BUFSIZE
#  undef HASH_BITS
#  define DIST_BUFSIZE 0x2000
#  define HASH_BITS    13
#endif

#ifdef ZIP_VERY_SMALL_MEM_USAGE
#  undef DIST_BUFSIZE
#  undef HASH_BITS
#  undef BITS
#  undef WSIZE
#  define DIST_BUFSIZE 0x800
#  define HASH_BITS    11
#  define BITS         12     /* >= HASH_BITS+1 */
#  define WSIZE        0x800  /* >= 2^(BITS-1) */
#endif

/* The minimum and maximum match lengths */
#define MIN_MATCH  3
#define MAX_MATCH  258

#define LIT_BUFSIZE DIST_BUFSIZE

DECLARE(uch, l_buf,  LIT_BUFSIZE);
DECLARE(uch, window, 2L*WSIZE);
DECLARE(uch, outbuf, OUTBUFSIZ);
DECLARE(ush, d_buf,  DIST_BUFSIZE);
#ifndef MAXSEG_64K
#   define tab_prefix prev    /* hash link (see below) */
#   define head (prev+WSIZE)  /* hash head (see below) */
    DECLARE(ush, tab_prefix, 1L<<BITS);
#else
#   define tab_prefix0 prev
#   define head tab_prefix1
    DECLARE(ush, tab_prefix0, 1L<<(BITS-1));
    DECLARE(ush, tab_prefix1, 1L<<(BITS-1));
#endif

#define tab_suffix window

/* internal file attribute */
#define UNKNOWN 0xffff
#define BINARY  0
#define ASCII   1

#define MIN_LOOKAHEAD (MAX_MATCH+MIN_MATCH+1)
/* Minimum amount of lookahead, except at the end of the input file.
 * See deflate.c for comments about the MIN_MATCH+1.
 */

#define MAX_DIST  (WSIZE-MIN_LOOKAHEAD)
/* In order to simplify the code, particularly on 16 bit machines, match
 * distances are limited to MAX_DIST instead of WSIZE.
 */

#define seekable()    0  /* force sequential output */
/* put_byte is used for the compressed output, put_ubyte for the
 * uncompressed output. However unlzw() uses window for its
 * suffix table instead of its output buffer, so it does not use put_ubyte
 * (to be cleaned up).
 */

#define put_byte(c) {outbuf[outcnt++]=(uch)(c); if (outcnt==OUTBUFSIZ)\
   flush_outbuf();}

/* Output a 16 bit value, lsb first */
#define put_short(w) \
{ if (outcnt < OUTBUFSIZ-2) { \
    outbuf[outcnt++] = (uch) ((w) & 0xff); \
    outbuf[outcnt++] = (uch) ((ush)(w) >> 8); \
  } else { \
    put_byte((uch)((w) & 0xff)); \
    put_byte((uch)((ush)(w) >> 8)); \
  } \
}

/* Output a 32 bit value to the bit stream, lsb first */
#define put_long(n) { \
    put_short((n) & 0xffff); \
    put_short(((ulg)(n)) >> 16); \
}

static int level = 6;        /* compression level (also used by trees.c) */
static unsigned near strstart; /* start of string to insert */
static long block_start;
/* window position at the beginning of the current output block. Gets
 * negative when the window is moved backwards.
 */

//增加两个接口
static bool sdk_zip_init = false;
static u_int8_t *psdk_out_buff = NULL;


static int test_mode;             /* test mode : do not write anything */

static int  ifd;                  /* input file descriptor */
static int  ofd;                  /* output file descriptor */
static int bytes_in;              /* number of input bytes */
static int bytes_out;             /* number of output bytes */
static int on_the_fly_state;
static int on_the_fly_eofile;
static char *on_the_fly_data_ptr;
static off_t on_the_fly_data_size;

static unsigned insize;           /* valid bytes in inbuf */
static unsigned inptr;            /* index of next byte to be processed in inbuf */
static unsigned outcnt;           /* bytes in output buffer */

static int method;                /* compression method */

static ulg crc;                   /* crc on uncompressed file data */

#ifdef DEBUG
int verbose = 0;
#endif

static char *input_data_ptr;      /* input buffer address when ifd = FD_MEMORY */
static char *output_data_ptr;     /* output buffer address when ofd = FD_MEMORY */
static off_t input_data_size;     /* required when ziping from memory */
static off_t output_max_size;     /* maximum size to write to output */

static int input_error;           /* an error occured while reading (I/O error or input max size reached) */
static int output_error;          /* an error occured while writing (I/O error or output max size reached) */

static void zip_unzip_restart(int test)
{
    outcnt = 0;
    if (ifd == FD_MEMORY)
	insize = input_data_size; /* input buffer already contains all input data */
    else
	insize = 0; /* no data in input buffer */
    inptr = 0; /* clear input buffer index */
    bytes_in = bytes_out = 0L;
    on_the_fly_state = 0;
    on_the_fly_eofile = 0;
    test_mode = test;
    input_error = 0;
    output_error = 0;
}

/* ========================================================================
 * Table of CRC-32's of all single-byte values (made by makecrc.c)
 */
static const ulg crc_32_tab[] = {
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

/* ===========================================================================
 * Run a set of bytes through the crc shift register.  If s is a NULL
 * pointer, then initialize the crc shift register contents instead.
 * Return the current crc in either case.
 */
static ulg updcrc(s, n)
    uch *s;                 /* pointer to bytes to pump through */
    unsigned n;             /* number of bytes in s[] */
{
    register ulg c;         /* temporary variable */

    static ulg crc = (ulg)0xffffffffL; /* shift register contents */

    if (s == NULL) {
	c = 0xffffffffL;
    } else {
	c = crc;
        if (n) do {
            c = crc_32_tab[((int)c ^ (*s++)) & 0xff] ^ (c >> 8);
        } while (--n);
    }
    crc = c;
    return c ^ 0xffffffffL;       /* (instead of ~c for 64-bit machines) */
}





/***************************zip*******************************************/

static int zip_init(int infd, int outfd, char *indataptr, char *outdataptr, int indatasize, int outmaxsize)
{
    int rc = 0;

    /* Allocate all global buffers (for DYN_ALLOC option) */
    ALLOC(uch, l_buf,  LIT_BUFSIZE, rc);
    ALLOC(uch, outbuf, OUTBUFSIZ, rc);
    ALLOC(ush, d_buf,  DIST_BUFSIZE, rc);
    ALLOC(uch, window, 2L*WSIZE, rc);
#ifndef MAXSEG_64K
    ALLOC(ush, tab_prefix, 1L<<BITS, rc);
#else
    ALLOC(ush, tab_prefix0, 1L<<(BITS-1), rc);
    ALLOC(ush, tab_prefix1, 1L<<(BITS-1), rc);
#endif
    ifd = infd;
    ofd = outfd;

    input_data_ptr = indataptr;
    output_data_ptr = outdataptr;
    input_data_size = indatasize;
    output_max_size = outmaxsize;

    zip_unzip_restart(0);

    return rc;
}

static int zip_stop(void)
{
    /* Allocate all global buffers (for DYN_ALLOC option) */
    FREE(l_buf);
    FREE(outbuf);
    FREE(d_buf);
    FREE(window);
#ifndef MAXSEG_64K
    FREE(tab_prefix);
#else
    FREE(tab_prefix0);
    FREE(tab_prefix1);
#endif

    return 0;
}

static int read_buffer (fd, buf, cnt)
     int fd;
     voidp buf;
     unsigned int cnt;
{
  int rc;

  if (fd == FD_ON_THE_FLY) {
      if (cnt > (unsigned int)on_the_fly_data_size)
	  cnt = on_the_fly_data_size;
      memcpy(buf, on_the_fly_data_ptr, cnt);
      on_the_fly_data_size -= cnt;
      on_the_fly_data_ptr += cnt;
      return cnt;
  } else {
      if ((input_data_size != (off_t)-1) && (bytes_in + cnt > (unsigned int)input_data_size)) {
	  cnt = input_data_size - bytes_in;
      }
      if (cnt == 0) return 0; /* return if nothing to read */
      if (fd == FD_MEMORY) {
	  memcpy(buf, input_data_ptr + bytes_in, cnt);
	  return cnt;
      } else {
	  rc = read(fd, buf, cnt);
	  return rc;
      }
  }
}

/*
 * Read a new buffer from the current input file, perform end-of-line
 * translation, and update the crc and input file size.
 * IN assertion: size >= 2 (for end-of-line translation)
 */
static int file_read(buf, size)
    char *buf;
    unsigned size;
{
    unsigned len;

    Assert(insize == 0, "read from input but input buffer not empty");

    len = read_buffer(ifd, buf, size);
    if (len == 0) return len;
    if (len == (unsigned)-1) {
	input_error = 1;
	return EOF;
    }

    crc = updcrc((uch*)buf, len);
    bytes_in += (off_t)len;
    return (int)len;
}

static int write_buffer (fd, buf, cnt)
     int fd;
     voidp buf;
     unsigned int cnt;
{
    int rc;
    if (test_mode)
	return cnt; /* write nothing */
    if ((output_max_size != (off_t)-1) && (bytes_out + cnt > (unsigned int)output_max_size)) {
	return -1; /* error : write will exceed the maximum output size */
    }
    if (fd == FD_MEMORY) {
	memcpy(output_data_ptr + bytes_out, buf, cnt); /* directly write data to memory */
	return cnt;
    } else {
	rc = write(fd, buf, cnt);
	return rc;
    }
}

/* same as write(), but also handles partial pipe writes and checks for error return. */
static void write_buf(fd, buf, cnt)
    int       fd;
    voidp     buf;
    unsigned  cnt;
{
    unsigned  n;

    while ((n = write_buffer (fd, buf, cnt)) != cnt) {
	if ((int)n <= 0) {
	    /* write() error or output full */
	    output_error = 1;
	    break;
	}
	cnt -= n;
	buf = (voidp)((char*)buf+n);
    }
}


/*------------------------- bits.c --------------------------*/

/* ===========================================================================
 * Local data used by the "bit string" routines.
 */

static unsigned short bi_buf;
/* Output buffer. bits are inserted starting at the bottom (least significant
 * bits).
 */

#define Buf_size (8 * 2*sizeof(char))
/* Number of bits used within bi_buf. (bi_buf might be implemented on
 * more than 16 bits on some systems.)
 */

static int bi_valid;
/* Number of valid bits in bi_buf.  All bits above the last valid bit
 * are always zero.
 */

#ifdef DEBUG
static off_t bits_sent;   /* bit length of the compressed data */
#endif

/* ===========================================================================
 * Write the output buffer outbuf[0..outcnt-1] and update bytes_out.
 * (used for the compressed data only)
 */
static void flush_outbuf()
{
    if (outcnt == 0) return;
    /* and add some pad if needed to match the min blocksize; only supposed to occur */
    /* at last write */
    while (outcnt & (ZIP_OUTMINBLKSIZE - 1))
	outbuf[outcnt++] = 0;
    write_buf(ofd, (char *)outbuf, outcnt);
    bytes_out += (off_t)outcnt;
    outcnt = 0;
}

/* ===========================================================================
 * Initialize the bit string routines.
 */
static void bi_init (zipfile)
    file_t zipfile; /* output zip file */
{
    bi_buf = 0;
    bi_valid = 0;
#ifdef DEBUG
    bits_sent = 0L;
#endif
}

/* ===========================================================================
 * Send a value on a given number of bits.
 * IN assertion: length <= 16 and value fits in length bits.
 */
static void send_bits(value, length)
    int value;  /* value to send */
    int length; /* number of bits */
{
#ifdef DEBUG
    Tracev((stderr," l %2d v %4x ", length, value));
    Assert(length > 0 && length <= 15, "invalid length");
    bits_sent += (off_t)length;
#endif
    /* If not enough room in bi_buf, use (valid) bits from bi_buf and
     * (16 - bi_valid) bits from value, leaving (width - (16-bi_valid))
     * unused bits in value.
     */
    if (bi_valid > (int)Buf_size - length) {
        bi_buf |= (value << bi_valid);
        put_short(bi_buf);
        bi_buf = (ush)value >> (Buf_size - bi_valid);
        bi_valid += length - Buf_size;
    } else {
        bi_buf |= value << bi_valid;
        bi_valid += length;
    }
}

/* ===========================================================================
 * Reverse the first len bits of a code, using straightforward code (a faster
 * method would use a table)
 * IN assertion: 1 <= len <= 15
 */
static unsigned bi_reverse(code, len)
    unsigned code; /* the value to invert */
    int len;       /* its bit length */
{
    register unsigned res = 0;
    do {
        res |= code & 1;
        code >>= 1, res <<= 1;
    } while (--len > 0);
    return res >> 1;
}

/* ===========================================================================
 * Write out any remaining bits in an incomplete byte.
 */
static void bi_windup()
{
    if (bi_valid > 8) {
        put_short(bi_buf);
    } else if (bi_valid > 0) {
        put_byte(bi_buf);
    }
    bi_buf = 0;
    bi_valid = 0;
#ifdef DEBUG
    bits_sent = (bits_sent+7) & ~7;
#endif
}

/* ===========================================================================
 * Copy a stored block to the zip file, storing first the length and its
 * one's complement if requested.
 */
static void copy_block(buf, len, header)
    char     *buf;    /* the input data */
    unsigned len;     /* its length */
    int      header;  /* true if block header must be written */
{
    bi_windup();              /* align on byte boundary */

    if (header) {
        put_short((ush)len);
        put_short((ush)~len);
#ifdef DEBUG
        bits_sent += 2*16;
#endif
    }
#ifdef DEBUG
    bits_sent += (off_t)len<<3;
#endif
    while (len--) {
	put_byte(*buf++);
    }
}


/*------------------------- deflate.c --------------------------*/

/* ===========================================================================
 * Constants
 */
#define MAX_BITS 15
/* All codes must not exceed MAX_BITS bits */

#define MAX_BL_BITS 7
/* Bit length codes must not exceed MAX_BL_BITS bits */

#define LENGTH_CODES 29
/* number of length codes, not counting the special END_BLOCK code */

#define LITERALS  256
/* number of literal bytes 0..255 */

#define END_BLOCK 256
/* end of block literal code */

#define L_CODES (LITERALS+1+LENGTH_CODES)
/* number of Literal or Length codes, including the END_BLOCK code */

#define D_CODES   30
/* number of distance codes */

#define BL_CODES  19
/* number of codes used to transfer the bit lengths */


static const int near extra_lbits[LENGTH_CODES] /* extra bits for each length code */
   = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

static const int near extra_dbits[D_CODES] /* extra bits for each distance code */
   = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static const int near extra_blbits[BL_CODES]/* extra bits for each bit length code */
   = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,2,3,7};

#define STORED_BLOCK 0
#define STATIC_TREES 1
#define DYN_TREES    2
/* The three kinds of block type */

/* Sizes of match buffers for literals/lengths and distances.  There are
 * 4 reasons for limiting LIT_BUFSIZE to 64K:
 *   - frequencies can be kept in 16 bit counters
 *   - if compression is not successful for the first block, all input data is
 *     still in the window so we can still emit a stored block even when input
 *     comes from standard input.  (This can also be done for all blocks if
 *     LIT_BUFSIZE is not greater than 32K.)
 *   - if compression is not successful for a file smaller than 64K, we can
 *     even emit a stored file instead of a stored block (saving 5 bytes).
 *   - creating new Huffman trees less frequently may not provide fast
 *     adaptation to changes in the input data statistics. (Take for
 *     example a binary file with poorly compressible code followed by
 *     a highly compressible string table.) Smaller buffer sizes give
 *     fast adaptation but have of course the overhead of transmitting trees
 *     more frequently.
 *   - I can't count above 4
 * The current code is general and allows DIST_BUFSIZE < LIT_BUFSIZE (to save
 * memory at the expense of compression). Some optimizations would be possible
 * if we rely on DIST_BUFSIZE == LIT_BUFSIZE.
 */

#define REP_3_6      16
/* repeat previous bit length 3-6 times (2 bits of repeat count) */

#define REPZ_3_10    17
/* repeat a zero length 3-10 times  (3 bits of repeat count) */

#define REPZ_11_138  18
/* repeat a zero length 11-138 times  (7 bits of repeat count) */

/* ===========================================================================
 * Local data
 */

/* Data structure describing a single value and its code string. */
typedef struct ct_data {
    union {
        ush  freq;       /* frequency count */
        ush  code;       /* bit string */
    } fc;
    union {
        ush  dad;        /* father node in Huffman tree */
        ush  len;        /* length of bit string */
    } dl;
} ct_data;

#define Freq fc.freq
#define Code fc.code
#define Dad  dl.dad
#define Len  dl.len

#define HEAP_SIZE (2*L_CODES+1)
/* maximum heap size */

static ct_data near dyn_ltree[HEAP_SIZE];   /* literal and length tree */
static ct_data near dyn_dtree[2*D_CODES+1]; /* distance tree */

static ct_data near static_ltree[L_CODES+2];
/* The static literal tree. Since the bit lengths are imposed, there is no
 * need for the L_CODES extra codes used during heap construction. However
 * The codes 286 and 287 are needed to build a canonical tree (see ct_init
 * below).
 */

static ct_data near static_dtree[D_CODES];
/* The static distance tree. (Actually a trivial tree since all codes use
 * 5 bits.)
 */

static ct_data near bl_tree[2*BL_CODES+1];
/* Huffman tree for the bit lengths */

typedef struct tree_desc {
    ct_data near *dyn_tree;      /* the dynamic tree */
    ct_data near *static_tree;   /* corresponding static tree or NULL */
    const int near *extra_bits;  /* extra bits for each code or NULL */
    int     extra_base;          /* base index for extra_bits */
    int     elems;               /* max number of elements in the tree */
    int     max_length;          /* max bit length for the codes */
    int     max_code;            /* largest code with non zero frequency */
} tree_desc;

static tree_desc near l_desc =
{dyn_ltree, static_ltree, extra_lbits, LITERALS+1, L_CODES, MAX_BITS, 0};

static tree_desc near d_desc =
{dyn_dtree, static_dtree, extra_dbits, 0,          D_CODES, MAX_BITS, 0};

static tree_desc near bl_desc =
{bl_tree, (ct_data near *)0, extra_blbits, 0,      BL_CODES, MAX_BL_BITS, 0};


static ush near bl_count[MAX_BITS+1];
/* number of codes at each bit length for an optimal tree */

static const uch near bl_order[BL_CODES]
   = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};
/* The lengths of the bit length codes are sent in order of decreasing
 * probability, to avoid transmitting the lengths for unused bit length codes.
 */

static int near heap[2*L_CODES+1]; /* heap used to build the Huffman trees */
static int heap_len;               /* number of elements in the heap */
static int heap_max;               /* element of largest frequency */
/* The sons of heap[n] are heap[2*n] and heap[2*n+1]. heap[0] is not used.
 * The same heap array is used to build all trees.
 */

static uch near depth[2*L_CODES+1];
/* Depth of each subtree used as tie breaker for trees of equal frequency */

static uch length_code[MAX_MATCH-MIN_MATCH+1];
/* length code for each normalized match length (0 == MIN_MATCH) */

static uch dist_code[512];
/* distance codes. The first 256 values correspond to the distances
 * 3 .. 258, the last 256 values correspond to the top 8 bits of
 * the 15 bit distances.
 */

static int near base_length[LENGTH_CODES];
/* First normalized length for each code (0 = MIN_MATCH) */

static int near base_dist[D_CODES];
/* First normalized distance for each code (0 = distance of 1) */

/* DECLARE(uch, l_buf, LIT_BUFSIZE);  buffer for literals or lengths */

/* DECLARE(ush, d_buf, DIST_BUFSIZE); buffer for distances */

static uch near flag_buf[(LIT_BUFSIZE/8)];
/* flag_buf is a bit array distinguishing literals from lengths in
 * l_buf, thus indicating the presence or absence of a distance.
 */

static unsigned last_lit;    /* running index in l_buf */
static unsigned last_dist;   /* running index in d_buf */
static unsigned last_flags;  /* running index in flag_buf */
static uch flags;            /* current flags not yet saved in flag_buf */
static uch flag_bit;         /* current bit used in flags */
/* bits are filled in flags starting at bit 0 (least significant).
 * Note: these flags are overkill in the current code since we don't
 * take advantage of DIST_BUFSIZE == LIT_BUFSIZE.
 */

static ulg opt_len;        /* bit length of current block with optimal trees */
static ulg static_len;     /* bit length of current block with static trees */

static off_t compressed_len; /* total bit length of compressed file */

static off_t input_len;      /* total byte length of input file */
/* input_len is for debugging only since we can get it by other means. */

static ush *file_type;        /* pointer to UNKNOWN, BINARY or ASCII */
static int *file_method;      /* pointer to DEFLATE or STORE */

#ifdef DEBUG
extern off_t bits_sent;  /* bit length of the compressed data */
#endif

/* ===========================================================================
 * Local (static) routines in this file.
 */

static void init_block     OF((void));
static void pqdownheap     OF((ct_data near *tree, int k));
static void gen_bitlen     OF((tree_desc near *desc));
static void gen_codes      OF((ct_data near *tree, int max_code));
static void build_tree     OF((tree_desc near *desc));
static void scan_tree      OF((ct_data near *tree, int max_code));
static void send_tree      OF((ct_data near *tree, int max_code));
static int  build_bl_tree  OF((void));
static void send_all_trees OF((int lcodes, int dcodes, int blcodes));
static void compress_block OF((ct_data near *ltree, ct_data near *dtree));
static void set_file_type  OF((void));


#ifndef DEBUG
#  define send_code(c, tree) send_bits(tree[c].Code, tree[c].Len)
   /* Send a code of the given tree. c and tree must not have side effects */

#else /* DEBUG */
#  define send_code(c, tree) \
     { if (verbose>1) msg_print(stderr,"\ncd %3d ",(c)); \
       send_bits(tree[c].Code, tree[c].Len); }
#endif

#define d_code(dist) \
   ((dist) < 256 ? dist_code[dist] : dist_code[256+((dist)>>7)])
/* Mapping from a distance to a distance code. dist is the distance - 1 and
 * must not have side effects. dist_code[256] and dist_code[257] are never
 * used.
 */
#ifndef MAX
#define MAX(a,b) (a >= b ? a : b)
#endif
/* the arguments must not have side effects */

/* ===========================================================================
 * Allocate the match buffer, initialize the various tables and save the
 * location of the internal file attribute (ascii/binary) and method
 * (DEFLATE/STORE).
 */
static void ct_init(attr, methodp)
    ush  *attr;   /* pointer to internal file attribute */
    int  *methodp; /* pointer to compression method */
{
    int n;        /* iterates over tree elements */
    int bits;     /* bit counter */
    int length;   /* length value */
    int code;     /* code value */
    int dist;     /* distance index */

    file_type = attr;
    file_method = methodp;
    compressed_len = input_len = 0L;

    if (static_dtree[0].Len != 0) return; /* ct_init already called */

    /* Initialize the mapping length (0..255) -> length code (0..28) */
    length = 0;
    for (code = 0; code < LENGTH_CODES-1; code++) {
        base_length[code] = length;
        for (n = 0; n < (1<<extra_lbits[code]); n++) {
            length_code[length++] = (uch)code;
        }
    }
    Assert (length == 256, "ct_init: length != 256");
    /* Note that the length 255 (match length 258) can be represented
     * in two different ways: code 284 + 5 bits or code 285, so we
     * overwrite length_code[255] to use the best encoding:
     */
    length_code[length-1] = (uch)code;

    /* Initialize the mapping dist (0..32K) -> dist code (0..29) */
    dist = 0;
    for (code = 0 ; code < 16; code++) {
        base_dist[code] = dist;
        for (n = 0; n < (1<<extra_dbits[code]); n++) {
            dist_code[dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "ct_init: dist != 256");
    dist >>= 7; /* from now on, all distances are divided by 128 */
    for ( ; code < D_CODES; code++) {
        base_dist[code] = dist << 7;
        for (n = 0; n < (1<<(extra_dbits[code]-7)); n++) {
            dist_code[256 + dist++] = (uch)code;
        }
    }
    Assert (dist == 256, "ct_init: 256+dist != 512");

    /* Construct the codes of the static literal tree */
    for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;
    n = 0;
    while (n <= 143) static_ltree[n++].Len = 8, bl_count[8]++;
    while (n <= 255) static_ltree[n++].Len = 9, bl_count[9]++;
    while (n <= 279) static_ltree[n++].Len = 7, bl_count[7]++;
    while (n <= 287) static_ltree[n++].Len = 8, bl_count[8]++;
    /* Codes 286 and 287 do not exist, but we must include them in the
     * tree construction to get a canonical Huffman tree (longest code
     * all ones)
     */
    gen_codes((ct_data near *)static_ltree, L_CODES+1);

    /* The static distance tree is trivial: */
    for (n = 0; n < D_CODES; n++) {
        static_dtree[n].Len = 5;
        static_dtree[n].Code = bi_reverse(n, 5);
    }

    /* Initialize the first block of the first file: */
    init_block();
}

/* ===========================================================================
 * Initialize a new block.
 */
static void init_block()
{
    int n; /* iterates over tree elements */

    /* Initialize the trees. */
    for (n = 0; n < L_CODES;  n++) dyn_ltree[n].Freq = 0;
    for (n = 0; n < D_CODES;  n++) dyn_dtree[n].Freq = 0;
    for (n = 0; n < BL_CODES; n++) bl_tree[n].Freq = 0;

    dyn_ltree[END_BLOCK].Freq = 1;
    opt_len = static_len = 0L;
    last_lit = last_dist = last_flags = 0;
    flags = 0; flag_bit = 1;
}

#define SMALLEST 1
/* Index within the heap array of least frequent node in the Huffman tree */


/* ===========================================================================
 * Remove the smallest element from the heap and recreate the heap with
 * one less element. Updates heap and heap_len.
 */
#define pqremove(tree, top) \
{\
    top = heap[SMALLEST]; \
    heap[SMALLEST] = heap[heap_len--]; \
    pqdownheap(tree, SMALLEST); \
}

/* ===========================================================================
 * Compares to subtrees, using the tree depth as tie breaker when
 * the subtrees have equal frequency. This minimizes the worst case length.
 */
#define smaller(tree, n, m) \
   (tree[n].Freq < tree[m].Freq || \
   (tree[n].Freq == tree[m].Freq && depth[n] <= depth[m]))

/* ===========================================================================
 * Restore the heap property by moving down the tree starting at node k,
 * exchanging a node with the smallest of its two sons if necessary, stopping
 * when the heap property is re-established (each father smaller than its
 * two sons).
 */
static void pqdownheap(tree, k)
    ct_data near *tree;  /* the tree to restore */
    int k;               /* node to move down */
{
    int v = heap[k];
    int j = k << 1;  /* left son of k */
    while (j <= heap_len) {
        /* Set j to the smallest of the two sons: */
        if (j < heap_len && smaller(tree, heap[j+1], heap[j])) j++;

        /* Exit if v is smaller than both sons */
        if (smaller(tree, v, heap[j])) break;

        /* Exchange v with the smallest son */
        heap[k] = heap[j];  k = j;

        /* And continue down the tree, setting j to the left son of k */
        j <<= 1;
    }
    heap[k] = v;
}

/* ===========================================================================
 * Compute the optimal bit lengths for a tree and update the total bit length
 * for the current block.
 * IN assertion: the fields freq and dad are set, heap[heap_max] and
 *    above are the tree nodes sorted by increasing frequency.
 * OUT assertions: the field len is set to the optimal bit length, the
 *     array bl_count contains the frequencies for each bit length.
 *     The length opt_len is updated; static_len is also updated if stree is
 *     not null.
 */
static void gen_bitlen(desc)
    tree_desc near *desc; /* the tree descriptor */
{
    ct_data near *tree  = desc->dyn_tree;
    const int near *extra = desc->extra_bits;
    int base            = desc->extra_base;
    int max_code        = desc->max_code;
    int max_length      = desc->max_length;
    ct_data near *stree = desc->static_tree;
    int h;              /* heap index */
    int n, m;           /* iterate over the tree elements */
    int bits;           /* bit length */
    int xbits;          /* extra bits */
    ush f;              /* frequency */
    int overflow = 0;   /* number of elements with bit length too large */

    for (bits = 0; bits <= MAX_BITS; bits++) bl_count[bits] = 0;

    /* In a first pass, compute the optimal bit lengths (which may
     * overflow in the case of the bit length tree).
     */
    tree[heap[heap_max]].Len = 0; /* root of the heap */

    for (h = heap_max+1; h < HEAP_SIZE; h++) {
        n = heap[h];
        bits = tree[tree[n].Dad].Len + 1;
        if (bits > max_length) bits = max_length, overflow++;
        tree[n].Len = (ush)bits;
        /* We overwrite tree[n].Dad which is no longer needed */

        if (n > max_code) continue; /* not a leaf node */

        bl_count[bits]++;
        xbits = 0;
        if (n >= base) xbits = extra[n-base];
        f = tree[n].Freq;
        opt_len += (ulg)f * (bits + xbits);
        if (stree) static_len += (ulg)f * (stree[n].Len + xbits);
    }
    if (overflow == 0) return;

    Trace((stderr,"\nbit length overflow\n"));
    /* This happens for example on obj2 and pic of the Calgary corpus */

    /* Find the first bit length which could increase: */
    do {
        bits = max_length-1;
        while (bl_count[bits] == 0) bits--;
        bl_count[bits]--;      /* move one leaf down the tree */
        bl_count[bits+1] += 2; /* move one overflow item as its brother */
        bl_count[max_length]--;
        /* The brother of the overflow item also moves one step up,
         * but this does not affect bl_count[max_length]
         */
        overflow -= 2;
    } while (overflow > 0);

    /* Now recompute all bit lengths, scanning in increasing frequency.
     * h is still equal to HEAP_SIZE. (It is simpler to reconstruct all
     * lengths instead of fixing only the wrong ones. This idea is taken
     * from 'ar' written by Haruhiko Okumura.)
     */
    for (bits = max_length; bits != 0; bits--) {
        n = bl_count[bits];
        while (n != 0) {
            m = heap[--h];
            if (m > max_code) continue;
            if (tree[m].Len != (unsigned) bits) {
                Trace((stderr,"code %d bits %d->%d\n", m, tree[m].Len, bits));
                opt_len += ((long)bits-(long)tree[m].Len)*(long)tree[m].Freq;
                tree[m].Len = (ush)bits;
            }
            n--;
        }
    }
}

/* ===========================================================================
 * Generate the codes for a given tree and bit counts (which need not be
 * optimal).
 * IN assertion: the array bl_count contains the bit length statistics for
 * the given tree and the field len is set for all tree elements.
 * OUT assertion: the field code is set for all tree elements of non
 *     zero code length.
 */
static void gen_codes (tree, max_code)
    ct_data near *tree;        /* the tree to decorate */
    int max_code;              /* largest code with non zero frequency */
{
    ush next_code[MAX_BITS+1]; /* next code value for each bit length */
    ush code = 0;              /* running code value */
    int bits;                  /* bit index */
    int n;                     /* code index */

    /* The distribution counts are first used to generate the code values
     * without bit reversal.
     */
    for (bits = 1; bits <= MAX_BITS; bits++) {
        next_code[bits] = code = (code + bl_count[bits-1]) << 1;
    }
    /* Check that the bit counts in bl_count are consistent. The last code
     * must be all ones.
     */
    Assert (code + bl_count[MAX_BITS]-1 == (1<<MAX_BITS)-1,
            "inconsistent bit counts");
    Tracev((stderr,"\ngen_codes: max_code %d ", max_code));

    for (n = 0;  n <= max_code; n++) {
        int len = tree[n].Len;
        if (len == 0) continue;
        /* Now reverse the bits */
        tree[n].Code = bi_reverse(next_code[len]++, len);

        Tracec(tree != static_ltree, (stderr,"\nn %3d %c l %2d c %4x (%x) ",
             n, (isgraph(n) ? n : ' '), len, tree[n].Code, next_code[len]-1));
    }
}

/* ===========================================================================
 * Construct one Huffman tree and assigns the code bit strings and lengths.
 * Update the total bit length for the current block.
 * IN assertion: the field freq is set for all tree elements.
 * OUT assertions: the fields len and code are set to the optimal bit length
 *     and corresponding code. The length opt_len is updated; static_len is
 *     also updated if stree is not null. The field max_code is set.
 */
static void build_tree(desc)
    tree_desc near *desc; /* the tree descriptor */
{
    ct_data near *tree   = desc->dyn_tree;
    ct_data near *stree  = desc->static_tree;
    int elems            = desc->elems;
    int n, m;          /* iterate over heap elements */
    int max_code = -1; /* largest code with non zero frequency */
    int node = elems;  /* next internal node of the tree */

    /* Construct the initial heap, with least frequent element in
     * heap[SMALLEST]. The sons of heap[n] are heap[2*n] and heap[2*n+1].
     * heap[0] is not used.
     */
    heap_len = 0, heap_max = HEAP_SIZE;

    for (n = 0; n < elems; n++) {
        if (tree[n].Freq != 0) {
            heap[++heap_len] = max_code = n;
            depth[n] = 0;
        } else {
            tree[n].Len = 0;
        }
    }

    /* The pkzip format requires that at least one distance code exists,
     * and that at least one bit should be sent even if there is only one
     * possible code. So to avoid special checks later on we force at least
     * two codes of non zero frequency.
     */
    while (heap_len < 2) {
        int new = heap[++heap_len] = (max_code < 2 ? ++max_code : 0);
        tree[new].Freq = 1;
        depth[new] = 0;
        opt_len--; if (stree) static_len -= stree[new].Len;
        /* new is 0 or 1 so it does not have extra bits */
    }
    desc->max_code = max_code;

    /* The elements heap[heap_len/2+1 .. heap_len] are leaves of the tree,
     * establish sub-heaps of increasing lengths:
     */
    for (n = heap_len/2; n >= 1; n--) pqdownheap(tree, n);

    /* Construct the Huffman tree by repeatedly combining the least two
     * frequent nodes.
     */
    do {
        pqremove(tree, n);   /* n = node of least frequency */
        m = heap[SMALLEST];  /* m = node of next least frequency */

        heap[--heap_max] = n; /* keep the nodes sorted by frequency */
        heap[--heap_max] = m;

        /* Create a new node father of n and m */
        tree[node].Freq = tree[n].Freq + tree[m].Freq;
        depth[node] = (uch) (MAX(depth[n], depth[m]) + 1);
        tree[n].Dad = tree[m].Dad = (ush)node;
#ifdef DUMP_BL_TREE
        if (tree == bl_tree) {
            msg_print(stderr,"\nnode %d(%d), sons %d(%d) %d(%d)",
		      node, tree[node].Freq, n, tree[n].Freq, m, tree[m].Freq);
        }
#endif
        /* and insert the new node in the heap */
        heap[SMALLEST] = node++;
        pqdownheap(tree, SMALLEST);

    } while (heap_len >= 2);

    heap[--heap_max] = heap[SMALLEST];

    /* At this point, the fields freq and dad are set. We can now
     * generate the bit lengths.
     */
    gen_bitlen((tree_desc near *)desc);

    /* The field len is now set, we can generate the bit codes */
    gen_codes ((ct_data near *)tree, max_code);
}

/* ===========================================================================
 * Scan a literal or distance tree to determine the frequencies of the codes
 * in the bit length tree. Updates opt_len to take into account the repeat
 * counts. (The contribution of the bit length codes will be added later
 * during the construction of bl_tree.)
 */
static void scan_tree (tree, max_code)
    ct_data near *tree; /* the tree to be scanned */
    int max_code;       /* and its largest code of non zero frequency */
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    if (nextlen == 0) max_count = 138, min_count = 3;
    tree[max_code+1].Len = (ush)0xffff; /* guard */

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            bl_tree[curlen].Freq += count;
        } else if (curlen != 0) {
            if (curlen != prevlen) bl_tree[curlen].Freq++;
            bl_tree[REP_3_6].Freq++;
        } else if (count <= 10) {
            bl_tree[REPZ_3_10].Freq++;
        } else {
            bl_tree[REPZ_11_138].Freq++;
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* ===========================================================================
 * Send a literal or distance tree in compressed form, using the codes in
 * bl_tree.
 */
static void send_tree (tree, max_code)
    ct_data near *tree; /* the tree to be scanned */
    int max_code;       /* and its largest code of non zero frequency */
{
    int n;                     /* iterates over all tree elements */
    int prevlen = -1;          /* last emitted length */
    int curlen;                /* length of current code */
    int nextlen = tree[0].Len; /* length of next code */
    int count = 0;             /* repeat count of the current code */
    int max_count = 7;         /* max repeat count */
    int min_count = 4;         /* min repeat count */

    /* tree[max_code+1].Len = -1; */  /* guard already set */
    if (nextlen == 0) max_count = 138, min_count = 3;

    for (n = 0; n <= max_code; n++) {
        curlen = nextlen; nextlen = tree[n+1].Len;
        if (++count < max_count && curlen == nextlen) {
            continue;
        } else if (count < min_count) {
            do { send_code(curlen, bl_tree); } while (--count != 0);

        } else if (curlen != 0) {
            if (curlen != prevlen) {
                send_code(curlen, bl_tree); count--;
            }
            Assert(count >= 3 && count <= 6, " 3_6?");
            send_code(REP_3_6, bl_tree); send_bits(count-3, 2);

        } else if (count <= 10) {
            send_code(REPZ_3_10, bl_tree); send_bits(count-3, 3);

        } else {
            send_code(REPZ_11_138, bl_tree); send_bits(count-11, 7);
        }
        count = 0; prevlen = curlen;
        if (nextlen == 0) {
            max_count = 138, min_count = 3;
        } else if (curlen == nextlen) {
            max_count = 6, min_count = 3;
        } else {
            max_count = 7, min_count = 4;
        }
    }
}

/* ===========================================================================
 * Construct the Huffman tree for the bit lengths and return the index in
 * bl_order of the last bit length code to send.
 */
static int build_bl_tree()
{
    int max_blindex;  /* index of last bit length code of non zero freq */

    /* Determine the bit length frequencies for literal and distance trees */
    scan_tree((ct_data near *)dyn_ltree, l_desc.max_code);
    scan_tree((ct_data near *)dyn_dtree, d_desc.max_code);

    /* Build the bit length tree: */
    build_tree((tree_desc near *)(&bl_desc));
    /* opt_len now includes the length of the tree representations, except
     * the lengths of the bit lengths codes and the 5+5+4 bits for the counts.
     */

    /* Determine the number of bit length codes to send. The pkzip format
     * requires that at least 4 bit length codes be sent. (appnote.txt says
     * 3 but the actual value used is 4.)
     */
    for (max_blindex = BL_CODES-1; max_blindex >= 3; max_blindex--) {
        if (bl_tree[bl_order[max_blindex]].Len != 0) break;
    }
    /* Update opt_len to include the bit length tree and counts */
    opt_len += 3*(max_blindex+1) + 5+5+4;
    Tracev((stderr, "\ndyn trees: dyn %lu, stat %lu", opt_len, static_len));

    return max_blindex;
}

/* ===========================================================================
 * Send the header for a block using dynamic Huffman trees: the counts, the
 * lengths of the bit length codes, the literal tree and the distance tree.
 * IN assertion: lcodes >= 257, dcodes >= 1, blcodes >= 4.
 */
static void send_all_trees(lcodes, dcodes, blcodes)
    int lcodes, dcodes, blcodes; /* number of codes for each tree */
{
    int rank;                    /* index in bl_order */

    Assert (lcodes >= 257 && dcodes >= 1 && blcodes >= 4, "not enough codes");
    Assert (lcodes <= L_CODES && dcodes <= D_CODES && blcodes <= BL_CODES,
            "too many codes");
    Tracev((stderr, "\nbl counts: "));
    send_bits(lcodes-257, 5); /* not +255 as stated in appnote.txt */
    send_bits(dcodes-1,   5);
    send_bits(blcodes-4,  4); /* not -3 as stated in appnote.txt */
    for (rank = 0; rank < blcodes; rank++) {
        Tracev((stderr, "\nbl code %2d ", bl_order[rank]));
        send_bits(bl_tree[bl_order[rank]].Len, 3);
    }

    send_tree((ct_data near *)dyn_ltree, lcodes-1); /* send the literal tree */

    send_tree((ct_data near *)dyn_dtree, dcodes-1); /* send the distance tree */
}

/* ===========================================================================
 * Determine the best encoding for the current block: dynamic trees, static
 * trees or store, and output the encoded block to the zip file. This function
 * returns the total compressed length for the file so far.
 */
static off_t flush_block(buf, stored_len, pad, eof)  
    char *buf;        /* input block, or NULL if too old */
    ulg stored_len;   /* length of input block */
    int pad;          /* pad output to byte boundary */ 
    int eof;          /* true if this is the last block for a file */
{
    ulg opt_lenb, static_lenb; /* opt_len and static_len in bytes */
    int max_blindex;  /* index of last bit length code of non zero freq */

    flag_buf[last_flags] = flags; /* Save the flags for the last 8 items */

     /* Check if the file is ascii or binary */
    if (*file_type == (ush)UNKNOWN) set_file_type();

    /* Construct the literal and distance trees */
    build_tree((tree_desc near *)(&l_desc));
    Tracev((stderr, "\nlit data: dyn %lu, stat %lu", opt_len, static_len));

    build_tree((tree_desc near *)(&d_desc));
    Tracev((stderr, "\ndist data: dyn %lu, stat %lu", opt_len, static_len));
    /* At this point, opt_len and static_len are the total bit lengths of
     * the compressed block data, excluding the tree representations.
     */

    /* Build the bit length tree for the above two trees, and get the index
     * in bl_order of the last bit length code to send.
     */
    max_blindex = build_bl_tree();

    /* Determine the best encoding. Compute first the block length in bytes */
    opt_lenb = (opt_len+3+7)>>3;
    static_lenb = (static_len+3+7)>>3;
    input_len += stored_len; /* for debugging only */

    Trace((stderr, "\nopt %lu(%lu) stat %lu(%lu) stored %lu lit %u dist %u ",
            opt_lenb, opt_len, static_lenb, static_len, stored_len,
            last_lit, last_dist));

    if (static_lenb <= opt_lenb) opt_lenb = static_lenb;

    /* If compression failed and this is the first and last block,
     * and if the zip file can be seeked (to rewrite the local header),
     * the whole file is transformed into a stored file:
     */
#ifdef FORCE_METHOD
    if (level == 1 && eof && compressed_len == 0L) { /* force stored file */
#else
    if (stored_len <= opt_lenb && eof && compressed_len == 0L && seekable()) {
#endif
        /* Since LIT_BUFSIZE <= 2*WSIZE, the input data must be there: */
	if (!buf)
	    msg_print(stderr, "ERROR: block vanished\n");

        copy_block(buf, (unsigned)stored_len, 0); /* without header */
        compressed_len = stored_len << 3;
        *file_method = STORED;

#ifdef FORCE_METHOD
    } else if (level == 2 && buf != (char*)0) { /* force stored block */
#else
    } else if (stored_len+4 <= opt_lenb && buf != (char*)0) {
                       /* 4: two words for the lengths */
#endif
        /* The test buf != NULL is only necessary if LIT_BUFSIZE > WSIZE.
         * Otherwise we can't have processed more than WSIZE input bytes since
         * the last block flush, because compression would have been
         * successful. If LIT_BUFSIZE <= WSIZE, it is never too late to
         * transform a block into a stored block.
         */
        send_bits((STORED_BLOCK<<1)+eof, 3);  /* send block type */
        compressed_len = (compressed_len + 3 + 7) & ~7L;
        compressed_len += (stored_len + 4) << 3;

        copy_block(buf, (unsigned)stored_len, 1); /* with header */

#ifdef FORCE_METHOD
    } else if (level == 3) { /* force static trees */
#else
    } else if (static_lenb == opt_lenb) {
#endif
        send_bits((STATIC_TREES<<1)+eof, 3);
        compress_block((ct_data near *)static_ltree, (ct_data near *)static_dtree);
        compressed_len += 3 + static_len;
    } else {
        send_bits((DYN_TREES<<1)+eof, 3);
        send_all_trees(l_desc.max_code+1, d_desc.max_code+1, max_blindex+1);
        compress_block((ct_data near *)dyn_ltree, (ct_data near *)dyn_dtree);
        compressed_len += 3 + opt_len;
    }
    Assert (compressed_len == bits_sent, "bad compressed size");
    init_block();

    if (eof) {
        Assert (input_len == bytes_in, "bad input size");
        bi_windup();
        compressed_len += 7;  /* align on byte boundary */
    } else if (pad && (compressed_len % 8) != 0) {
        send_bits((STORED_BLOCK<<1)+eof, 3);  /* send block type */
        compressed_len = (compressed_len + 3 + 7) & ~7L;
        copy_block(buf, 0, 1); /* with header */
    }

    return compressed_len >> 3;
}

/* ===========================================================================
 * Save the match info and tally the frequency counts. Return true if
 * the current block must be flushed.
 */
static int ct_tally (dist, lc)
    int dist;  /* distance of matched string */
    int lc;    /* match length-MIN_MATCH or unmatched char (if dist==0) */
{
    l_buf[last_lit++] = (uch)lc;
    if (dist == 0) {
        /* lc is the unmatched char */
        dyn_ltree[lc].Freq++;
    } else {
        /* Here, lc is the match length - MIN_MATCH */
        dist--;             /* dist = match distance - 1 */
        Assert((ush)dist < (ush)MAX_DIST &&
               (ush)lc <= (ush)(MAX_MATCH-MIN_MATCH) &&
               (ush)d_code(dist) < (ush)D_CODES,  "ct_tally: bad match");

        dyn_ltree[length_code[lc]+LITERALS+1].Freq++;
        dyn_dtree[d_code(dist)].Freq++;

        d_buf[last_dist++] = (ush)dist;
        flags |= flag_bit;
    }
    flag_bit <<= 1;

    /* Output the flags if they fill a byte: */
    if ((last_lit & 7) == 0) {
        flag_buf[last_flags++] = flags;
        flags = 0, flag_bit = 1;
    }
    /* Try to guess if it is profitable to stop the current block here */
    if (level > 2 && (last_lit & 0xfff) == 0) {
        /* Compute an upper bound for the compressed length */
        ulg out_length = (ulg)last_lit*8L;
        ulg in_length = (ulg)strstart-block_start;
        int dcode;
        for (dcode = 0; dcode < D_CODES; dcode++) {
            out_length += (ulg)dyn_dtree[dcode].Freq*(5L+extra_dbits[dcode]);
        }
        out_length >>= 3;
        Trace((stderr,"\nlast_lit %u, last_dist %u, in %ld, out ~%ld(%ld%%) ",
               last_lit, last_dist, in_length, out_length,
               100L - out_length*100L/in_length));
        if (last_dist < last_lit/2 && out_length < in_length/2) return 1;
    }
    return (last_lit == LIT_BUFSIZE-1 || last_dist == DIST_BUFSIZE);
    /* We avoid equality with LIT_BUFSIZE because of wraparound at 64K
     * on 16 bit machines and because stored blocks are restricted to
     * 64K-1 bytes.
     */
}

/* ===========================================================================
 * Send the block data compressed using the given Huffman trees
 */
static void compress_block(ltree, dtree)
    ct_data near *ltree; /* literal tree */
    ct_data near *dtree; /* distance tree */
{
    unsigned dist;      /* distance of matched string */
    int lc;             /* match length or unmatched char (if dist == 0) */
    unsigned lx = 0;    /* running index in l_buf */
    unsigned dx = 0;    /* running index in d_buf */
    unsigned fx = 0;    /* running index in flag_buf */
    uch flag = 0;       /* current flags */
    unsigned code;      /* the code to send */
    int extra;          /* number of extra bits to send */

    if (last_lit != 0) do {
        if ((lx & 7) == 0) flag = flag_buf[fx++];
        lc = l_buf[lx++];
        if ((flag & 1) == 0) {
            send_code(lc, ltree); /* send a literal byte */
            Tracecv(isgraph(lc), (stderr," '%c' ", lc));
        } else {
            /* Here, lc is the match length - MIN_MATCH */
            code = length_code[lc];
            send_code(code+LITERALS+1, ltree); /* send the length code */
            extra = extra_lbits[code];
            if (extra != 0) {
                lc -= base_length[code];
                send_bits(lc, extra);        /* send the extra length bits */
            }
            dist = d_buf[dx++];
            /* Here, dist is the match distance - 1 */
            code = d_code(dist);
            Assert (code < D_CODES, "bad d_code");

            send_code(code, dtree);       /* send the distance code */
            extra = extra_dbits[code];
            if (extra != 0) {
                dist -= base_dist[code];
                send_bits(dist, extra);   /* send the extra distance bits */
            }
        } /* literal or match pair ? */
        flag >>= 1;
    } while (lx < last_lit);

    send_code(END_BLOCK, ltree);
}

/* ===========================================================================
 * Set the file type to ASCII or BINARY, using a crude approximation:
 * binary if more than 20% of the bytes are <= 6 or >= 128, ascii otherwise.
 * IN assertion: the fields freq of dyn_ltree are set and the total of all
 * frequencies does not exceed 64K (to fit in an int on 16 bit machines).
 */
static void set_file_type()
{
    int n = 0;
    unsigned ascii_freq = 0;
    unsigned bin_freq = 0;
    while (n < 7)        bin_freq += dyn_ltree[n++].Freq;
    while (n < 128)    ascii_freq += dyn_ltree[n++].Freq;
    while (n < LITERALS) bin_freq += dyn_ltree[n++].Freq;
    *file_type = bin_freq > (ascii_freq >> 2) ? BINARY : ASCII;
}


/*------------------------- deflate.c --------------------------*/

/* To save space (see unlzw.c), we overlay prev+head with tab_prefix and
 * window with tab_suffix. Check that we can do this:
 */
#if (WSIZE<<1) > (1<<BITS)
   #error: cannot overlay window with tab_suffix and prev with tab_prefix0
#endif
#if HASH_BITS > BITS-1
   #error: cannot overlay head with tab_prefix1
#endif

#define HASH_SIZE (unsigned)(1<<HASH_BITS)
#define HASH_MASK (HASH_SIZE-1)
#define WMASK     (WSIZE-1)
/* HASH_SIZE and WSIZE must be powers of two */

#define NIL 0
/* Tail of hash chains */

#define FAST 4
#define SLOW 2
/* speed options for the general purpose bit flag */

#ifndef TOO_FAR
#  define TOO_FAR 4096
#endif
/* Matches of length 3 are discarded if their distance exceeds TOO_FAR */

/* ===========================================================================
 * Local data used by the "longest match" routines.
 */

typedef ush Pos;
typedef unsigned IPos;
/* A Pos is an index in the character window. We use short instead of int to
 * save space in the various tables. IPos is used only for parameter passing.
 */

/* DECLARE(uch, window, 2L*WSIZE); */
/* Sliding window. Input bytes are read into the second half of the window,
 * and move to the first half later to keep a dictionary of at least WSIZE
 * bytes. With this organization, matches are limited to a distance of
 * WSIZE-MAX_MATCH bytes, but this ensures that IO is always
 * performed with a length multiple of the block size. Also, it limits
 * the window size to 64K, which is quite useful on MSDOS.
 * To do: limit the window size to WSIZE+BSZ if SMALL_MEM (the code would
 * be less efficient).
 */

/* DECLARE(Pos, prev, WSIZE); */
/* Link to older string with same hash index. To limit the size of this
 * array to 64K, this link is maintained only for the last 32K strings.
 * An index in this array is thus a window index modulo 32K.
 */

/* DECLARE(Pos, head, 1<<HASH_BITS); */
/* Heads of the hash chains or NIL. */

static ulg window_size = (ulg)2*WSIZE;
/* window size, 2*WSIZE except for MMAP or BIG_MEM, where it is the
 * input file length plus MIN_LOOKAHEAD.
 */

static unsigned ins_h;  /* hash index of string to be inserted */

#define H_SHIFT  ((HASH_BITS+MIN_MATCH-1)/MIN_MATCH)
/* Number of bits by which ins_h and del_h must be shifted at each
 * input step. It must be such that after MIN_MATCH steps, the oldest
 * byte no longer takes part in the hash key, that is:
 *   H_SHIFT * MIN_MATCH >= HASH_BITS
 */

static unsigned int near prev_length;
/* Length of the best match at previous step. Matches not greater than this
 * are discarded. This is used in the lazy match evaluation.
 */

static unsigned near match_start;   /* start of matching string */
static int           eofile;        /* flag set at end of input file */
static unsigned      lookahead;     /* number of valid bytes ahead in window */

static unsigned near max_chain_length;
/* To speed up deflation, hash chains are never searched beyond this length.
 * A higher limit improves compression ratio but degrades the speed.
 */

static unsigned int max_lazy_match;
/* Attempt to find a better match only when the current match is strictly
 * smaller than this value. This mechanism is used only for compression
 * levels >= 4.
 */
#define max_insert_length  max_lazy_match
/* Insert new strings in the hash table only if the match length
 * is not greater than this length. This saves time but degrades compression.
 * max_insert_length is used only for compression levels <= 3.
 */

static int compr_level;
/* compression level (1..9) */

static unsigned near good_match;
/* Use a faster search when the previous match is longer than this */

/* Values for max_lazy_match, good_match and max_chain_length, depending on
 * the desired pack level (0..9). The values given below have been tuned to
 * exclude worst case performance for pathological files. Better values may be
 * found for specific files.
 */

typedef struct config {
   ush good_length; /* reduce lazy search above this match length */
   ush max_lazy;    /* do not perform lazy search above this match length */
   ush nice_length; /* quit search above this match length */
   ush max_chain;
} config;

#ifdef  FULL_SEARCH
# define nice_match MAX_MATCH
#else
  static int near nice_match; /* Stop searching when current match exceeds this */
#endif

static const config configuration_table[10] = {
/*      good lazy nice chain */
/* 0 */ {0,    0,  0,    0},  /* store only */
/* 1 */ {4,    4,  8,    4},  /* maximum speed, no lazy matches */
/* 2 */ {4,    5, 16,    8},
/* 3 */ {4,    6, 32,   32},

/* 4 */ {4,    4, 16,   16},  /* lazy matches */
/* 5 */ {8,   16, 32,   32},
/* 6 */ {8,   16, 128, 128},
/* 7 */ {8,   32, 128, 256},
/* 8 */ {32, 128, 258, 1024},
/* 9 */ {32, 258, 258, 4096}}; /* maximum compression */

/* Note: the deflate() code requires max_lazy >= MIN_MATCH and max_chain >= 4
 */

#define EQUAL 0
/* result of memcmp for equal strings */

/* ===========================================================================
 *  Prototypes for local functions.
 */
static void fill_window   OF((void));

static int  longest_match OF((IPos cur_match));
#ifdef ASMV
  static void match_init OF((void)); /* asm code initialization */
#endif

#ifdef DEBUG
  static void check_match OF((IPos start, IPos match, int length));
#endif

/* ===========================================================================
 * Update a hash value with the given input byte
 * IN  assertion: all calls to to UPDATE_HASH are made with consecutive
 *    input characters, so that a running hash key can be computed from the
 *    previous key instead of complete recalculation each time.
 */
#define UPDATE_HASH(h,c) (h = (((h)<<H_SHIFT) ^ (c)) & HASH_MASK)

/* ===========================================================================
 * Insert string s in the dictionary and set match_head to the previous head
 * of the hash chain (the most recent string with same hash key). Return
 * the previous length of the hash chain.
 * IN  assertion: all calls to to INSERT_STRING are made with consecutive
 *    input characters and the first MIN_MATCH bytes of s are valid
 *    (except for the last MIN_MATCH-1 bytes of the input file).
 */
#define INSERT_STRING(s, match_head) \
   (UPDATE_HASH(ins_h, window[(s) + MIN_MATCH-1]), \
    prev[(s) & WMASK] = match_head = head[ins_h], \
    head[ins_h] = (s))

/* ===========================================================================
 * Initialize the "longest match" routines for a new file
 */
static void lm_init (pack_level, flags)
    int pack_level; /* 0: store, 1: best speed, 9: best compression */
    ush *flags;     /* general purpose bit flag */
{
    register unsigned j;

    if (on_the_fly_state == 2) goto oth2; 

    if (pack_level < 1 || pack_level > 9) msg_print(stderr, "ERROR: bad pack level\n");
    compr_level = pack_level;

    /* Initialize the hash table. */
#if defined(MAXSEG_64K) && HASH_BITS == 15
    for (j = 0;  j < HASH_SIZE; j++) head[j] = NIL;
#else
    memzero((char*)head, HASH_SIZE*sizeof(*head));
#endif
    /* prev will be initialized on the fly */

    /* Set the default configuration parameters:
     */
    max_lazy_match   = configuration_table[pack_level].max_lazy;
    good_match       = configuration_table[pack_level].good_length;
#ifndef FULL_SEARCH
    nice_match       = configuration_table[pack_level].nice_length;
#endif
    max_chain_length = configuration_table[pack_level].max_chain;
    if (pack_level == 1) {
       *flags |= FAST;
    } else if (pack_level == 9) {
       *flags |= SLOW;
    }
    /* ??? reduce max_chain_length for binary files */
 
    strstart = 0;
    block_start = 0L;
#ifdef ASMV
    match_init(); /* initialize the asm code */
#endif

    lookahead = file_read((char*)window,
			  sizeof(int) <= 2 ? (unsigned)WSIZE : 2*WSIZE);

    if (lookahead == 0 || lookahead == (unsigned)EOF) {
       eofile = 1, lookahead = 0;
       return;
    }
    eofile = 0;
    /* Make sure that we always have enough lookahead. This is important
     * if input comes from a device such as a tty.
     */
    if (on_the_fly_state) on_the_fly_state = 2;
    oth2:
    while (lookahead < MIN_LOOKAHEAD && !eofile) fill_window();
    if (eofile && on_the_fly_state && !on_the_fly_eofile) return;

    ins_h = 0;
    for (j=0; j<MIN_MATCH-1; j++) UPDATE_HASH(ins_h, window[j]);
    /* If lookahead < MIN_MATCH, ins_h is garbage, but this is
     * not important since only literal bytes will be emitted.
     */
}

/* ===========================================================================
 * Set match_start to the longest match starting at the given string and
 * return its length. Matches shorter or equal to prev_length are discarded,
 * in which case the result is equal to prev_length and match_start is
 * garbage.
 * IN assertions: cur_match is the head of the hash chain for the current
 *   string (strstart) and its distance is <= MAX_DIST, and prev_length >= 1
 */
#ifndef ASMV
/* For MSDOS, OS/2 and 386 Unix, an optimized version is in match.asm or
 * match.s. The code is functionally equivalent, so you can use the C version
 * if desired.
 */
static int longest_match(cur_match)
    IPos cur_match;                             /* current match */
{
    unsigned chain_length = max_chain_length;   /* max hash chain length */
    register uch *scan = window + strstart;     /* current string */
    register uch *match;                        /* matched string */
    register int len;                           /* length of current match */
    int best_len = prev_length;                 /* best match length so far */
    IPos limit = strstart > (IPos)MAX_DIST ? strstart - (IPos)MAX_DIST : NIL;
    /* Stop when cur_match becomes <= limit. To simplify the code,
     * we prevent matches with the string of window index 0.
     */

/* The code is optimized for HASH_BITS >= 8 and MAX_MATCH-2 multiple of 16.
 * It is easy to get rid of this optimization if necessary.
 */
#if HASH_BITS < 8 || MAX_MATCH != 258
   #error: Code too clever
#endif

#ifdef UNALIGNED_OK
    /* Compare two bytes at a time. Note: this is not always beneficial.
     * Try with and without -DUNALIGNED_OK to check.
     */
    register uch *strend = window + strstart + MAX_MATCH - 1;
    register ush scan_start = *(ush*)scan;
    register ush scan_end   = *(ush*)(scan+best_len-1);
#else
    register uch *strend = window + strstart + MAX_MATCH;
    register uch scan_end1  = scan[best_len-1];
    register uch scan_end   = scan[best_len];
#endif

    /* Do not waste too much time if we already have a good match: */
    if (prev_length >= good_match) {
        chain_length >>= 2;
    }
    Assert(strstart <= window_size-MIN_LOOKAHEAD, "insufficient lookahead");

    do {
        Assert(cur_match < strstart, "no future");
        match = window + cur_match;

        /* Skip to next match if the match length cannot increase
         * or if the match length is less than 2:
         */
#if (defined(UNALIGNED_OK) && MAX_MATCH == 258)
        /* This code assumes sizeof(unsigned short) == 2. Do not use
         * UNALIGNED_OK if your compiler uses a different size.
         */
        if (*(ush*)(match+best_len-1) != scan_end ||
            *(ush*)match != scan_start) continue;

        /* It is not necessary to compare scan[2] and match[2] since they are
         * always equal when the other bytes match, given that the hash keys
         * are equal and that HASH_BITS >= 8. Compare 2 bytes at a time at
         * strstart+3, +5, ... up to strstart+257. We check for insufficient
         * lookahead only every 4th comparison; the 128th check will be made
         * at strstart+257. If MAX_MATCH-2 is not a multiple of 8, it is
         * necessary to put more guard bytes at the end of the window, or
         * to check more often for insufficient lookahead.
         */
        scan++, match++;
        do {
        } while (*(ush*)(scan+=2) == *(ush*)(match+=2) &&
                 *(ush*)(scan+=2) == *(ush*)(match+=2) &&
                 *(ush*)(scan+=2) == *(ush*)(match+=2) &&
                 *(ush*)(scan+=2) == *(ush*)(match+=2) &&
                 scan < strend);
        /* The funny "do {}" generates better code on most compilers */

        /* Here, scan <= window+strstart+257 */
        Assert(scan <= window+(unsigned)(window_size-1), "wild scan");
        if (*scan == *match) scan++;

        len = (MAX_MATCH - 1) - (int)(strend-scan);
        scan = strend - (MAX_MATCH-1);

#else /* UNALIGNED_OK */

        if (match[best_len]   != scan_end  ||
            match[best_len-1] != scan_end1 ||
            *match            != *scan     ||
            *++match          != scan[1])      continue;

        /* The check at best_len-1 can be removed because it will be made
         * again later. (This heuristic is not always a win.)
         * It is not necessary to compare scan[2] and match[2] since they
         * are always equal when the other bytes match, given that
         * the hash keys are equal and that HASH_BITS >= 8.
         */
        scan += 2, match++;

        /* We check for insufficient lookahead only every 8th comparison;
         * the 256th check will be made at strstart+258.
         */
        do {
        } while (*++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 *++scan == *++match && *++scan == *++match &&
                 scan < strend);

        len = MAX_MATCH - (int)(strend - scan);
        scan = strend - MAX_MATCH;

#endif /* UNALIGNED_OK */

        if (len > best_len) {
            match_start = cur_match;
            best_len = len;
            if (len >= nice_match) break;
#ifdef UNALIGNED_OK
            scan_end = *(ush*)(scan+best_len-1);
#else
            scan_end1  = scan[best_len-1];
            scan_end   = scan[best_len];
#endif
        }
    } while ((cur_match = prev[cur_match & WMASK]) > limit
	     && --chain_length != 0);

    return best_len;
}
#endif /* ASMV */

#ifdef DEBUG
/* ===========================================================================
 * Check that the match at match_start is indeed a match.
 */
static void check_match(start, match, length)
    IPos start, match;
    int length;
{
    /* check that the match is indeed a match */
    if (memcmp((char*)window + match,
                (char*)window + start, length) != EQUAL) {
        msg_print(stderr,
		  "ERROR: invalid match : start %d, match %d, length %d\n",
		  start, match, length);
    }
    if (verbose > 1) {
        msg_print(stderr,"\\[%d,%d]", start-match, length);
        do { putc(window[start++], stderr); } while (--length != 0);
    }
}
#else
#  define check_match(start, match, length)
#endif

/* ===========================================================================
 * Fill the window when the lookahead becomes insufficient.
 * Updates strstart and lookahead, and sets eofile if end of input file.
 * IN assertion: lookahead < MIN_LOOKAHEAD && strstart + lookahead > 0
 * OUT assertions: at least one byte has been read, or eofile is set;
 *    file reads are performed for at least two bytes (required for the
 *    translate_eol option).
 */
static void fill_window()
{
    register unsigned n, m;
    unsigned more = (unsigned)(window_size - (ulg)lookahead - (ulg)strstart);
    /* Amount of free space at the end of the window. */

    /* If the window is almost full and there is insufficient lookahead,
     * move the upper half to the lower one to make room in the upper half.
     */
    if (more == (unsigned)EOF) {
        /* Very unlikely, but possible on 16 bit machine if strstart == 0
         * and lookahead == 1 (input done one byte at time)
         */
        more--;
    } else if (strstart >= WSIZE+MAX_DIST) {
        /* By the IN assertion, the window is not empty so we can't confuse
         * more == 0 with more == 64K on a 16 bit machine.
         */
        Assert(window_size == (ulg)2*WSIZE, "no sliding with BIG_MEM");

        memcpy((char*)window, (char*)window+WSIZE, (unsigned)WSIZE);
        match_start -= WSIZE;
        strstart    -= WSIZE; /* we now have strstart >= MAX_DIST: */

        block_start -= (long) WSIZE;

        for (n = 0; n < HASH_SIZE; n++) {
            m = head[n];
            head[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
        }
        for (n = 0; n < WSIZE; n++) {
            m = prev[n];
            prev[n] = (Pos)(m >= WSIZE ? m-WSIZE : NIL);
            /* If n is not on any hash chain, prev[n] is garbage but
             * its value will never be used.
             */
        }
        more += WSIZE;
    }
    /* At this point, more >= 2 */
    if (!eofile) {
        n = file_read((char*)window+strstart+lookahead, more);
        if (n == 0 || n == (unsigned)EOF) {
            eofile = 1;
        } else {
            lookahead += n;
        }
    }
}

/* ===========================================================================
 * Flush the current block, with given end-of-file flag.
 * IN assertion: strstart is set to the end of the current match.
 */
#define FLUSH_BLOCK(eof) \
   flush_block(block_start >= 0L ? (char*)&window[(unsigned)block_start] : \
                (char*)NULL, (long)strstart - block_start, flush-1, (eof)) 


/* ===========================================================================
 * Same as above, but achieves better compression. We use a lazy
 * evaluation for matches: a match is finally adopted only if there is
 * no better match at the next window position.
 */
static off_t deflate()
{
    static IPos hash_head;          /* head of hash chain */
    static IPos prev_match;         /* previous match */
    static int flush;               /* set if current block must be flushed */
    static int match_available;     /* set if previous match exists */
    static unsigned match_length;   /* length of best match */

    if (on_the_fly_state == 3) goto oth3; 

    match_available = 0;
    match_length = MIN_MATCH-1;

    /* Process the input block. */
    while (lookahead != 0) {
        /* Insert the string window[strstart .. strstart+2] in the
         * dictionary, and set hash_head to the head of the hash chain:
         */
        INSERT_STRING(strstart, hash_head);

        /* Find the longest match, discarding those <= prev_length.
         */
        prev_length = match_length, prev_match = match_start;
        match_length = MIN_MATCH-1;

        if (hash_head != NIL && prev_length < max_lazy_match &&
            strstart - hash_head <= MAX_DIST &&
            strstart <= window_size - MIN_LOOKAHEAD) {
            /* To simplify the code, we prevent matches with the string
             * of window index 0 (in particular we have to avoid a match
             * of the string with itself at the start of the input file).
             */
            match_length = longest_match (hash_head);
            /* longest_match() sets match_start */
            if (match_length > lookahead) match_length = lookahead;

            /* Ignore a length 3 match if it is too distant: */
            if (match_length == MIN_MATCH && strstart-match_start > TOO_FAR){
                /* If prev_match is also MIN_MATCH, match_start is garbage
                 * but we will ignore the current match anyway.
                 */
                match_length--;
            }
        }
        /* If there was a match at the previous step and the current
         * match is not better, output the previous match:
         */
        if (prev_length >= MIN_MATCH && match_length <= prev_length) {

            check_match(strstart-1, prev_match, prev_length);

            flush = ct_tally(strstart-1-prev_match, prev_length - MIN_MATCH);

            /* Insert in hash table all strings up to the end of the match.
             * strstart-1 and strstart are already inserted.
             */
            lookahead -= prev_length-1;
            prev_length -= 2;
            do {
                strstart++;
                INSERT_STRING(strstart, hash_head);
                /* strstart never exceeds WSIZE-MAX_MATCH, so there are
                 * always MIN_MATCH bytes ahead. If lookahead < MIN_MATCH
                 * these bytes are garbage, but it does not matter since the
                 * next lookahead bytes will always be emitted as literals.
                 */
            } while (--prev_length != 0);
            match_available = 0;
            match_length = MIN_MATCH-1;
            strstart++;

            if (flush) FLUSH_BLOCK(0), block_start = strstart;
        } else if (match_available) {
            /* If there was no match at the previous position, output a
             * single literal. If there was a match but the current match
             * is longer, truncate the previous match to a single literal.
             */
            Tracevv((stderr,"%c",window[strstart-1]));
            flush = ct_tally (0, window[strstart-1]);

            if (flush) FLUSH_BLOCK(0), block_start = strstart;

            strstart++;
            lookahead--;
        } else {
            /* There is no previous match to compare with, wait for
             * the next step to decide.
             */
             
            match_available = 1;

            strstart++;
            lookahead--;
        }
        Assert (strstart <= bytes_in && lookahead <= bytes_in, "a bit too far");

        /* Make sure that we always have enough lookahead, except
         * at the end of the input file. We need MAX_MATCH bytes
         * for the next match, plus MIN_MATCH bytes to insert the
         * string following the next match.
         */
	if (on_the_fly_state) on_the_fly_state = 3;
	oth3:
        while (lookahead < MIN_LOOKAHEAD && !eofile) fill_window();
	if (eofile && on_the_fly_state && !on_the_fly_eofile) return 0;
    }
    if (match_available) ct_tally (0, window[strstart-1]);

    return FLUSH_BLOCK(1); /* eof */
}


/*------------------------- zip.c --------------------------*/

/* ===========================================================================
 * Deflate in to out.
 * IN assertions: the input and output buffers are cleared.
 *   The variables time_stamp and save_orig_name are initialized.
 */
static int zip(void)
{
    static uch  flags;         /* general purpose bit flags */
    static ush  attr;          /* ascii/binary flag */
    static ush  deflate_flags; /* pkzip -es, -en or -ex equivalent */
    static ulg  stamp;

    /* for on the fly compression : restart from where we stopped */
    if (on_the_fly_state == 2) goto oth2;
    if (on_the_fly_state == 3) goto oth3;

    flags = 0;
    attr = 0;  
    deflate_flags = 0;
    stamp = 0;

    outcnt = 0;

    /* Write the header to the gzip file. See algorithm.doc for the format */

    method = DEFLATED;
    put_byte(GZIP_MAGIC[0]); /* magic header */
    put_byte(GZIP_MAGIC[1]);
    put_byte(DEFLATED);      /* compression method */

    put_byte(flags);         /* general flags */
    put_long (stamp);

    /* Write deflated file to zip file */
    crc = updcrc(0, 0);

    bi_init(ofd);
    ct_init(&attr, &method);

    oth2:
    lm_init(level, &deflate_flags);
    if (eofile && on_the_fly_state && !on_the_fly_eofile) {eofile = 0; return 0;}

    put_byte((uch)deflate_flags); /* extra flags */
    put_byte(OS_CODE);            /* OS identifier */

    oth3:
    (void)deflate();
    if (input_error || output_error) goto inouterr;
    if (eofile && on_the_fly_state && !on_the_fly_eofile) {eofile = 0; return 0;}

    /* Write the crc and uncompressed size */
    put_long(crc);
    put_long((ulg)bytes_in);

    flush_outbuf();

    inouterr:
    if (input_error) {
	msg_print(stderr, "ERROR: input I/O error or maximum input size reached\n");
	return -30;
    }
    if (output_error) {
	msg_print(stderr, "ERROR: output I/O error or maximum output size reached\n");
	return -30;
    }

    return 0;
}

/* for "on the fly" compression : add some data to compress */
static int zip_add_data(char *in_data, int count)
{
    if (input_error || output_error) return -30;
    if (!on_the_fly_state) {
	if (count == 0)
	    return 0; /* end of data at first call -> nothing to do */
	on_the_fly_state = 1;
    }
    if (count == 0) on_the_fly_eofile = 1; /* end of data */
    on_the_fly_data_size = count;
    on_the_fly_data_ptr = in_data;
    return zip();
}

/********************************************************************/


/********************unzip**********************************************/

#define	OLD_GZIP_MAGIC "\037\236" /* Magic header for gzip 0.5 = freeze 1.x */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

#define MAX_WSIZE 0x8000 /* the maximum window size used when gziping data */

#define get_byte()  (inptr < insize ? uinbuf[(on_the_fly_state) ? inptr++ & (UNZIP_INBUFSIZ - 1) : inptr++] : fill_inbuf())

/* Macros for getting two-byte and four-byte header values */
#define SH(p) ((ush)(uch)((p)[0]) | ((ush)(uch)((p)[1]) << 8))
#define LG(p) ((ulg)(SH(p)) | ((ulg)(SH((p)+2)) << 16))

#define INBUFCOUNT (insize - inptr)

static int on_the_fly_done = 0;
static int can_check_crc = 0;

DECLARE(uch, uinbuf,  UNZIP_INBUFSIZ);

#ifdef UNZIP_OUTBUFSIZ
DECLARE(uch, uoutbuf,  UNZIP_OUTBUFSIZ);
#endif

/* ---- from puff.c----
 *
 * Maximums for allocations and loops.  It is not useful to change these --
 * they are fixed by the deflate format.
 */
#define MAXBITS 15              /* maximum bits in a code */
#define MAXLCODES 286           /* maximum number of literal/length codes */
#define MAXDCODES 30            /* maximum number of distance codes */
#define MAXCODES (MAXLCODES+MAXDCODES)  /* maximum codes lengths to read */
#define FIXLCODES 288           /* number of fixed literal/length codes */
/* ---- end of from puff.c---- ***/
#ifdef UNZIP_CODES_DATA_OVERLAY
typedef union /* to save some memory we overlay the structures are we do not need both at the same time */
#else
typedef struct /* allocate both structures, but as the first is a fixed one, it will ony be filled once (faster) */
#endif
{
    struct { /* 1276 bytes */
	short lencnt[MAXBITS+1], lensym[FIXLCODES];
	short distcnt[MAXBITS+1], distsym[MAXDCODES];
	short lengths[FIXLCODES];
    } fixed;
    struct { /* 1328 bytes */
	short lengths[MAXCODES]; /* descriptor code lengths */
	short lencnt[MAXBITS+1], lensym[MAXLCODES]; /* lencode memory */
	short distcnt[MAXBITS+1], distsym[MAXDCODES]; /* distcode memory */
    } dynamic;
} t_codes_data;
DECLARE(t_codes_data, codes_data, 1);

static int unzip_init(int infd, int outfd, char *indataptr, char *outdataptr, int indatasize, int outmaxsize)
{
    int rc = 0;

    ifd = infd;
    ofd = outfd;

    input_data_ptr = indataptr;
    output_data_ptr = outdataptr;
    input_data_size = indatasize;
    output_max_size = outmaxsize;

    if (ifd == FD_MEMORY) {
	/* input is in memory so no need to allocate a buffer for input data */
	uinbuf = (uch *)indataptr;
    } else {
	/* allocate a buffer for loading blocks of input data */
	ALLOC(uch, uinbuf,  UNZIP_INBUFSIZ, rc);
    }
#ifdef UNZIP_OUTBUFSIZ
    if (ofd != FD_MEMORY) {
	ALLOC(uch, uoutbuf,  UNZIP_OUTBUFSIZ, rc);
    }
#endif
    ALLOC(t_codes_data, codes_data, 1, rc);

    zip_unzip_restart(0);

    return rc;
}

static int unzip_stop(void)
{
    /* Allocate all global buffers (for DYN_ALLOC option) */
    if (ifd != FD_MEMORY) {
	FREE(uinbuf);
    }
#ifdef UNZIP_OUTBUFSIZ
    if (ofd != FD_MEMORY) {
	FREE(uoutbuf);
    }
#endif
    FREE(codes_data);
    return 0;
}

#ifdef UNZIP_OUTBUFSIZ
#  ifndef UNZIP_OUTMAXBLKSIZE
#    define UNZIP_OUTMAXBLKSIZE UNZIP_OUTBUFSIZ
#  endif
#  ifndef UNZIP_OUTMINBLKSIZE
#    define UNZIP_OUTMINBLKSIZE 1
#  endif
static int write_at_blksize(int fd, uch *buf, int size)
{
    int count, rc;
    for (count = 0; count < size; ) {
	rc = write(fd, buf, ((size - count) > UNZIP_OUTMAXBLKSIZE) ? UNZIP_OUTMAXBLKSIZE : size - count);
	if (rc == 0) break;
	if (rc < 0) return rc;
	count += rc;
	buf += rc;
    }
    return count;
}
#endif

static void write_byte(int fd, uch byte)
{
    updcrc(&byte, 1);

    if (test_mode) {
	/* test mode */
	if (fd == FD_MEMORY) {
	    /* only MAX_WSIZE bytes of memory required for test */
	    output_data_ptr[bytes_out & (MAX_WSIZE - 1)] = byte;
	}
#ifdef UNZIP_OUTBUFSIZ
	else
	{
	    /* a circular output buffer is used, so fill it with data byte */
	    int obufindex = bytes_out & (UNZIP_OUTBUFSIZ - 1);
	    uoutbuf[obufindex] = byte;
	}
#endif
	bytes_out++;
	return;
    }

    /* not test mode */

    if ((output_max_size != (off_t)-1) && (bytes_out >= output_max_size)) {
	/* maximum output size reached */
	output_error = 1;
	return;
    }

    if (fd == FD_MEMORY) {
	/* write is to memory */
	output_data_ptr[bytes_out] = byte; /* write data byte */
    } else {
	int rc = 1;
#ifdef UNZIP_OUTBUFSIZ
	/* a circular output buffer is used, so fill it with data byte, */
	/* and write it each time it is full if not in test mode */
	int obufindex = bytes_out & (UNZIP_OUTBUFSIZ - 1);
	uoutbuf[obufindex] = byte;
	if (obufindex == UNZIP_OUTBUFSIZ - 1)
	    rc = write_at_blksize(fd, uoutbuf, UNZIP_OUTBUFSIZ);
#else
	/* write data byte */
	rc = write(fd, &byte, 1);
#endif
	if (rc <= 0) {
	    output_error = 1; /* I/O error or output full */
	    return;
	}
    }

    bytes_out++;
}

static void write_flush(int fd) {
#ifdef UNZIP_OUTBUFSIZ
    /* end of gunzip : flush data still in buffer, and add some pad if needed to match the min blocksize */
    int obufindex = bytes_out & (UNZIP_OUTBUFSIZ - 1);
    while (obufindex & (UNZIP_OUTMINBLKSIZE - 1))
	uoutbuf[obufindex++] = 0;
    if (!test_mode)
	write_at_blksize(fd, uoutbuf, obufindex);
#endif 
}

uch read_written_byte_at_offset(int fd, int offset)
{
    uch byte;
    off_t cur_offset;

    if (fd == FD_MEMORY) {
	/* output is to memory, so just reread the data written */
	if (!test_mode)
	    byte = output_data_ptr[offset];
	else
	    byte = output_data_ptr[offset & (MAX_WSIZE - 1)]; /* only MAX_WSIZE used for test */
    } else {
	/* output is to a file or device */
#ifdef UNZIP_OUTBUFSIZ
	if (bytes_out - offset <= UNZIP_OUTBUFSIZ)
	{
	    /* a buffer is used and requested data is still in the circular buffer */
	    int obufindex = offset & (UNZIP_OUTBUFSIZ - 1);
	    byte = uoutbuf[obufindex];
	    return byte;
	}
#endif
	/* no buffer is used or data is no more in the buffer */
	if (test_mode) {
	    /* as nothing has been written to device/file, it is impossible to reread data, */
	    /* so just return 0 as dummy data. It will not be possible to check CRC, only size */
	    /* will be checked */
	    can_check_crc = 0;
	    return 0;
	}
	/* seek back in the data written to reread data */
	cur_offset = lseek(fd, 0, SEEK_CUR); /* save current offset */
	lseek(fd, offset, SEEK_SET);
	read(fd, &byte, 1);
	lseek(fd, cur_offset, SEEK_SET); /* restore saved offset for next write */
    }

    return byte;
}

/* Fill the input buffer. This is called only when the buffer is empty */
static int fill_inbuf(void)
{
    int rc;
    int len;
    static int input_limited = 0;

    if (ifd == FD_MEMORY) {
	/* error: should not be there as memory buffer is supposed to contain all data */
	msg_print(stderr, "ERROR: actual size of data is greater than specified size\n");
	input_error = 1;
	return 0; /* return something */
    }
    if (ifd == FD_ON_THE_FLY) {
	/* error: should not be there as unzip_add_data() is supposed to have filled uinbuf */
	msg_print(stderr, "ERROR: on the fly input processing underrun\n");
	input_error = 1;
	return 0; /* return something */
    }

    /* input buffer is currently empty, read as much as possible to fill it */
    insize = 0;
    inptr = 0;
    len = UNZIP_INBUFSIZ - insize;
    if (bytes_in == 0) input_limited = 0;
    if (input_limited) {
	/* the previous read has been limited to not read more than input_data_size */
	/* but as we are here again, this means that unzip() needs more data than allowed */
	input_error = 1; /* error : more than input_data_size required */
	return 0;
    }
    if ((input_data_size != (off_t)-1) && (bytes_in + len > input_data_size)) {
	len = input_data_size - bytes_in;
	input_limited = 1;
    }
    do {
	rc = read(ifd, (char *)uinbuf + insize, len);
	if (rc < 0) {input_error = 1; return 0;} /* input I/O error */
	if (rc == 0) {
	    /* no more data available */
	    if (insize) break; /* at least one byte in input buffer -> OK, we can return a byte of data */
	    /* still nothing in buffer -> nothing to return -> error */
	    input_error = 1;
	    return 0;
	}
	len -=rc;
	insize += rc;
	bytes_in += rc;
    } while (insize < UNZIP_INBUFSIZ);

    return uinbuf[inptr++]; /* return a byte of data */
}


/*------------------------- puff.c --------------------------*/

#define local static            /* for local function definitions */
//#define NIL ((unsigned char *)0)        /* for no output option */

/* input and output state */
struct state {
    int bitbuf;                 /* bit buffer */
    int bitcnt;                 /* number of bits in bit buffer */
};

/*
 * Return need bits from the input stream.  This always leaves less than
 * eight bits in the buffer.  bits() works properly for need == 0.
 *
 * Format notes:
 *
 * - Bits are stored in bytes from the least significant bit to the most
 *   significant bit.  Therefore bits are dropped from the bottom of the bit
 *   buffer, using shift right, and new bytes are appended to the top of the
 *   bit buffer, using shift left.
 */
local int bits(struct state *s, int need)
{
    long val;           /* bit accumulator (can use up to 20 bits) */

    /* load at least need bits into val */
    val = s->bitbuf;
    while (s->bitcnt < need) {
        val |= (long)(get_byte()) << s->bitcnt;  /* load eight bits */
 	if (input_error) return 0;
	s->bitcnt += 8;
    }

    /* drop need bits and update buffer, always zero to seven bits left */
    s->bitbuf = (int)(val >> need);
    s->bitcnt -= need;

    /* return need bits, zeroing the bits above that */
    return (int)(val & ((1L << need) - 1));
}

/*
 * Process a stored block.
 *
 * Format notes:
 *
 * - After the two-bit stored block type (00), the stored block length and
 *   stored bytes are byte-aligned for fast copying.  Therefore any leftover
 *   bits in the byte that has the last bit of the type, as many as seven, are
 *   discarded.  The value of the discarded bits are not defined and should not
 *   be checked against any expectation.
 *
 * - The second inverted copy of the stored block length does not have to be
 *   checked, but it's probably a good idea to do so anyway.
 *
 * - A stored block can have zero length.  This is sometimes used to byte-align
 *   subsets of the compressed data for random access or partial recovery.
 */
local int stored(struct state *s)
{
    static unsigned len;       /* length of stored block */

    //fprintf(stderr, "S\n"); fflush(stderr);
    if (on_the_fly_state >= 6) goto oth6;

    /* discard leftover bits from current byte (assumes s->bitcnt < 8) */
    s->bitbuf = 0;
    s->bitcnt = 0;

    /* get length and check against its one's complement */
    len = get_byte();
    len |= get_byte() << 8;
    if (get_byte() != (~len & 0xff) ||
        get_byte() != ((~len >> 8) & 0xff))
        return -2;                              /* didn't match complement! */
    if (input_error) return -30;

    /* copy len bytes from in to out */
    if (on_the_fly_state) on_the_fly_state = 6;
    oth6:
    while (len) {
	if (on_the_fly_state && (INBUFCOUNT < UNZIP_ONTHEFLY_MIN_LOOKAHEAD) && !on_the_fly_eofile) {
	    on_the_fly_done = 1;
	    return 0;
	}
	write_byte(ofd, get_byte());
	if (input_error) return -30;
	len--;
    }
    if (on_the_fly_state) on_the_fly_state = 5;

    /* done with a valid stored block */
    return 0;
}

/*
 * Huffman code decoding tables.  count[1..MAXBITS] is the number of symbols of
 * each length, which for a canonical code are stepped through in order.
 * symbol[] are the symbol values in canonical order, where the number of
 * entries is the sum of the counts in count[].  The decoding process can be
 * seen in the function decode() below.
 */
struct huffman {
    short *count;       /* number of symbols of each length */
    short *symbol;      /* canonically ordered symbols */
};

/*
 * Decode a code from the stream s using huffman table h.  Return the symbol or
 * a negative value if there is an error.  If all of the lengths are zero, i.e.
 * an empty code, or if the code is incomplete and an invalid code is received,
 * then -10 is returned after reading MAXBITS bits.
 *
 * Format notes:
 *
 * - The codes as stored in the compressed data are bit-reversed relative to
 *   a simple integer ordering of codes of the same lengths.  Hence below the
 *   bits are pulled from the compressed data one at a time and used to
 *   build the code value reversed from what is in the stream in order to
 *   permit simple integer comparisons for decoding.  A table-based decoding
 *   scheme (as used in zlib) does not need to do this reversal.
 *
 * - The first code for the shortest length is all zeros.  Subsequent codes of
 *   the same length are simply integer increments of the previous code.  When
 *   moving up a length, a zero bit is appended to the code.  For a complete
 *   code, the last code of the longest length will be all ones.
 *
 * - Incomplete codes are handled by this decoder, since they are permitted
 *   in the deflate format.  See the format notes for fixed() and dynamic().
 */
#ifdef SLOW
local int decode(struct state *s, struct huffman *h)
{
    int len;            /* current number of bits in code */
    int code;           /* len bits being decoded */
    int first;          /* first code of length len */
    int count;          /* number of codes of length len */
    int index;          /* index of first code of length len in symbol table */

    code = first = index = 0;
    for (len = 1; len <= MAXBITS; len++) {
        code |= bits(s, 1);             /* get next bit */
	if (input_error) return -1;
        count = h->count[len];
        if (code - count < first)       /* if length len, return symbol */
            return h->symbol[index + (code - first)];
        index += count;                 /* else update for next length */
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -10;                         /* ran out of codes */
}

/*
 * A faster version of decode() for real applications of this code.   It's not
 * as readable, but it makes puff() twice as fast.  And it only makes the code
 * a few percent larger.
 */
#else /* !SLOW */
local int decode(struct state *s, struct huffman *h)
{
    int len;            /* current number of bits in code */
    int code;           /* len bits being decoded */
    int first;          /* first code of length len */
    int count;          /* number of codes of length len */
    int index;          /* index of first code of length len in symbol table */
    int bitbuf;         /* bits from stream */
    int left;           /* bits left in next or left to process */
    short *next;        /* next number of codes */

    bitbuf = s->bitbuf;
    left = s->bitcnt;
    code = first = index = 0;
    len = 1;
    next = h->count + 1;
    while (1) {
        while (left--) {
            code |= bitbuf & 1;
            bitbuf >>= 1;
            count = *next++;
            if (code - count < first) { /* if length len, return symbol */
                s->bitbuf = bitbuf;
                s->bitcnt = (s->bitcnt - len) & 7;
                return h->symbol[index + (code - first)];
            }
            index += count;             /* else update for next length */
            first += count;
            first <<= 1;
            code <<= 1;
            len++;
        }
        left = (MAXBITS+1) - len;
        if (left == 0) break;
        bitbuf = get_byte();
	if (input_error) return -1;
        if (left > 8) left = 8;
    }
    return -10;                         /* ran out of codes */
}
#endif /* SLOW */

/*
 * Given the list of code lengths length[0..n-1] representing a canonical
 * Huffman code for n symbols, construct the tables required to decode those
 * codes.  Those tables are the number of codes of each length, and the symbols
 * sorted by length, retaining their original order within each length.  The
 * return value is zero for a complete code set, negative for an over-
 * subscribed code set, and positive for an incomplete code set.  The tables
 * can be used if the return value is zero or positive, but they cannot be used
 * if the return value is negative.  If the return value is zero, it is not
 * possible for decode() using that table to return an error--any stream of
 * enough bits will resolve to a symbol.  If the return value is positive, then
 * it is possible for decode() using that table to return an error for received
 * codes past the end of the incomplete lengths.
 *
 * Not used by decode(), but used for error checking, h->count[0] is the number
 * of the n symbols not in the code.  So n - h->count[0] is the number of
 * codes.  This is useful for checking for incomplete codes that have more than
 * one symbol, which is an error in a dynamic block.
 *
 * Assumption: for all i in 0..n-1, 0 <= length[i] <= MAXBITS
 * This is assured by the construction of the length arrays in dynamic() and
 * fixed() and is not verified by construct().
 *
 * Format notes:
 *
 * - Permitted and expected examples of incomplete codes are one of the fixed
 *   codes and any code with a single symbol which in deflate is coded as one
 *   bit instead of zero bits.  See the format notes for fixed() and dynamic().
 *
 * - Within a given code length, the symbols are kept in ascending order for
 *   the code bits definition.
 */
local int construct(struct huffman *h, short *length, int n)
{
    int symbol;         /* current symbol when stepping through length[] */
    int len;            /* current length when stepping through h->count[] */
    int left;           /* number of possible codes left of current length */
    short offs[MAXBITS+1];      /* offsets in symbol table for each length */

    /* count number of codes of each length */
    for (len = 0; len <= MAXBITS; len++)
        h->count[len] = 0;
    for (symbol = 0; symbol < n; symbol++)
        (h->count[length[symbol]])++;   /* assumes lengths are within bounds */
    if (h->count[0] == n)               /* no codes! */
        return 0;                       /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    left = 1;                           /* one possible code of zero length */
    for (len = 1; len <= MAXBITS; len++) {
        left <<= 1;                     /* one more bit, double codes left */
        left -= h->count[len];          /* deduct count from possible codes */
        if (left < 0) return left;      /* over-subscribed--return negative */
    }                                   /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    for (len = 1; len < MAXBITS; len++)
        offs[len + 1] = offs[len] + h->count[len];

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    for (symbol = 0; symbol < n; symbol++)
        if (length[symbol] != 0)
            h->symbol[offs[length[symbol]]++] = symbol;

    /* return zero for complete set, positive for incomplete set */
    return left;
}

/*
 * Decode literal/length and distance codes until an end-of-block code.
 *
 * Format notes:
 *
 * - Compressed data that is after the block type if fixed or after the code
 *   description if dynamic is a combination of literals and length/distance
 *   pairs terminated by and end-of-block code.  Literals are simply Huffman
 *   coded bytes.  A length/distance pair is a coded length followed by a
 *   coded distance to represent a string that occurs earlier in the
 *   uncompressed data that occurs again at the current location.
 *
 * - Literals, lengths, and the end-of-block code are combined into a single
 *   code of up to 286 symbols.  They are 256 literals (0..255), 29 length
 *   symbols (257..285), and the end-of-block symbol (256).
 *
 * - There are 256 possible lengths (3..258), and so 29 symbols are not enough
 *   to represent all of those.  Lengths 3..10 and 258 are in fact represented
 *   by just a length symbol.  Lengths 11..257 are represented as a symbol and
 *   some number of extra bits that are added as an integer to the base length
 *   of the length symbol.  The number of extra bits is determined by the base
 *   length symbol.  These are in the static arrays below, lens[] for the base
 *   lengths and lext[] for the corresponding number of extra bits.
 *
 * - The reason that 258 gets its own symbol is that the longest length is used
 *   often in highly redundant files.  Note that 258 can also be coded as the
 *   base value 227 plus the maximum extra value of 31.  While a good deflate
 *   should never do this, it is not an error, and should be decoded properly.
 *
 * - If a length is decoded, including its extra bits if any, then it is
 *   followed a distance code.  There are up to 30 distance symbols.  Again
 *   there are many more possible distances (1..32768), so extra bits are added
 *   to a base value represented by the symbol.  The distances 1..4 get their
 *   own symbol, but the rest require extra bits.  The base distances and
 *   corresponding number of extra bits are below in the static arrays dist[]
 *   and dext[].
 *
 * - Literal bytes are simply written to the output.  A length/distance pair is
 *   an instruction to copy previously uncompressed bytes to the output.  The
 *   copy is from distance bytes back in the output stream, copying for length
 *   bytes.
 *
 * - Distances pointing before the beginning of the output data are not
 *   permitted.
 *
 * - Overlapped copies, where the length is greater than the distance, are
 *   allowed and common.  For example, a distance of one and a length of 258
 *   simply copies the last byte 258 times.  A distance of four and a length of
 *   twelve copies the last four bytes three times.  A simple forward copy
 *   ignoring whether the length is greater than the distance or not implements
 *   this correctly.  You should not use memcpy() since its behavior is not
 *   defined for overlapped arrays.  You should not use memmove() or bcopy()
 *   since though their behavior -is- defined for overlapping arrays, it is
 *   defined to do the wrong thing in this case.
 */
local int codes(struct state *s,
                struct huffman *lencode,
                struct huffman *distcode)
{
    static int symbol;         /* decoded symbol */
    static int len;            /* length for copy */
    static unsigned dist;      /* distance for copy */
    static const short lens[29] = { /* Size base for length codes 257..285 */
        3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
        35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short lext[29] = { /* Extra bits for length codes 257..285 */
        0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
        3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dists[30] = { /* Offset base for distance codes 0..29 */
        1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
        257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
        8193, 12289, 16385, 24577};
    static const short dext[30] = { /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
        7, 7, 8, 8, 9, 9, 10, 10, 11, 11,
        12, 12, 13, 13};

    /* decode literals and length/distance pairs */
    do {
	if (on_the_fly_state && (INBUFCOUNT < UNZIP_ONTHEFLY_MIN_LOOKAHEAD) && !on_the_fly_eofile) {
	    on_the_fly_done = 1;
	    return 0;
	}

        symbol = decode(s, lencode);
        if (symbol < 0) return symbol;  /* invalid symbol */
        if (symbol < 256) {             /* literal: symbol is the byte */
            /* write out the literal */
	    write_byte(ofd, symbol);
        }
        else if (symbol > 256) {        /* length */
            /* get and compute length */
            symbol -= 257;
            if (symbol >= 29) return -10;       /* invalid fixed code */
            len = lens[symbol] + bits(s, lext[symbol]);

            /* get and check distance */
            symbol = decode(s, distcode);
            if (symbol < 0) return symbol;      /* invalid symbol */
            dist = dists[symbol] + bits(s, dext[symbol]);
	    if (input_error) return -1;
#ifndef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
            if ((int)dist > bytes_out)
                return -11;     /* distance too far back */
#endif

            /* copy length bytes from distance bytes back */
	    while (len--) {
		write_byte(ofd,
#ifdef INFLATE_ALLOW_INVALID_DISTANCE_TOOFAR_ARRR
			   dist > bytes_out ? 0 :
#endif
			   read_written_byte_at_offset(ofd, bytes_out - dist));
	    }
        }
    } while (symbol != 256);            /* end of block symbol */

    /* done with a valid fixed or dynamic block */
    return 0;
}

/*
 * Process a fixed codes block.
 *
 * Format notes:
 *
 * - This block type can be useful for compressing small amounts of data for
 *   which the size of the code descriptions in a dynamic block exceeds the
 *   benefit of custom codes for that block.  For fixed codes, no bits are
 *   spent on code descriptions.  Instead the code lengths for literal/length
 *   codes and distance codes are fixed.  The specific lengths for each symbol
 *   can be seen in the "for" loops below.
 *
 * - The literal/length code is complete, but has two symbols that are invalid
 *   and should result in an error if received.  This cannot be implemented
 *   simply as an incomplete code since those two symbols are in the "middle"
 *   of the code.  They are eight bits long and the longest literal/length\
 *   code is nine bits.  Therefore the code must be constructed with those
 *   symbols, and the invalid symbols must be detected after decoding.
 *
 * - The fixed distance codes also have two invalid symbols that should result
 *   in an error if received.  Since all of the distance codes are the same
 *   length, this can be implemented as an incomplete code.  Then the invalid
 *   codes are detected while decoding.
 */
local int fixed(struct state *s)
{
    static int virgin = 1;
    static struct huffman lencode, distcode;

    //fprintf(stderr, "F\n"); fflush(stderr);
    if (on_the_fly_state >= 6) goto oth6;

    /* build fixed huffman tables if needed */
    if (virgin) {
        int symbol;
        short *lengths = codes_data->fixed.lengths;

        /* init lencode and distcode structure pointers (must be done before calling construct()) */
        lencode.count = codes_data->fixed.lencnt;
        lencode.symbol = codes_data->fixed.lensym;
        distcode.count = codes_data->fixed.distcnt;
        distcode.symbol = codes_data->fixed.distsym;

        /* literal/length table */
        for (symbol = 0; symbol < 144; symbol++)
            lengths[symbol] = 8;
        for (; symbol < 256; symbol++)
            lengths[symbol] = 9;
        for (; symbol < 280; symbol++)
            lengths[symbol] = 7;
        for (; symbol < FIXLCODES; symbol++)
            lengths[symbol] = 8;
        construct(&lencode, lengths, FIXLCODES);

        /* distance table */
        for (symbol = 0; symbol < MAXDCODES; symbol++)
            lengths[symbol] = 5;
        construct(&distcode, lengths, MAXDCODES);

#ifndef UNZIP_CODES_DATA_OVERLAY
        /* do this just once as codes will never be overwritten */
        virgin = 0;
#endif
    }

    /* decode data until end-of-block code */
    {
	int err;
	if (on_the_fly_state) on_the_fly_state = 6;
        oth6:
	err = codes(s, &lencode, &distcode);
	if (on_the_fly_done) return 0;
	if (on_the_fly_state) on_the_fly_state = 5;
	return err;
    }
}

/*
 * Process a dynamic codes block.
 *
 * Format notes:
 *
 * - A dynamic block starts with a description of the literal/length and
 *   distance codes for that block.  New dynamic blocks allow the compressor to
 *   rapidly adapt to changing data with new codes optimized for that data.
 *
 * - The codes used by the deflate format are "canonical", which means that
 *   the actual bits of the codes are generated in an unambiguous way simply
 *   from the number of bits in each code.  Therefore the code descriptions
 *   are simply a list of code lengths for each symbol.
 *
 * - The code lengths are stored in order for the symbols, so lengths are
 *   provided for each of the literal/length symbols, and for each of the
 *   distance symbols.
 *
 * - If a symbol is not used in the block, this is represented by a zero as
 *   as the code length.  This does not mean a zero-length code, but rather
 *   that no code should be created for this symbol.  There is no way in the
 *   deflate format to represent a zero-length code.
 *
 * - The maximum number of bits in a code is 15, so the possible lengths for
 *   any code are 1..15.
 *
 * - The fact that a length of zero is not permitted for a code has an
 *   interesting consequence.  Normally if only one symbol is used for a given
 *   code, then in fact that code could be represented with zero bits.  However
 *   in deflate, that code has to be at least one bit.  So for example, if
 *   only a single distance base symbol appears in a block, then it will be
 *   represented by a single code of length one, in particular one 0 bit.  This
 *   is an incomplete code, since if a 1 bit is received, it has no meaning,
 *   and should result in an error.  So incomplete distance codes of one symbol
 *   should be permitted, and the receipt of invalid codes should be handled.
 *
 * - It is also possible to have a single literal/length code, but that code
 *   must be the end-of-block code, since every dynamic block has one.  This
 *   is not the most efficient way to create an empty block (an empty fixed
 *   block is fewer bits), but it is allowed by the format.  So incomplete
 *   literal/length codes of one symbol should also be permitted.
 *
 * - If there are only literal codes and no lengths, then there are no distance
 *   codes.  This is represented by one distance code with zero bits.
 *
 * - The list of up to 286 length/literal lengths and up to 30 distance lengths
 *   are themselves compressed using Huffman codes and run-length encoding.  In
 *   the list of code lengths, a 0 symbol means no code, a 1..15 symbol means
 *   that length, and the symbols 16, 17, and 18 are run-length instructions.
 *   Each of 16, 17, and 18 are follwed by extra bits to define the length of
 *   the run.  16 copies the last length 3 to 6 times.  17 represents 3 to 10
 *   zero lengths, and 18 represents 11 to 138 zero lengths.  Unused symbols
 *   are common, hence the special coding for zero lengths.
 *
 * - The symbols for 0..18 are Huffman coded, and so that code must be
 *   described first.  This is simply a sequence of up to 19 three-bit values
 *   representing no code (0) or the code length for that symbol (1..7).
 *
 * - A dynamic block starts with three fixed-size counts from which is computed
 *   the number of literal/length code lengths, the number of distance code
 *   lengths, and the number of code length code lengths (ok, you come up with
 *   a better name!) in the code descriptions.  For the literal/length and
 *   distance codes, lengths after those provided are considered zero, i.e. no
 *   code.  The code length code lengths are received in a permuted order (see
 *   the order[] array below) to make a short code length code length list more
 *   likely.  As it turns out, very short and very long codes are less likely
 *   to be seen in a dynamic code description, hence what may appear initially
 *   to be a peculiar ordering.
 *
 * - Given the number of literal/length code lengths (nlen) and distance code
 *   lengths (ndist), then they are treated as one long list of nlen + ndist
 *   code lengths.  Therefore run-length coding can and often does cross the
 *   boundary between the two sets of lengths.
 *
 * - So to summarize, the code description at the start of a dynamic block is
 *   three counts for the number of code lengths for the literal/length codes,
 *   the distance codes, and the code length codes.  This is followed by the
 *   code length code lengths, three bits each.  This is used to construct the
 *   code length code which is used to read the remainder of the lengths.  Then
 *   the literal/length code lengths and distance lengths are read as a single
 *   set of lengths using the code length codes.  Codes are constructed from
 *   the resulting two sets of lengths, and then finally you can start
 *   decoding actual compressed data in the block.
 *
 * - For reference, a "typical" size for the code description in a dynamic
 *   block is around 80 bytes.
 */
local int dynamic(struct state *s)
{
    static int nlen, ndist, ncode;             /* number of lengths in descriptor */
    static int index;                          /* index of lengths[] */
    static int err;                            /* construct() return value */
    static short *lengths;              /* descriptor code lengths */
    struct huffman lencode, distcode;   /* length and distance codes */
    static const short order[19] =      /* permutation of code length codes */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    //fprintf(stderr, "D\n"); fflush(stderr);
    if (on_the_fly_state >= 6) goto oth6;

    /* init structure pointers */
    lengths = codes_data->dynamic.lengths;
    lencode.count = codes_data->dynamic.lencnt;
    lencode.symbol = codes_data->dynamic.lensym;
    distcode.count = codes_data->dynamic.distcnt;
    distcode.symbol = codes_data->dynamic.distsym;

    /* get number of lengths in each table, check lengths */
    nlen = bits(s, 5) + 257;
    ndist = bits(s, 5) + 1;
    ncode = bits(s, 4) + 4;
    if (input_error) return -30;
    if (nlen > MAXLCODES || ndist > MAXDCODES)
        return -3;                      /* bad counts */

    /* read code length code lengths (really), missing lengths are zero */
    for (index = 0; index < ncode; index++) {
        lengths[order[index]] = bits(s, 3);
	if (input_error) return -30;
    }
    for (; index < 19; index++)
        lengths[order[index]] = 0;

    /* build huffman table for code lengths codes (use lencode temporarily) */
    err = construct(&lencode, lengths, 19);
    if (err != 0) return -4;            /* require complete code set here */

    /* read length/literal and distance code length tables */
    index = 0;
    while (index < nlen + ndist) {
        int symbol;             /* decoded value */
        int len;                /* last length to repeat */

        symbol = decode(s, &lencode);
	if (input_error) return -30;
        if (symbol < 16)                /* length in 0..15 */
            lengths[index++] = symbol;
        else {                          /* repeat instruction */
            len = 0;                    /* assume repeating zeros */
            if (symbol == 16) {         /* repeat last length 3..6 times */
                if (index == 0) return -5;      /* no last length! */
                len = lengths[index - 1];       /* last length */
                symbol = 3 + bits(s, 2);
            }
            else if (symbol == 17)      /* repeat zero 3..10 times */
                symbol = 3 + bits(s, 3);
            else                        /* == 18, repeat zero 11..138 times */
                symbol = 11 + bits(s, 7);
	    if (input_error) return -30;
            if (index + symbol > nlen + ndist)
                return -6;              /* too many lengths! */
            while (symbol--)            /* repeat last or zero symbol times */
                lengths[index++] = len;
        }
    }

    /* check for end-of-block code -- there better be one! */
    if (lengths[256] == 0)
        return -9;

    /* build huffman table for literal/length codes */
    err = construct(&lencode, lengths, nlen);
    if (err < 0 || (err > 0 && nlen - lencode.count[0] != 1))
        return -7;      /* only allow incomplete codes if just one code */

    /* build huffman table for distance codes */
    err = construct(&distcode, lengths + nlen, ndist);
    if (err < 0 || (err > 0 && ndist - distcode.count[0] != 1))
        return -8;      /* only allow incomplete codes if just one code */

    /* decode data until end-of-block code */
    {
	int err;
	if (on_the_fly_state) on_the_fly_state = 6;
        oth6:
	err = codes(s, &lencode, &distcode);
	if (on_the_fly_done) return 0;
	if (on_the_fly_state) on_the_fly_state = 5;
	return err;
    }
}

local int inflate()
/* decompress an inflated entry */
{
    struct state s;             /* input/output state */
    static int last, type;      /* block information */
    static int err;             /* return value */

    if (on_the_fly_state >= 4) goto oth4;
 
    s.bitbuf = 0;
    s.bitcnt = 0;

    /* process blocks until last block or error */
    do {
	last = bits(&s, 1);         /* one if last block */
	type = bits(&s, 2);         /* block type 0..3 */
	if (input_error) return -30;

	if (on_the_fly_state) on_the_fly_state = 4;
        oth4:

	err = type == 0 ? stored(&s) :
	    (type == 1 ? fixed(&s) :
	     (type == 2 ? dynamic(&s) :
	      -1));               /* type == 3, invalid */

	if (input_error) return -30;
	if (on_the_fly_done) return 0;
	if (err != 0) break;        /* return with error */
    } while (!last);

    if (err == 0)
	write_flush(ofd);

    return err;
}


/*------------------------- unzip.c --------------------------*/

/* ========================================================================
 * Check the magic number of the input file.
 * Return the compression method, or a negative value in case of error
 * Set inptr to the offset of the next byte to be processed.
 * This function may be called repeatedly for an input file consisting
 * of several contiguous gzip'ed members.
 * IN assertions: there is at least one remaining compressed member.
 *   If the member is a zip file, it must be the only one.
 */
static int get_method(void)
{
    uch flags;     /* compression flags */
    char magic[2]; /* magic header */
    ulg stamp;     /* time stamp */

    magic[0] = (char)get_byte();
    magic[1] = (char)get_byte();
    method = -1; /* unknown yet */

    /* assume multiple members in gzip file except for record oriented I/O */
    if (memcmp(magic, GZIP_MAGIC, 2) == 0
        || memcmp(magic, OLD_GZIP_MAGIC, 2) == 0) {

	method = (int)get_byte();
	if (method != DEFLATED) {
	    msg_print(stderr,
		      "ERROR: unknown method %d -- not supported\n", method);
	    return -17;
	}
	flags  = (uch)get_byte();

	if ((flags & ENCRYPTED) != 0) {
	    msg_print(stderr,
		      "ERROR: input is encrypted -- not supported\n");
	    return -18;
	}
	if ((flags & CONTINUATION) != 0) {
	    msg_print(stderr,
		      "ERROR: input is a multi-part gzip file -- not supported\n");
	    return -19;
	}
	if ((flags & RESERVED) != 0) {
	    msg_print(stderr,
		      "ERROR: input has flags 0x%x -- not supported\n", flags);
	    return -20;
	}
	stamp  = (ulg)get_byte();
	stamp |= ((ulg)get_byte()) << 8;
	stamp |= ((ulg)get_byte()) << 16;
	stamp |= ((ulg)get_byte()) << 24;

	(void)get_byte();  /* Ignore extra flags for the moment */
	(void)get_byte();  /* Ignore OS type for the moment */

	if ((flags & CONTINUATION) != 0) {
	    unsigned part = (unsigned)get_byte();
	    part |= ((unsigned)get_byte())<<8;
	    msg_print(stderr,"WARNING: part number %u\n", part);
	}
	if ((flags & EXTRA_FIELD) != 0) {
	    unsigned len = (unsigned)get_byte();
	    len |= ((unsigned)get_byte())<<8;
	    msg_print(stderr,"WARNING: extra field of %u bytes ignored\n", len);
	    while (len--) (void)get_byte();
	}

	/* Skip original file name */
	if ((flags & ORIG_NAME) != 0) {
		char c;
		do {c=get_byte();} while (c != 0);
	}

	/* Discard file comment if any */
	if ((flags & COMMENT) != 0) {
	    while (get_byte() != 0) /* null */ ;
	}
    }

    if (method >= 0) return method;
    msg_print (stderr, "ERROR: not in gzip format\n");

    return -16;
}

/* ===========================================================================
 * Unzip in to out.  This routine works gzip files.
 *
 * IN assertions: the buffer inbuf contains already the beginning of
 *   the compressed data, from offsets inptr to insize-1 included.
 *   The magic header has already been checked. The output buffer is cleared.
 *
 * Return value
 *   0:  successful inflate
 *  -1:  invalid block type (type == 3)
 *  -2:  stored block length did not match one's complement
 *  -3:  dynamic block code description: too many length or distance codes
 *  -4:  dynamic block code description: code lengths codes incomplete
 *  -5:  dynamic block code description: repeat lengths with no first length
 *  -6:  dynamic block code description: repeat more than specified lengths
 *  -7:  dynamic block code description: invalid literal/length code lengths
 *  -8:  dynamic block code description: invalid distance code lengths
 *  -9:  dynamic block code description: missing end-of-block code
 * -10:  invalid literal/length or distance code in fixed or dynamic block
 * -11:  distance is too far back in fixed or dynamic block
 * -16 : not in gzip format
 * -17 : unknown gzip method
 * -18 : input is encrypted : not supported
 * -19 : input is a multi-part gzip file : not supported
 * -20 : input has flags 0x%x : not supported
 * -21 : bad output crc
 * -22 : bad ouput size
 * -30 : input error (I/O error or not enough data)
 */
static int _unzip(void)
{
    static ulg orig_crc;       /* original crc */
    static ulg orig_len;       /* original uncompressed length */
    static uch buf[8];
    static int n;

    if (on_the_fly_state >= 3) goto oth3;

    orig_crc = 0;
    orig_len = 0;

    updcrc(NULL, 0);           /* initialize crc */

    /* Decompress */
    if (method == DEFLATED)  {
	int res;
	if (on_the_fly_state) on_the_fly_state = 3;
        oth3:
	res = inflate();
	if (input_error || output_error) goto io_error;
	if (res) {
	    msg_print(stderr,"ERROR: invalid compressed data--format violated\n");
	    return -11;
	}
	if (on_the_fly_done) return 0;
    } else {
	msg_print(stderr,"ERROR: internal error, invalid method\n");
	return -12;
    }

    /* crc32  (see algorithm.doc)
     * uncompressed input size modulo 2^32
     */
    for (n = 0; n < 8; n++) {
      buf[n] = (uch)get_byte(); /* may cause an error if EOF */
    }
    orig_crc = LG(buf);
    orig_len = LG(buf+4);

    io_error:
    if (input_error) {
	msg_print(stderr, "ERROR: input I/O error or maximum input size reached\n");
	return -15;
    }
    if (output_error) {
	msg_print(stderr, "ERROR: output I/O error or maximum output size reached\n");
	return -16;
    }

    /* Validate decompression */
    if (can_check_crc) {
	if (orig_crc != updcrc(buf, 0)) {
	    msg_print(stderr, "ERROR: invalid compressed data--crc error\n");
	    return -21;
	}
    } else {
	msg_print(stderr, "WARNING: can't check CRC (output buffer size too low); only length is checked.\n");
    }
    if (orig_len != (ulg)(bytes_out & 0xffffffff)) {
	msg_print(stderr, "ERROR: invalid compressed data--length error\n");
	return -22;
    }

    return 0;
}

static int unzip(void)
{
    if (on_the_fly_state >= 2) goto oth2;
    can_check_crc = 1;
    method = get_method();
    if (method < 0)
	return method;
    if (on_the_fly_state) on_the_fly_state = 2;
    oth2:
    return _unzip();
}

/* for "on the fly" unzip : add some data to unzip */
static int unzip_add_data(char *in_data, int count)
{
    int rc;

    if (input_error || output_error) return -30;
    if (!on_the_fly_state) {
	if (count == 0)
	    return 0; /* end of data at first call -> nothing to do */
	on_the_fly_state = 1;
    }
    if (on_the_fly_eofile) return 0; /* end of data already processed */
    if (count == 0) on_the_fly_eofile = 1; /* end of data */

    on_the_fly_data_size = count;
    on_the_fly_data_ptr = in_data;

    if (!on_the_fly_eofile) {
	/* we have some data to process, add to the input buffer */
	while (on_the_fly_data_size > 0) {
	    /* still have some data to add, add it until buffer full or no more data to add */
	    while (INBUFCOUNT < UNZIP_INBUFSIZ) {
		if (on_the_fly_data_size == 0)
		    return 0; /* all data added */
		uinbuf[insize++ & (UNZIP_INBUFSIZ - 1)] = *on_the_fly_data_ptr;
		on_the_fly_data_ptr++;
		on_the_fly_data_size--;
	    }
	    /* unzip as much as possible */
	    on_the_fly_done = 0;
	    rc = unzip();
	    if (rc) break;
	}
    } else {
	/* no more data to unzip -> process data already in buffer */
	on_the_fly_done = 0;
	rc = unzip();
    }

    return rc;
}

/****************************************************************/

static bool do_sdk_zip_init()
{
	if (sdk_zip_init) {
		return true;
	}

	if(psdk_out_buff) {
		free(psdk_out_buff);
		psdk_out_buff = NULL;
	}
	psdk_out_buff = calloc(DEMO_MEMORY_BUFFER_SIZE, 1);
	if (!psdk_out_buff) {
		return false;
	}

	sdk_zip_init = true;

	return true;
}

static int _sdk_comm_zip(u_int8_t *pin_data, u_int32_t in_data_len, u_int8_t *p_out_data, u_int32_t *pout_data_len)
{
    int infd = FD_MEMORY;
    int outfd = FD_MEMORY;
	int rc = 0;

	rc = zip_init(infd, outfd, pin_data, psdk_out_buff, in_data_len, DEMO_MEMORY_BUFFER_SIZE);
	if (rc != 0) {
		return -2;
	}
	rc = zip();
	if (rc == 0) {
		//memdumpone("psdk_out_buff", psdk_out_buff, bytes_out);
		memcpy(p_out_data, psdk_out_buff, bytes_out);
		*pout_data_len = (u_int32_t)bytes_out;
	} else {
		zip_stop();
	}

	return rc;
}

static int _sdk_comm_unzip(u_int8_t *pin_data, u_int32_t in_data_len, u_int8_t *p_out_data, u_int32_t *pout_data_len)
{
    int infd = FD_MEMORY;
    int outfd = FD_MEMORY;
	int rc = 0;
	
	rc = unzip_init(infd, outfd, pin_data, psdk_out_buff, in_data_len, DEMO_MEMORY_BUFFER_SIZE);
	if (rc != 0) {
		return -2;
	}
	zip_unzip_restart(1);
	rc = unzip();
	if (rc == 0) {
		memcpy(p_out_data, psdk_out_buff, bytes_out);
		*pout_data_len = (u_int32_t)bytes_out;
	} else {
		unzip_stop();
	}

	return rc;
}

int sdk_comm_zip(u_int8_t *pin_data, u_int32_t in_data_len, u_int8_t *p_out_data, u_int32_t *pout_data_len)
{
	
	if (!do_sdk_zip_init()) {
		return -1;
	}
	
	return _sdk_comm_zip(pin_data, in_data_len, p_out_data, pout_data_len);
}

int sdk_comm_unzip(u_int8_t *pin_data, u_int32_t in_data_len, u_int8_t *p_out_data, u_int32_t *pout_data_len)
{
	
	if (!do_sdk_zip_init()) {
		return -1;
	}
	
	return _sdk_comm_unzip(pin_data, in_data_len, p_out_data, pout_data_len);
}

	
