# generate_config_site.cmake
set(CONFIG_SITE_CONTENT "
#ifndef _LIBCPP_CONFIG_SITE
#define _LIBCPP_CONFIG_SITE

// Disable threading support
#define _LIBCPP_HAS_NO_THREADS

// Disable exceptions
#define _LIBCPP_NO_EXCEPTIONS

// Disable RTTI
#define _LIBCPP_NO_RTTI

// Disable filesystem support
#define _LIBCPP_HAS_NO_FILESYSTEM

// Disable locale support
#define _LIBCPP_HAS_NO_LOCALIZATION

// Disable wide character support
#define _LIBCPP_HAS_NO_WIDE_CHARACTERS

// Disable C++20 concepts (optional)
#define _LIBCPP_HAS_NO_CONCEPTS

// Disable C++20 coroutines (optional)
#define _LIBCPP_HAS_NO_COROUTINES

#endif // _LIBCPP_CONFIG_SITE
")

# Write the content to __config_site
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/__config_site" "${CONFIG_SITE_CONTENT}")