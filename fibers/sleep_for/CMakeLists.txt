begin_task()

# Dependencies
task_link_libraries(asio)

# Library
add_task_library(exe)

# Playground
add_playground(play)

# Tests

# Coroutine
add_task_test_dir(tests/coroutine coroutine_tests)

# Fibers
add_task_test_dir(tests/fibers/yield fibers_yield_tests)
add_task_test_dir(tests/fibers/sleep_for-1 fibers_sleep_for_tests-1)
add_task_test_dir(tests/fibers/sleep_for-2 fibers_sleep_for_tests-2)

end_task()
