#ifdef __EMSCRIPTEN__
#include "WebIO.h"

EM_ASYNC_JS(char*, requestInputAsync, (), {
    return new Promise(function(resolve, reject) {
        let resolved = false;
        let myResolve = function(input) {
            if (resolved) return;
            resolved = true;
            if (input === null || input === undefined) input = "";
            var lengthBytes = lengthBytesUTF8(input) + 1;
            var stringOnWasmHeap = _malloc(lengthBytes);
            if (stringOnWasmHeap !== 0) {
                stringToUTF8(input, stringOnWasmHeap, lengthBytes);
            }
            resolve(stringOnWasmHeap);
        };
        if (window.isRealtimeMode && (!window.inputQueue || window.inputQueue.length === 0)) {
            myResolve("");
        } else {
            window.resolveInput = myResolve;
        }
    });
});

#endif
