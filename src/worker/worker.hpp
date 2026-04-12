#ifndef WORKER_H_
#define WORKER_H_

#include "parsing/parsing.hpp"

#include <cstdint>
#include <optional>

namespace worker {

struct WorkLoopResult {
    uint32_t sleep_time_ms;
    std::optional<parsing::BinCollection> next_collection;
};

WorkLoopResult do_work_loop();

} // namespace worker

#endif // WORKER_H_