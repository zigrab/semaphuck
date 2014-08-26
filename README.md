#Semaphuck

Semaphuck is a proof of concept application designed to automate breaking programs with improper permissions on their global semaphores

This is accomplished by attempting to randomly wait and post all global semaphores

<sup>Inspired by an anonymous individual whose original implementation incensed an entire yeargroup</sup>

:warning:  ***May cause system instability***


##Compilation

Just execute `make`

Three builds are created:
 * `denum` - Debug build - includes gdb debug symbols and `-fstack-protector-all`
 * `enum`  - Normal build
 * `oenum` - O3 build - enables `-O3`, frame pointers and silences extraneous output

##Usage

```
Usage: ./enum [-huc]
-h, --help              This text
-u, --unattended        Continuously rescan for additional semaphores
-c, --create-mutex      Create a permissive semaphore to demonstrate what can happen
```

##Future work
- [ ] Make use of pthreads instead of forking to lower resource consumption
- [ ] Increase speed of mutex operations and possibly add more intentional deadlock creators
- [ ] Link with libjumper for even more **fun**

