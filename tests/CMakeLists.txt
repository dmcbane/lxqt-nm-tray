find_package(Qt6 REQUIRED COMPONENTS Test)

foreach (name popupanchor settings statustext)
    add_executable(tst_${name} tst_${name}.cpp)
    target_link_libraries(tst_${name} PRIVATE lxqtnmtray_logic Qt6::Test)
    add_test(NAME ${name} COMMAND tst_${name})
    set_tests_properties(${name} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endforeach ()

if (BUILD_PLUGIN)
    add_executable(tst_pluginload tst_pluginload.cpp)
    target_include_directories(tst_pluginload PRIVATE ${LXQT_PANEL_INCLUDE_DIR}/lxqt)
    target_compile_definitions(tst_pluginload PRIVATE PLUGIN_PATH="$<TARGET_FILE:nmtray>")
    target_link_libraries(tst_pluginload PRIVATE Qt6::Widgets Qt6::Test)
    add_dependencies(tst_pluginload nmtray)
    add_test(NAME pluginload COMMAND tst_pluginload)
    set_tests_properties(pluginload PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endif ()

# The Wi-Fi password prompt lives in nm-tray's nmmodel.cpp (anonymous namespace, modal dialog),
# so it can't be unit tested. This guards against bumping the submodule to a revision that
# puts the SSID unescaped into the rich-text-capable QLabel again.
add_test(NAME nmtray_core_escapes_ssid_in_password_prompt
    COMMAND grep -q "Password is needed for connection to '%1':\").arg(accessPoint->ssid().toHtmlEscaped())"
            ${NM_TRAY_DIR}/src/nmmodel.cpp)
