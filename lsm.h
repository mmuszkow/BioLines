/*
* BioLines (https://github.com/mmuszkow/BioLines)
* Detection of filamentous structures in biological microscopic images
* Copyright(C) 2017 Maciek Muszkowski
*
* This program is free software : you can redistribute it and / or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.

* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
* GNU General Public License for more details.

* You should have received a copy of the GNU General Public License
* along with this program.If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __LSM_H__
#define __LSM_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "lzw.h"

#define TIFF_VERSION 42

struct tiff_header {
	/* II - little endian, MM - big endian. */
	uint8_t endianess[2];
	/* Always 42 */
	uint16_t version;
	/* Offset of the first IFD. */
	uint16_t first_ifd_offset;
};

#define LSM_CHANNEL_COLORS_READ_SIZE (sizeof(struct lsm_channel_names_colors) - sizeof(char**) - sizeof(uint32_t*))

struct lsm_channel_names_colors {
	uint32_t block_size;
	uint32_t number_colors;
	uint32_t number_names;
	uint32_t colors_offset;
	uint32_t names_offset;
	uint32_t mono;

	char**    names;
	uint32_t* colors;
};

#define LSM_CODE    0x494C
#define LSM_VERSION 0x0400

#define LSM_INFO_STRUCT_READ_SIZE (sizeof(struct lsm_info_v4) - sizeof(struct lsm_channel_names_colors) - sizeof(uint32_t*))

struct lsm_info_v4 {
	/* 0x494C */
	uint16_t code;
	/* Version (4). */
	uint16_t version;
	/* This structure size. */
	uint32_t length;
	uint32_t dimension_x;
	uint32_t dimension_y;
	uint32_t dimension_z;
	uint32_t dimension_channels;
	uint32_t dimension_time;
	uint32_t intensity_data_type;
	uint32_t thumbnail_x;
	uint32_t thumbnail_y;
	double pixel_size_x;
	double pixel_size_y;
	double pixel_size_z;
	double origin_x;
	double origin_y;
	double origin_z;
	uint16_t scan_type; // +88
	uint16_t spectral_scan;
	uint32_t datatype2;
	uint32_t vector_overlay_offset;
	uint32_t input_lut_overlay_offset;
	uint32_t output_lut_overlay_offset;
	uint32_t channel_colors_offset; // +108 offset_channel_names_colors
	double time_interval;
	uint8_t reserved[4];
	uint32_t scan_information_offset; // + 120 offset_channel_datatypes
	uint32_t application_tag_offset;
	uint32_t timestamp_offset;
	uint32_t event_list_offset;
	uint32_t roi_overlay_offset;
	uint32_t bleach_roi_overlay_offset;
	uint8_t reserved2[4];
	double display_aspect_x;
	double display_aspect_y;
	double display_aspect_z;
	double display_aspect_time;
	uint32_t mean_of_rois_overlay_offset;
	uint32_t topo_isoline_overlay_offset;
	uint32_t topo_profile_overlay_offset;
	uint32_t linescan_overlay_offset;
	uint32_t toolbar_flags;
	uint32_t channel_wavelength_offset;
	uint8_t reserved4[56];
	uint32_t dimension_p; // + 264
	uint32_t dimension_m;
	uint32_t rotations;
	uint32_t phases;
	uint32_t illuminations;
	uint8_t reserved5[52];
	uint32_t tile_position_offset;
	uint8_t reserved6[36];
	uint32_t position_offset;

	struct lsm_channel_names_colors colors;
	uint32_t* scan_information;
};

#define TIFF_TAG_NEW_SUBFILE_TYPE  0x00FE
#define TIFF_TAG_IMAGE_WIDTH       0x0100
#define TIFF_TAG_IMAGE_LENGTH      0x0101
#define TIFF_TAG_BITS_PER_SAMPLE   0x0102
#define TIFF_TAG_COMPRESSION       0x0103
#define TIFF_TAG_PHOTOMETRIC_INTERPRETATION 0x0106
#define TIFF_TAG_STRIP_OFFSETS     0x0111
#define TIFF_TAG_SAMPLES_PER_PIXEL 0x0115
#define TIFF_TAG_STRIP_BYTE_COUNTS 0x0117
#define TIFF_TAG_PLANAR_CONF       0x011C
#define TIFF_TAG_PREDICTOR         0x013D
#define TIFF_TAG_LSM_INFO_OFFSET   0x866C

#define TIFF_COMPRESSION_NONE      1
#define TIFF_COMPRESSION_LZW       5

/* Bit 0 is 1 if the image is a reduced-resolution version of another image */
#define TIFF_FILETYPE_REDUCEDIMAGE_MASK 1

/* Image File Directory (IFD), only tags that appear in LSM. */
struct tiff_ifd {
	/* 0x00FE - A general indication of the kind of data contained in this subfile. */
	uint32_t tag_new_subfile_type;
	/* 0x0100 - The number of columns in the image, i.e., the number of pixels per row. */
	uint32_t tag_image_width;
	/* 0x0101 - The number of rows of pixels in the image. */
	uint32_t tag_image_length;
	/* 0x0102 - Number of bits per component. */
	uint32_t tag_bits_per_sample_length;
	uint16_t* tag_bits_per_sample;
	/* 0x0103 - Compression scheme used on the image data. */
	uint8_t tag_compression;
	/* 0x0106 - The color space of the image data. */
	uint16_t tag_photometric_interpretation;
	/* 0x0111 - For each strip, the byte offset of that strip. */
	uint32_t tag_strip_offsets_length;
	uint32_t* tag_strip_offsets;
	/* 0x0115 - The number of components per pixel. */
	uint16_t tag_samples_per_pixel;
	/* 0x0117 - For each strip, the number of bytes in the strip after compression. */
	uint32_t tag_strip_byte_counts_length;
	uint32_t* tag_strip_byte_counts;
	/* 0x011C - How the components of each pixel are stored. */
	uint16_t tag_planar_configuration;
	/* 0x013D - A mathematical operator that is applied to the image data before an encoding scheme is applied. */
	uint16_t tag_predictor;
	/* 0x866C - Offset fro LSM specific information. */
	struct lsm_info_v4 tag_lsm_info;
};

/* Image File Directory (IFD) field. */
struct tiff_ifd_field {
	/* Tag (yeah). */
	uint16_t tag;
	/* Field data type: 1 - byte, 2 - ASCII, 3 - word, 4 - dword, uword, 5 - rational. */
	uint16_t type;
	/* Length of the field in units of the data type. */
	uint32_t length;
	/* Data offset of the field. */
	uint32_t offset;
};

struct lsm_file {
	struct tiff_ifd* ifd;
	int ifd_length;
};



bool lsm_read_lsm_info(FILE* f, struct tiff_ifd* ifd, struct tiff_ifd_field* field) {

	if (field->length < LSM_INFO_STRUCT_READ_SIZE)
		return false;
	if (fseek(f, field->offset, SEEK_SET) != 0)
		return false;
	if (fread(&ifd->tag_lsm_info, LSM_INFO_STRUCT_READ_SIZE, 1, f) != 1)
		return false;
	if (ifd->tag_lsm_info.code != LSM_CODE)
		return false;
	if (ifd->tag_lsm_info.version != LSM_VERSION)
		return false;
	if (ifd->tag_lsm_info.length != field->length)
		return false;

	uint32_t channels = ifd->tag_lsm_info.dimension_channels;
	if (ifd->tag_lsm_info.scan_information_offset != 0) {

		if (fseek(f, ifd->tag_lsm_info.scan_information_offset, SEEK_SET) != 0)
			return false;

		ifd->tag_lsm_info.scan_information = (uint32_t*)malloc(4 * channels);
		if (!ifd->tag_lsm_info.scan_information)
			return false;

		if (fread(ifd->tag_lsm_info.scan_information, 4, channels, f) != channels)
			return false;
	}

	if (ifd->tag_lsm_info.channel_colors_offset != 0) {

		if (fseek(f, ifd->tag_lsm_info.channel_colors_offset, SEEK_SET) != 0)
			return false;
		if (fread(&ifd->tag_lsm_info.colors, LSM_CHANNEL_COLORS_READ_SIZE, 1, f) != 1)
			return false;

		// colors
		if (fseek(f, ifd->tag_lsm_info.channel_colors_offset + ifd->tag_lsm_info.colors.colors_offset, SEEK_SET) != 0)
			return false;
		ifd->tag_lsm_info.colors.colors = (uint32_t*)malloc(4 * ifd->tag_lsm_info.colors.number_colors);
		if (!ifd->tag_lsm_info.colors.colors)
			return false;
		if (fread(ifd->tag_lsm_info.colors.colors, 4, ifd->tag_lsm_info.colors.number_colors, f) != ifd->tag_lsm_info.colors.number_colors)
			return false;

		// names
		if (fseek(f, ifd->tag_lsm_info.channel_colors_offset + ifd->tag_lsm_info.colors.names_offset, SEEK_SET) != 0)
			return false;
		ifd->tag_lsm_info.colors.names = (char**)malloc(sizeof(char*) * ifd->tag_lsm_info.colors.number_names);
		if (!ifd->tag_lsm_info.colors.names)
			return false;
		for (int i = 0; i<ifd->tag_lsm_info.colors.number_names; i++)
			ifd->tag_lsm_info.colors.names[i] = NULL;
		for (int i = 0; i<ifd->tag_lsm_info.colors.number_names; i++) {
			uint32_t str_len;
			if (fread(&str_len, 4, 1, f) != 1)
				return false;
			if (str_len == 0)
				continue;
			if (str_len > 1024) // verify the length
				return false;
			ifd->tag_lsm_info.colors.names[i] = (char*)malloc(str_len + 1);
			if (!ifd->tag_lsm_info.colors.names[i])
				return false;
			if (fread(ifd->tag_lsm_info.colors.names[i], 1, str_len, f) != str_len)
				return false;
			ifd->tag_lsm_info.colors.names[i][str_len] = 0;
		}

	}

	return true;
}

void lsm_free(struct lsm_file* lsm) {
	if (lsm->ifd) {
		for (int ifd_idx = 0; ifd_idx < lsm->ifd_length; ifd_idx++) {
			if (lsm->ifd[ifd_idx].tag_bits_per_sample) free(lsm->ifd[ifd_idx].tag_bits_per_sample);
			if (lsm->ifd[ifd_idx].tag_strip_offsets) free(lsm->ifd[ifd_idx].tag_strip_offsets);
			if (lsm->ifd[ifd_idx].tag_strip_byte_counts) free(lsm->ifd[ifd_idx].tag_strip_byte_counts);
			if (lsm->ifd[ifd_idx].tag_lsm_info.scan_information) free(lsm->ifd[ifd_idx].tag_lsm_info.scan_information);
			if (lsm->ifd[ifd_idx].tag_lsm_info.colors.colors) free(lsm->ifd[ifd_idx].tag_lsm_info.colors.colors);
			if (lsm->ifd[ifd_idx].tag_lsm_info.colors.names) {
				for (int name = 0; name < lsm->ifd[ifd_idx].tag_lsm_info.colors.number_names; name++)
					if (lsm->ifd[ifd_idx].tag_lsm_info.colors.names[name]) free(lsm->ifd[ifd_idx].tag_lsm_info.colors.names[name]);
				free(lsm->ifd[ifd_idx].tag_lsm_info.colors.names);
			}
		}
		free(lsm->ifd);
	}
	free(lsm);
}

struct lsm_file* lsm_open(FILE* f) {

	if (!f)
		return NULL;

	struct lsm_file* lsm = (struct lsm_file*) calloc(1, sizeof(struct lsm_file));
	if (!lsm)
		return NULL;

	// read and parse TIFF header
	struct tiff_header header;
	if (fseek(f, 0, SEEK_SET) != 0) {
		lsm_free(lsm);
		return NULL;
	}
	if (fread(&header, sizeof(struct tiff_header), 1, f) != 1) {
		lsm_free(lsm);
		return NULL;
	}
	// LSM uses Intel-byte-order ONLY
	if (header.endianess[0] != 'I' || header.endianess[1] != 'I') {
		lsm_free(lsm);
		return NULL;
	}
	if (header.version != TIFF_VERSION) {
		lsm_free(lsm);
		return NULL;
	}

	// read IFDs count
	uint16_t offset = header.first_ifd_offset;
	lsm->ifd_length = 0;
	while (offset != 0) {
		uint16_t ifd_fields_count;
		lsm->ifd_length++;
		if (fseek(f, offset, SEEK_SET) != 0) {
			lsm_free(lsm);
			return NULL;
		}
		if (fread(&ifd_fields_count, 2, 1, f) != 1) {
			lsm_free(lsm);
			return NULL;
		}
		if (fseek(f, offset + 2 + ifd_fields_count * 12, SEEK_SET) != 0) {
			lsm_free(lsm);
			return NULL;
		}
		if (fread(&offset, 2, 1, f) != 1) {
			lsm_free(lsm);
			return NULL;
		}
	}

	// initialize IFDs
	lsm->ifd = (struct tiff_ifd*) calloc(sizeof(struct tiff_ifd), lsm->ifd_length);
	if (!lsm->ifd) {
		lsm_free(lsm);
		return NULL;
	}

	// read IFDs
	offset = header.first_ifd_offset;
	int ifd_idx = 0;
	while (offset != 0) {
		uint16_t ifd_fields_count;
		struct tiff_ifd_field field;
		if (fseek(f, offset, SEEK_SET) != 0) {
			lsm_free(lsm);
			return NULL;
		}
		if (fread(&ifd_fields_count, 2, 1, f) != 1) {
			lsm_free(lsm);
			return NULL;
		}
		for (int field_idx = 0; field_idx < ifd_fields_count; field_idx++) {
			if (fseek(f, offset + 2 + field_idx * 12, SEEK_SET) != 0) {
				lsm_free(lsm);
				return NULL;
			}
			if (fread(&field, sizeof(struct tiff_ifd_field), 1, f) != 1) {
				lsm_free(lsm);
				return NULL;
			}

			// parse field
			switch (field.tag) {
			case TIFF_TAG_NEW_SUBFILE_TYPE:
				lsm->ifd[ifd_idx].tag_new_subfile_type = field.offset;
				break;
			case TIFF_TAG_IMAGE_WIDTH:
				lsm->ifd[ifd_idx].tag_image_width = field.offset;
				break;
			case TIFF_TAG_IMAGE_LENGTH:
				lsm->ifd[ifd_idx].tag_image_length = field.offset;
				break;
			case TIFF_TAG_BITS_PER_SAMPLE:
				lsm->ifd[ifd_idx].tag_bits_per_sample_length = field.length;
				if (field.length > 0) {
					lsm->ifd[ifd_idx].tag_bits_per_sample = (uint16_t*)malloc(2 * field.length);
					if (!lsm->ifd[ifd_idx].tag_bits_per_sample) {
						lsm_free(lsm);
						return NULL;
					}
					if (fseek(f, field.offset, SEEK_SET) != 0) {
						lsm_free(lsm);
						return NULL;
					}
					if (fread(lsm->ifd[ifd_idx].tag_bits_per_sample, 2, field.length, f) != field.length) {
						lsm_free(lsm);
						return NULL;
					}
				}
				break;
			case TIFF_TAG_COMPRESSION:
				lsm->ifd[ifd_idx].tag_compression = field.offset & 0xFF;
				break;
			case TIFF_TAG_PREDICTOR:
				lsm->ifd[ifd_idx].tag_compression = field.offset & 0xFFFF;
				break;
			case TIFF_TAG_PHOTOMETRIC_INTERPRETATION:
				lsm->ifd[ifd_idx].tag_photometric_interpretation = field.offset & 0xFFFF;
				break;
			case TIFF_TAG_STRIP_OFFSETS:
				lsm->ifd[ifd_idx].tag_strip_offsets_length = field.length;
				if (field.length > 0) {
					lsm->ifd[ifd_idx].tag_strip_offsets = (uint32_t*)malloc(4 * field.length);
					if (field.length == 1)
						lsm->ifd[ifd_idx].tag_strip_offsets[0] = field.offset;
					else {
						if (fseek(f, field.offset, SEEK_SET) != 0) {
							lsm_free(lsm);
							return NULL;
						}
						if (!lsm->ifd[ifd_idx].tag_strip_offsets) {
							lsm_free(lsm);
							return NULL;
						}
						if (fread(lsm->ifd[ifd_idx].tag_strip_offsets, 4, field.length, f) != field.length) {
							lsm_free(lsm);
							return NULL;
						}
					}
				}
				break;
			case TIFF_TAG_STRIP_BYTE_COUNTS:
				lsm->ifd[ifd_idx].tag_strip_byte_counts_length = field.length;
				if (field.length > 0) {
					lsm->ifd[ifd_idx].tag_strip_byte_counts = (uint32_t*)malloc(4 * field.length);
					if (field.length == 1)
						lsm->ifd[ifd_idx].tag_strip_byte_counts[0] = field.offset;
					else {
						if (fseek(f, field.offset, SEEK_SET) != 0) {
							lsm_free(lsm);
							return NULL;
						}
						if (!lsm->ifd[ifd_idx].tag_strip_byte_counts) {
							lsm_free(lsm);
							return NULL;
						}
						if (fread(lsm->ifd[ifd_idx].tag_strip_byte_counts, 4, field.length, f) != field.length) {
							lsm_free(lsm);
							return NULL;
						}
					}
				}
				break;
			case TIFF_TAG_SAMPLES_PER_PIXEL:
				lsm->ifd[ifd_idx].tag_samples_per_pixel = field.offset & 0xFFFF;
				break;
			case TIFF_TAG_PLANAR_CONF:
				lsm->ifd[ifd_idx].tag_planar_configuration = field.offset & 0xFFFF;
				break;
			case TIFF_TAG_LSM_INFO_OFFSET:
				if (!lsm_read_lsm_info(f, &lsm->ifd[ifd_idx], &field)) {
					lsm_free(lsm);
					return NULL;
				}
				break;
			default:
				break;
			}
		}
		// read next IFD offset
		if (fseek(f, offset + 2 + ifd_fields_count * 12, SEEK_SET) != 0) {
			lsm_free(lsm);
			return NULL;
		}
		if (fread(&offset, 2, 1, f) != 1) {
			lsm_free(lsm);
			return NULL;
		}
		ifd_idx++;
	}

	return lsm;
}

// free with free() after using
void* lsm_read_pixel_data(FILE* f, struct tiff_ifd* ifd, int strip) {
	if (!f || !ifd || strip < 0 || strip >= ifd->tag_strip_offsets_length)
		return NULL;

	if (ifd->tag_strip_offsets_length != ifd->tag_strip_byte_counts_length)
		return NULL;

	if (fseek(f, ifd->tag_strip_offsets[strip], SEEK_SET) != 0)
		return NULL;

	// LZW compression
	if (ifd->tag_compression == TIFF_COMPRESSION_LZW && ifd->tag_predictor == 2) {
		struct lzw_buff* enc = lzw_buff_alloc(1, ifd->tag_strip_byte_counts[strip]);
		if (!enc)
			return NULL;

		if (fread(&enc->data, 1, ifd->tag_strip_byte_counts[strip], f) != ifd->tag_strip_byte_counts[strip]) {
			free(enc);
			return NULL;
		}

		struct lzw_buff* dec = lzw_decode(enc);
		free(enc);
		if (!dec)
			return NULL;

		void* data = malloc(dec->length); // later we can compute the length from w, h and bits per pixel
		if (!data) {
			free(dec);
			return NULL;
		}

		memcpy(data, &dec->data, dec->length);
		free(dec);

		return data;
	}

	// no compression
	void* data = malloc(ifd->tag_strip_byte_counts[strip]);
	if (!data)
		return NULL;

	if (fread(data, 1, ifd->tag_strip_byte_counts[strip], f) != ifd->tag_strip_byte_counts[strip]) {
		free(data);
		return NULL;
	}

	return data;
}

void lsm_print_info(struct lsm_file* lsm) {
	if (!lsm) {
		puts("Not a proper .lsm file");
		return;
	}

	for (int ifd_idx = 0; ifd_idx < lsm->ifd_length; ifd_idx++) {
		printf("Directory %d\n", ifd_idx);
		printf("\tType (0-full image, 1-thumbnail): %d\n", lsm->ifd[ifd_idx].tag_new_subfile_type);
		printf("\tWidth: %d\n", lsm->ifd[ifd_idx].tag_image_width);
		printf("\tHeight: %d\n", lsm->ifd[ifd_idx].tag_image_length);
		printf("\tCompression (1-None, 5-LZW): %d\n", lsm->ifd[ifd_idx].tag_compression);
		printf("\tStripes: %d\n", lsm->ifd[ifd_idx].tag_strip_offsets_length);
		printf("\tData type: %d\n", lsm->ifd[ifd_idx].tag_lsm_info.intensity_data_type);

		if (lsm->ifd[ifd_idx].tag_bits_per_sample_length > 0) {
			printf("\tBits per sample: ");
			for (int channel = 0; channel < lsm->ifd[ifd_idx].tag_bits_per_sample_length; channel++)
				printf("%d ", lsm->ifd[ifd_idx].tag_bits_per_sample[channel]);
			puts("");
		}

		if (lsm->ifd[ifd_idx].tag_lsm_info.colors.number_colors == lsm->ifd[ifd_idx].tag_lsm_info.colors.number_names) {
			puts("\tColors:");
			for (int color = 0; color < lsm->ifd[ifd_idx].tag_lsm_info.colors.number_colors; color++) {
				printf("\t\t0x%.8X (%s)\n",
					lsm->ifd[ifd_idx].tag_lsm_info.colors.colors[color],
					lsm->ifd[ifd_idx].tag_lsm_info.colors.names[color]);
			}
		}

		puts("");
	}
}
/*
struct rgba {
uint8_t r, g, b, a;
};

union u_rgba {
struct rgba rgba;
uint32_t uint32;
};

void lsm_built_lut8(union u_rgba* lut, uint32_t color) {
if((color & 0x00FFFFFF) != 0x00FFFFFF) {
union u_rgba rgba;
rgba.uint32 = color;
for(int i=1; i<256; i++) {
lut[i].rgba.a = 0;
lut[i].rgba.r = (rgba.rgba.b / 255.0f) * i;
lut[i].rgba.g = (rgba.rgba.g / 255.0f) * i;
lut[i].rgba.b = (rgba.rgba.r / 255.0f) * i;
}
} else {
lut[0].uint32 = 0;
for(int i=1; i<256; i++)
lut[i].uint32 = lut[i-1].uint32 + 0x00010101;
}
}
*/


#endif
