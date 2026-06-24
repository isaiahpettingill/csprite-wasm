# csprite
A simple pixel art editor.

![csprite ui preview](https://csprite.github.io/media/csprite-preview.png)

> [!NOTE]
> _This README is intended as a technical overview of
> the software, Intended for developers. If you're an user, Please
> visit [csprite.github.io](https://csprite.github.io)_

Minimum System Requirements:
- Linux, Windows 7 or Later (Future plans to support Mac & Mobile)
- Atleast 512MB of Usable RAM
- 64-bit Processor (Future plans to support 32-bit processors in future)
- OpenGL v3.0 or Later

You can download pre-built binaries of the software [here](https://github.com/csprite/csprite/actions/workflows/build.yml?query=branch%3Ac) (Requires login).

The main aim of this software is to be simple on it's own
in terms of code & UX. With functionality to write plugins
to add support for various things like File Formats, etc.

## Compiling

Requirements for Windows:
- Windows 7 or Later
- VS Build Tools 2019 or Later With [Clang Support](https://learn.microsoft.com/en-us/cpp/build/clang-support-msbuild)
  ([Direct Download](https://aka.ms/vs/16/release/vs_BuildTools.exe))
- BusyBox for Windows ([Direct Download](https://frippery.org/files/busybox/busybox.exe))
- Python v3.0+ ([Direct Download](https://www.python.org/downloads)) (Make sure to select "Add To Path" option)

Requirements for Linux:
- POSIX Compliant Shell
- [GCC](https://repology.org/project/gcc/versions) or [Clang](https://repology.org/project/clang/versions)
  (And [libomp-dev](https://packages.debian.org/search?keywords=libomp-dev) if using Clang)
  
  > On Debian/Ubuntu (And maybe other distros as well), If you're using LLVM-Clang
  > toolchain then `libomp-dev` has to be installed as it doesn't come packaged with
  > the toolchain for some reason.
- [GLFW3](https://repology.org/project/glfw/versions) v3.1 or Later.
- [MOLD - Modern Linker](https://github.com/rui314/mold)
  
  > MOLD is used for speeding up the linking process. It is completely optional & Even
  > unnecessary if you're just building for use. It was able to cut my linking time
  > by a second or two which makes things a bit less annoying.
- [Python v3.0+](https://repology.org/project/python3/versions)

### Web / Emscripten

Requirements:
- [Emscripten](https://emscripten.org/docs/getting_started/downloads.html) with `emcc` and `em++` on `PATH`.
- Python v3.0+ for generating embedded assets.

Build both browser artifacts:

```sh
./build_web.sh
```

Outputs are written to `build/web/`:
- `csprite.html`, `csprite.js`, `csprite.wasm`: the interactive ImGui/WebGL editor.
- `csprite_editor.js`, `csprite_editor.wasm`: a modular ES build for embedding the full editor on an existing canvas.
- `csprite_api.js`, `csprite_api.wasm`: an ES module for embedding csprite drawing operations in a JavaScript site.

The embeddable editor and raster API are packaged for npm as `@josemancharo/csprite-wasm` with TypeScript declarations.
The GitHub Pages deployment is available at <https://isaiahpettingill.github.io/csprite-wasm/>.
Pushing to `master` deploys GitHub Pages and publishes the npm package when `packages/csprite-wasm/package.json` contains a version that is not already on npm.

Run the interactive build through a local HTTP server, for example:

```sh
emrun build/web/csprite.html
```

Embed the programmatic API from JavaScript:

```js
import createCspriteModule from "./build/web/csprite_api.js";

const csprite = await createCspriteModule();
const doc = csprite._csprite_create(64, 64);

csprite._csprite_clear(doc, 0, 0, 0, 0);
csprite._csprite_set_rgba(doc, 255, 0, 0, 255);
csprite._csprite_draw_line(doc, 0, 0, 63, 63);

const ptr = csprite._csprite_pixels(doc);
const size = csprite._csprite_pixels_size(doc);
const pixels = new Uint8ClampedArray(csprite.HEAPU8.buffer, ptr, size);

const canvas = document.querySelector("canvas");
const ctx = canvas.getContext("2d");
canvas.width = csprite._csprite_width(doc);
canvas.height = csprite._csprite_height(doc);
ctx.putImageData(new ImageData(pixels, canvas.width, canvas.height), 0, 0);

csprite._csprite_destroy(doc);
```

To pass an existing RGBA buffer into WASM:

```js
const input = new Uint8Array(width * height * 4);
const inputPtr = csprite._malloc(input.byteLength);
csprite.HEAPU8.set(input, inputPtr);
csprite._csprite_copy_rgba_in(doc, inputPtr, input.byteLength);
csprite._free(inputPtr);
```

## Roadmap
- [ ] Tool Preview - Just have a buffer on which user's action
      are cleared (by storing dirty from previous user input) &
      drawn on every user input. When it's time to commit just
      blit the buffer or blend it?
- [x] Variable Brush/Eraser Sizes
- [ ] Undo Redo
- [ ] File IO
  - [ ] BMP (<https://entropymine.com/jason/bmpsuite/bmpsuite/html/bmpsuite.html>)
  - [ ] QOI (<https://github.com/phoboslab/qoi/issues/69>)
- [ ] Layers with Blending Modes
  - [ ] Alpha
  - [ ] Addition
  - [ ] Subtraction
  - [ ] Difference
  - [ ] Multiply
  - [ ] Screen
  - [ ] Overlay
  - [ ] Darken
  - [ ] Lighten
  - [ ] Color Dodge/Burn
  - [ ] Hard/Soft Light
- [ ] Plugin System
- [ ] Node Based Post Processing Effects
  - [ ] Basic Effects like Lightness, Saturation, etc.
  - [ ] Dithering Effects (Like DitherBoy)
    - <https://www.visgraf.impa.br/Courses/ip00/proj/Dithering1/random_dithering.html>
    - <https://en.wikipedia.org/wiki/Ordered_dithering>
  - [ ] Convolutional Filters
- [ ] Unit Testing (<https://youtu.be/21JlBOxgGwY>)
- [ ] UTF-8 Support (Maybe)
- [ ] Docking (Maybe)
- [ ] Color Pickers for Normal, Specular, Roughness & Height Maps (<https://youtu.be/gUkY8ZoRfuQ>)
- [ ] Generate Normal Map Functionality (<https://youtu.be/-rJdOc9WZS4>)
- [ ] Procedural Math Based Art (Maybe)
- [ ] MacOS Port
- [ ] Mobile Port
- [ ] Misc
  - [ ] [Porter Duff's Alpha Functions](https://www.pismin.com/10.1145/800031.808606)
  - [ ] <https://lodev.org/cgtutor/floodfill.html#Scanline_Floodfill_Algorithm_With_Stack>
  - [ ] Use SIMD To Accelerate Various Drawing Tasks. Related:
    - <https://github.com/ermig1979/Simd>
    - <https://youtu.be/x9Scb5Mku1g>
    - <https://youtu.be/ulmjqD6Y4do>

## References

Here are resources that have helped me while developing
this software.

- [It's probably time to stop recommending Clean Code](https://qntm.org/clean)
- [Enter The Arena: Simplifying Memory Management (2023)](https://youtu.be/TZ5a3gCCZYo)
- [Compositing and Blending Level 1](https://www.w3.org/TR/compositing-1/)
- [The Beauty of Bresenham's Algorithm](https://zingl.github.io/bresenham.html)
- [CS-3388 Computer Graphics Winter 2020](https://www.csd.uwo.ca/~sbeauche/CS3388/CS3388-Bresenham.pdf)
- [Introduction to Computer Graphics, Fall 2021](https://github.com/cs123tas/projects/tree/master/brush)
- [Wu's Algorithm for anti-aliased line drawing](https://leetarxiv.substack.com/p/an-efficient-anti-aliasing-technique)

## Controls

- LMB To Select Primary Color In Palette.
- RMB To Select Secondary Color In Palette.
- Hover At Primary Color's Edge In Palette & LMB + Drag To Reorder It.
- (Ctrl + Plus/Minus) or (Regular Scroll) To Zoom In/Out.
- Plus/Minus To Increase/Decrease Brush Size.
