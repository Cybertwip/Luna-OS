#pragma once

#include <stdint.h>
#include "Graphics.h"

// GPU Commands
enum GPUCommand {
    GPU_CMD_CLEAR,
    GPU_CMD_DRAW_TRIANGLE,
    GPU_CMD_SET_MVP
};

// GPU Command Buffer
struct GPUCommandBuffer {
    uint32_t* buffer;
    uint32_t capacity;
    uint32_t size;
};

// Initialize GPU
void gpu_init(GPUCommandBuffer* cb, uint32_t capacity);
void gpu_submit_command(GPUCommandBuffer* cb, uint32_t command, const void* data);
void gpu_process_commands(GPUCommandBuffer* cb, Graphics* gfx);