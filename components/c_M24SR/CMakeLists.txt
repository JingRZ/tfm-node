idf_component_register(
    SRCS 
        "i2c_config.c" 
        "c_M24SR.c"
        "cmdFactory.c"    
        "Message.c"
        "Record.c"
        "RecordHeader.c"
        "RecordText.c"        
        "EmptyRecord.c"  
    INCLUDE_DIRS "include"
    REQUIRES 
        driver
)