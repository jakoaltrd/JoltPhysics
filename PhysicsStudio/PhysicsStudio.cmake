# PhysicsStudio application

set(PHYSICS_STUDIO_ROOT ${PHYSICS_REPO_ROOT}/PhysicsStudio)

set(PHYSICS_STUDIO_SRC_FILES
	${PHYSICS_STUDIO_ROOT}/PhysicsStudio.cpp
	${PHYSICS_STUDIO_ROOT}/PhysicsStudio.h
	${PHYSICS_STUDIO_ROOT}/PhysicsStudioMain.cpp
	${PHYSICS_STUDIO_ROOT}/CustomScenes/BasicPlaygroundTest.cpp
	${PHYSICS_STUDIO_ROOT}/CustomScenes/BasicPlaygroundTest.h
)

source_group(TREE ${PHYSICS_STUDIO_ROOT} FILES ${PHYSICS_STUDIO_SRC_FILES})

add_executable(PhysicsStudio ${PHYSICS_STUDIO_SRC_FILES})
target_include_directories(PhysicsStudio PUBLIC ${PHYSICS_STUDIO_ROOT} ${PHYSICS_REPO_ROOT})
target_link_libraries(PhysicsStudio LINK_PUBLIC Jolt TestFramework)
target_precompile_headers(PhysicsStudio PRIVATE ${PHYSICS_REPO_ROOT}/TestFramework/TestFramework.h)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Windows")
	target_link_options(PhysicsStudio PUBLIC "/SUBSYSTEM:WINDOWS")
endif()

# Copy assets and set working directory
set_property(TARGET PhysicsStudio PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${PHYSICS_REPO_ROOT}")