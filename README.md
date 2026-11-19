# tagstream

A high-speed stream tagger written in C++20. It reads a stream of text and labels every line with the entities it mentions, matching thousands of patterns in a single pass with a hand-written Aho-Corasick automaton.

On 10,398 company names it tags about **2.1 million lines per second (163 MB/s)**, roughly **1,770 times faster** than checking each pattern one by one. Every result is checked against a simple reference matcher with seeded property tests.

## What it does

You give tagstream a file of patterns, each mapped to a tag. It reads lines from standard input and prints each line with the tags it matched.

```
$ printf "Apple and Microsoft shares fall\nRoyal Bank raises dividend\nPineapple prices rise\nAPPLE's results beat\n" \
    | ./build/tagstream data/sample.tsv
AAPL,MSFT	Apple and Microsoft shares fall
RY	Royal Bank raises dividend
-	Pineapple prices rise
AAPL	APPLE's results beat
```

Matching rules:

- **Case-insensitive:** "APPLE", "Apple" and "apple" all match.
- **Whole words only:** a match must not be glued to a letter or digit on either side, so "Pineapple" does not match "Apple", while "APPLE's" and "(Apple)" do.
- **Overlapping matches:** "Bank of Montreal" and "Montreal" are both reported from the same text.
- **Each tag once per line,** sorted, even if several patterns map to it.

## Why it exists

Checking each line against every pattern costs time proportional to the number of patterns. With 10,000 patterns and a constant stream of text, that is too slow.

Aho-Corasick builds one automaton from all the patterns, then reads each line exactly once and reports every match along the way. Matching time depends on the length of the text and the number of matches, not on how many patterns are loaded.

The same technique is used for linking news to the companies it mentions, scanning logs for known error signatures, filtering chat, and matching threat signatures in security tools.

## How it works

1. **Trie.** Every pattern (lowercased) is inserted into a prefix tree. Patterns that start the same way share nodes: 10,398 company names produce 143,950 nodes.
2. **Failure links.** A breadth-first search gives each node a link to the longest proper suffix of its path that is also in the trie. When the next character has no matching child, the automaton follows this link instead of re-reading text.
3. **Inherited outputs.** Each node also reports the patterns of its failure node, so a short pattern ending inside a longer one ("he" inside "she") is never missed.
4. **Flat transition table.** After construction, every transition is precomputed into one contiguous array, with failure links already folded in. Matching becomes one array read per character, with no tree lookups and no loop. To keep the table small, only characters that appear in some pattern get a column (47 for company names, plus one column for all other characters).
5. **Whole-word check.** When a pattern ends at a position, its start is derived from its length and both sides are checked with the same function the reference matcher uses.

## Building

Requirements: a C++20 compiler (Clang 14+ or GCC 11+) and CMake 3.20+. GoogleTest is downloaded automatically.

```
git clone https://github.com/RimaNafougui/tagstream.git
cd tagstream
cmake -B build
cmake --build build
```

## Usage

```
./build/tagstream <patterns.tsv> < input.txt
```

The pattern file is tab-separated, one pattern per line, with the text first and the tag second:

```
Apple	AAPL
Microsoft	MSFT
Royal Bank	RY
```

Lines without a tab are skipped. Output is one line per input line: the matched tags (comma-separated, or `-` when there are none), a tab, then the original line.

## Testing

```
ctest --test-dir build --output-on-failure
```

29 tests, written with GoogleTest:

- **Unit tests** cover the trie, failure links, inherited outputs, case folding and word boundaries.
- **Property tests** generate random patterns and text from fixed seeds and check that the automaton returns exactly the same tags as a naive reference matcher, about 22,500 random lines per run. Small alphabets with spaces and capitals force many overlaps and boundary cases. A failure prints its seed, so it can be replayed exactly.

To run the tests with AddressSanitizer and UndefinedBehaviorSanitizer:

```
cmake -B build-asan -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="-fsanitize=address,undefined -fno-omit-frame-pointer"
cmake --build build-asan
ctest --test-dir build-asan --output-on-failure
```

## Benchmarks

Measured on a MacBook Pro (Apple M4 Pro, Apple clang 21) with a Release build. Patterns are the 10,398 company names in the SEC company tickers list; input is 200,000 synthetic headlines, about half mentioning a company. The naive matcher is timed on 2,000 lines because it is too slow for the full set.

| Metric | Naive | Automaton (std::map trie) | Automaton (flat table) |
|---|---|---|---|
| Throughput (lines/s) | about 1,200 | 1.20 million | **2.12 million** |
| Throughput (MB/s) | n/a | 92.2 | **163.5** |
| Speedup vs naive | 1x | 976x | **1,771x** |
| Build time | n/a | 12.8 ms | 17.9 ms |
| Peak memory (whole process) | n/a | 59.5 MB | 87.3 MB |

The flat table is 1.77 times faster than the `std::map` version, in exchange for a 27.6 MB table and 5 ms more build time. Both versions find the same 213,698 tags, and the benchmark checks that they agree with the naive matcher.

Full output and conditions: [bench/RESULTS.md](bench/RESULTS.md).

To reproduce:

```
curl -A "Your Name your@email.com" -o data/company_tickers.json https://www.sec.gov/files/company_tickers.json
python3 scripts/make_bench_data.py data/company_tickers.json 200000
cmake -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
./build-release/tagstream_bench data/companies.tsv data/bench_input.txt
```

The SEC asks for a User-Agent identifying you. The generated data files are not committed.

## Project layout

```
tagstream/
  include/tagstream/   matcher.hpp (patterns, naive matcher, word rules), automaton.hpp
  src/                 matcher.cpp, automaton.cpp, main.cpp (command-line program)
  tests/               unit and property tests
  bench/               benchmark program and recorded results
  scripts/             benchmark data generator
  data/                sample.tsv (small example patterns)
  CMakeLists.txt
```

## Design trade-offs

- **Memory for speed.** The flat table stores a full row per node (48 columns of 4 bytes), using about 28 MB for company names in exchange for constant-time transitions.
- **Static pattern set.** The automaton is built once (about 18 ms here). Changing patterns means rebuilding, which suits data where patterns change rarely and text arrives constantly.
- **ASCII case folding.** Lowercasing is byte by byte. Full Unicode case folding is out of scope.
- **One tag set per line.** Each line returns a `std::set` of tag strings, which allocates memory per line. This has not been profiled yet and is the likely next bottleneck.

## Data notes

The SEC list has one entry per ticker, so some names map to several tickers (share classes, units and warrants). One mention can therefore produce several tags, for example `BRTM,BRTMU,BRTMW` for B&R Technology Merger Corp.

## Roadmap

- [x] CMake build and GoogleTest setup
- [x] Naive reference matcher
- [x] Trie and Aho-Corasick failure links
- [x] Seeded property tests against the naive matcher
- [x] Case-insensitive matching and word boundaries
- [x] Flat transition table
- [x] Benchmark harness and measured results
- [ ] Continuous integration with GitHub Actions
- [ ] Benchmark on real headlines instead of synthetic ones

Possible extensions:

- [ ] Multiple worker threads
- [ ] Reloading patterns without a restart
- [ ] TCP mode so other programs can stream lines in

## Author

Rima Nafougui, Computer Science, University of Montreal.
