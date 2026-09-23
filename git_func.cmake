cmake_minimum_required(VERSION 3.8...3.25)

# Single character tag describing what kind of build this is. When left empty
# the tag is derived from the current git branch, which is what local builds
# want. CI sets it explicitly (-DCANDLESDK_VERSION_TAG_OVERRIDE=r for releases)
# because the build containers cannot always read the repository's git
# metadata, and the branch guess would then silently degrade to 'x'.
set(CANDLESDK_VERSION_TAG_OVERRIDE
    ""
    CACHE STRING
          "Single character version tag to use instead of the git derived one")

# The tag ends up as a C char literal (CANDLETOOL_VTAG) and in package file
# names, so anything but one letter or digit would break the build or names.
if(NOT CANDLESDK_VERSION_TAG_OVERRIDE STREQUAL ""
   AND NOT CANDLESDK_VERSION_TAG_OVERRIDE MATCHES "^[A-Za-z0-9]$")
  message(
    FATAL_ERROR
      "CANDLESDK_VERSION_TAG_OVERRIDE must be a single letter or digit, got "
      "'${CANDLESDK_VERSION_TAG_OVERRIDE}'")
endif()

if(UNIX)
  execute_process(
    COMMAND git log -1 --format=%h
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_HASH
    ERROR_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(
    COMMAND git rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GIT_BRANCH
    ERROR_VARIABLE GIT_BRANCH
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(GIT_HASH MATCHES "git")
    set(GIT_HASH "XXXXXXXX")
    set(GIT_BRANCH "not_git_repo")
  endif()
else()
  set(GIT_HASH "XXXXXXXX")
  set(GIT_BRANCH "not_git_repo")
endif()

function(get_tag TAG_VAR)
  if(NOT CANDLESDK_VERSION_TAG_OVERRIDE STREQUAL "")
    set(${TAG_VAR}
        '${CANDLESDK_VERSION_TAG_OVERRIDE}'
        PARENT_SCOPE)
  elseif(GIT_BRANCH STREQUAL "main")
    set(${TAG_VAR}
        'm'
        PARENT_SCOPE)
  elseif(GIT_BRANCH STREQUAL "devel")
    set(${TAG_VAR}
        'd'
        PARENT_SCOPE)
  else()
    set(${TAG_VAR}
        'x'
        PARENT_SCOPE)
  endif()
endfunction()
