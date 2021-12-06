# Copyright - Benjamin Laugraud <blaugraud@ulg.ac.be> - 2016

# Extract the filename from a path.
# For instance: "/a/b/c/filename.cc" -> "filename.cc".
#
# PATH_TO_FILE:
#   Path to a file.
#
# FILE_FROM_PATH (return value):
#   File name extracted from ${PATH_TO_FILE}.
function(benlaug_file_from_path PATH_TO_FILE)
  string(REGEX REPLACE ".*/" "" FILE_FROM_PATH ${PATH_TO_FILE})
  set(FILE_FROM_PATH ${FILE_FROM_PATH} PARENT_SCOPE)
endfunction(benlaug_file_from_path PATH_TO_FILE)

# Remove the extension of a file name.
# For instance: "filename.cc" -> "filename".
#
# FILENAME:
#   Name of a file.
#
# WITHOUT_EXTENSION (return value):
#   ${FILENAME} without its extension.
function(benlaug_remove_extension FILENAME)
  string(FIND ${FILENAME} "." STR_POINT REVERSE)
  string(SUBSTRING ${FILENAME} 0 ${STR_POINT} WITHOUT_EXTENSION)

  set(WITHOUT_EXTENSION ${WITHOUT_EXTENSION} PARENT_SCOPE)
endfunction(benlaug_remove_extension FILENAME)

# Extract the name of an executable from a path to a source file.
# For instance: "/a/b/c/my_exec.cc" -> "my_exec".
#
# SOURCE_FILE:
#   Path to a source file.
#
# EXE_NAME (return value):
#   Name of the executable relative to ${SOURCE_FILE}.
function(benlaug_executable_name SOURCE_FILE)
  benlaug_file_from_path(${SOURCE_FILE})
  benlaug_remove_extension(${FILE_FROM_PATH})

  set(EXE_NAME ${WITHOUT_EXTENSION} PARENT_SCOPE)
endfunction(benlaug_executable_name)
