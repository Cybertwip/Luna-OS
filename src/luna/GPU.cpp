#include "GPU.h"
#include "microgl/math/matrix_4x4.h"

void gpu_init(GPUCommandBuffer* cb, uint32_t capacity) {
    cb->buffer = new uint32_t[capacity];
    cb->capacity = capacity;
    cb->size = 0;
}

void gpu_submit_command(GPUCommandBuffer* cb, uint32_t command, const void* data) {
    if (cb->size + 2 > cb->capacity) return; // Simplified
    cb->buffer[cb->size++] = command;
    cb->buffer[cb->size++] = (uint32_t)data; // Store pointer (simplified)
}

void gpu_process_commands(GPUCommandBuffer* cb, Graphics* gfx) {
    uint32_t* ptr = cb->buffer;
    while (ptr < cb->buffer + cb->size) {
        uint32_t cmd = *ptr++;
        uint32_t data = *ptr++;

        switch (cmd) {
            case GPU_CMD_CLEAR: {
                uint32_t color = data;
                uint32_t* buffer = gfx->getBackBuffer();
                for (uint32_t i = 0; i < gfx->getWidth() * gfx->getHeight(); i++)
                    buffer[i] = color;
                break;
            }
            case GPU_CMD_SET_MVP: {
                // Assume data is a pointer to the MVP matrix
                // (In reality, you'd pass the matrix data)
                break;
            }
            case GPU_CMD_DRAW_TRIANGLE: {
                // Assume data is a pointer to triangle vertices
                // (Implement rasterization here)
                break;
            }
        }
    }
    cb->size = 0; // Reset buffer
}