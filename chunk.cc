#include "chunk.hh"

#include <byteswap.h>

#include <cassert>

static ChunkType idToChunkType(const ID &id) {
  uint32_t idVal = *reinterpret_cast<const uint32_t *>(id);
  if (*(uint32_t *)"FORM" == idVal) return ChunkType::FORM;
  return ChunkType::OTHER;
}

std::unique_ptr<Chunk> Chunk::create(std::ifstream &fp,
                                     std::unique_ptr<ChunkHeader> hdr) {
  std::unique_ptr<Chunk> chunk;
  switch (idToChunkType(hdr->id)) {
    case FORM:
      chunk = std::make_unique<Form>(std::move(hdr));
      break;
    case MARKER:
      chunk = std::make_unique<Marker>(std::move(hdr));
      break;
    case OTHER:
      chunk = std::make_unique<Chunk>(std::move(hdr));
      break;
    default:
      std::cerr << "Unknown chunk type." << std::endl;
      exit(EXIT_FAILURE);
  }
  if (!chunk->parse(fp)) {
    std::cerr << "Error parsing chunk of type " << chunk->getType() << '\n';
    exit(EXIT_FAILURE);
  }

  return chunk;
}

void Chunk::writeChunk(const Chunk &chunk, const std::string &prefix) {
  static int counter;
  auto name = prefix + '.' + std::to_string(++counter) + '.' +
              idToString(chunk.getHeader()->id) + ".dat";
  std::ofstream ofs(name);
  if (!ofs) {
    std::cerr << "Error creating chunk:  " << name << std::endl;
    exit(EXIT_FAILURE);
  }
  ofs.write(reinterpret_cast<const char *>(chunk.getData().data()),
            chunk.getData().size());
  if (!ofs) {
    std::cerr << "Error writing chunk data:  " << name << std::endl;
    exit(EXIT_FAILURE);
  }
}

// This acts like a factory producing different types of chunks.
std::unique_ptr<Chunk> AIFF::readChunkBody(std::ifstream &fp,
                                           std::unique_ptr<ChunkHeader> hdr) {
  const auto begin = fp.tellg();
  auto chunk = Chunk::create(fp, std::move(hdr));
  const auto end = fp.tellg();

  // Read the chunk's data into a blob.
  fp.seekg(begin);
  auto buffer = new uint8_t[chunk->getHeader()->size];
  if (!buffer) {
    std::cerr << "Error allocating buffer to store chunk data." << std::endl;
    exit(EXIT_FAILURE);
  }
  fp.read(reinterpret_cast<char *>(buffer), chunk->getHeader()->size);
  if (!fp) {
    std::cerr << "Error reading chunk contents." << std::endl;
    exit(EXIT_FAILURE);
  }
  chunk->getData().assign(&buffer[0], &buffer[end - begin]);
  delete[] buffer;
  fp.seekg(end);

  return chunk;
}

std::unique_ptr<ChunkHeader> AIFF::readHeader(std::ifstream &fp) {
  assert(fp && "Invalid file handler.");
  auto hdr = std::make_unique<ChunkHeader>();
  fp.read(reinterpret_cast<char *>(hdr.get()), sizeof(hdr));
  if (fp.eof())
    return nullptr;
  else if (!fp) {
    std::cerr << "Error reading in the header." << std::endl;
    exit(EXIT_FAILURE);
  }

  // Check that the ID is valid.
  for (size_t i = 0; i < sizeof(id_t); ++i) {
    if (hdr->id[i] < ' ' || hdr->id[i] > '~') {
      std::cerr << "Invalid header ID." << std::endl;
      exit(EXIT_FAILURE);
    }
  }

  hdr->size = __bswap_32(hdr->size);
  return hdr;
}

std::unique_ptr<Chunk> AIFF::readChunk(std::ifstream &fp) {
  if (fp.eof()) return nullptr;
  assert(fp && "Invalid file handler.");
  std::unique_ptr<ChunkHeader> hdr = readHeader(fp);
  if (!hdr) return nullptr;
  return readChunkBody(fp, std::move(hdr));
}

void AIFF::readChunks() {
  if (!_file) {
    std::cerr << "Invalid file handle, ensure that " << _filename
              << " is readable." << std::endl;
    return;
  }
  while (auto chunk = readChunk(_file)) chunks.push_back(std::move(chunk));
}

void AIFF::dump() const {
  for (const auto &chunk : chunks) chunk->dump();
}

bool Form::parse(std::ifstream &fp) {
  const auto end = (int32_t)fp.tellg() + getHeader()->size;
  fp.read(formType, sizeof(ID));
  if (!fp) {
    std::cerr << "Error reading formType." << std::endl;
    exit(EXIT_FAILURE);
  }

  auto str = idToString(formType);
  if (str != "AIFF" && str != "AIFC") {
    std::cerr << "Unexpected form type: " << str << std::endl;
    exit(EXIT_FAILURE);
  }

  while (fp.tellg() < end) {
    auto chunk = AIFF::readChunk(fp);
    if (!chunk) break;
    chunks.push_back(std::move(chunk));
  }

  if (fp.tellg() < end) {
    std::cerr << "Error reading all of the chunks making up a FORM instance."
              << std::endl;
    exit(EXIT_FAILURE);
  }

  return true;
}

void Form::dump() const {
  std::cout << *getHeader() << " (" << chunks.size() << " chunks)" << std::endl;
  unsigned i = 0;
  for (const auto &chunk : chunks) {
    std::cout << "  " << ++i << ") ";
    chunk->dump();
  }
}

void Form::write(const std::string &prefix) const {
  for (const auto &chunk : chunks) chunk->write(prefix);
}

bool Marker::parse(std::ifstream &fp) { return true; }

bool Chunk::parse(std::ifstream &fp) {
  // Skip past this chunk.
  fp.seekg(_hdr->size, fp.cur);
  return true;
}
