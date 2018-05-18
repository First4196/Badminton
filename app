#!/bin/bash

./bin/input $@ -save
./bin/bg $@ -save
./bin/court $@ -save
./bin/tag $@ -save
./bin/player $@ -save -savecsv