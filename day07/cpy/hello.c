#include <python3.5m/Python.h>

#include <stdio.h>

PyObject *hello_world(PyObject *self, PyObject *args)
{
    puts("Hello, World");
    Py_RETURN_NONE;
}

PyObject *goodbye_world(PyObject *self, PyObject *args)
{
    char *str;
    if(!PyArg_ParseTuple(args, "[(sssii)]", &str)) {
        return NULL;
    }

    printf("Goodbye, %s\n", str);
    Py_RETURN_NONE;
}

struct PyMethodDef methods[] = {
    {"HelloWorld", hello_world, METH_VARARGS, "Method docstring"},
    {"goodbye", goodbye_world, METH_VARARGS, "Says goodbye"},
    { NULL, NULL, 0, NULL}
};

struct PyModuleDef module = {
    PyModuleDef_HEAD_INIT, "hello", "Docstring for hello", -1, methods
};

PyMODINIT_FUNC PyInit_hello(void)
{
    return PyModule_Create(&module);
}
