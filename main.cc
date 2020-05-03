#include <unistd.h>

#include <iostream>

#include "chunk.hh"

static void usage(const char *execname) {
  std::cout << "Usage: " << execname << " [-h] [-x]  <.aif file>" << std::endl
            << "  x: eXtract the chunks to disk " << std::endl
            << "     (This will overwrite filenames file.<chunkname>.<id>.dat"
            << std::endl
            << "     where <file> is the input file name without the prefix.)"
            << std::endl
            << "  h: This help message." << std::endl;
}

int main(int argc, char **argv) {
  int opt;
  bool writeChunks = false;

  while ((opt = getopt(argc, argv, "xh")) != -1) {
    switch (opt) {
      case 'h':
        usage(argv[0]);
        return 0;
      case 'x':
        writeChunks = true;
        break;
      default:
        std::cerr << "Unknown argument (see -h for help)" << std::endl;
        return -1;
    }
  }

  if (optind >= argc) {
    std::cerr << "Missing filename argument." << std::endl;
    return -1;
  }

  const char *fname = argv[optind];
  AIFF aiff(fname);
  aiff.readChunks();
  aiff.dump();

  if (writeChunks)
    for (auto &chunk : aiff)
      chunk->write(fname);

  return 0;
}
