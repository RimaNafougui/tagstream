# tagstream

A high-speed stream tagger written in C++20. It reads a live stream of text and labels every line with the entities it mentions, matching thousands of patterns in a single pass with a hand-written Aho-Corasick automaton.

> Status: in development. Benchmark figures below are placeholders until they are measured. See the roadmap for what is done.

## What it does

You give tagstream a list of patterns, each mapped to a tag. It then reads lines from standard input and prints each line with the tags it matched.

Example with company names mapped to stock tickers:

```
$ ./tagstream --patterns companies.tsv < headlines.txt
AAPL,MSFT    Apple and Microsoft shares fall after earnings
RY           Royal Bank raises dividend
-            Weather warning issued for Montreal
```

## Why it exists

Checking each line against every pattern one at a time costs time proportional to the number of patterns. With 10,000 patterns and thousands of lines per second, that is too slow.

The Aho-Corasick algorithm builds one automaton from all the patterns, then reads each line exactly once and reports every match along the way. Matching time depends on the length of the text and the number of matches, not on how many patterns are loaded.

The same engine applies to several real problems:

- Financial news: linking headlines to the companies they mention.
- Monitoring: scanning logs for known error signatures.
- Moderation: catching banned terms in chat in real time.
- Security: scanning data for known threat signatures.

## How it works

1. **Trie.** Every pattern is inserted into a prefix tree, one character per edge.
2. **Failure links.** A breadth-first search gives each node a link to the longest proper suffix that is also a prefix of some pattern. When a character does not match, the automaton follows this link without re-reading the text.
3. **Output links.** Each node knows which patterns end there, including shorter patterns that end inside longer ones, so overlapping matches are all reported.
4. **Flat layout.** After construction, nodes are stored in contiguous arrays instead of heap-allocated objects, which keeps lookups cache-friendly.

### Matching rules

- Case-insensitive: "APPLE", "Apple" and "apple" all match.
- Word boundaries: "Apple" does not match inside "pineapple".
- Overlapping matches: "Bank of Montreal" and "Montreal" can both be reported from the same text.

## Building

Requirements: a C++20 compiler (GCC 11+ or Clang 14+) and CMake 3.20+.

```
git clone https://github.com/rimanafougui/tagstream.git
cd tagstream
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Usage

```
./build/tagstream --patterns <file.tsv> < input.txt
```

The pattern file is tab-separated, one pattern per line:

```
Apple Inc	AAPL
Apple	AAPL
Microsoft	MSFT
Royal Bank	RY
```

Output is one line per input line: the matched tags (comma-separated, or `-` when there are none), a tab, then the original line.

## Testing

```
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DTAGSTREAM_SANITIZE=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

- **Unit tests** (GoogleTest) cover the trie, failure links, boundary rules and case folding.
- **Property tests** generate random patterns and text from a fixed seed, then check that the automaton returns exactly the same matches as a naive reference matcher. Any failure can be reproduced from its seed.
- Test builds run with AddressSanitizer and UndefinedBehaviorSanitizer.
- GitHub Actions builds and tests every push.

## Benchmarks

To be measured. The benchmark compares tagstream with the naive matcher on the same input.

| Metric               | Naive baseline | tagstream |
| -------------------- | -------------- | --------- |
| Patterns loaded      | TBD            | TBD       |
| Automaton build time | n/a            | TBD       |
| Throughput (lines/s) | TBD            | TBD       |
| Throughput (MB/s)    | TBD            | TBD       |
| Memory               | TBD            | TBD       |

Machine, compiler and dataset will be listed here with the results. Reproduce with:

```
./build/tagstream_bench --patterns data/companies.tsv --input data/headlines.txt
```

## Project layout

```
tagstream/
  include/tagstream/   public headers (automaton, matcher)
  src/                 implementation and CLI
  tests/               unit and property tests
  bench/               benchmark harness and naive baseline
  data/                sample patterns and input
  CMakeLists.txt
```

## Roadmap

- [ ] Repository, CMake build, first passing test
- [ ] Naive reference matcher
- [ ] Trie and Aho-Corasick failure links
- [ ] Property tests against the naive matcher
- [ ] Case-insensitive matching and word boundaries
- [ ] Flat array layout
- [ ] Benchmark harness and measured results
- [ ] Run on real company names and a public headlines dataset

Stretch goals:

- [ ] Multiple worker threads
- [ ] Reloading patterns without a restart
- [ ] TCP mode so other programs can stream lines in

## Design trade-offs

- **Memory for speed.** The flat layout uses more memory per node than a sparse map of children, in exchange for constant-time transitions.
- **Static pattern set.** The automaton is built once. Adding a pattern means rebuilding, which is acceptable when patterns change rarely and text arrives constantly.
- **Byte-level matching.** Case folding covers ASCII. Full Unicode case folding is out of scope for the first version.

## Data

Sample patterns can be built from the company names and tickers published by the U.S. Securities and Exchange Commission. Any dataset used for benchmarks will be credited here with its licence.

## Author

Rima Nafougui, Computer Science, University of Montreal.
