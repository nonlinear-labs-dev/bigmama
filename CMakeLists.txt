cmake_minimum_required(VERSION 3.0)
project(bigmama)

set(LIBS
  glibmm-2.4
  giomm-2.4
  glib-2.0
  gio-2.0
  alsa
  )

function(addLib name)
  pkg_check_modules(${name} REQUIRED ${name})
  include_directories(${${name}_INCLUDE_DIRS})
  link_directories(${${name}_LIBRARY_DIRS})
endfunction(addLib)

find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

foreach(lib ${LIBS})
  addLib(${lib})
endforeach(lib)

include_directories(src)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wdouble-promotion")

file(GLOB_RECURSE SOURCE_FILES src/*.cpp)

add_executable(bigmama ${SOURCE_FILES})

TARGET_LINK_LIBRARIES(bigmama pthread)

function(linkLib name)
  pkg_check_modules(${name} REQUIRED ${name})
  target_link_libraries(bigmama ${${name}_LIBRARIES})
endfunction(linkLib)

foreach(lib ${LIBS})
  linkLib(${lib})
endforeach(lib)

install(TARGETS bigmama
  DESTINATION ${CMAKE_INSTALL_PREFIX}/nonlinear/bigmama
  )

ADD_CUSTOM_TARGET(
	touch-cmakelists ALL
	COMMAND ${CMAKE_COMMAND} -E touch ${CMAKE_CURRENT_LIST_FILE}
)


