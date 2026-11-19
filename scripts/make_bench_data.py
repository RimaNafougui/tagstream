"""Builds benchmark data from the SEC company list.

Usage: python3 scripts/make_bench_data.py data/company_tickers.json 200000
Writes data/companies.tsv (patterns) and data/bench_input.txt (synthetic headlines).
"""
import json
import random
import sys

source, line_count = sys.argv[1], int(sys.argv[2])

with open(source) as f:
    data = json.load(f)

companies = []
for entry in data.values():
    title, ticker = entry["title"].strip(), entry["ticker"].strip()
    if title and ticker and "\t" not in title:
        companies.append((title, ticker))

with open("data/companies.tsv", "w") as f:
    for title, ticker in companies:
        f.write(f"{title}\t{ticker}\n")

words = ("shares rise fall after earnings report analysts expect market today record "
         "quarter guidance investors deal merger outlook revenue profit warning").split()

random.seed(42)  # same file every time
with open("data/bench_input.txt", "w") as f:
    for _ in range(line_count):
        parts = random.choices(words, k=random.randint(6, 12))
        if random.random() < 0.5:  # half the lines mention a company
            parts.insert(random.randrange(len(parts) + 1), random.choice(companies)[0])
        f.write(" ".join(parts) + "\n")

print(f"{len(companies)} patterns, {line_count} lines")