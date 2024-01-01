=== Chess Assist ===

A frontend for UCI engines, written in C.
It serves functionalities through a REST HTTP Api.

Currently implemented endpoints are the following:

/uci/position?fen={0}
/uci/bestmove
/uci/is_ready

== Websites integration ==

In the folder /browser you can find websites integrations.
You can easily cheat with any engine you may want.

== How to build ==

git clone https://github.com/dveqi/chess-assist.git
cd chess-assist
mkdir build && cd build
cmake ..
make

== How to use ==

./uci -P "path to uci engine"

You can also enable verbose output with -V

== Demo ==

Stockfish and Chess.com: https://youtu.be/zcNad3IWXHA
