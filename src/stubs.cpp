// src/stubs.cpp
extern "C" {
    void abort() { while(1); }
    void exit(int) { while(1); }
    void _Exit(int) { while(1); }
}