#pragma once

#include <Python.h>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

#define VERBOSE

static void dprintf(char* fmt, ...) {
#ifdef VERBOSE
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
#else
#endif
}

static PyObject* get_verbose_flag(PyObject* self, PyObject* args) {
    return PyLong_FromLong(Py_VerboseFlag);
}
