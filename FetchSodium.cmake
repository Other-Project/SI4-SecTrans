include(FetchContent)

FetchContent_Declare(Sodium
    GIT_REPOSITORY https://github.com/robinlinden/libsodium-cmake.git
    GIT_TAG 99f14233eab1d4f7f49c2af4ec836f2e701c445e
)
set(SODIUM_DISABLE_TESTS ON)
FetchContent_MakeAvailable(Sodium)
