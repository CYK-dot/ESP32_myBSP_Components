idf_component_register(
    SRCS "elink_ping.c"
    INCLUDE_DIRS "mavlink_gen" "include" 
    REQUIRES clib_list
)
