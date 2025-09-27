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
        WORKING_DIRECTORY ${DIST_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/installer
        COMMAND ${CMAKE_COMMAND} -E tar cvf ${CMAKE_BINARY_DIR}/installer/${PACKAGE_NAME}.zip --format=zip .
        COMMAND ${CMAKE_COMMAND} -E echo "Artifact in: installer/${PACKAGE_NAME}.zip")

if (APPLE)
    message(STATUS "Configuring for mac installer")
    add_custom_command(
            TARGET installer
            POST_BUILD
            USES_TERMINAL
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory installer
            COMMAND ${CMAKE_SOURCE_DIR}/asset/installer/make_macos_pkg.sh ${PROJECT_NAME} ${DIST_DIR} ${PROJECT_VERSION} ${CMAKE_BINARY_DIR}/installer ${PACKAGE_NAME}
    )
elseif (WIN32)
    message(STATUS "Configuring for windows installer")
#     add_custom_command(
#             TARGET obxf-installer
#             POST_BUILD
#             WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
#             COMMAND ${CMAKE_COMMAND} -E make_directory installer
#             COMMAND ${CMAKE_SOURCE_DIR}/libs/sst/sst-plugininfra/scripts/installer_win/make_installer.bat "OB-Xf" ${OBXF_PRODUCT_DIR} ${CMAKE_SOURCE_DIR}/resources/installer_win ${CMAKE_BINARY_DIR}/installer "${OBXF_DATE}-${GIT_COMMIT_HASH}" "${CMAKE_SOURCE_DIR}/assets/installer"
#     )
endif()