cmake . -B -DCMAKE_BUILD_TYPE=Release auto-build

cd auto-build || exit

make

mv Homework2 ..

cd ..
