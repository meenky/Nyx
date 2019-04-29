#include <nyx/filesystem.h>

#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>


using namespace nyx;


Filesystem::Filesystem(const std::string &sys, const std::vector<std::string> &user):
  paths() {
  // fill the path vector
  paths.reserve(user.size() + 2);
  paths.emplace_back(".");
  paths.emplace_back(sys);
  paths.insert(paths.end(), user.begin(), user.end());

  // normalize the path vector
  for(auto iter = paths.begin(); iter != paths.end(); ++iter) {
retry:
    if(iter->size() == 0) { // remove empty strings
      if((iter = paths.erase(iter)) != paths.end()) {
        goto retry; // don't increment the iterator again
      }
      else {
        break;
      }
    }

    while(iter->back() == '/') {
      iter->resize(iter->size() - 1); // remove trailing slashes
    }
  }
}


std::shared_ptr<std::string> Filesystem::locate(const std::string &filename) const {
  struct stat buffer;
  std::string fullname;

  // simple linear search
  for(auto &path : paths) {
    fullname.assign(path).append(1, '/').append(filename);

    if(!stat(fullname.c_str(), &buffer) && (buffer.st_mode & S_IFMT) == S_IFREG) {
      return std::make_shared<std::string>(fullname);
    }
  }

  return nullptr;
}

