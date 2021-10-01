typecheck:
	mypy .\src\

test: typecheck
	pytest

lines_of_code:
	pygount --format=summary .
	
run: typecheck
	python .\src\main.py .\tests\test.ram