begin_task()

# Library
add_task_library(exe)

# Playground
add_playground(play)

# Tests

# Thread Pool
add_task_test_dir(tests/executors/thread_pool/unit thread_pool_unit_tests)
add_task_test_dir(tests/executors/thread_pool/stress thread_pool_stress_tests)

# Manual
add_task_test_dir(tests/executors/manual/unit manual_unit_tests)

# Strand
add_task_test_dir(tests/executors/strand/unit strand_unit_tests)
add_task_test_dir(tests/executors/strand/stress strand_stress_tests)
add_task_test_dir(tests/executors/strand/mo strand_mo_tests)

end_task()
