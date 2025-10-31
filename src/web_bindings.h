/*
 * WebAssembly Bindings for August-Rio Moop Stack
 * ===============================================
 *
 * JavaScript API bindings for WebAssembly compilation
 */

#ifndef WEB_BINDINGS_H
#define WEB_BINDINGS_H

#include <emscripten.h>

// WebAssembly API functions
#ifdef __cplusplus
extern "C" {
#endif

EMSCRIPTEN_KEEPALIVE
const char* compile_moop(const char* source, const char* options_json);

EMSCRIPTEN_KEEPALIVE
void free_result(void* result);

EMSCRIPTEN_KEEPALIVE
const char* get_version();

EMSCRIPTEN_KEEPALIVE
const char* get_capabilities();

#ifdef __cplusplus
}
#endif

#endif // WEB_BINDINGS_H