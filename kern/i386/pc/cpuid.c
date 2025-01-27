#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define CPUID_GETVENDORSTRING 0
#define CPUID_GETFEATURES 1
#define CPUID_FEAT_EDX_FPU (1 << 0)

// Function to simulate CPUID instruction (simplified and not accurate for all architectures)
static void __cpuid(uint32_t i, uint32_t regs[4]) {
    // This is a dummy implementation since we can't actually call CPUID in C without inline assembly
    regs[0] = regs[1] = regs[2] = regs[3] = 0;
}

// Get CPU Vendor String
char *GetCPUVendor(void) {
    static char str[13] = {0};
    static uint8_t ran = 0;
    if (!ran) {
        uint32_t regs[4];
        
        __cpuid(CPUID_GETVENDORSTRING, regs);
        
        memcpy(&str[0], &regs[1], 4); // EBX
        memcpy(&str[4], &regs[3], 4); // EDX
        memcpy(&str[8], &regs[2], 4); // ECX
        str[12] = '\0';
        ran = 1;
    }
    return str;
}

// Check if CPU supports a feature (very basic simulation)
uint8_t CPUSupportsFeature(uint32_t featureid) {
    uint32_t regs[4];
    __cpuid(CPUID_GETFEATURES, regs);
    return (regs[2] & featureid) != 0;
}

// Check if CPU supports an extended feature (very basic simulation)
uint8_t CPUSupportsFeatureExt(uint32_t featureid) {
    uint32_t regs[4];
    __cpuid(CPUID_GETFEATURES, regs);
    return (regs[3] & featureid) != 0;
}

// Check if CPU supports CPUID (this is a placeholder as we can't implement this accurately in pure C)
bool CPUSupportsCPUID(void) {
    // This is a dummy implementation since we can't actually check this in pure C
    return 1; // Assume CPUID is supported for the sake of this example
}

void GetCPUInfo(void) {
    // Since we can't truly check for CPUID support in C without assembly, we'll just proceed:
    printf("CPU: %s\n", GetCPUVendor());
    printf("CPU has FPU: %d\n", CPUSupportsFeature(CPUID_FEAT_EDX_FPU));
}
