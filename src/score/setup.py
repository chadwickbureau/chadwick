from distutils.core import setup, Extension

setup(name="chadwick",
      version="0.3.1",
      description="A baseball scorebook application",
      author="Theodore Turocy",
      author_email="turocy@econmail.tamu.edu",
      url="http://chadwick.sourceforge.net",
      packages=["chadwick"],
      package_dir={ "chadwick": "." })

