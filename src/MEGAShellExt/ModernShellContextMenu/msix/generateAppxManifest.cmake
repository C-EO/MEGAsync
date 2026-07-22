set(AppxManifestPath "${CMAKE_CURRENT_LIST_DIR}/AppxManifest.xml")
set(MSIPath "${OutputPath}/msi")
set(MSIName "MEGAShellExt.msix")
set(AssetsFolder "assets")

if(NOT EXISTS "${MSIPath}")
    file(MAKE_DIRECTORY ${MSIPath})
endif()

# Copy the assets folder to the build dir (and remove the previous one, in case it is outdated)
file(REMOVE_RECURSE "${MSIPath}/${AssetsFolder}")
file(COPY "${CMAKE_CURRENT_LIST_DIR}/${AssetsFolder}" DESTINATION "${MSIPath}/")
message(STATUS "Assets folder copied")

set(MEGA_DESKTOP_APP_CERTIFICATE_PUBLISHER $ENV{MEGA_DESKTOP_APP_CERTIFICATE_PUBLISHER})

message (STATUS "Using certificate publisher: ${MEGA_DESKTOP_APP_CERTIFICATE_PUBLISHER}")

# AppxManifest.xml must be generated BEFORE makepri new is invoked, because
# makepri reads the package Identity Name from it (via /mn) and uses it as
# the resource-map name inside resources.pri. The resource-map name MUST
# match the package Identity Name, otherwise WACK fails with hr = 0x80073B1F
# (APPX_E_INVALID_RESOURCE).
configure_file(${CMAKE_CURRENT_LIST_DIR}/AppxManifest.xml.in ${MSIPath}/AppxManifest.xml @ONLY)

execute_process(
    COMMAND makepri createconfig "/o" "/cf" "${MSIPath}/priconfig.xml" "/dq" "en-US"
)

execute_process(
    COMMAND makepri new "/cf" "${MSIPath}/priconfig.xml" "/pr" "${MSIPath}"
    "/mn" "${MSIPath}/AppxManifest.xml" "/o" "/of" "${MSIPath}/resources.pri"
)

# Create msix package in the root build dir.
#
# /nv (no validation) is required for sparse packages: makeappx's pack-time
# validator does not honor <uap10:AllowExternalContent> for the Application
# Executable= attribute nor for COM Class Path= when those point at files at
# the ExternalLocationUri (i.e. MEGAsync.exe and the shell-ext DLL). Without
# /nv, pack fails with "The file name 'MEGAsync.exe' ... doesn't exist in
# the package". This matches Microsoft's official SparsePackage sample.
#
# Trade-off: /nv disables ALL manifest validation, so manifest/asset
# mismatches (e.g. a Logo PNG referenced in the manifest but missing from
# the assets folder) will not be caught at build time and will only surface
# during WACK / Store certification.
execute_process(
    COMMAND makeappx pack "/nv" "/o" "/d" "${MSIPath}" "/p" "${OutputPath}/${MSIName}"
    RESULT_VARIABLE makeappx_result
)
if(NOT makeappx_result EQUAL 0)
    message(FATAL_ERROR "makeappx pack failed (exit ${makeappx_result}). See output above.")
endif()
