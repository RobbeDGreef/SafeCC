#!/bin/bash

cd tests
./cmpltests
if [ "$?" -eq "0" ]
then
./runtests 
fi
cd ..
