# isolate-network

A tool for running programs in their own network namespace

`isolate-network cmd arg1 arg2 ...` is equivalent to `ip netns add <name> && ip netns exec <name> ip link set lo up && ip netns exec <name> cmd arg1 arg2 ... ; ip netns delete <name>` except that:

1. It does not require root/`CAP_SYS_ADMIN`
2. The namespace does not actually have a name, and cannot be used in `ip netns` commands

This is mainly useful for test suites that use hard-coded ports which are already in use, and for proprietary programs which perform blocking requests to domains which no longer respond.
