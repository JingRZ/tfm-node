idf_component_register(
    SRCS 
        "src/main.c"
        "src/init.c"
    INCLUDE_DIRS "include"
    EMBED_TXTFILES ${project_dir}/main/src/server_cert/github_cert.pem

)
file(GLOB_RECURSE EXTRA_SOURCES "src/*.c" "src/handler/**/*.c")
target_sources(${COMPONENT_LIB} PRIVATE ${EXTRA_SOURCES})