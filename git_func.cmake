cmake_minimum_required(VERSION 3.8)

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

if(${GIT_HASH} MATCHES "git")
  set(GIT_HASH "XXXXXXXX")
  set(GIT_BRANCH "not_git_repo")
endif()
  function(get_tag TAG_VAR)
    if(${GIT_BRANCH} STREQUAL "main")
      set(${TAG_VAR}
          'm'
          PARENT_SCOPE)
    elseif(${GIT_BRANCH} STREQUAL "devel")
      set(${TAG_VAR}
          'd'
          PARENT_SCOPE)
    else()
      set(${TAG_VAR}
          'x'
          PARENT_SCOPE)
    endif()
  endfunction()
else()
  function(get_tag TAG_VAR)
  set(${TAG_VAR}
            'x'
            PARENT_SCOPE)
  endfunction()
endif()
