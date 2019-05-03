#include <fstream>
#include <iostream>

#include "nyx/example/image.h"
#include "nyx/example/protobuf.h"


using namespace nyx::example::image;
using namespace nyx::example::protobuf;


static bool slurp(const std::string &name, std::vector<uint8_t> &data) {
  std::ifstream ifs(name.c_str(), std::ios::in | std::ios::binary | std::ios::ate);

  if(ifs) {
    std::ifstream::pos_type fileSize = ifs.tellg();
    ifs.seekg(0, std::ios::beg);

    data.resize(fileSize);
    ifs.read(reinterpret_cast<char *>(&data[0]), fileSize);

    return true;
  }

  return false;
}


int main(int argc, char **argv) {
  std::vector<uint8_t> data;

  for(int i = 1; i < argc; ++i) {
    std::string name(argv[i]);

    if(!name.compare(name.size() - 4, 4, ".png")) {
      png file_chunks;

      if(slurp(name, data)) {
        file_chunks.consume(&data[0], data.size());
        std::cout << "File " << name << " contained " << file_chunks.chunks.size() <<
                    " chunks" << std::endl;
      }
    }
    else if(!name.compare(name.size() - 6, 6, ".proto")) {
      message raw_message;

      if(slurp(name, data)) {
        raw_message.consume(&data[0], data.size());
        std::cout << "File " << name << " contained " << raw_message.fields.size() <<
                    " fields" << std::endl;

        for(auto &field : raw_message.fields) {
          std::cout << "  field[" << field.header.number << "] is type " <<
                       static_cast<int>(field.header.type) << std::endl;
        }
      }
    }
  }

  return 0;
}

