add_executable (http-client-async-ssl
        ../root_certificates.hpp
        main.cpp
    )

target_link_libraries (http-client-async-ssl
    ${OPENSSL_LIBRARIES}
    pthread
)