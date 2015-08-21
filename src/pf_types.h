#ifndef PF_TYPES_H_DIEGO_2014_06_28_10_56
#define	PF_TYPES_H_DIEGO_2014_06_28_10_56

// header file to define common types, allowing to add UNICODE support for filenames

#include <string>
#include <array>
#include <cstdint>

namespace pf {

using HeaderSize = std::uint16_t;
using Version = std::uint16_t;
using FileIndex = std::uint16_t;
using FilePos = std::uint64_t;
    
typedef char char_type;
typedef std::basic_string<char_type> Filename;
typedef std::basic_string<char_type> ExternalFilename;

constexpr Version currentVersion = 1;

constexpr char_type separator = '/';
constexpr size_t MaxInternalName = 200;
typedef std::array<char_type,MaxInternalName> InternalName;

struct BStr {
    unsigned short size;
    InternalName name;
};

struct FileEntry {
    BStr filename;
    FilePos pos;
    FilePos size;
};

}

#endif	/* PF_TYPES_H */
