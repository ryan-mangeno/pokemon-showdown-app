#include "pkmpch.h"

#include "util/json_loader.h"
#include "core/logger.h"
#include "net/netconfig.h"
#include "net/wsclient.h"
#include "protocol/parser.h"

int main() {

    pkm::Logger::init();
    pkm::net::NetConfig ncfg;
    pkm::JsonLoader::load(ncfg, NET_CONFIG_PATH.c_str());

    pkm::net::WsClient client(ncfg);
    if (client.connect()) {
        auto msgs = pkm::protocol::parse(client.receive());
        for (auto& msg : msgs) {
            PK_INFO("Room: {0}", msg.room_id);
            PK_INFO("Type: {0}", msg.type);
            for (auto& arg : msg.args) {
                PK_INFO("Arg: {0}", arg);
            }
        }
    }

    client.close();

    return 0;
}
