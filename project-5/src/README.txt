
2 new system calls made for protect and unprotect

In unprotect we do something similar but run it with multiple iterations

readonlycode2 works but readonlycode1 and null1 do not

Added proc.c int mprotect and int munprotect in defs.h

mprotect in vm.c checks length of page, then checks if page is aligned, then loops through each page, then clears writing bit so that the page can't be written, and after the end of the loop puts address of the page directory in control register

munprotect makes the page RW unlike mprotect