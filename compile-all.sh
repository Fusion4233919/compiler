#!/bin/zsh

cmake .
make

./compiler tester/qsort/qsort.cmm -e
cp ./mhl ./ta-test/quicksort/
./ta-test/quicksort/darwin-amd64 ./ta-test/quicksort/mhl

./compiler tester/matrix-mult/matrix.cmm -e
cp ./mhl ./ta-test/matrix-multiplication
./ta-test/matrix-multiplication/darwin-amd64 ./ta-test/matrix-multiplication/mhl

./compiler tester/auto-advisor/auto.cmm -e
cp ./mhl ./ta-test/auto-advisor/
./ta-test/auto-advisor/darwin-amd64 ./ta-test/auto-advisor/mhl

./compiler tester/lambda/lambda.cmm -e

./mhl
