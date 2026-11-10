# The loggers can be used from several threads (e.g. the network one): link the platform thread library
find_package(Threads REQUIRED)

younity_add_module(logger LIBRARIES Threads::Threads)
