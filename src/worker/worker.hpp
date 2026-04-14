#ifndef WORKER_H_
#define WORKER_H_

#include "parsing/parsing.hpp"

#include <cstdint>
#include <tuple>

namespace bin_unicorn {

struct WorkLoopResult {
    bool success{};
    uint32_t sleep_time_ms{};
    std::tuple<BinCollection, BinCollection> next_collections;
};

WorkLoopResult do_work_loop();

} // namespace bin_unicorn

#endif // WORKER_H_