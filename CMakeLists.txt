cmake_minimum_required(VERSION 3.10...3.31)

project(
    libsmackerdec
    LANGUAGES CXX
    VERSION 1.0.0.1
    HOMEPAGE_URL "http://homepage.eircom.net/~duncandsl/avp/"
    DESCRIPTION "a smacker video decoder library base on FFMPEG")

# ============================================================================
# CPack
# ============================================================================
set(CPACK_PACKAGE_VENDOR "smackerdec-Project")
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README)
set(CPACK_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/COPYING)
set(CPACK_RESOURCE_FILE_README ${CMAKE_CURRENT_SOURCE_DIR}/README)

# ============================================================================
# configuration
# ============================================================================

option(BUILD_SHARED "Enable building smackerdec shared library" ON)
option(BUILD_STATIC "Enable building smackerdec static library" ON)
option(CREATE_INSTALL "Enable installation of smackerdec" ON)
if(WIN32)
    option(INSTALL_COMPAT_DLL "Install a copy as libsmackerdec.dll" ON)
endif(WIN32)

get_property(IS_MULTI GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)

if(NOT DEFINED CMAKE_BUILD_TYPE AND NOT IS_MULTI)
    message(STATUS "No CMAKE_BUILD_TYPE set -- using Release")
    set(CMAKE_BUILD_TYPE Release)
endif(NOT DEFINED CMAKE_BUILD_TYPE AND NOT IS_MULTI)

include(CMakePackageConfigHelpers)
include(CPack)
include(GNUInstallDirs)

set(CPACK_INCLUDED TRUE)

set(SMACKERDEC_PC ${CMAKE_BINARY_DIR}/smackerdec.pc)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/smackerdec.pc.cmakein ${SMACKERDEC_PC} @ONLY)
include_directories(${CMAKE_BINARY_DIR} ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/include)

# ============================================================================
# smackerdec
# ============================================================================

set(SMACKERDEC_HDRS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/BitReader.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/FileStream.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/HuffmanVLC.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/LogError.h"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/SmackerDecoder.h")

set(SMACKERDEC_SRCS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/BitReader.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/FileStream.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/HuffmanVLC.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/LogError.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/src/SmackerDecoder.cpp")

if(WIN32)
    set(smackerdec_static_suffix "s")
    set(CMAKE_DEBUG_POSTFIX "-d")
endif(WIN32)

if(BUILD_SHARED)
    add_library(
        smackerdec_dynamic SHARED ${SMACKERDEC_SRCS} ${SMACKERDEC_HDRS}
                    $<$<OR:$<BOOL:${WIN32}>,$<BOOL:${CYGWIN}>>:${CMAKE_CURRENT_SOURCE_DIR}/win32/smackerdec.def>)
    add_library(SMACKERDEC::SMACKERDEC ALIAS smackerdec_dynamic)
    target_include_directories(
        smackerdec_dynamic
        PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

    set(INSTALL_VERSION ${smackerdec_VERSION})

    if(NOT CYGWIN)
        set_target_properties(smackerdec_dynamic PROPERTIES SOVERSION ${smackerdec_VERSION_MAJOR}
                                              VERSION ${INSTALL_VERSION})
    endif(NOT CYGWIN)

    set_target_properties(
        smackerdec_dynamic
        PROPERTIES DEFINE_SYMBOL SMACKERDEC_DLL
                   EXPORT_NAME SMACKERDEC
                   OUTPUT_NAME libsmackerdec
                   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
endif(BUILD_SHARED)

if(BUILD_STATIC)
    add_library(smackerdec_static STATIC ${SMACKERDEC_SRCS} ${SMACKERDEC_HDRS})
    add_library(SMACKERDEC::SMACKERDECSTATIC ALIAS smackerdec_static)
    target_include_directories(
        smackerdec_static
        PUBLIC $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}>
               $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
    set_target_properties(
        smackerdec_static
        PROPERTIES EXPORT_NAME SMACKERDECSTATIC
                   OUTPUT_NAME smackerdec${smackerdec_static_suffix}
                   RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/lib)
endif(BUILD_STATIC)

if(CREATE_INSTALL)
    if(BUILD_SHARED)
        install(
            TARGETS smackerdec_dynamic
            COMPONENT Runtime
            EXPORT smackerdecSharedExport
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
        install(
            EXPORT smackerdecSharedExport
            FILE SMACKERDEC-shared.cmake
            NAMESPACE SMACKERDEC::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smackerdec)
        if(INSTALL_COMPAT_DLL)
            install(
                FILES $<TARGET_FILE:smackerdec_dynamic>
                COMPONENT Runtime
                RENAME libsmackerdec.dll
                DESTINATION "${CMAKE_INSTALL_BINDIR}")
        endif(INSTALL_COMPAT_DLL)

        if(MSVC)
            install(
                FILES $<TARGET_PDB_FILE:smackerdec_dynamic>
                COMPONENT Runtime
                DESTINATION ${CMAKE_INSTALL_BINDIR}
                CONFIGURATIONS Debug OR RelWithDebInfo
                OPTIONAL)
        endif(MSVC)
    endif(BUILD_SHARED)

    if(BUILD_STATIC)
        install(
            TARGETS smackerdec_static
            COMPONENT Development
            EXPORT smackerdecStaticExport
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}")
        install(
            EXPORT smackerdecStaticExport
            FILE SMACKERDEC-static.cmake
            NAMESPACE SMACKERDEC::
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smackerdec)

        if(INSTALL_COMPAT_DLL AND MINGW)
            install(
                FILES $<TARGET_FILE:smackerdec_static>
                COMPONENT Development
                RENAME libsmackerdec.dll.a
                DESTINATION "${CMAKE_INSTALL_LIBDIR}")
        endif(INSTALL_COMPAT_DLL AND MINGW)
    endif(BUILD_STATIC)

    configure_package_config_file(
        ${CMAKE_SOURCE_DIR}/smackerdecConfig.cmake.in
        ${CMAKE_BINARY_DIR}/SMACKERDECConfig.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smackerdec)

    write_basic_package_version_file(
        "${CMAKE_BINARY_DIR}/SMACKERDECConfigVersion.cmake"
        VERSION "${smackerdec_VERSION}"
        COMPATIBILITY AnyNewerVersion)

    install(FILES ${CMAKE_BINARY_DIR}/SMACKERDECConfig.cmake
                  ${CMAKE_BINARY_DIR}/SMACKERDECConfigVersion.cmake
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/smackerdec)
    install(
        FILES ${SMACKERDEC_HDRS}
        COMPONENT Development
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")
    install(
        FILES ${SMACKERDEC_PC}
        COMPONENT Development
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif(CREATE_INSTALL)
