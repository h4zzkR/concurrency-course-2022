begin_task()

# Library
add_task_library(exe)

# Playground
add_playground(play)

# Tests

# Coroutine
add_task_test_dir(tests/coroutine/standalone coroutine_tests)
add_task_test_dir(tests/coroutine/processors processors_tests)
add_task_test_dir(tests/coroutine/generators generators_tests)

# Thread Pool
add_task_test_dir(tests/thread_pool/unit thread_pool_unit_tests)
add_task_test_dir(tests/thread_pool/stress thread_pool_stress_tests)

# Fibers
add_task_test_dir(tests/fibers/core/ fibers_core_tests)
add_task_test_dir(tests/fibers/racy fibers_racy_tests)

end_task()
