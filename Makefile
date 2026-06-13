.PHONY: all engine daemon clean

all: engine daemon

engine:
	cmake -S . -B build
	cmake --build build

daemon:
	go build -o encryption-daemon ./cmd/daemon

clean:
	rm -rf build encryption-daemon daemon
