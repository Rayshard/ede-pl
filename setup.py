import setuptools

with open("README.md") as fh:
    long_description = fh.read()

setuptools.setup(
    name="ede",
    version="0.0.1",
    author="Rayshard Thompson",
    author_email="rayshard.t123@gmail.com",
    description="Ede Programming Language",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/Rayshard/ede-pl",
    package_dir={"": "src"},
    packages=setuptools.find_packages(where="src"),
    python_requires=">=3.10",
    install_requires=["click"],
    extras_require={"dev": ["pytest", "mypy"]},
    scripts=["src/ede/ede.bat"]
)