# batchlines

	producer | batchlines -t TIMEOUT -m MAXLINES FILENAME .EXT

- Read lines from stdin.
- Write them into a `FILENAME.tmp`
- After `TIMEOUT` seconds (and the end) rename `FILENAME.tmp` to `FILENAME.NNN.EXT`
- If there are more `.EXT`, then this is cycled.
- If there is some data left (partial line) this is printed to stdout

# Usage

	git clone https://github.com/hilbix/batchlines.git
	make
	sudo make install

