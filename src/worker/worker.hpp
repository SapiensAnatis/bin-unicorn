#ifndef WORKER_H_
#define WORKER_H_

#include "parsing/parsing.hpp"

#include <cstdint>
#include <tuple>

namespace worker {

struct WorkLoopResult {
    bool success{};
    uint32_t sleep_time_ms{};
    std::tuple<parsing::BinCollection, parsing::BinCollection> next_collections;
};

WorkLoopResult do_work_loop();

} // namespace worker

#endif // WORKER_H_