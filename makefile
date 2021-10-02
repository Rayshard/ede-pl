typecheck:
	mypy --namespace-packages -p "ede"

test:
	pytest

lines_of_code:
	pygount --format=summary .