# Embed the server root certificate into the final binary
#
# (If this was a component, we would set COMPONENT_EMBED_TXTFILES here.)
set(requires "")
idf_build_get_property(target IDF_TARGET)

list(APPEND requires
    esp_system
    esp_event
    driver
    nvs_flash
    esp_netif
    esp_wifi
    esp_http_client
    esp_http_server
    protocol_examples_common
    esp-tls
    # This
    Core
)

file(GLOB_RECURSE app_sources "Src/*.c")

idf_component_register(
    SRCS ${app_sources}
    INCLUDE_DIRS "Inc"
    REQUIRES ${requires}
)
