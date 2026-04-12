#ifndef WORKER_H_
#define WORKER_H_

#include "parsing/parsing.hpp"

#include <cstdint>
#include <expected>
#include <optional>
#include <span>

namespace worker {

struct WorkLoopResult {
    uint32_t sleep_time_ms;
    std::optional<parsing::BinCollection> next_collection;
};

WorkLoopResult do_work_loop(std::span<char> http_buffer);

} // namespace worker

#endif // WORKER_H_