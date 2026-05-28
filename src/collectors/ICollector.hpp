#pragma once

namespace sectop{

    class ICollector{
        public: 
            virtual void collect() = 0;

            virtual ~ICollector() = default;
    };
}

