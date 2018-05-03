# batchlines

	producer | batchlines -t TIMEOUT -n MAXLINES FILENAME .EXT

- Read lines from stdin.
- Write them into a `FILENAME.tmp`
- After `TIMEOUT` seconds rename `FILENAME.tmp` to `FILENAME.NNN.EXT`
- If there are more `.EXT`, then this is cycled.
- At the end, rename `FILENAME.tmp` to `FILENAME.EXT` (first `.EXT`)
- If there is some data left (partial line) this is printed to stdout unless Option `-q` is present

# Usage

	git clone https://github.com/hilbix/batchlines.git
	cd batchlines
	git submodule update --init
	make
	sudo make install

## Help

Get help with `batchlines -h`:

```
Usage: batchlines [options] filename [.ext..]
                version 0.1.0 compiled May  3 2018
        Write stdin to given filename.tmp, line-by-line. (.tmp see -e)
        At the end sync file and rename it to filename.ext (atomically)
        Multiple .ext are cycled, missing .ext is '' (none)
        The incomplete last lines (missing NL) is written to stdout.
        On SIGUSR1, file is synced plus rotated to filename.NNN.ext
        Rotation can be done after given time (-t) or lines (-n), too
Options:
        -h      this help
        -e ext  intermediate extension
                (default '.tmp')
        -i char input linefeed character ('' is NUL, NUL is always EOL)
                (default LF 0x0a 10 '\n')
        -n max  rotate after the given number of lines (default: 0=never)
        -o char input linefeed character ('' is NUL, NUL is always EOL)
                (default LF 0x0a 10 '\n')
        -q      quiet (do not output last incomplete line)
        -s      safe mode (try not to overwrite files)
        -t sec  rotate after the given timeout in seconds (default: 0=none)
        -u      unsynced (do not sync file)
        -w nr   width of counter (for numbered rotates)
                (default 1)

```

