//BleskOS

/*
* MIT License
* Copyright (c) 2023-2025 Vendelín Slezák
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
* The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

dword_t logging_mem, logging_mem_end, logging_mem_draw_pointer, logging_mem_pointer;

void initalize_logging(void);
void developer_program_log(void);
void skip_logs(void);
void show_log(void);
void log(char *string);
void log_unicode(word_t *string);
void log_var(dword_t value);
void log_var_with_space(dword_t value);
void log_hex(dword_t value);
void log_hex_with_space(dword_t value);
void log_hex_specific_size(dword_t value, dword_t chars);
void log_hex_specific_size_with_space(dword_t value, dword_t chars);