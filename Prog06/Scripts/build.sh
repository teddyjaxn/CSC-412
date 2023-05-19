#!/bin/bash

g++ -Wall ../Version1/main.cpp ../Version1/gl_frontEnd.cpp -lm -lGL -lglut -lpthread -o ../Version1/cell

g++ -Wall ../Version2/main.cpp ../Version2/gl_frontEnd.cpp -lm -lGL -lglut -lpthread -o ../Version2/cell