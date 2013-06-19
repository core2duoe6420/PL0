#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifndef __LINUX__

int compile_asm_link(char * asm_file, char * obj_file, char * exe_file)
{
    char cmd[1024];
    sprintf(cmd, "nasm -f win32 %s -o %s", asm_file, obj_file);
    system(cmd);

    if(exe_file)
        sprintf(cmd, "link %s libcmt.lib /OUT:%s", obj_file, exe_file);
    else
        sprintf(cmd, "link %s libcmt.lib", obj_file);
    system(cmd);
    return 0;
}

#else

int compile_asm_link(char * asm_file, char * obj_file, char * exe_file)
{
    char cmd[1024];
    sprintf(cmd, "nasm -f elf %s -o %s", asm_file, obj_file);
    system(cmd);

    if(exe_file)
        sprintf(cmd, "gcc %s -o %s", obj_file, exe_file);
    else
        sprintf(cmd, "gcc %s", obj_file);
    system(cmd);
    return 0;
}

#endif