#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <mach/audit_token.h>
#include <sandbox.h>          // cần thiết cho sandbox_check
#include "fishhook.h"

// Khai báo hàm hook
int (*orig_sandbox_check)(const char *path, const char *operation, ...);
int (*orig_sandbox_check_with_audit_token)(audit_token_t token, const char *path, const char *operation, ...);

// Hàm thay thế
int my_sandbox_check(const char *path, const char *operation, ...) {
    return 0; // luôn cho phép
}

int my_sandbox_check_with_audit_token(audit_token_t token, const char *path, const char *operation, ...) {
    return 0;
}

// Constructor
__attribute__((constructor)) void init() {
    // Hook các hàm sandbox
    struct rebinding rebindings[] = {
        {"sandbox_check", my_sandbox_check, (void *)&orig_sandbox_check},
        {"sandbox_check_with_audit_token", my_sandbox_check_with_audit_token, (void *)&orig_sandbox_check_with_audit_token}
    };
    rebind_symbols(rebindings, 2);

    // Load Frida gadget
    void *handle = dlopen("@loader_path/frida-gadget.dylib", RTLD_NOW);
    if (handle) {
        void (*frida_init)(void) = dlsym(handle, "frida_init");
        if (frida_init) frida_init();
    }
}
