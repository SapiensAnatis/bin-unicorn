#ifndef WORKER_HPP
#define WORKER_HPP

#include "parsing/parsing.hpp"

#include <chrono>
#include <cstdint>
#include <tuple>

namespace bin_unicorn {

struct WorkLoopResult {
    bool success{};
    uint32_t sleep_time_ms{};
    std::chrono::year_month_day rbc_server_date;
    std::tuple<BinCollection, BinCollection> next_collections;
};

WorkLoopResult do_work_loop();

} // namespace bin_unicorn

#endif // WORKER_HPP