all ::
	cd c && gcc -o srv -O3 srv.c
	cd cpp && g++ -o srv -O3 -std=c++14 srv.cpp
	cd crystal && crystal build --release srv.cr
	cd dotnet && dotnet publish -c release -r osx-x64
	cd go && go build srv.go
	cd java && javac -d classes srv.java
	cd nim && nim c srv.nim
	cd rust && cargo build --release
	cd v && v -prod srv.v

test ::
	wrk -t12 -c400 -d30s http://127.0.0.1:7012/

clean ::
	cd c && rm -f srv
	cd cpp && rm -f srv
	cd crystal && rm -f srv srv.dwarf
	cd dotnet && rm -rf bin obj
	cd go && rm -f srv
	cd java && rm -rf classes
	cd nim && rm -f srv
	cd rust && rm -rf Cargo.lock target
	cd v && rm -f srv

openresty ::
	openresty -c "$(PWD)/openresty/nginx.conf"

openresty-stop ::
	openresty -s stop

c ::
	cd c && ./srv

cpp ::
	cd cpp && ./srv

crystal ::
	cd crystal && ./srv

dotnet ::
	cd dotnet && ./bin/release/net5.0/osx-x64/srv

go ::
	cd go && ./srv

java ::
	cd java && java -cp classes srv.srv

lowjs ::
	cd javascript && low --max-old-space-size=1024 srv

lua ::
	cd lua && lua srv.lua

nim ::
	cd nim && ./srv

nodejs ::
	cd javascript && node srv

python ::
	cd python && python3 srv.py

rust ::
	cd rust && ./target/release/srv

v ::
	cd v && ./srv
