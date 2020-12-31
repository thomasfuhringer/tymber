import setuptools

with open("README.md", "r", encoding="utf-8") as fh:
    long_description = fh.read()

setuptools.setup(
    name="tymber",
    version="0.9.0",
    author="Thomas Führinger",
    author_email="thomasfuhringer@live.com",
    description="GUI tookit for Windows",
    long_description="GUI tookit for Windows",
    long_description_content_type="text/markdown",
    url="https://github.com/thomasfuhringer/tymber",
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License (GPL)",
        "Operating System :: Microsoft :: Windows",
    ],
    python_requires=">=3.9",
)
