#!/bin/bash
clang-format -verbose -i -style=file */*.cpp
clang-format -verbose -i -style=file */*/*.cpp
#clang-format -verbose -i -style=file test/*.cpp
clang-format -verbose -i -style=file include/*.hpp
