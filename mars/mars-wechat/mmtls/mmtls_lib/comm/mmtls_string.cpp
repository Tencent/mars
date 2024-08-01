#include "mmtls_string.h"

namespace mmtls {

String operator+(const String& lhs, const String& rhs) {
    String str;
    str.append(lhs);
    str.append(rhs);
    return str;
}

bool operator==(const char* lhs, const String& rhs) {
    return rhs.operator==(lhs);
}

bool operator==(const std::string& lhs, const String& rhs) {
    return rhs.operator==(lhs);
}

bool operator==(const char* lhs, const StringPiece& rhs) {
    return rhs.operator==(lhs);
}

bool operator==(const std::string& lhs, const StringPiece& rhs) {
    return rhs.operator==(lhs);
}

}  // namespace mmtls
