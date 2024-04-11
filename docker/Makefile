build:
	docker build -t "macaulay1:0.1" .

build-x86:
	docker build --platform=linux/amd64 -t "macaulay1:0.1" .

build-i386:
	docker build --platform=linux/i386 -t "macaulay1:0.1" .

run:
	echo ${PWD}
	docker run --rm -it -v ${PWD}/..:/tmp -w /tmp macaulay1:0.1

run-i386:
	echo ${PWD}
	docker run --rm -it  --platform=linux/i386 -v ${PWD}/..:/tmp -w /tmp macaulay1:0.1

run-x86:
	echo ${PWD}
	docker run --rm -it --platform=linux/amd64 -v ${PWD}/..:/tmp -w /tmp macaulay1:0.1

# add-alias:
# 	alias valgrind='docker run -it -v $PWD:/tmp -w /tmp macaulay2:1.22'
