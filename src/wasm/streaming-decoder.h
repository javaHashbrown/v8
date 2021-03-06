// Copyright 2017 the V8 project authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef V8_WASM_STREAMING_DECODER_H_
#define V8_WASM_STREAMING_DECODER_H_

#include <memory>
#include <vector>

#include "src/base/macros.h"
#include "src/utils/vector.h"
#include "src/wasm/compilation-environment.h"
#include "src/wasm/wasm-constants.h"
#include "src/wasm/wasm-result.h"

namespace v8 {
namespace internal {
namespace wasm {
class NativeModule;

// This class is an interface for the StreamingDecoder to start the processing
// of the incoming module bytes.
class V8_EXPORT_PRIVATE StreamingProcessor {
 public:
  virtual ~StreamingProcessor() = default;
  // Process the first 8 bytes of a WebAssembly module. Returns true if the
  // processing finished successfully and the decoding should continue.
  virtual bool ProcessModuleHeader(Vector<const uint8_t> bytes,
                                   uint32_t offset) = 0;

  // Process all sections but the code section. Returns true if the processing
  // finished successfully and the decoding should continue.
  virtual bool ProcessSection(SectionCode section_code,
                              Vector<const uint8_t> bytes, uint32_t offset) = 0;

  // Process the start of the code section. Returns true if the processing
  // finished successfully and the decoding should continue.
  virtual bool ProcessCodeSectionHeader(int num_functions, uint32_t offset,
                                        std::shared_ptr<WireBytesStorage>,
                                        int code_section_length) = 0;

  // Process a function body. Returns true if the processing finished
  // successfully and the decoding should continue.
  virtual bool ProcessFunctionBody(Vector<const uint8_t> bytes,
                                   uint32_t offset) = 0;

  // Report the end of a chunk.
  virtual void OnFinishedChunk() = 0;
  // Report the end of the stream. If the stream was successful, all
  // received bytes are passed by parameter. If there has been an error, an
  // empty array is passed.
  virtual void OnFinishedStream(OwnedVector<uint8_t> bytes) = 0;
  // Report an error detected in the StreamingDecoder.
  virtual void OnError(const WasmError&) = 0;
  // Report the abortion of the stream.
  virtual void OnAbort() = 0;

  // Attempt to deserialize the module. Supports embedder caching.
  virtual bool Deserialize(Vector<const uint8_t> module_bytes,
                           Vector<const uint8_t> wire_bytes) = 0;
};

// The StreamingDecoder takes a sequence of byte arrays, each received by a call
// of {OnBytesReceived}, and extracts the bytes which belong to section payloads
// and function bodies.
class V8_EXPORT_PRIVATE StreamingDecoder {
 public:
  virtual ~StreamingDecoder() = default;

  // The buffer passed into OnBytesReceived is owned by the caller.
  virtual void OnBytesReceived(Vector<const uint8_t> bytes) = 0;

  virtual void Finish() = 0;

  virtual void Abort() = 0;

  // Notify the StreamingDecoder that compilation ended and the
  // StreamingProcessor should not be called anymore.
  virtual void NotifyCompilationEnded() = 0;

  // Caching support.
  // Sets the callback that is called after the module is fully compiled.
  using ModuleCompiledCallback =
      std::function<void(const std::shared_ptr<NativeModule>&)>;
  virtual void SetModuleCompiledCallback(ModuleCompiledCallback callback) = 0;
  // Passes previously compiled module bytes from the embedder's cache.
  virtual bool SetCompiledModuleBytes(
      Vector<const uint8_t> compiled_module_bytes) = 0;

  virtual void NotifyNativeModuleCreated(
      const std::shared_ptr<NativeModule>& native_module) = 0;

  virtual Vector<const char> url() = 0;
  virtual void SetUrl(Vector<const char> url) = 0;
  static std::unique_ptr<StreamingDecoder> CreateAsyncStreamingDecoder(
      std::unique_ptr<StreamingProcessor> processor);
};

}  // namespace wasm
}  // namespace internal
}  // namespace v8

#endif  // V8_WASM_STREAMING_DECODER_H_
