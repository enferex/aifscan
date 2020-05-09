aifscan: Extract AIFF/AIFC audio data.
======================================
aifscan is a tool for exploring (and extracting) data from AIFF/AIFC audio
files.  Running this program without any options produces a list of chunks
contained in the AIFF/AIFC file.

The '-x' option can be specified to aifscan to write the chunks contained in
the input AIFF/AIFC file to disk.  aifscan will create a file for each chunk
using the following convention: `<input file name>.<chunk index number>.<chunk
id name>.dat` FORM chunks are not written to disk, instead the child chunks
are.  aifscan will overwrite a file if the filename already exists. 

Building
--------
1. Invoke `make` to build this puppy.

References
----------
* [AIFF Spec](http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/AIFF/Docs/AIFF-1.3.pdf)
* [AIFC Spec](http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/AIFF/Docs/AIFF-C.9.26.91.pdf)

Contact
-------
Matt Davis: https://github.com/enferex
