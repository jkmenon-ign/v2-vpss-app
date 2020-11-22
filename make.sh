#!/bin/sh

rm -rf lib/*

cd h-scaler
make clean
make
cd ..

cd v-scaler
make clean
make
cd ..

cd h-resampler
make clean
make
cd ..

cd v-resampler
make clean
make
cd ..


cd axis_switch
make clean
make
cd ..


cd xgpio
make clean
make
cd ..

cd vprocss
make clean
make
cd ..

cd csc
make clean
make 
cd ..

ls lib/
