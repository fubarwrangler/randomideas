
add_executable(rsort rsort.c)
add_executable(interp interp.c)

add_executable(readline-test readline-test.c)
add_library(myreadline SHARED readline.c)
target_link_libraries(readline-test myreadline)
