# Kernel Modules

The `kernel_module.c` creates 4 different handle mechanisms for bottom halves of a interrupt treatment.

They are:
* Tasklets
* Workqueue
* Ktimer
* Kthread

All of them will run a generic function called `handler()` that sleep for 500ms.

The **tasklet** will run immediately, the **workqueues** will run the first one immediately and the second scheduled to 5 seconds after the module insertion.
The **Ktimer**, as the name already says, will run also scheduled (the same 5 secs).
The **Kthread** will run in 4 different ways:
1. Immediately without a specified CPU.
1. Immediately with a specified CPU.
1. Immediately without a  an CPU.
1. Scheduled without a  an CPU (5 secs too).

## Compile proccess
1. The `Makefile` will do all the work, you just need to have it installed.

2. In a linux shell run `make` into the cloned folder from this repo.
3. You should see that the compilation proccess will generates some files, now you need to insert the compiled module.
4. `insmod ./kernel_module.ko` will insert your module to kernel's code.
5. Alright, if you've done everything right so far with `dmesg` command you should see the kernel log and the messages from the freshly added module.
6. To remove the loaded kernel module run rmmod kernel_module.

###### Please read the code and if you have any doubts don't hesitate to contact me <mariobomfim@ufu.br>

