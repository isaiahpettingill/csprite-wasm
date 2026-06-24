# @josemancharo/csprite-wasm

Embeddable WebAssembly build of csprite. It includes the full canvas-based editor and direct access to csprite document creation, raw RGBA pixel buffers, and basic drawing operations.

The interactive browser build is deployed at:

https://isaiahpettingill.github.io/csprite-wasm/

## Install

```sh
npm install @josemancharo/csprite-wasm
```

## Quick Start

Mount the interactive editor to a canvas:

```ts
import { mountCspriteEditor } from "@josemancharo/csprite-wasm/editor";

await mountCspriteEditor({ canvas: document.querySelector("canvas")! });
```

The editor currently uses Emscripten/GLFW's global `#canvas` target, so mount one editor instance per page.

Load and save raw RGBA pixels through the returned handle:

```ts
const editor = await mountCspriteEditor({ canvas });
editor.loadRgba(16, 16, pixels);
const edited = editor.readRgba();
```

## Raster API

```ts
import createCspriteModule from "@josemancharo/csprite-wasm";

const csprite = await createCspriteModule();
const doc = csprite._csprite_create(64, 64);

csprite._csprite_clear(doc, 0, 0, 0, 0);
csprite._csprite_set_rgba(doc, 255, 0, 0, 255);
csprite._csprite_draw_line(doc, 0, 0, 63, 63);

const width = csprite._csprite_width(doc);
const height = csprite._csprite_height(doc);
const ptr = csprite._csprite_pixels(doc);
const size = csprite._csprite_pixels_size(doc);
const pixels = new Uint8ClampedArray(csprite.HEAPU8.buffer, ptr, size);

const canvas = document.querySelector("canvas")!;
canvas.width = width;
canvas.height = height;
canvas.getContext("2d")!.putImageData(new ImageData(pixels, width, height), 0, 0);

csprite._csprite_destroy(doc);
```

## Loading The WASM File

Most bundlers can load the package entry point and its sibling `.wasm` file directly. If your bundler relocates assets, pass Emscripten's `locateFile` option:

```ts
const csprite = await createCspriteModule({
  locateFile: (path) => new URL(`./${path}`, import.meta.url).toString(),
});
```

## Raw RGBA Input

Use `_malloc`, copy bytes into `HEAPU8`, then pass the pointer to `_csprite_copy_rgba_in`.

```ts
const input = new Uint8Array(width * height * 4);
const inputPtr = csprite._malloc(input.byteLength);

csprite.HEAPU8.set(input, inputPtr);
csprite._csprite_copy_rgba_in(doc, inputPtr, input.byteLength);
csprite._free(inputPtr);
```

## API

Documents are numeric WASM pointers. Always call `_csprite_destroy` when finished.

Core functions:
- `_csprite_create(width, height)`
- `_csprite_destroy(doc)`
- `_csprite_width(doc)` / `_csprite_height(doc)`
- `_csprite_pixels(doc)` / `_csprite_pixels_size(doc)`
- `_csprite_clear(doc, r, g, b, a)`
- `_csprite_copy_rgba_in(doc, pixelsPtr, size)`
- `_csprite_get_pixel(doc, x, y)`
- `_csprite_set_pixel(doc, x, y, r, g, b, a)`
- `_csprite_set_rgba(doc, r, g, b, a)`
- `_csprite_set_brush_size(doc, size)`
- `_csprite_set_filled(doc, filled)`
- `_csprite_draw_line(doc, x0, y0, x1, y1)`
- `_csprite_draw_rect(doc, x0, y0, x1, y1)`
- `_csprite_draw_ellipse(doc, x0, y0, x1, y1)`
- `_csprite_draw_circle(doc, x, y)`

TypeScript declarations are included in the package.

## Publishing

This package is configured for npm trusted publishing from GitHub Actions. Pushes to `master` publish the package when the current `package.json` version is not already on npm. Releases and manual workflow dispatch can also run the publish workflow.

Trusted publisher settings on npm:
- Package: `@josemancharo/csprite-wasm`
- Provider: GitHub Actions
- Repository owner: `isaiahpettingill`
- Repository name: `csprite-wasm`
- Workflow filename: `publish-npm.yml`
- Environment name: `npm`
