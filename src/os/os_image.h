/*
 * Copyright (c) 2018 Milan Suk
 *
 * Use of this software is governed by the Business Source License included
 * in the LICENSE file and at www.mariadb.com/bsl11.
 *
 * Change Date: 2024-11-01
 *
 * On the date above, in accordance with the Business Source License, use
 * of this software will be governed by version 2 or later of the General
 * Public License.
 */






/*
BOOL Image4_initJpeg(Image4* self, const UCHAR* buffer, const UINT buffer_size)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);
    
	jpeg_mem_src(&cinfo, buffer, buffer_size);

	int rc = jpeg_read_header(&cinfo, TRUE);
	if(rc != 1)
		return FALSE;

	jpeg_start_decompress(&cinfo);


//	int width = cinfo.output_width;
//	int height = cinfo.output_height;
	int pixel_size = cinfo.output_components;

	*self = Image4_initSize(Vec2i_init2(cinfo.output_width, cinfo.output_height));


	UCHAR* buffer_array[1];
	int wbytes = cinfo.output_width*pixel_size;
	buffer_array[0] = calloc(1, wbytes);
	int y = 0;
	while(cinfo.output_scanline < cinfo.output_height)
	{
		jpeg_read_scanlines(&cinfo, buffer_array, 1);
		if(pixel_size==3)
			Image4_put3(self, Vec2i_init2(0, y), buffer_array[0], wbytes);
                y++;

	}
	free(buffer_array[0]);

	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);

	return TRUE;
}







typedef struct Image4Png_s	//help struct
{
	const UCHAR* buffer;
	UBIG buffer_size;
}Image4Png;

void _Image4_readPng(png_structp png, png_bytep data, png_size_t length)
{
	Image4Png* p = png_get_io_ptr(png);
	
	if(length > p->buffer_size)
		length = p->buffer_size;
		
    memcpy(data, p->buffer, length);
    
    p->buffer += length;
    p->buffer_size -= length;
}

BOOL Image4_initPng(Image4* self, const UCHAR* buffer, const UBIG buffer_size)
{
	if(png_sig_cmp(buffer, 0, 8) != 0)
		return FALSE;


	png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
	if(!png)
		return FALSE;

	png_infop info = png_create_info_struct(png);
	if(!info)
	{
		png_destroy_read_struct(&png, 0, 0);
		return FALSE;
	}


	//error handling
	png_bytep* rows = NULL;
	if(setjmp(png_jmpbuf(png)))
	{
		png_destroy_read_struct(&png, &info, 0);
		free(rows);
		Image4_free(self);
		return FALSE;
	}


	//read function
	Image4Png p;
	p.buffer = buffer;
	p.buffer_size = buffer_size;
	png_set_read_fn(png, (png_voidp)&p, &_Image4_readPng);



	//convert

	//must by 4bytes per pixel! ...
	png_read_info(png, info);
	png_uint_32 bitdepth   = png_get_bit_depth(png, info);
	png_uint_32 pixel_size   = png_get_channels(png, info);
	png_uint_32 color_type = png_get_color_type(png, info);
	switch(color_type)
	{
		case PNG_COLOR_TYPE_PALETTE:
			png_set_palette_to_rgb(png);
			pixel_size = 3;           
		break;
		case PNG_COLOR_TYPE_GRAY:
			if(bitdepth < 8)
				png_set_expand_gray_1_2_4_to_8(png);
			bitdepth = 8;
		break;
	}
	if(png_get_valid(png, info, PNG_INFO_tRNS))
	{
		png_set_tRNS_to_alpha(png);
		pixel_size++;
	}
	if(bitdepth == 16)
		png_set_strip_16(png);
	png_read_update_info(png, info);


	//alloc
        const int width = png_get_image_width(png, info);
	const int height = png_get_image_height(png, info);
	*self = Image4_initSize(Vec2i_init2(width, height));


	//read
	UCHAR* buffer_array[1];
	int wbytes = width * pixel_size;
	buffer_array[0] = calloc(1, wbytes);
	int y = 0;
	while(y < height)
	{
		//png_read_rows(png, buffer_array, 0, 1);
                png_read_row(png, buffer_array[0], 0);
		if(pixel_size==3)
			Image4_put3(self, Vec2i_init2(0, y), buffer_array[0], wbytes);
		else
		if(pixel_size==4)
			Image4_put4(self, Vec2i_init2(0, y), buffer_array[0], wbytes);
		//Std_array_print(buffer_array[0], wbytes);
		y++;
	}
	free(buffer_array[0]);


	//clean
	png_read_end(png, info);
	png_destroy_read_struct(&png, &info, 0);
	return TRUE;
}
*/

