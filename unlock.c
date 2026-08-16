#include <stdio.h>
#include <dlfcn.h>
#include <string.h>
#include <mach/audit_token.h>
#include "fishhook.h"

// Con trỏ hàm gốc
int (*orig_sandbox_check)(const char *path, const char *operation, ...);
int (*orig_sandbox_check_with_audit_token)(audit_token_t token, const char *path, const char *operation, ...);

// Hàm thay thế - luôn trả về 0 (cho phép)
int my_sandbox_check(const char *path, const char *operation, ...) {
    return 0;
}

int my_sandbox_check_with_audit_token(audit_token_t token, const char *path, const char *operation, ...) {
    return 0;
}

// Constructor được gọi khi dylib được load
__attribute__((constructor)) void init() {
    // Hook các hàm sandbox
    struct rebinding rebindings[] = {
        {"sandbox_check", my_sandbox_check, (void *)&orig_sandbox_check},
        {"sandbox_check_with_audit_token", my_sandbox_check_with_audit_token, (void *)&orig_sandbox_check_with_audit_token}
    };
    rebind_symbols(rebindings, 2);

    // Tải Frida gadget từ cùng thư mục với dylib này
    void *handle = dlopen("@loader_path/frida-gadget.dylib", RTLD_NOW);
    if (handle) {
        void (*frida_init)(void) = dlsym(handle, "frida_init");
        if (frida_init) frida_init();
    }
}
