cmake_minimum_required(VERSION 3.5)	 # Проверка версии CMake.
										# Если версия установленой программы
										# старее указаной, произайдёт аварийный выход.

project(luapng C)			# Название проекта

find_package(Lua REQUIRED)
find_package(PNG REQUIRED)

add_library(luapng SHARED src/luapng.c)
target_link_libraries(luapng PRIVATE lua PNG::PNG)

install(TARGETS luapng
		RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}
		LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}
		ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX})