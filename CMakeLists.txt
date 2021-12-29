cmake_minimum_required(VERSION 3.19 FATAL_ERROR)

# ---- Project ----

# Note: update this to your new project's name and version
project(
  VulkanLearn
  VERSION 1.0
  LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- Include guards ----

if(PROJECT_SOURCE_DIR STREQUAL PROJECT_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()


# ---- Add dependencies via CPM ----
# see https://github.com/TheLartians/CPM.cmake for more info
include(cmake/CPM.cmake)

# PackageProject.cmake will be used to make our target installable
CPMAddPackage("gh:TheLartians/PackageProject.cmake@1.8.0")

CPMAddPackage(
  NAME spdlog
  GIT_TAG v1.9.2
  GITHUB_REPOSITORY gabime/spdlog
  OPTIONS "SPDLOG_INSTALL YES" # create an installable target
)
set_target_properties(spdlog PROPERTIES FOLDER "third_party" )

CPMAddPackage(
  NAME glfw
  GIT_TAG 3.3.5
  GITHUB_REPOSITORY glfw/glfw
  OPTIONS "GLFW_BUILD_EXAMPLES OFF" "GLFW_BUILD_TESTS OFF" "GLFW_BUILD_DOCS OFF" "GLFW_VULKAN_STATIC YES" "GLFW_INSTALL YES"# create an installable target
)
set_target_properties(glfw PROPERTIES FOLDER "third_party" )
set_target_properties(uninstall PROPERTIES FOLDER "third_party" )
set_target_properties(update_mappings PROPERTIES FOLDER "third_party" )

CPMAddPackage(
  NAME GLM
  GIT_TAG 0.9.9.8
  GITHUB_REPOSITORY g-truc/glm
  DOWNLOAD_ONLY True
)

if(GLM_ADDED)
  set(GLM_SOURCE_DIR_ ${GLM_SOURCE_DIR})
  set(GLM_BINARY_DIR_ ${GLM_BINARY_DIR})
  add_library(GLM INTERFACE)
  target_include_directories(GLM INTERFACE $<BUILD_INTERFACE:${GLM_SOURCE_DIR_}>
                         $<INSTALL_INTERFACE:include/GLM-0.9.9.8>)
  set_target_properties(GLM PROPERTIES FOLDER "third_party" )

  packageProject(
    NAME GLM
    VERSION 0.9.9.8
    NAMESPACE GLM
    BINARY_DIR ${GLM_BINARY_DIR_}
    INCLUDE_DIR ${GLM_SOURCE_DIR_}
    INCLUDE_DESTINATION include/GLM-0.9.9.8
    VERSION_HEADER "${VERSION_HEADER_LOCATION}"
    COMPATIBILITY SameMajorVersion
  )
endif()


CPMAddPackage(
  NAME imgui
  GIT_TAG v1.85
  GITHUB_REPOSITORY ocornut/imgui
  DOWNLOAD_ONLY True
)

if(imgui_ADDED)
  set(imgui_SOURCE_DIR_ ${imgui_SOURCE_DIR})
  set(imgui_BINARY_DIR_ ${imgui_BINARY_DIR})
  
  file(GLOB imgui_sources CONFIGURE_DEPENDS  "${imgui_SOURCE_DIR_}/*.cpp")
  file(GLOB imgui_impl CONFIGURE_DEPENDS  
  "${imgui_SOURCE_DIR_}/backends/imgui_impl_glfw.cpp" 
  "${imgui_SOURCE_DIR_}/backends/imgui_impl_glfw.h"
  "${imgui_SOURCE_DIR_}/backends/imgui_impl_vulkan.cpp" 
  "${imgui_SOURCE_DIR_}/backends/imgui_impl_vulkan.h")
  add_library(imgui STATIC ${imgui_sources} ${imgui_impl})

  target_include_directories(imgui PUBLIC $<BUILD_INTERFACE:${imgui_SOURCE_DIR_}> $<INSTALL_INTERFACE:include/imgui-1.85>)
  
  target_link_libraries(imgui PUBLIC glfw Vulkan::Vulkan)

  set_target_properties(imgui PROPERTIES FOLDER "third_party" )

  packageProject(
    NAME imgui
    VERSION 1.85
    NAMESPACE imgui
    BINARY_DIR ${imgui_BINARY_DIR_}
    INCLUDE_DIR ${imgui_SOURCE_DIR_}
    INCLUDE_DESTINATION include/imgui-1.85
    VERSION_HEADER "${VERSION_HEADER_LOCATION}"
    COMPATIBILITY SameMajorVersion
  )
endif()
# ---- Add source files ----

# Note: globbing sources is considered bad practice as CMake's generators may not detect new files
# automatically. Keep that in mind when changing files, or explicitly mention them here.
file(GLOB_RECURSE headers CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")
file(GLOB_RECURSE sources CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/source/*.h")
file(GLOB_RECURSE shaders CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.vert"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.frag"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.comp"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.geom"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.tesc"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.tese"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.mesh"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.task"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.rgen"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.rchit"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.rmiss"
                                            "${CMAKE_CURRENT_SOURCE_DIR}/shader/*.rcall")
# ---- Create library ----

# Note: for header-only libraries change all PUBLIC flags to INTERFACE and create an interface
# target: add_library(${PROJECT_NAME} INTERFACE)
add_library(${PROJECT_NAME} ${headers} ${sources} ${shaders})
SOURCE_GROUP("Shader Files" FILES ${shaders})

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 17)

# being a cross-platform target, we enforce standards conformance on MSVC
target_compile_options(${PROJECT_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->")

# Link dependencies
find_package(Vulkan REQUIRED COMPONENTS glslc FATAL_ERROR)
target_link_libraries(${PROJECT_NAME} PRIVATE spdlog::spdlog)
target_link_libraries(${PROJECT_NAME} PUBLIC GLM)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)

find_program(glslc_executable NAMES glslc HINTS Vulkan::glslc)
set(EMBED_SHADER_FOLDER "generated_embed_shader")
	# For each shader, we create a header file
	foreach(SHADER ${shaders})

		# Prepare a header name and a global variable for this shader
		get_filename_component(SHADER_NAME ${SHADER} NAME)
		string(REPLACE "." "_" HEADER_NAME ${SHADER_NAME})
		string(TOUPPER ${HEADER_NAME} GLOBAL_SHADER_VAR)

		set(SHADER_HEADER "${PROJECT_SOURCE_DIR}/${EMBED_SHADER_FOLDER}/${HEADER_NAME}.h")

		add_custom_target(
			${HEADER_NAME}
			# Compile any GLSL shader into SPIR-V shader
			COMMAND ${glslc_executable} ${SHADER} -o ${PROJECT_SOURCE_DIR}/${EMBED_SHADER_FOLDER}/${SHADER_NAME}.spv
			# Make a C header file with the SPIR-V shader
			COMMAND ${CMAKE_COMMAND} -DPATH="${PROJECT_SOURCE_DIR}/${EMBED_SHADER_FOLDER}/${SHADER_NAME}.spv" -DHEADER="${SHADER_HEADER}" -DGLOBAL="${GLOBAL_SHADER_VAR}" -P "${CMAKE_CURRENT_SOURCE_DIR}/embed_resource.cmake"
			# Rebuild the header file if the shader is updated
			DEPENDS ${SHADER}
			COMMENT "Building ${PROJECT_SOURCE_DIR}/${EMBED_SHADER_FOLDER}/${SHADER_NAME}.spv and embedding it into ${SHADER_HEADER}"
		)

    set_target_properties(${HEADER_NAME} PROPERTIES FOLDER "embed_shaders" )
		# Add the custom target like a dependencies of the project
		add_dependencies(${PROJECT_NAME} ${HEADER_NAME})

		message(STATUS "Generating build commands for ${SHADER}")
	endforeach()
#target_include_directories(${PROJECT_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})

target_include_directories(
  ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:include/${PROJECT_NAME}-${PROJECT_VERSION}>
                         $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/${EMBED_SHADER_FOLDER}>)

add_executable(${PROJECT_NAME}_entry main.cpp)
target_link_libraries( ${PROJECT_NAME}_entry PRIVATE ${PROJECT_NAME} )

# ---- Create an installable target ----
# this allows users to install and find the library via `find_package()`.

# the location where the project's version header will be placed should match the project's regular
# header paths
string(TOLOWER ${PROJECT_NAME}/version.h VERSION_HEADER_LOCATION)

packageProject(
  NAME ${PROJECT_NAME}
  VERSION ${PROJECT_VERSION}
  NAMESPACE ${PROJECT_NAME}
  BINARY_DIR ${PROJECT_BINARY_DIR}
  INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include
  INCLUDE_DESTINATION include/${PROJECT_NAME}-${PROJECT_VERSION}
  VERSION_HEADER "${VERSION_HEADER_LOCATION}"
  COMPATIBILITY SameMajorVersion
  DEPENDENCIES "fmt 7.1.3"
  DEPENDENCIES "spdlog v1.9.2"
)
