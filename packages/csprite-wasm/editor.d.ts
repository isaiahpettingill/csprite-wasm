import type { CspriteModuleOptions } from "./index.d.ts";

export interface CspriteEditorMountOptions extends Pick<CspriteModuleOptions, "print" | "printErr"> {
  canvas: HTMLCanvasElement;
}

export interface CspriteEditorHandle {
  module: unknown;
  canvas: HTMLCanvasElement;
  loadRgba(width: number, height: number, pixels: Uint8Array | Uint8ClampedArray): boolean;
  readRgba(): { width: number; height: number; pixels: Uint8Array };
  dispose(): void;
}

export function mountCspriteEditor(options: CspriteEditorMountOptions): Promise<CspriteEditorHandle>;
