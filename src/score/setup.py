from distutils.core import setup, Extension

setup(name="chadwick",
      version="0.4.0",
      description="A baseball scorebook application",
      author="Theodore Turocy",
      author_email="drarbiter@gmail.com",
      url="http://chadwick.sourceforge.net",
      packages=["chadwick"],
      package_dir={ "chadwick": "." })

