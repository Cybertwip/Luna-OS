extern "C" {
#include "arch/i386/descriptor_tables.h"
#include "arch/i386/multiboot.h"
#include "sys/scheduler.h"
#include "drivers/timer.h"
#include "drivers/voltron.h"
#include <kernel.h>
}

#include "drivers/ide.h"

#include "drivers/fat32.h"

#include "luna/Graphics.h"

#define PORTABLEGL_IMPLEMENTATION
#define PGL_DISABLE_COLOR_MASK
#include "portablegl.h"

#include <stdio.h>

#define WIDTH 1024
#define HEIGHT 768

#ifndef FPS_EVERY_N_SECS
#define FPS_EVERY_N_SECS 2
#endif

#define FPS_DELAY (FPS_EVERY_N_SECS*1000)

static uint32_t* bbufpix; // Framebuffer pointer declared as external

static glContext the_Context;

typedef struct My_Uniforms
{
    vec4 v_color;
} My_Uniforms;

void cleanup(void);
void setup_context(void);
int handle_events(void);

void identity_vs(float* vs_output, vec4* vertex_attribs, Shader_Builtins* builtins, void* uniforms);
void uniform_color_fs(float* fs_input, Shader_Builtins* builtins, void* uniforms);

int fn(void *arg) {
    for (;;) {
        int i;
        for (i = 0; i < 80; i++) {
            printk("a = %d\n", i);
        }
    }
    return 6;
}



int main(uint32_t magic, multiboot_info_t* mb_info) {
    if (magic != 0x2BADB002) {
        panic("0x%X\n Invalid Boot Record", magic);
        // Optionally: print error, halt, or return an error code
        return -1; // Return error code
    }

    init_descriptor_tables();

    kernel_io_init();

    IDE ide;
    Fat32 fat32(ide);

    fat32.mount();


    init_filesystem();

    init_paging(mb_info);
    init_timer(20);

    __asm__ volatile ("sti");
    init_scheduler(init_threading());

    // start operations here
    uint32_t *stack = kmalloc(0x400) + 0x3F0;
    thread_t *t = create_thread(&fn, (void *)0x567, stack);

    Graphics gfx;

    uint32_t physBackBuffer;
    bbufpix = static_cast<uint32_t*>(kmalloc0_ap(WIDTH * HEIGHT * sizeof(uint32_t), &physBackBuffer));
    
    setup_context();
        
    float points[] = { -0.5, -0.5, 0,
                       0.5, -0.5, 0,
                       0,    0.5, 0 };

    GLuint program = pglCreateProgram(identity_vs, uniform_color_fs, 0, NULL, GL_FALSE);
    glUseProgram(program);

    My_Uniforms the_uniforms;
    pglSetUniform(&the_uniforms);

    vec4 Red = { 1.0f, 0.0f, 0.0f, 1.0f };
    the_uniforms.v_color = Red;

    //No default VAO in core profile, so we set one up
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint triangle;
    glGenBuffers(1, &triangle);
    glBindBuffer(GL_ARRAY_BUFFER, triangle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    int old_time = 0, new_time = 0, counter = 0;
    int ms;

    size_t bufferSizeBytes = WIDTH * HEIGHT * 4;

    while (handle_events()) {
        glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the triangle
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Ensure correct buffer size and pitch

        memcpy((void*)gfx.getBackBuffer(), (void*)bbufpix, WIDTH * HEIGHT * sizeof(uint32_t));

        gfx.swapBuffers();
    }

    // Cleanup OpenGL resources
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &triangle);
    glDeleteProgram(program);

    cleanup();


    return 0xDEADBABA;
}



void identity_vs(float* vs_output, vec4* vertex_attribs, Shader_Builtins* builtins, void* uniforms)
{
    PGL_UNUSED(vs_output);
    PGL_UNUSED(uniforms);
    builtins->gl_Position = vertex_attribs[0];
}

void uniform_color_fs(float* fs_input, Shader_Builtins* builtins, void* uniforms)
{
    PGL_UNUSED(fs_input);
    builtins->gl_FragColor = ((My_Uniforms*)uniforms)->v_color;
}

void setup_context()
{
    // Initialize portableGL context using bbufpix
    if (!init_glContext(&the_Context, &bbufpix, WIDTH, HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000)) {
        
        puts("Failed to initialize glContext");
        //exit(0);
    }
}

void cleanup()
{
    free_glContext(&the_Context);
}

int handle_events()
{
    // Use an alternative mechanism for handling events or just return 1 for an infinite loop
    return 1;
}