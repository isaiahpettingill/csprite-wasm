#include "raster/gfx.h"

#include <emscripten/emscripten.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
	Raster image;
	RGBAU8 color;
	U32 brush_size;
	B32 filled;
} Csprite_Doc;

#define CSPRITE_EXPORT EMSCRIPTEN_KEEPALIVE

static B32 csprite_doc_valid(Csprite_Doc* doc) {
	return doc != NULL && doc->image.data != NULL && doc->image.dim.w > 0 && doc->image.dim.h > 0;
}

CSPRITE_EXPORT Csprite_Doc* csprite_create(U32 width, U32 height) {
	if (width == 0 || height == 0) {
		return NULL;
	}

	Csprite_Doc* doc = malloc(sizeof(*doc));
	if (doc == NULL) {
		return NULL;
	}

	memset(doc, 0, sizeof(*doc));
	U64 pixel_count = (U64)width * (U64)height;
	if (pixel_count > (U64)SIZE_MAX / sizeof(*doc->image.data)) {
		free(doc);
		return NULL;
	}

	doc->image.dim = rect(width, height);
	doc->image.data = calloc((size_t)pixel_count, sizeof(*doc->image.data));
	if (doc->image.data == NULL) {
		free(doc);
		return NULL;
	}
	doc->color = rgbau8(255, 255, 255, 255);
	doc->brush_size = 1;
	doc->filled = 1;

	return doc;
}

CSPRITE_EXPORT void csprite_destroy(Csprite_Doc* doc) {
	if (doc == NULL) {
		return;
	}
	free(doc->image.data);
	free(doc);
}

CSPRITE_EXPORT U32 csprite_width(Csprite_Doc* doc) {
	return csprite_doc_valid(doc) ? doc->image.dim.w : 0;
}

CSPRITE_EXPORT U32 csprite_height(Csprite_Doc* doc) {
	return csprite_doc_valid(doc) ? doc->image.dim.h : 0;
}

CSPRITE_EXPORT U32 csprite_pixels_size(Csprite_Doc* doc) {
	if (!csprite_doc_valid(doc)) {
		return 0;
	}

	U64 size = (U64)doc->image.dim.w * (U64)doc->image.dim.h * sizeof(RGBAU8);
	return size <= UINT32_MAX ? (U32)size : 0;
}

CSPRITE_EXPORT U8* csprite_pixels(Csprite_Doc* doc) {
	return csprite_doc_valid(doc) ? (U8*)doc->image.data : NULL;
}

CSPRITE_EXPORT void csprite_set_rgba(Csprite_Doc* doc, U8 r, U8 g, U8 b, U8 a) {
	if (doc != NULL) {
		doc->color = rgbau8(r, g, b, a);
	}
}

CSPRITE_EXPORT void csprite_set_brush_size(Csprite_Doc* doc, U32 size) {
	if (doc != NULL) {
		doc->brush_size = size < 1 ? 1 : size;
	}
}

CSPRITE_EXPORT void csprite_set_filled(Csprite_Doc* doc, B32 filled) {
	if (doc != NULL) {
		doc->filled = filled != 0;
	}
}

CSPRITE_EXPORT void csprite_clear(Csprite_Doc* doc, U8 r, U8 g, U8 b, U8 a) {
	if (!csprite_doc_valid(doc)) {
		return;
	}

	RGBAU8 color = rgbau8(r, g, b, a);
	U64 pixel_count = (U64)doc->image.dim.w * (U64)doc->image.dim.h;
	for EachIndex(i, pixel_count) {
		doc->image.data[i] = color;
	}
}

CSPRITE_EXPORT B32 csprite_copy_rgba_in(Csprite_Doc* doc, const U8* pixels, U32 size) {
	U32 expected_size = csprite_pixels_size(doc);
	if (expected_size == 0 || pixels == NULL || size < expected_size) {
		return 0;
	}

	memcpy(doc->image.data, pixels, expected_size);
	return 1;
}

CSPRITE_EXPORT U32 csprite_get_pixel(Csprite_Doc* doc, S32 x, S32 y) {
	if (!csprite_doc_valid(doc) || x < 0 || y < 0 || x >= (S32)doc->image.dim.w || y >= (S32)doc->image.dim.h) {
		return 0;
	}

	RGBAU8 p = doc->image.data[(y * doc->image.dim.w) + x];
	return ((U32)p.r << 24) | ((U32)p.g << 16) | ((U32)p.b << 8) | (U32)p.a;
}

CSPRITE_EXPORT void csprite_set_pixel(Csprite_Doc* doc, S32 x, S32 y, U8 r, U8 g, U8 b, U8 a) {
	if (!csprite_doc_valid(doc) || x < 0 || y < 0 || x >= (S32)doc->image.dim.w || y >= (S32)doc->image.dim.h) {
		return;
	}

	doc->image.data[(y * doc->image.dim.w) + x] = rgbau8(r, g, b, a);
}

CSPRITE_EXPORT void csprite_draw_line(Csprite_Doc* doc, S32 x0, S32 y0, S32 x1, S32 y1) {
	if (csprite_doc_valid(doc)) {
		rs_gfx_draw_line(&doc->image, doc->color, v2s32(x0, y0), v2s32(x1, y1));
	}
}

CSPRITE_EXPORT void csprite_draw_rect(Csprite_Doc* doc, S32 x0, S32 y0, S32 x1, S32 y1) {
	if (csprite_doc_valid(doc)) {
		rs_gfx_draw_rect(&doc->image, doc->color, v2s32(x0, y0), v2s32(x1, y1));
	}
}

CSPRITE_EXPORT void csprite_draw_ellipse(Csprite_Doc* doc, S32 x0, S32 y0, S32 x1, S32 y1) {
	if (csprite_doc_valid(doc)) {
		rs_gfx_draw_ellipse(&doc->image, doc->color, v2s32(x0, y0), v2s32(x1, y1));
	}
}

CSPRITE_EXPORT void csprite_draw_circle(Csprite_Doc* doc, S32 x, S32 y) {
	if (csprite_doc_valid(doc)) {
		rs_gfx_draw_circle(&doc->image, doc->color, v2s32(x, y), doc->brush_size, doc->filled);
	}
}
