import { loadWasmInstance } from './load';
import GB from './gb';

let inst: GB | null = null;
let HEAP8 = new Int8Array();
let HEAPU8 = new Uint8Array();
let HEAP32 = new Int32Array();

const clock_gettime = (clk_id: number, tp: number): number => {
  const now = Date.now();
  HEAP32[tp >> 2] = (now / 1e3) | 0;
  HEAP32[(tp + 4) >> 2] = ((now % 1e3) * 1e3 * 1e3) | 0;
  return 0;
};

let lastGrowTimestamp = 0;
const emscripten_notify_memory_growth = (idx: number) => {
  if (lastGrowTimestamp) {
    console.info('gb.wasm: Memory Grow: ', inst!.memory.buffer.byteLength);
  }
  lastGrowTimestamp = Date.now();
  HEAP8 = new Int8Array(inst!.memory.buffer);
  HEAPU8 = new Uint8Array(inst!.memory.buffer);
  HEAP32 = new Int32Array(inst!.memory.buffer);
};

const importObj = {
  env: {
    renderCanvas: (canvasId: number, buf: number) => {
      const canvas = <HTMLCanvasElement> document.getElementById('canvas');
      const ctx = <CanvasRenderingContext2D> canvas.getContext('2d');
      ctx.fillStyle = '#FF0000';
      ctx.fillRect(0, 0, 160, 144);
      const imgData = ctx.createImageData(160, 144);
      for (let i = 0; i < 144 * 160; ++i) {
        const idx = i * 4;
        const datum = HEAPU8[buf + i];
        imgData.data[idx] = datum;
        imgData.data[idx + 1] = datum;
        imgData.data[idx + 2] = datum;
        imgData.data[idx + 3] = 255;
      }
      ctx.putImageData(imgData, 0, 0);
    },
    clock_gettime,
    emscripten_notify_memory_growth,
    printAsciiBuffer: (ptr: number) => {
      let end = ptr;
      while (HEAP8[end]) {
        ++ end;
      }
      const d = new TextDecoder('ascii');
      console.log(d.decode(HEAP8.buffer.slice(ptr, end)));
    },
  },
  wasi_snapshot_preview1: {
    args_sizes_get: () => { console.error('GG'); throw Error(); },
    args_get: () => { console.error('GG'); throw Error(); },
    proc_exit: () => { console.error('GG'); throw Error(); },
    environ_sizes_get: () => { console.error('GG'); throw Error(); },
    environ_get: () => { console.error('GG'); throw Error(); },
    fd_close: () => { console.error('GG'); throw Error(); },
    fd_write: () => { console.error('GG'); throw Error(); },
    fd_seek: () => { console.error('GG'); throw Error(); },
  },
};

let instPromise = (async () => {
  const res = await loadWasmInstance(importObj);
  if (!res) {
    throw Error('WASM was not loaded');
  }
  inst = res.exports as GB;
  emscripten_notify_memory_growth(0);
  return inst;
})();

export const getInstance = async (): Promise<GB> => {
  return await instPromise;
};

export const getMemoryGrowTimestamp = (): number => {
  return lastGrowTimestamp;
};
