#include <string>
#include <stdint.h>

using namespace std;

struct IndexFile
{
    string base_name;
    uint32_t file_count;

    IndexFile(string b, uint32_t f): base_name(b), file_count(f)
    {}
};

class IndexFileManager
{
public:
    vector<IndexFile*> index_file_vec_;
    IndexFileManager()
    {}
    int32_t ScanAllIndexFile();
};
