cmake . -DCMAKE_BUILD_TYPE=Release  -B auto-build

cd auto-build || exit

make

mv Homework2 ..

cd ..
