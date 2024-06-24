#include <string>
#include <vector>

TString get_hash_input(const std::string& inputfile);
std::vector<std::string> load_filenames(const std::string& inputfile, const bool verbose=false);

TString get_hash_input(const std::string& inputfile)
{
  auto s = gSystem->GetFromPipe(::Form("md5sum %s", inputfile.c_str()));
  return s(0, 32);
}

std::vector<std::string> load_filenames(const std::string& inputfile, const bool verbose)
{
  TString s = gSystem->GetFromPipe(::Form("file %s", inputfile.c_str()));
  const bool isroot = s.Contains("ROOT file");
  const bool istext = s.Contains("ASCII text");
  if (isroot) {
    return std::vector<std::string>{inputfile};
  }
  if (istext) {
    std::vector<std::string > files;
    std::ifstream file(inputfile);
    std::string line;
    while(std::getline(file, line)) {
      files.push_back(line); 
      if (verbose) {
        std::cout << "Find file name: " << line << "\n";
      }
    }
    return files;
  }
  return {};
}
