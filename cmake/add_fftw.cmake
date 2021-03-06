if (WIN32)
  message(FATAL_ERROR "The fftw backend is not supported on Windows. Please use the mkl.")
endif ()

set(fftw_dir ${CMAKE_CURRENT_BINARY_DIR}/fftw)
set(fftw_install_dir ${CMAKE_CURRENT_BINARY_DIR}/fftw/build)

if (NOT EXISTS ${fftw_dir})
  message(STATUS "Downloading FFTW sources")
  set(zip_file ${fftw_dir}/fftw.tar.gz)
  file(DOWNLOAD "http://www.fftw.org/fftw-3.3.7.tar.gz" ${zip_file} SHOW_PROGRESS)
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf fftw.tar.gz
                  WORKING_DIRECTORY ${fftw_dir})

  # Configure
  set(fftw_configure_command ./configure --prefix=${fftw_install_dir} --enable-static --enable-float)
  if (CMAKE_CXX_FLAGS)
    set(fftw_configure_command ${fftw_configure_command} CPPFLAGS=${CMAKE_CXX_FLAGS})
  endif()
  if (CMAKE_C_FLAGS)
    set(fftw_configure_command ${fftw_configure_command} CFLAGS=${CMAKE_C_FLAGS})
  endif()
  if (rtff_fftw_extra_configure_flags)
    set(fftw_configure_command ${fftw_configure_command} ${rtff_fftw_extra_configure_flags})
  endif()

  message(STATUS "FFTW - Configure: ${fftw_configure_command}")
  execute_process(
    COMMAND ${fftw_configure_command}
    WORKING_DIRECTORY ${fftw_dir}/fftw-3.3.7/
  )

  message(STATUS "FFTW - Make")
  execute_process(
    COMMAND make
    WORKING_DIRECTORY ${fftw_dir}/fftw-3.3.7/
  )

  message(STATUS "FFTW - Install")
  execute_process(
    COMMAND make install
    WORKING_DIRECTORY ${fftw_dir}/fftw-3.3.7/
  )

  find_library(fftw3f
    NAMES fftw3f
    PATHS ${fftw_install_dir}/lib
    NO_DEFAULT_PATH
  )
  message(STATUS "FFTW - Found at ${fftw3f}")
else()
  find_library(fftw3f
    NAMES fftw3f
    PATHS ${fftw_install_dir}/lib
    NO_DEFAULT_PATH
  )
  message(STATUS "FFTW - Found at ${fftw3f}")
endif()

# -- Create lib target
add_library(fftw INTERFACE)
target_link_libraries(fftw
  INTERFACE
    ${fftw3f}
)
target_include_directories(fftw
  INTERFACE
    ${fftw_install_dir}/include
)
install(FILES ${fftw3f} DESTINATION lib)

set(fftw_libraries fftw)
