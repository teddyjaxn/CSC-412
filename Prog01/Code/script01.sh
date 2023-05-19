#!/bin/bash

gcc prog01.c -o "$1"

./"$1"
./"$1" one
./"$1" one two
./"$1" one two three
./"$1" one two three four
