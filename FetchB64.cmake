include(FetchContent)

# Declare our target. We want the lastest stable version, not the master.
# Also specify GIT_SHALLOW to avoid cloning branch we don't care about
FetchContent_Declare(
  b64
  GIT_REPOSITORY https://github.com/jwerle/b64.c.git
  GIT_TAG        0.1.0
  GIT_SHALLOW    1
)

# Download cmocka, and execute its cmakelists.txt
FetchContent_MakeAvailable(b64)
