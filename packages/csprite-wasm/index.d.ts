export interface CspriteModuleOptions {
  locateFile?: (path: string, prefix: string) => string;
  wasmBinary?: ArrayBuffer | Uint8Array;
  print?: (text: string) => void;
  printErr?: (text: string) => void;
}

export type CspriteDocument = number;
export type WasmPointer = number;

export interface CspriteModule {
  HEAPU8: Uint8Array;
  HEAP32: Int32Array;
  ccall: (ident: string, returnType: string | null, argTypes: string[], args: unknown[]) => unknown;
  cwrap: (ident: string, returnType: string | null, argTypes: string[]) => (...args: unknown[]) => unknown;

  _malloc(size: number): WasmPointer;
  _free(ptr: WasmPointer): void;

  _csprite_create(width: number, height: number): CspriteDocument;
  _csprite_destroy(doc: CspriteDocument): void;
  _csprite_width(doc: CspriteDocument): number;
  _csprite_height(doc: CspriteDocument): number;
  _csprite_pixels_size(doc: CspriteDocument): number;
  _csprite_pixels(doc: CspriteDocument): WasmPointer;
  _csprite_set_rgba(doc: CspriteDocument, r: number, g: number, b: number, a: number): void;
  _csprite_set_brush_size(doc: CspriteDocument, size: number): void;
  _csprite_set_filled(doc: CspriteDocument, filled: number): void;
  _csprite_clear(doc: CspriteDocument, r: number, g: number, b: number, a: number): void;
  _csprite_copy_rgba_in(doc: CspriteDocument, pixels: WasmPointer, size: number): number;
  _csprite_get_pixel(doc: CspriteDocument, x: number, y: number): number;
  _csprite_set_pixel(doc: CspriteDocument, x: number, y: number, r: number, g: number, b: number, a: number): void;
  _csprite_draw_line(doc: CspriteDocument, x0: number, y0: number, x1: number, y1: number): void;
  _csprite_draw_rect(doc: CspriteDocument, x0: number, y0: number, x1: number, y1: number): void;
  _csprite_draw_ellipse(doc: CspriteDocument, x0: number, y0: number, x1: number, y1: number): void;
  _csprite_draw_circle(doc: CspriteDocument, x: number, y: number): void;
}

export default function createCspriteModule(options?: CspriteModuleOptions): Promise<CspriteModule>;
