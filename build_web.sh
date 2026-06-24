#!/bin/sh

set -eu

CC=${CC:-emcc}
CXX=${CXX:-em++}
NATIVE_CXX=${NATIVE_CXX:-c++}
BUILD=${BUILD:-build/web}

SCRIPT_DIR=$(CDPATH= cd "$(dirname "$0")" && pwd)
cd "$SCRIPT_DIR"

COMMON_FLAGS='-O3 -Wall -Wextra -Isrc -Ivendor/log.c/include -Ivendor/stb/include -DTARGET_WEB=1 -DBUILD_RELEASE=1 -DCIMGUI_NO_EXPORT=1'
APP_C_FLAGS='-DCIMGUI_USE_GLFW=1 -DCIMGUI_USE_OPENGL3=1 -DCIMGUI_DEFINE_ENUMS_AND_STRUCTS=1 -DIMGUI_IMPL_OPENGL_ES3=1'
APP_CPP_FLAGS='-DCIMGUI_USE_GLFW=1 -DCIMGUI_USE_OPENGL3=1 -DIMGUI_IMPL_OPENGL_ES3=1'
APP_C_SOURCES='src/app/main.c src/app/gui.c src/app/render.c src/app/editor.c src/os/os.c src/os/gfx.c src/base/arena.c src/base/string.c src/raster/raster.c src/raster/gfx.c src/raster/math.c src/assets/assets.c vendor/log.c/src/log.c vendor/stb/impl.c'
APP_CPP_SOURCES='src/cimgui/impl.cpp'
APP_EXPORTS='["_malloc","_free","_main","_csprite_editor_width","_csprite_editor_height","_csprite_editor_pixels_size","_csprite_editor_pixels","_csprite_editor_load_rgba"]'
API_C_SOURCES='src/web/api.c src/raster/gfx.c src/raster/math.c'
API_EXPORTS='["_malloc","_free","_csprite_create","_csprite_destroy","_csprite_width","_csprite_height","_csprite_pixels_size","_csprite_pixels","_csprite_set_rgba","_csprite_set_brush_size","_csprite_set_filled","_csprite_clear","_csprite_copy_rgba_in","_csprite_get_pixel","_csprite_set_pixel","_csprite_draw_line","_csprite_draw_rect","_csprite_draw_ellipse","_csprite_draw_circle"]'

if [ "${1:-}" = "clean" ]; then
	rm -rf "$BUILD"
	exit 0
fi

if ! [ -f src/assets/assets.inl ]; then
	if [ -x "$(command -v python3)" ]; then
		PYTHON=python3
	elif [ -x "$(command -v python)" ]; then
		PYTHON=python
	else
		echo "Python not found!"
		exit 1
	fi
	rm -f tools/font2inl.out
	$PYTHON tools/create_icons.py
	$PYTHON tools/create_assets.py --cxx="$NATIVE_CXX"
fi

mkdir -p "$BUILD/app" "$BUILD/api"

APP_OBJECTS=''
for source in $APP_C_SOURCES; do
	object="$BUILD/app/$source.o"
	mkdir -p "$(dirname "$object")"
	$CC $COMMON_FLAGS $APP_C_FLAGS -std=gnu99 -c "$source" -o "$object"
	APP_OBJECTS="$APP_OBJECTS $object"
done

for source in $APP_CPP_SOURCES; do
	object="$BUILD/app/$source.o"
	mkdir -p "$(dirname "$object")"
	$CXX $COMMON_FLAGS $APP_CPP_FLAGS -fno-exceptions -fno-rtti -c "$source" -o "$object"
	APP_OBJECTS="$APP_OBJECTS $object"
done

$CXX $APP_OBJECTS -o "$BUILD/csprite.html" \
	-sUSE_GLFW=3 \
	-sFULL_ES3=1 \
	-sMIN_WEBGL_VERSION=2 \
	-sMAX_WEBGL_VERSION=2 \
	-sALLOW_MEMORY_GROWTH=1 \
	-sEXPORTED_FUNCTIONS="$APP_EXPORTS"

$CXX $APP_OBJECTS -o "$BUILD/csprite_editor.js" \
	-sMODULARIZE=1 \
	-sEXPORT_ES6=1 \
	-sEXPORT_NAME=createCspriteEditorModule \
	-sENVIRONMENT=web \
	-sUSE_GLFW=3 \
	-sFULL_ES3=1 \
	-sMIN_WEBGL_VERSION=2 \
	-sMAX_WEBGL_VERSION=2 \
	-sALLOW_MEMORY_GROWTH=1 \
	-sEXPORTED_FUNCTIONS="$APP_EXPORTS"

API_OBJECTS=''
for source in $API_C_SOURCES; do
	object="$BUILD/api/$source.o"
	mkdir -p "$(dirname "$object")"
	$CC $COMMON_FLAGS -std=gnu99 -c "$source" -o "$object"
	API_OBJECTS="$API_OBJECTS $object"
done

$CC $API_OBJECTS -o "$BUILD/csprite_api.js" \
	-sMODULARIZE=1 \
	-sEXPORT_ES6=1 \
	-sEXPORT_NAME=createCspriteModule \
	-sENVIRONMENT=web,worker,node \
	-sALLOW_MEMORY_GROWTH=1 \
	-sEXPORTED_RUNTIME_METHODS='["ccall","cwrap"]' \
	-sEXPORTED_FUNCTIONS="$API_EXPORTS"
