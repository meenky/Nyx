#include <nyx.h>

#include <array>
#include <string>
#include <vector>
#include <getopt.h>
#include <iostream>
#include <stdlib.h>


#ifndef DEFAULT_SYSROOT
#  define DEFAULT_SYSROOT  "/usr/include/nyx"
#endif

#ifndef DEFAULT_LANGUAGE
#  define DEFAULT_LANGUAGE "c++"
#endif

struct Settings {
  Settings():
    inputs(),
    outdir("."),
    options(),
    sysroot(DEFAULT_SYSROOT),
    language(DEFAULT_LANGUAGE),
    includes() {
  }

  std::vector<std::string> inputs;
  std::string              outdir;
  std::vector<std::string> options;
  std::string              sysroot;
  std::string              language;
  std::vector<std::string> includes;
};


static void processCommandLine(Settings &, int, char **);


int main(int argc, char **argv) {
  Settings settings;
  int retVal = EXIT_SUCCESS;

  processCommandLine(settings, argc, argv);

  nyx::Registry registry;
  nyx::Filesystem fs(settings.sysroot, settings.includes);

  for(auto &file : settings.inputs) {
    if(!registry.parse(fs, file)) {
      retVal = EXIT_FAILURE;
      break;
    }
  }

  if(retVal == EXIT_SUCCESS) {
    if(auto plan = nyx::Plan::generate(registry)) {
      if(auto plugin = nyx::Plugin::load(fs, settings.language, settings.options)) {
        retVal = plugin->execute(*plan);
      }
      else {
        retVal = EXIT_FAILURE;
      }
    }
    else {
      retVal = EXIT_FAILURE;
    }
  }

  return retVal;
}


static const std::array<struct option, 10> LOPTS{
  option{ "help",     no_argument,       nullptr, 'h' }, // print help and exit
  option{ "include",  required_argument, nullptr, 'I' }, // add a user include directory
  option{ "lang",     required_argument, nullptr, 'l' }, // select output language
  option{ "language", required_argument, nullptr, 'l' }, // select output language
  option{ "outdir",   required_argument, nullptr, 'o' }, // select base output directory
  option{ "opt",      required_argument, nullptr, 'O' }, // specify a language specific option
  option{ "option",   required_argument, nullptr, 'O' }, // specify a language specific option
  option{ "sysroot",  required_argument, nullptr, 'S' }, // change the system include directory
  option{ "ver",      no_argument,       nullptr, 'v' }, // print version info and exit
  option{ "version",  no_argument,       nullptr, 'v' }  // print version info and exit
};


static const char *SHOPTS = "hI:l:O:o:S:v";


static void processCommandLine(Settings &settings, int argc, char **argv) {
  int opt;
  bool shouldExit = false;

  while((opt = getopt_long(argc, argv, SHOPTS, LOPTS.data(), nullptr)) != -1) {
    switch(opt) {
      case 'h':
        std::cout <<
          "Usage: " << *argv << " [hIlOoSv]" << std::endl <<
          "  Where: " << std::endl <<
          "    -h, --help                    print this message and exit" << std::endl <<
          "    -I, --include DIR             add a directory to search for imports" << std::endl <<
          "    -l, --lang, --language LANG   select an output language" << std::endl <<
          "    -o, --outdir DIR              specify the base output directory" << std::endl <<
          "    -O, --opt, --option OPT       pass an option to the output plugin" << std::endl <<
          "    -S, --sysroot DIR             specify the system import directory" << std::endl <<
          "    -v, --ver, --version          print the version and exit" << std::endl;
        shouldExit = true;
      break;

      case 'I':
        settings.includes.emplace_back(optarg);
      break;

      case 'l':
        settings.language.assign(optarg);
      break;

      case 'o':
        settings.outdir.assign(optarg);
      break;

      case 'O':
        settings.options.emplace_back(optarg);
      break;

      case 'S':
        settings.sysroot.assign(optarg);
      break;

      case 'v':
        std::cout << *argv << " " << NYX_VERSION_STRING << std::endl;
        shouldExit = true;
      break;

      case '?':
        exit(EXIT_FAILURE);
      break;
    }
  }

  if(shouldExit) {
    exit(EXIT_SUCCESS);
  }

  // place the output directory in the list of plugin options
  if(settings.outdir != ".") {
    std::string outdir;
    outdir.reserve(settings.outdir.size() + 7);
    outdir.append("outdir=").append(settings.outdir);
    settings.options.emplace_back(outdir);
  }

  for(int arg = optind; arg < argc; ++arg) {
    settings.inputs.emplace_back(argv[arg]);
  }
}
