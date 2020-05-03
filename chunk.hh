// AIFF and AIFC Specs
// http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/AIFF/Docs/AIFF-1.3.pdf
// http://www-mmsp.ece.mcgill.ca/Documents/AudioFormats/AIFF/Docs/AIFF-C.9.26.91.pdf
#ifndef __CHUNK_HH
#define __CHUNK_HH

#include <cstdint>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using ID = char[4];

enum ChunkType {
  FORM,
  MARKER,
  OTHER,
};

static std::string idToString(char const *id) { return std::string(id, 4); }

struct ChunkHeader {
  ID id;
  int32_t size;
  friend std::ostream &operator<<(std::ostream &os, const ChunkHeader &hdr) {
    return os << "Chunk: " << idToString(hdr.id) << ' ' << hdr.size << " bytes";
  }
};

class Chunk {
  std::unique_ptr<ChunkHeader> _hdr;
  ChunkType _type;
  virtual bool parse(std::ifstream &fp);
  std::vector<uint8_t> _data;

 public:
  Chunk(std::unique_ptr<ChunkHeader> header, ChunkType type = OTHER)
      : _hdr(std::move(header)), _type(type) {}
  virtual ~Chunk() {}
  ChunkType getType() const { return _type; }
  std::vector<uint8_t> &getData() { return _data; }
  virtual void dump() const { std::cout << *_hdr << std::endl; }
  virtual void read(std::ifstream &fp) {}
  const ChunkHeader *getHeader() const { return _hdr.get(); }
  const std::vector<uint8_t> &getData() const { return _data; }
  static void writeChunk(const Chunk &chunk, const std::string &prefix);
  virtual void write(const std::string &prefix) const {
    writeChunk(*this, prefix);
  }
  static std::unique_ptr<Chunk> create(std::ifstream &fp,
                                       std::unique_ptr<ChunkHeader> hdr);
};

struct Form : public Chunk {
  // The _hdr->size is sizeof(formType) + sizeof(all the chunks in
  // this->chunks));
  ID formType;
  std::vector<std::unique_ptr<Chunk>> chunks;
  void dump() const override;
  Form(std::unique_ptr<ChunkHeader> hdr) : Chunk(std::move(hdr), FORM) {}
  void write(const std::string &prefix) const override;

 private:
  bool parse(std::ifstream &fp) override;
};

struct Marker : public Chunk {
  struct MarkerEntry {
    uint16_t markerID;
    uint32_t position;
    std::string markerName;
  };
  std::vector<std::unique_ptr<MarkerEntry>> markers;
  Marker(std::unique_ptr<ChunkHeader> hdr) : Chunk(std::move(hdr), MARKER) {}

 private:
  bool parse(std::ifstream &fp) override;
};

// This represents the audio file.
class AIFF {
  using ChunkContainer = std::vector<std::unique_ptr<Chunk>>;
  ChunkContainer chunks;
  std::string _filename;
  std::ifstream _file;
  static std::unique_ptr<ChunkHeader> readHeader(std::ifstream &fp);
  static std::unique_ptr<Chunk> readChunkBody(std::ifstream &fp,
                                              std::unique_ptr<ChunkHeader> hdr);

 public:
  AIFF(const char *fname) : _filename(fname), _file(fname) {}
  void readChunks();
  void dump() const;
  ChunkContainer::const_iterator begin() { return chunks.begin(); }
  ChunkContainer::const_iterator end() { return chunks.end(); }

  static std::unique_ptr<Chunk> readChunk(std::ifstream &fp);
};

#endif  // __CHUNK_HH
