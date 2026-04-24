#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include <optional>
#include <string>
#include <nlohmann/json.hpp>

#include "src/tastyworks/TastyWorks.h"
#include "src/streamer/DX_LinkStreamer.h"
#include "src/algo/Signal.h"
#include "src/model/TradeableAsset.h"

class Orchestrator
{
    private:
        TastyWorksClient& twClient_;
        DX_LinkStreamer& dxlStreamer_;
        TradeableAsset trAsset_;
    
    public:
        Orchestrator(TastyWorksClient& client, DX_LinkStreamer& streamer);

        std::optional<nlohmann::json> build_order_body(Signal signal, double balance) const;
        void on_signal(Signal signal);
};

#endif // ORCHESTRATOR_H
