#include "storage.h"

namespace storage
{

Storage::Storage(Logger *logger)
: logger_(logger), id_mapper_(logger)
{

}

}
