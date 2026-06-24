import createCspriteEditorModule from "./dist/csprite_editor.js";

let mountedCanvas;

export async function mountCspriteEditor(options) {
  const canvas = options?.canvas;
  if (!(canvas instanceof HTMLCanvasElement)) {
    throw new TypeError("mountCspriteEditor requires an HTMLCanvasElement.");
  }

  const existingCanvas = document.getElementById("canvas");
  if (existingCanvas && existingCanvas !== canvas) {
    throw new Error("csprite editor can only mount when document #canvas is available.");
  }

  canvas.id = "canvas";
  canvas.tabIndex = canvas.tabIndex < 0 ? 0 : canvas.tabIndex;
  canvas.oncontextmenu = (event) => event.preventDefault();
  mountedCanvas = canvas;

  const module = await createCspriteEditorModule({
    canvas,
    locateFile: (path, prefix) => (path.endsWith(".wasm") ? new URL(`./dist/${path}`, import.meta.url).toString() : `${prefix}${path}`),
    print: options?.print ?? (() => {}),
    printErr: options?.printErr ?? (() => {}),
  });

  canvas.focus();
  canvas.addEventListener("pointerdown", focusMountedCanvas);

  return {
    module,
    canvas,
    loadRgba(width, height, pixels) {
      const bytes = pixels instanceof Uint8Array ? pixels : new Uint8Array(pixels.buffer, pixels.byteOffset, pixels.byteLength);
      const ptr = module._malloc(bytes.byteLength);
      try {
        module.HEAPU8.set(bytes, ptr);
        return Boolean(module._csprite_editor_load_rgba(width, height, ptr, bytes.byteLength));
      } finally {
        module._free(ptr);
      }
    },
    readRgba() {
      const width = module._csprite_editor_width();
      const height = module._csprite_editor_height();
      const size = module._csprite_editor_pixels_size();
      const ptr = module._csprite_editor_pixels();
      return { width, height, pixels: module.HEAPU8.slice(ptr, ptr + size) };
    },
    dispose() {
      canvas.removeEventListener("pointerdown", focusMountedCanvas);
      if (mountedCanvas === canvas) mountedCanvas = undefined;
    },
  };
}

function focusMountedCanvas() {
  mountedCanvas?.focus();
}
