#!/bin/bash

cd ~/git/retrosheet/event/regular
diff -b <(cwbox -y ${1} ${1}*.EV?) <(wine ~/bin/BOX.EXE -y ${1} ${1}*.EV? | fromdos)
