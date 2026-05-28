#pragma once

#include "ICollector.hpp"

#include <vector>
#include <string>
#include <cstdint>

namespace sectop{
    struct Connection {
        std::string state;
        std::string local_ip;
        uint16_t local_port;
        std::string remote_ip;
        uint16_t remote_port;
        uint32_t inode;
    };


    class NetCollector : public ICollector{
        private:
            int nl_fd;
            std::vector <Connection> current_connections;
            std::string get_state_string(uint8_t state);

        public:
            NetCollector();
            ~NetCollector() override;

            void collect() override;

            const std::vector<Connection>& get_connections() const;
    };
}
