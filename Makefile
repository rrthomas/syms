# Makefile for syms

all: README.md

pg100.txt:
	wget https://www.gutenberg.org/ebooks/100.txt.utf-8 -O $@

check: pg100.txt syms
	mypy syms
	time ./syms pg100.txt >/dev/null

release:
	git diff --exit-code && \
	rm -rf ./dist && \
	mkdir dist && \
	python3 setup.py sdist bdist_wheel && \
	twine upload dist/* && \
	git tag v$$(python3 setup.py --version) && \
	git push --tags

README.md: syms README.md.in Makefile
	cp README.md.in README.md
	printf '\n```\n' >> README.md
	./syms --help >> README.md
	printf '```\n' >> README.md
