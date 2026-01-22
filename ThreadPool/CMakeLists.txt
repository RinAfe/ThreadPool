# CMakeList.txt: проект CMake для ThreadPool; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (ThreadPool "main.cpp" "include/ThreadPool.h" "src/ThreadPool.cpp")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET ThreadPool PROPERTY CXX_STANDARD 20)
endif()

target_include_directories(
	ThreadPool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
	ThreadPool PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
