
#include "manager.hpp"

#include <client.hpp>
#include <misc/logger.hpp>
#include <ui/manager.hpp>
#include <ui/gumpmenu.hpp>

#include "packetlist.hpp"

namespace uome {
namespace net {

Manager* Manager::singleton_ = 0;

bool Manager::create(const Config& config) {
    if (!singleton_) {
        try {
            singleton_ = new Manager(config);
        } catch (const std::exception& ex) {
            LOGARG_CRITICAL(LOGTYPE_NETWORK, "Error initializing world::Manager: %s", ex.what());
            return false;
        }
    }

    return true;
}

Manager* Manager::getSingleton() {
    if (!singleton_) {
        throw Exception("uome::net::Manager Singleton not created yet");
    }

    return singleton_;
}

void Manager::destroy() {
    if (singleton_) {
        delete singleton_;
        singleton_ = NULL;
    }
}

Manager::Manager(const Config& config) {
}

Manager::~Manager() {
    LOG_INFO(LOGTYPE_NETWORK, "Manager shutdown");
}

void Manager::step() {
    if (socket_.isConnected()) {
        // handle received packets
        boost::shared_ptr<Packet> packet = socket_.getNextPacket();
        while (packet) {
            packet->onReceive();
            packet = socket_.getNextPacket();
        }

        // send packets to server
        socket_.sendAll();
    }
}

boost::shared_ptr<Packet> Manager::createPacket(uint8_t id) {
    boost::shared_ptr<Packet> ret;

    switch (id) {
        case 0xA8: ret.reset(new packets::ServerList()); break;
        default: ret.reset(new packets::Unknown(id)); break;
    }

    return ret;
}

bool Manager::connect(ui::GumpMenu* menu, const std::string& parameter) {
    CL_LineEdit* ipCom = dynamic_cast<CL_LineEdit*>(menu->get_named_item("loginhost"));
    if (!ipCom) {
        ui::Manager::getSingleton()->openMessageBox("Unable to find input field for the server host. Did you change the login gump template?");
        return false;
    }

    CL_LineEdit* portCom = dynamic_cast<CL_LineEdit*>(menu->get_named_item("loginport"));
    if (!portCom) {
        ui::Manager::getSingleton()->openMessageBox("Unable to find input field for the server port. Did you change the login gump template?");
        return false;
    }

    CL_LineEdit* accCom = dynamic_cast<CL_LineEdit*>(menu->get_named_item("loginaccount"));
    if (!accCom) {
        ui::Manager::getSingleton()->openMessageBox("Unable to find input field for the account name. Did you change the login gump template?");
        return false;
    }

    CL_LineEdit* pwCom = dynamic_cast<CL_LineEdit*>(menu->get_named_item("loginpassword"));
    if (!pwCom) {
        ui::Manager::getSingleton()->openMessageBox("Unable to find input field for the password. Did you change the login gump template?");
        return false;
    }

    std::string host = ipCom->get_text();
    int port = portCom->get_text_int();
    if (port <= 0) {
        ui::Manager::getSingleton()->openMessageBox("Unable to find parse port number");
        return false;
    }

    if (socket_.connect(host, port)) {
        Client::getSingleton()->setState(Client::STATE_LOGIN);

        socket_.writeSeed(0);
        // send packet
        UnicodeString name(accCom->get_text().c_str());
        UnicodeString pass(pwCom->get_text().c_str());
        packets::LoginRequest req(name, pass);
        socket_.write(req);

        return true;
    } else {
        ui::Manager::getSingleton()->openMessageBox("Could not connect to server");
        return false;
    }
}

void Manager::disconnect() {
    socket_.close();
}

}
}