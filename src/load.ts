import wasmBinaryFile from './gb.wasm.bin';

export const loadWasmInstance = async (
  importObj: any
): Promise<WebAssembly.Instance | null> => {
  const res = await fetch(wasmBinaryFile);
  if (!res['ok']) {
    console.error('Failed to load wasm binary file at ' + wasmBinaryFile);
    return null;
  }
  const binary = await res.arrayBuffer();
  const mod = await WebAssembly.compile(binary);
  const output = await WebAssembly.instantiate(mod, importObj);
  return output;
};
