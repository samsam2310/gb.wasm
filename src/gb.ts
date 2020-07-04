export default interface GB extends Record<string, WebAssembly.ExportValue> {
  memory: WebAssembly.Memory;
  malloc(size: number): number;
  free(ptr: number): void;
  createGameboy(rom: number, canvasId: number): number;
  runGameboy(gb: number): boolean;
}
