#!/usr/local/bin/python

from distutils.core import setup, Extension

module1 = Extension('hello', sources = ['hello.c'])

setup(name = 'Hello',
        version = '1.0',
        description = 'This is a Hello package',
        ext_modules = [module1])
