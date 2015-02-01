/*
 * libcamsrcjpegenc-sw-libjpeg
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jeongmo Yang <jm80.yang@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* ==============================================================================
|  INCLUDE FILES								|
===============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <jpeglib.h>

#include <camsrcjpegenc_sub.h>
#include <mm_util_imgp.h>

/*-------------------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal					|
-------------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal					|
-------------------------------------------------------------------------------*/
/*
 * Declaration for jpeg codec
 */
#define JPEG_MODULE_NAME        "LIBJPEGSW"     /* A module name. Printing this name, a user of this submodule knows what module he used. */

/* I420 */
#define JPEGENC_ROUND_UP_2(num)  (((num)+1)&~1)
#define JPEGENC_ROUND_UP_4(num)  (((num)+3)&~3)
#define JPEGENC_ROUND_UP_8(num)  (((num)+7)&~7)

#define I420_Y_ROWSTRIDE(width) (JPEGENC_ROUND_UP_4(width))
#define I420_U_ROWSTRIDE(width) (JPEGENC_ROUND_UP_8(width)/2)
#define I420_V_ROWSTRIDE(width) ((JPEGENC_ROUND_UP_8(I420_Y_ROWSTRIDE(width)))/2)

#define I420_Y_OFFSET(w,h) (0)
#define I420_U_OFFSET(w,h) (I420_Y_OFFSET(w,h)+(I420_Y_ROWSTRIDE(w)*JPEGENC_ROUND_UP_2(h)))
#define I420_V_OFFSET(w,h) (I420_U_OFFSET(w,h)+(I420_U_ROWSTRIDE(w)*JPEGENC_ROUND_UP_2(h)/2))

#define I420_SIZE(w,h)     (I420_V_OFFSET(w,h)+(I420_V_ROWSTRIDE(w)*JPEGENC_ROUND_UP_2(h)/2))

/* Limitations */
#define MAX_JPG_WIDTH                           3264
#define MAX_JPG_HEIGHT                          2448
#define MAX_ENCODED_RESULT_SIZE(width, height)  (width * height)

typedef struct _alloc_data alloc_data;
struct _alloc_data {
	guint             size;
	unsigned char    *data;
};


/* To use dlog instead of printf */
#ifdef USE_DLOG
#include <dlog.h>
#endif

enum {
    JPEGENC_LIBJPEGSW_LOG = 0,
    JPEGENC_LIBJPEGSW_INFO,
    JPEGENC_LIBJPEGSW_WARNING,
    JPEGENC_LIBJPEGSW_ERROR,
    JPEGENC_LIBJPEGSW_NUM,
};

enum {
    JPEGENC_LIBJPEGSW_COLOR_LOG = 0,
    JPEGENC_LIBJPEGSW_COLOR_INFO = 32,
    JPEGENC_LIBJPEGSW_COLOR_WARNING = 36,
    JPEGENC_LIBJPEGSW_COLOR_ERROR = 31,
    JPEGENC_LIBJPEGSW_COLOR_NUM = 4,
};


#if defined (USE_DLOG)

#define ___jpegenc_libjpegsw_log(class, msg, args...)   SLOG(class, "CAMSRCJPEGENC", msg, ##args)

#define _jpegenc_libjpegsw_log(msg, args...) {\
    ___jpegenc_libjpegsw_log(LOG_DEBUG, msg, ##args);\
}

#define _jpegenc_libjpegsw_info(msg, args...) {\
    ___jpegenc_libjpegsw_log(LOG_DEBUG, msg, ##args);\
}

#define _jpegenc_libjpegsw_warning(msg, args...) {\
    ___jpegenc_libjpegsw_log(LOG_WARN, msg, ##args);\
}

#define _jpegenc_libjpegsw_error(msg, args...) {\
    ___jpegenc_libjpegsw_log(LOG_ERROR, msg, ##args);\
}

#else /* defined (USE_DLOG) */

#define JPEGENC_LIBJPEGSW_LOG_TMP_BUF_SIZE 256

#define _jpegenc_libjpegsw_log(msg, args...) {\
    char tmp_buf[JPEGENC_LIBJPEGSW_LOG_TMP_BUF_SIZE];\
    snprintf(tmp_buf, JPEGENC_LIBJPEGSW_LOG_TMP_BUF_SIZE-1, "[LOG] [%s:%d] [%s] %s", __FILE__, __LINE__, __func__, (msg));\
    _jpegenc_libjpegsw_print(JPEGENC_LIBJPEGSW_LOG, (tmp_buf), ##args);\
}

#define _jpegenc_libjpegsw_info(msg, args...) {\
    char tmp_buf[JPEGENC_LIBJPEGSW_LOG_TMP_BUF_SIZE];\
    snprintf(tmp_buf, JPEGENC_LIBJPEGSW_LOG_TMP_BUF_SIZE-1, "[INFO] [%s:%d] [%s] %s", __FILE__, __LINE__, __func__, (msg));\
    _jpegenc_libjpegsw_print(JPEGENC_LIBJPEGSW_INFO, (tmp_buf), ##args);\
}

#define _jpegenc_libjpegsw_warning(msg, args...) {\
    char tmp_buf[JPEGENC_LIBJPEGSW_LOG_TMP_BUF_SIZE];\
    snprintf(tmp_buf, JPEGENC_LIBJPEGSW_LOG_TMP_BUF_SIZE-1, "[WARNING] [%s:%d] [%s] %s", __FILE__, __LINE__, __func__, (msg));\
    _jpegenc_libjpegsw_print(JPEGENC_LIBJPEGSW_WARNING, (tmp_buf), ##args);\
}

#define _jpegenc_libjpegsw_error(msg, args...) {\
    char tmp_buf[JPEGENC_LIBJPEGSW_LOG_TMP_BUF_SIZE];\
    snprintf(tmp_buf, JPEGENC_LIBJPEGSW_LOG_TMP_BUF_SIZE-1, "[ERROR] [%s:%d] [%s] %s", __FILE__, __LINE__, __func__, (msg));\
    _jpegenc_libjpegsw_print(JPEGENC_LIBJPEGSW_ERROR, (tmp_buf), ##args);\
}

#endif /* defined (USE_DLOG) */


/*-------------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:							|
-------------------------------------------------------------------------------*/
/* STATIC INTERNAL FUNCTION */
static void _jpegenc_libjpegsw_error_handle(j_common_ptr cinfo);
static void _jpegenc_init_destination (j_compress_ptr cinfo);
static boolean _jpegenc_flush_destination (j_compress_ptr cinfo);
static void _jpegenc_term_destination (j_compress_ptr cinfo);
#ifdef _USE_YUV_TO_RGB888_
static gboolean _jpegenc_convert_YUV_to_RGB888(unsigned char *src, int src_fmt, guint width, guint height, unsigned char **dst, unsigned int *dst_len);
#endif /* _USE_YUV_TO_RGB888_ */
static gboolean _jpegenc_convert_YUYV_to_I420(unsigned char *src, guint width, guint height, unsigned char **dst, unsigned int *dst_len);
static gboolean _jpegenc_convert_UYVY_to_I420(unsigned char *src, guint width, guint height, unsigned char **dst, unsigned int *dst_len);
static gboolean _jpegenc_convert_NV12_to_I420(unsigned char *src, guint width, guint height, unsigned char **dst, unsigned int *dst_len);

#ifndef USE_DLOG
static void _jpegenc_libjpegsw_print(int log_type, char *msg, ...);
#endif /* USE_DLOG */


/*===============================================================================
|  FUNCTION DEFINITIONS								|
===============================================================================*/
/*-------------------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:						|
-------------------------------------------------------------------------------*/
/*-----------------------------------------------
|		CAMSRCJPEGENC SUBMODULE		|
-----------------------------------------------*/
gboolean camsrcjpegencsub_get_info ( jpegenc_internal_info *info )
{
	gboolean bret = FALSE;
	_jpegenc_libjpegsw_info("%s [SUB:%s]\n",__func__, JPEG_MODULE_NAME);

	if (!info)
	{
		_jpegenc_libjpegsw_error("Null pointer[info].\n");
		goto exit;
	}

	/* input variables */
	info->version = 1;                              /* Interface version that current submodule follows */
	info->mem_addr_type = MEMORY_ADDRESS_VIRTUAL;   /* Memory type of input buffer */
	info->input_fmt_list[0] = COLOR_FORMAT_I420;    /* A color format list that jpeg encoder can retreive */
	info->input_fmt_list[1] = COLOR_FORMAT_YUYV;
	info->input_fmt_list[2] = COLOR_FORMAT_UYVY;
	info->input_fmt_list[3] = COLOR_FORMAT_NV12;
	info->input_fmt_list[4] = COLOR_FORMAT_RGB;
	info->input_fmt_list[5] = COLOR_FORMAT_RGBA;
	info->input_fmt_list[6] = COLOR_FORMAT_BGRA;
	info->input_fmt_num = 7;                        /* Total number of color format that the encoder supports */
	info->input_fmt_recommend = COLOR_FORMAT_I420;  /* Recommended color format */
	info->progressive_mode_support = TRUE;          /* Whether the encoder supports progressive encoding */

	bret = TRUE;
exit:
	_jpegenc_libjpegsw_info("%s leave\n",__func__);
	return bret;
}


int  camsrcjpegencsub_encode ( jpegenc_parameter *enc_param )
{
	/* video state */
	unsigned int width, height;
	int bufsize = 0;
	alloc_data ad = {0, NULL};

	/* the jpeg line buffer */
	JSAMPARRAY raw_data[3];
	JSAMPROW data_y[DCTSIZE*2];
	JSAMPROW data_cb[DCTSIZE];
	JSAMPROW data_cr[DCTSIZE];
	unsigned int i = 0;
	int j = 0;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_destination_mgr jdest;

	/* properties */
	//gint jpeg_mode;

	/* general */
	gboolean bret = TRUE;
	int src_fmt = COLOR_FORMAT_NOT_SUPPORT;
	unsigned int src_len = 0;
	unsigned char *src_data = NULL;

	/* Check supported color format and do converting if needed */
	switch (enc_param->src_fmt) {
	case COLOR_FORMAT_YUYV:
		bret = _jpegenc_convert_YUYV_to_I420(enc_param->src_data,
		                                     enc_param->width, enc_param->height,
		                                     &src_data, &src_len);
		src_fmt = COLOR_FORMAT_I420;
		break;
	case COLOR_FORMAT_UYVY:
		bret = _jpegenc_convert_UYVY_to_I420(enc_param->src_data,
		                                     enc_param->width, enc_param->height,
		                                     &src_data, &src_len);
		src_fmt = COLOR_FORMAT_I420;
		break;
	case COLOR_FORMAT_NV12:
		bret = _jpegenc_convert_NV12_to_I420(enc_param->src_data,
		                                     enc_param->width, enc_param->height,
		                                     &src_data, &src_len);
		src_fmt = COLOR_FORMAT_I420;
		break;
	case COLOR_FORMAT_I420:
	case COLOR_FORMAT_RGB:
	case COLOR_FORMAT_RGBA:
	case COLOR_FORMAT_BGRA:
		_jpegenc_libjpegsw_info("Do JPEG encode without color converting...");
		src_fmt = enc_param->src_fmt;
		src_data = enc_param->src_data;
		break;
	default:
		_jpegenc_libjpegsw_error("NOT Supported format [%d]", enc_param->src_fmt);
		return FALSE;
	}

	if (bret == FALSE) {
		enc_param->result_data = NULL;
		if (src_data &&
		    src_data != enc_param->src_data) {
			free(src_data);
			src_data = NULL;
		}

		return FALSE;
	}

	_jpegenc_libjpegsw_info("%s [SUB:%s]\n",__func__, JPEG_MODULE_NAME);
	memset (&cinfo, 0, sizeof (struct jpeg_compress_struct));
	memset (&jerr, 0, sizeof (struct jpeg_error_mgr));
	memset (&jdest, 0, sizeof (struct jpeg_destination_mgr));

	jerr.reset_error_mgr = _jpegenc_libjpegsw_error_handle;
	cinfo.err = jpeg_std_error (&jerr);
	jpeg_create_compress (&cinfo);

	jdest.init_destination = _jpegenc_init_destination;
	jdest.empty_output_buffer = _jpegenc_flush_destination;
	jdest.term_destination = _jpegenc_term_destination;
	cinfo.dest = &jdest;

	cinfo.image_width = width = enc_param->width;
	cinfo.image_height = height = enc_param->height;
	cinfo.client_data = &ad;

	_jpegenc_libjpegsw_info ("width %d, height %d, format %d", width, height, src_fmt);

	switch (src_fmt) {
		case COLOR_FORMAT_I420:
			_jpegenc_libjpegsw_info ("setting format to I420");

			bufsize = I420_SIZE (width, height);
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_YCbCr;

			jpeg_set_defaults (&cinfo);

			cinfo.raw_data_in = TRUE;
			cinfo.do_fancy_downsampling = FALSE;

			raw_data[0] = data_y;
			raw_data[1] = data_cb;
			raw_data[2] = data_cr;
		break;
		case COLOR_FORMAT_RGBA:
			bufsize = width * height * 4;
			_jpegenc_libjpegsw_info ("setting format to RGBA");
			cinfo.input_components = 4;
			cinfo.in_color_space = JCS_EXT_RGBA;

			jpeg_set_defaults (&cinfo);

			cinfo.raw_data_in = FALSE;
		break;
		case COLOR_FORMAT_BGRA:
			bufsize = width * height * 4;
			_jpegenc_libjpegsw_info ("setting format to BGRA");
			cinfo.input_components = 4;
			cinfo.in_color_space = JCS_EXT_BGRA;

			jpeg_set_defaults (&cinfo);

			cinfo.raw_data_in = FALSE;
		break;
		case COLOR_FORMAT_RGB:
		default:
			/* Wrong value. So set RGB for default. */
			bufsize = width * height * 3;
			_jpegenc_libjpegsw_info ("setting format to RGB24");
			cinfo.input_components = 3;
			cinfo.in_color_space = JCS_RGB;

			jpeg_set_defaults (&cinfo);

			cinfo.raw_data_in = FALSE;
		break;
	}

	jpeg_suppress_tables (&cinfo, TRUE);

	/* Set remaining settings. */
	if (enc_param->jpeg_mode == JPEG_MODE_PROGRESSIVE)
		cinfo.progressive_mode = TRUE;
	else
		cinfo.progressive_mode = FALSE;
	cinfo.smoothing_factor = 0;
	cinfo.dct_method = JDCT_FASTEST;

	jpeg_set_quality (&cinfo, enc_param->jpeg_quality, TRUE);

	_jpegenc_libjpegsw_info ("Set quality - %d", enc_param->jpeg_quality);

	ad.size = MAX_ENCODED_RESULT_SIZE(width, height);
	ad.data = malloc(ad.size);
	if (ad.data == NULL) {
		enc_param->result_data = NULL;
		if (src_data &&
		    src_data != enc_param->src_data) {
			free(src_data);
			src_data = NULL;
		}

		return FALSE;
	}

	jdest.next_output_byte = ad.data;
	jdest.free_in_buffer = ad.size;

	_jpegenc_libjpegsw_info("compress start(%p, %d, %p, %d)",
	                        src_data, src_len, jdest.next_output_byte, jdest.free_in_buffer);
	jpeg_start_compress(&cinfo, TRUE);

	_jpegenc_libjpegsw_info("got src buffer of %lu bytes", src_len);
	_jpegenc_libjpegsw_info("allocated result buffer size %d bytes", ad.size);

	switch (src_fmt) {
		case COLOR_FORMAT_I420:
		{
			int check_num = 0;
			int size_y = 0;
			int size_u = 0;
			int size_v = 0;
			int offset_y = 0;
			int offset_uv = 0;
			unsigned char *base_y = NULL;
			unsigned char *base_u = NULL;
			unsigned char *base_v = NULL;
			unsigned char *temp_y = NULL;
			unsigned char *temp_u = NULL;
			unsigned char *temp_v = NULL;

			size_y = width * height;
			size_u = size_y >> 2;
			size_v = size_u;

			base_y = src_data;
			base_u = base_y + size_y;
			base_v = base_u + size_u;

			check_num = DCTSIZE << 1;

			/* prepare for raw input */
			_jpegenc_libjpegsw_info ("compressing I420");

			for (i = 0 ; i < height ; i += check_num) {
				for (j = 0 ; j < check_num ; j++) {
					offset_y = width * (i + j);
					if (offset_y < size_y) {
						temp_y = base_y + offset_y;
					}
					if (temp_y) {
						data_y[j] = (JSAMPROW)temp_y;
					}
					if (j % 2 == 0) {
						offset_uv = offset_y >> 2;
						if (offset_uv < size_u) {
							temp_u = base_u + offset_uv;
							temp_v = base_v + offset_uv;
						}
						if (temp_u) {
							data_cb[j>>1] = (JSAMPROW)temp_u;
						}
						if (temp_v) {
							data_cr[j>>1] = (JSAMPROW)temp_v;
						}
					}
				}
				jpeg_write_raw_data (&cinfo, raw_data, check_num);
			}

			_jpegenc_libjpegsw_info ("set data done");
		}
		break;
		case COLOR_FORMAT_RGBA:
		case COLOR_FORMAT_BGRA:
		{
			JSAMPROW *jbuf = NULL;
			unsigned char *buf = src_data;

			_jpegenc_libjpegsw_info ("compressing RGB");

			while (cinfo.next_scanline < cinfo.image_height)
			{
				jbuf = (JSAMPROW *) (&buf);
				jpeg_write_scanlines(&cinfo, (JSAMPROW *)jbuf, 1);
				buf += width * 4;
			}
		}
		break;
		case COLOR_FORMAT_RGB:
		default:
		{
			/* Wrong value. So set RGB for default. */
			JSAMPROW *jbuf = NULL;
			unsigned char *buf = src_data;

			_jpegenc_libjpegsw_info ("compressing RGB");

			while (cinfo.next_scanline < cinfo.image_height)
			{
				jbuf = (JSAMPROW *) (&buf);
				jpeg_write_scanlines(&cinfo, (JSAMPROW *)jbuf, 1);
				buf += width * 3;
			}
		}
		break;
	}

	jpeg_finish_compress (&cinfo);

	enc_param->result_len = ad.size - jdest.free_in_buffer;

	jpeg_destroy_compress(&cinfo);

	if (src_data &&
	    src_data != enc_param->src_data) {
		free(src_data);
		src_data = NULL;
	}

	enc_param->result_data = ad.data;

	_jpegenc_libjpegsw_info("compressing done,the result length is: %d",enc_param->result_len);

	bret = TRUE;

	return bret;
}


/*-------------------------------------------------------------------------------
|    LOCAL FUNCTION DEFINITIONS:						|
-------------------------------------------------------------------------------*/
/* Core functions */
static void
_jpegenc_libjpegsw_error_handle(j_common_ptr cinfo)
{
   _jpegenc_libjpegsw_error("Libjpeg error!!!");

   return;
}


static void
_jpegenc_init_destination (j_compress_ptr cinfo)
{
  _jpegenc_libjpegsw_warning("init_destination");
}


static boolean
_jpegenc_flush_destination (j_compress_ptr cinfo)
{
  alloc_data *ad = (alloc_data *)cinfo->client_data;
  unsigned char *new_data = NULL;
  int new_size = 0;

  if (ad && ad->data) {
    _jpegenc_libjpegsw_warning("jpeg output buffer too small! - size %d", ad->size);

    new_size = ad->size * 2;
    new_data = (unsigned char *)malloc(new_size);
    if (new_data) {
      memcpy(new_data, ad->data, ad->size);
      free(ad->data);
      ad->data = new_data;
      cinfo->dest->next_output_byte = new_data + ad->size;
      cinfo->dest->free_in_buffer = new_size - ad->size;
      ad->size = new_size;
      _jpegenc_libjpegsw_warning("new data is allocated - %p, size %d",
                                 ad->data, ad->size);
    } else {
      _jpegenc_libjpegsw_warning("failed to alloc new_data. size %d", new_size);
    }
  } else {
    _jpegenc_libjpegsw_warning("client data is NULL");
    return FALSE;
  }

  return TRUE;
}


static void
_jpegenc_term_destination (j_compress_ptr cinfo)
{
  _jpegenc_libjpegsw_warning("term_source");
}


#ifdef _USE_YUV_TO_RGB888_
static gboolean
_jpegenc_convert_YUV_to_RGB888(unsigned char *src, int src_fmt, guint width, guint height, unsigned char **dst, unsigned int *dst_len)
{
	int ret = 0;
	int src_cs = MM_UTIL_IMG_FMT_UYVY;
	int dst_cs = MM_UTIL_IMG_FMT_RGB888;
	unsigned int dst_size = 0;

	if( src_fmt == COLOR_FORMAT_YUYV )
	{
		_jpegenc_libjpegsw_info( "Convert YUYV to RGB888\n" );
		src_cs = MM_UTIL_IMG_FMT_YUYV;
	}
	else if( src_fmt == COLOR_FORMAT_UYVY )
	{
		_jpegenc_libjpegsw_info( "Convert UYVY to RGB888\n" );
		src_cs = MM_UTIL_IMG_FMT_UYVY;
	}
	else if( src_fmt == COLOR_FORMAT_NV12 )
	{
		_jpegenc_libjpegsw_info( "Convert NV12 to RGB888\n" );
		src_cs = MM_UTIL_IMG_FMT_NV12;
	}
	else
	{
		_jpegenc_libjpegsw_error( "NOT supported format [%d]\n", src_fmt );
		return FALSE;
	}

	ret = mm_util_get_image_size( dst_cs, width, height, &dst_size );
	if (ret != 0) {
		_jpegenc_libjpegsw_error("mm_util_get_image_size failed [%x]\n", ret);
		return FALSE;
	}

	*dst = malloc( dst_size );
	if( *dst == NULL )
	{
		_jpegenc_libjpegsw_error( "malloc failed\n" );
		return FALSE;
	}

	*dst_len = dst_size;
	ret = mm_util_convert_colorspace( src, width, height, src_cs, *dst, dst_cs );
	if( ret == 0 )
	{
		_jpegenc_libjpegsw_info( "Convert [dst_size:%d] OK.\n", dst_size );
		return TRUE;
	}
	else
	{
		free( *dst );
		*dst = NULL;

		_jpegenc_libjpegsw_error( "Convert [size:%d] FAILED.\n", dst_size );
		return FALSE;
	}
}
#endif /* _USE_YUV_TO_RGB888_ */


static gboolean _jpegenc_convert_YUYV_to_I420(unsigned char *src, guint width, guint height, unsigned char **dst, unsigned int *dst_len)
{
	unsigned int i = 0;
	int j = 0;
	int src_offset = 0;
	int dst_y_offset = 0;
	int dst_u_offset = 0;
	int dst_v_offset = 0;
	int loop_length = 0;
	unsigned int dst_size = 0;
	unsigned char *dst_data = NULL;

	if (!src || !dst || !dst_len) {
		_jpegenc_libjpegsw_error("NULL pointer %p, %p, %p", src, dst, dst_len);
		return FALSE;
	}

	dst_size = (width * height * 3) >> 1;

	_jpegenc_libjpegsw_info("YUVY -> I420 : %dx%d, dst size %d", width, height, dst_size);

	dst_data = (unsigned char *)malloc(dst_size);
	if (!dst_data) {
		_jpegenc_libjpegsw_error("failed to alloc dst_data. size %d", dst_size);
		return FALSE;
	}

	loop_length = width << 1;
	dst_u_offset = width * height;
	dst_v_offset = dst_u_offset + (dst_u_offset >> 2);

	_jpegenc_libjpegsw_info("offset y %d, u %d, v %d", dst_y_offset, dst_u_offset, dst_v_offset);

	for (i = 0 ; i < height ; i++) {
		for (j = 0 ; j < loop_length ; j += 2) {
			dst_data[dst_y_offset++] = src[src_offset++]; /*Y*/

			if (i % 2 == 0) {
				if (j % 4 == 0) {
					dst_data[dst_u_offset++] = src[src_offset++]; /*U*/
				} else {
					dst_data[dst_v_offset++] = src[src_offset++]; /*V*/
				}
			} else {
				src_offset++;
			}
		}
	}

	*dst = dst_data;
	*dst_len = dst_size;

	_jpegenc_libjpegsw_info("DONE: YUVY -> I420 : %dx%d, dst data %p, size %d",
	                        width, height, *dst, dst_size);

	return TRUE;
}


static gboolean _jpegenc_convert_UYVY_to_I420(unsigned char *src, guint width, guint height, unsigned char **dst, unsigned int *dst_len)
{
	unsigned int i = 0;
	int j = 0;
	int src_offset = 0;
	int dst_y_offset = 0;
	int dst_u_offset = 0;
	int dst_v_offset = 0;
	int loop_length = 0;
	unsigned int dst_size = 0;
	unsigned char *dst_data = NULL;

	if (!src || !dst || !dst_len) {
		_jpegenc_libjpegsw_error("NULL pointer %p, %p, %p", src, dst, dst_len);
		return FALSE;
	}

	dst_size = (width * height * 3) >> 1;

	_jpegenc_libjpegsw_info("UYVY -> I420 : %dx%d, dst size %d", width, height, dst_size);

	dst_data = (unsigned char *)malloc(dst_size);
	if (!dst_data) {
		_jpegenc_libjpegsw_error("failed to alloc dst_data. size %d", dst_size);
		return FALSE;
	}

	loop_length = width << 1;
	dst_u_offset = width * height;
	dst_v_offset = dst_u_offset + (dst_u_offset >> 2);

	_jpegenc_libjpegsw_info("offset y %d, u %d, v %d", dst_y_offset, dst_u_offset, dst_v_offset);

	for (i = 0 ; i < height ; i++) {
		for (j = 0 ; j < loop_length ; j += 2) {
			if (i % 2 == 0) {
				if (j % 4 == 0) {
					dst_data[dst_u_offset++] = src[src_offset++]; /*U*/
				} else {
					dst_data[dst_v_offset++] = src[src_offset++]; /*V*/
				}
			} else {
				src_offset++;
			}

			dst_data[dst_y_offset++] = src[src_offset++]; /*Y*/
		}
	}

	*dst = dst_data;
	*dst_len = dst_size;

	_jpegenc_libjpegsw_info("DONE: UYVY -> I420 : %dx%d, dst data %p, size %d",
	                        width, height, *dst, dst_size);

	return TRUE;
}


static gboolean _jpegenc_convert_NV12_to_I420(unsigned char *src, guint width, guint height, unsigned char **dst, unsigned int *dst_len)
{
	int i = 0;
	int src_offset = 0;
	int dst_y_offset = 0;
	int dst_u_offset = 0;
	int dst_v_offset = 0;
	int loop_length = 0;
	unsigned int dst_size = 0;
	unsigned char *dst_data = NULL;

	if (!src || !dst || !dst_len) {
		_jpegenc_libjpegsw_error("NULL pointer %p, %p, %p", src, dst, dst_len);
		return FALSE;
	}

	dst_size = (width * height * 3) >> 1;

	_jpegenc_libjpegsw_info("NV12 -> I420 : %dx%d, dst size %d", width, height, dst_size);

	dst_data = (unsigned char *)malloc(dst_size);
	if (!dst_data) {
		_jpegenc_libjpegsw_error("failed to alloc dst_data. size %d", dst_size);
		return FALSE;
	}

	loop_length = width << 1;
	dst_u_offset = width * height;
	dst_v_offset = dst_u_offset + (dst_u_offset >> 2);

	_jpegenc_libjpegsw_info("offset y %d, u %d, v %d", dst_y_offset, dst_u_offset, dst_v_offset);

	/* memcpy Y */
	memcpy(dst_data, src, dst_u_offset);

	loop_length = dst_u_offset >> 1;
	src_offset = dst_u_offset;

	/* set U and V */
	for (i = 0 ; i < loop_length ; i++) {
		if (i % 2 == 0) {
			dst_data[dst_u_offset++] = src[src_offset++];
		} else {
			dst_data[dst_v_offset++] = src[src_offset++];
		}
	}

	*dst = dst_data;
	*dst_len = dst_size;

	_jpegenc_libjpegsw_info("DONE: NV12 -> I420 : %dx%d, dst data %p, size %d",
	                        width, height, *dst, dst_size);

	return TRUE;
}


#ifndef USE_DLOG
/* Functions for printf log */
#define BUF_LEN 256
static void _jpegenc_libjpegsw_print(int log_type, char *msg, ...)
{
	int log_color = 0;
	char work_buf1[BUF_LEN+1];
	va_list arg;

	va_start(arg, msg);
	vsnprintf(work_buf1, BUF_LEN-100, msg, arg);
	va_end(arg);

	switch (log_type) {
	case JPEGENC_LIBJPEGSW_LOG:
		log_color = JPEGENC_LIBJPEGSW_COLOR_LOG;
		break;
	case JPEGENC_LIBJPEGSW_INFO:
		log_color = JPEGENC_LIBJPEGSW_COLOR_INFO;
		break;
	case JPEGENC_LIBJPEGSW_WARNING:
		log_color = JPEGENC_LIBJPEGSW_COLOR_WARNING;
		break;
	case JPEGENC_LIBJPEGSW_ERROR:
		log_color = JPEGENC_LIBJPEGSW_COLOR_ERROR;
		break;
	default:
		break;
	}

	printf("\033[%dm%s\033[0m", log_color, work_buf1);

	return;
}
#endif /* USE_DLOG */
