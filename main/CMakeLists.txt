idf_component_register(
    SRCS 
        "app_main.c"
        "./src"
        "./src/Bluetooth/bluetooth.c"

    INCLUDE_DIRS 
        "."
        "./include"
        "./include/Bluetooth"

    EMBED_FILES

    EMBED_TXTFILES
    
)