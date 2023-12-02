idf_component_register(
    SRCS 
        "app_main.c"
        "./src"
        "./src/hal"
        "./src/gps/gps.c"

    INCLUDE_DIRS 
        "."
        "./include"
        "./include/hal"
        "./include/gps"

    EMBED_FILES

    EMBED_TXTFILES
    
)