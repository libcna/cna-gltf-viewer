if(NOT DEFINED CONVERTER OR NOT DEFINED TEST_ROOT)
    message(FATAL_ERROR "CONVERTER and TEST_ROOT must be supplied.")
endif()

file(REMOVE_RECURSE "${TEST_ROOT}")
file(MAKE_DIRECTORY "${TEST_ROOT}")

set(INPUT "${TEST_ROOT}/triangle.gltf")
set(OUTPUT "${TEST_ROOT}/converted")

file(WRITE "${INPUT}" [=[{
  "asset": { "version": "2.0" },
  "buffers": [
    {
      "uri": "data:application/octet-stream;base64,AAAAAAAAAAAAAAAAAACAPwAAAAAAAAAAAAAAAAAAgD8AAAAAAAAAAAEAAgA=",
      "byteLength": 42
    }
  ],
  "bufferViews": [
    { "buffer": 0, "byteOffset": 0, "byteLength": 36, "target": 34962 },
    { "buffer": 0, "byteOffset": 36, "byteLength": 6, "target": 34963 }
  ],
  "accessors": [
    {
      "bufferView": 0,
      "componentType": 5126,
      "count": 3,
      "type": "VEC3",
      "min": [0, 0, 0],
      "max": [1, 1, 0]
    },
    { "bufferView": 1, "componentType": 5123, "count": 3, "type": "SCALAR" }
  ],
  "meshes": [
    { "primitives": [ { "attributes": { "POSITION": 0 }, "indices": 1 } ] }
  ],
  "nodes": [ { "mesh": 0 } ],
  "scenes": [ { "nodes": [0] } ],
  "scene": 0
}
]=])

execute_process(
    COMMAND "${CONVERTER}" "${INPUT}" "${OUTPUT}" triangle
    RESULT_VARIABLE conversionResult
    OUTPUT_VARIABLE conversionOutput
    ERROR_VARIABLE conversionError)

if(NOT conversionResult EQUAL 0)
    message(FATAL_ERROR
        "CNA conversion failed with ${conversionResult}.\nstdout:\n${conversionOutput}\nstderr:\n${conversionError}")
endif()

set(MODEL "${OUTPUT}/triangle.cnj")
if(NOT EXISTS "${MODEL}")
    message(FATAL_ERROR "The converter did not produce ${MODEL}.")
endif()

file(READ "${MODEL}" modelJson)
string(FIND "${modelJson}" "\"type\": \"Model\"" modelTypeOffset)
if(modelTypeOffset EQUAL -1)
    message(FATAL_ERROR "The generated CNJ is not a Model asset.")
endif()
