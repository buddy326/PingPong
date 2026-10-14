#!/bin/sh
# Builds the game for the browser (WebAssembly) into docs/, for GitHub Pages.
# Requires Emscripten (brew install emscripten).
set -e

RAYLIB_SRC=build/external/raylib-master/src

# Build raylib for the web once
if [ ! -f "$RAYLIB_SRC/libraylib.web.a" ]; then
	make -C "$RAYLIB_SRC" PLATFORM=PLATFORM_WEB -B
fi

mkdir -p docs
em++ src/main.cpp -o docs/index.html \
	-I"$RAYLIB_SRC" -Iinclude \
	"$RAYLIB_SRC/libraylib.web.a" \
	-DPLATFORM_WEB -Os -Wall \
	-s USE_GLFW=3 -s ASYNCIFY \
	--shell-file web/shell.html

echo "Web build written to docs/"
