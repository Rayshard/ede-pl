typecheck:
	mypy .\src\

test: typecheck
	pytest

run: typecheck
	python .\src\main.py .\tests\test.ram