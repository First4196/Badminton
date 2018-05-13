#!/bin/bash

./bin/prep $@ -save
./bin/bg $@ -save
./bin/court $@ -save
./bin/tag $@ -save
./bin/player $@ -save -savecsv