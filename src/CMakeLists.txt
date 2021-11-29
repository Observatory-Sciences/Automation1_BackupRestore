add_library(src backuprestore.cxx)

target_include_directories(src
                           PUBLIC "${PROJECT_SOURCE_DIR}/include"
                           )
                           
install(TARGETS src DESTINATION ${PROJECT_SOURCE_DIR}/lib)