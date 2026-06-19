if(NOT DEFINED OUTPUT_PATH)
  message(FATAL_ERROR "OUTPUT_PATH is required")
endif()

file(WRITE "${OUTPUT_PATH}" "{\n  \"highest_sequential_cleared_level\": 0\n}\n")
