#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "../File/File.h"

#include <istream>
#include <fstream>

namespace Archive
{
	static void LoadBinaryArchive(boost::archive::binary_iarchive & Archive, File::CFile * File);
	static void LoadBinaryArchive(boost::archive::binary_iarchive & Archive, std::ifstream & File);
}