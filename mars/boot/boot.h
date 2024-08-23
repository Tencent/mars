//
// Created by Changpeng Pan on 2024/8/20.
//

#ifndef MMNET_BOOT_H
#define MMNET_BOOT_H
#include <string>

namespace mars {
namespace boot {

struct SetupConfig {
    int encoder_version;
    std::string encoder_name;
};

}  // namespace boot
}  // namespace mars
#endif  // MMNET_BOOT_H
