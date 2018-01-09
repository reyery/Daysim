#${CMAKE_COMMAND} -DRADIANCE_VERSION=v -DVERSION_OUT_FILE=v -DVERSION_IN_FILE=src/rt/VERSION -DVERSION_GOLD=src/rt/Version.c -P src/common/create_version.cmake

# if the gold version exists then use that instead
if(EXISTS "${VERSION_GOLD}")
  configure_file("${VERSION_GOLD}" "${VERSION_OUT_FILE}" COPYONLY)
  return()
endif()

MACRO (TODAY RESULT)
    IF (WIN32)
        EXECUTE_PROCESS(COMMAND "cmd" " /C date /T" OUTPUT_VARIABLE ${RESULT})
        #string(REGEX REPLACE "(..)/(..)/..(..).*" "\\1/\\2/\\3" ${RESULT} ${${RESULT}})
    ELSEIF(UNIX)
        EXECUTE_PROCESS(COMMAND "date" "+%d/%m/%Y" OUTPUT_VARIABLE ${RESULT})
        #string(REGEX REPLACE "(..)/(..)/..(..).*" "\\1/\\2/\\3" ${RESULT} ${${RESULT}})
    ELSE (WIN32)
        MESSAGE(SEND_ERROR "date not implemented")
        SET(${RESULT} 000000)
    ENDIF (WIN32)
ENDMACRO (TODAY)

find_program(DATE date)
if(DATE)
  execute_process(COMMAND ${DATE} "+%F"
    OUTPUT_VARIABLE DATE_STR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
else()
  #execute_process(COMMAND echo %DATE% %TIME% OUTPUT_VARIABLE DATE_STR)
  TODAY(DATE_STR)
  string(STRIP "${DATE_STR}" DATE_STR)
endif()
find_program(WHO whoami)
if(WHO)
  execute_process(COMMAND ${WHO}
    OUTPUT_VARIABLE WHO_STR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()
find_program(HOSTNAME hostname)
if(HOSTNAME)
  execute_process(COMMAND ${HOSTNAME}
    OUTPUT_VARIABLE HOST_STR
    OUTPUT_STRIP_TRAILING_WHITESPACE
  )
endif()

file(READ "${VERSION_IN_FILE}" VERSION)
string(STRIP "${VERSION}" VERSION)
set(CONTENTS "DAYSIM lastmod ${DATE_STR} by ${WHO_STR} on ${HOST_STR} (based on RADIANCE ${VERSION} by G. Ward)")
message("${CONTENTS}")
string(REPLACE "\\" "\\\\" CONTENTS "${CONTENTS}") # look for instances of the escape character
file(WRITE "${VERSION_OUT_FILE}" "char VersionID[]=\"${CONTENTS}\";\n")
