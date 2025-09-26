# get_target_property(MegaDirt_VST3_PATH  MegaDirt_VST3 JUCE_PLUGIN_ARTEFACT_FILE)
# set(CMAKE_INSTALL_PREFIX "packager")
# install(FILES "${MegaDirt_VST3_PATH}" DESTINATION "VST3" COMPONENT MegaDirt_VST3)
# install(TARGETS MegaDirt_Standalone DESTINATION "Standalone" COMPONENT MegaDirt_Standalone)

set(DIST_DIR ${CMAKE_BINARY_DIR}/dist)
file(MAKE_DIRECTORY ${DIST_DIR})

add_custom_target(dist)
add_custom_target(installer)

function(package format)
    get_target_property(output_dir ${PROJECT_NAME} RUNTIME_OUTPUT_DIRECTORY)

    if(TARGET ${PROJECT_NAME}_${format})
        add_dependencies(dist ${PROJECT_NAME}_${format})
        add_custom_command(
                TARGET dist
                POST_BUILD
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                COMMAND echo "Installing ${output_dir}/${format} to ${DIST_DIR}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory ${output_dir}/${format}/ ${DIST_DIR}/
        )
    endif()
endfunction()

package(VST3)
package(Standalone)

add_dependencies(installer dist)

set(PACKAGE_NAME ${PROJECT_NAME}-${PROJECT_VERSION}-${BUILD_ID}-${CMAKE_SYSTEM_NAME})

add_custom_command(
        TARGET installer
        POST_BUILD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/LICENSE ${DIST_DIR}/
)

add_custom_command(
        TARGET installer
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/installer
        COMMAND ${CMAKE_COMMAND} -E tar cvf ${CMAKE_BINARY_DIR}/installer/${PACKAGE_NAME}.zip --format=zip ${DIST_DIR}
        COMMAND ${CMAKE_COMMAND} -E echo "Artifact in: installer/${PACKAGE_NAME}.zip")