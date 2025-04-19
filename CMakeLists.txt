cmake_minimum_required(VERSION 3.10)
project(itush)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(include)

set(SRC ${CMAKE_SOURCE_DIR}/source)

set(SOURCES
	${SRC}/linkedlist.c
	${SRC}/free.c
	${SRC}/check.c
	${SRC}/display.c
	${SRC}/lexer/lex.yy.c
	${SRC}/lexer/bison.tab.c
	${SRC}/parser/parser.c
	${SRC}/executor/executor.c
)

add_executable(${PROJECT_NAME} ${SOURCES})
