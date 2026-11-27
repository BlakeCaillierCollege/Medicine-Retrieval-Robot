# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.cpp)

set(page_directory "${CMAKE_CURRENT_LIST_DIR}/../data")
set(page_files
    "${page_directory}/page.html"
    "${CMAKE_CURRENT_LIST_DIR}/controlServer/page.js"
    "${page_directory}/page.css")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${page_files})
file(READ "${page_directory}/page.html" PAGE_HTML)
file(READ "${CMAKE_CURRENT_LIST_DIR}/controlServer/page.js" PAGE_JS)
file(READ "${page_directory}/page.css" PAGE_CSS)
set(page_generated_dir "${CMAKE_CURRENT_LIST_DIR}/../.pio/generated")
file(MAKE_DIRECTORY "${page_generated_dir}")
configure_file("${CMAKE_CURRENT_LIST_DIR}/controlServer/page_assets.cpp.in"
               "${page_generated_dir}/page_assets.cpp" @ONLY)

idf_component_register(SRCS ${app_sources}
                            "${page_generated_dir}/page_assets.cpp"
                       INCLUDE_DIRS "controlServer"
                                    "espControls"
                       REQUIRES esp_wifi
                                esp_event
                                esp_netif
                                esp_http_server
                                nvs_flash)


