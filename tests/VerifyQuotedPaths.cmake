if(NOT DEFINED VIEWER OR NOT DEFINED SOURCE_MODEL OR NOT DEFINED TEST_ROOT)
    message(FATAL_ERROR "VIEWER, SOURCE_MODEL and TEST_ROOT must be supplied.")
endif()

# The offline path runs CNA's converter through the shell. Apostrophes and spaces in the input
# and output paths must survive that quoting.
file(REMOVE_RECURSE "${TEST_ROOT}")
set(INPUT_DIRECTORY "${TEST_ROOT}/it's a model")
set(OUTPUT_DIRECTORY "${TEST_ROOT}/viewer's cnj")
set(CAPTURE "${TEST_ROOT}/capture.png")
file(MAKE_DIRECTORY "${INPUT_DIRECTORY}")
file(COPY_FILE "${SOURCE_MODEL}" "${INPUT_DIRECTORY}/model's copy.glb")

execute_process(
    COMMAND "${VIEWER}" "${INPUT_DIRECTORY}/model's copy.glb"
        --output "${OUTPUT_DIRECTORY}" --capture "${CAPTURE}"
    RESULT_VARIABLE viewerResult
    OUTPUT_VARIABLE viewerOutput
    ERROR_VARIABLE viewerError
    TIMEOUT 60)

if(NOT viewerResult EQUAL 0)
    message(FATAL_ERROR
        "The viewer failed with ${viewerResult}.\nstdout:\n${viewerOutput}\nstderr:\n${viewerError}")
endif()

set(MODEL "${OUTPUT_DIRECTORY}/scene.cnj")
if(NOT EXISTS "${MODEL}")
    message(FATAL_ERROR "The converter did not produce ${MODEL}.\nstdout:\n${viewerOutput}")
endif()

file(READ "${MODEL}" modelJson)
string(FIND "${modelJson}" "\"type\": \"Model\"" modelTypeOffset)
if(modelTypeOffset EQUAL -1)
    message(FATAL_ERROR "The generated CNJ is not a Model asset.")
endif()

if(NOT EXISTS "${CAPTURE}")
    message(FATAL_ERROR "The viewer did not write ${CAPTURE}.\nstdout:\n${viewerOutput}")
endif()
