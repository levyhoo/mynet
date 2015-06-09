#include <common/Stdafx.h>
#include <net/NetCommon.h>
#include <net/NetClient.h>
#include <net/NetServer.h>
using namespace net;
// #include <net/NetPackage.h>
// #include <net/ProxyHelper.h>


void sendData(NetClient* client)
{
    NetPackage pkg;
    pkg.m_pkgHeader.m_seq = (r_int64)1024 * 1024 * 1024 + 123455;
    pkg.m_pkgHeader.m_flag.reserved = 15;
    pkg.m_pkgHeader.m_flag.compress = 6;
    pkg.m_pkgContent = ByteArray(6, '*');
    ByteArray arrData = pkg.encode();
    client->sendData(arrData, arrData.size());
}

void testProxy()
{
    io_service ioservice;
    NetClient client(ioservice, "210.14.136.68", 43322);

    ProxyInfo info;
    info.ip   = "10.10.1.66";

    //info.port = 8082;
    //info.type = PROXY_TYPE_HTTP;

    info.port = 1082;
    info.type = PROXY_TYPE_SOCKS5;

    info.need_check = true;
    info.username = "test";
    info.password = "test123";

    client.setProxy(info);

    client.start();
    ioservice.run();

}

void testClient()
{
    io_service ioservice;
    NetClient client(ioservice, "127.0.0.1", 12345);
    client.start(false);

    asio::deadline_timer sendTimer(ioservice);
    sendTimer.expires_from_now(boost::posix_time::seconds(3));
    sendTimer.async_wait(boost::bind(&sendData, &client));

    ioservice.run();
}

void testServer()
{
    io_service ioservice;
    NetServer server(ioservice, "0.0.0.0", 12345);
    server.start();
    ioservice.run();
}

int main()
{
    boost::thread *serverThread = new boost::thread(&testServer);
    //boost::thread *clientThread = new boost::thread(&testClient);
    //boost::thread *clientThread2 = new boost::thread(&testProxy);

    serverThread->join();
    //clientThread->join();
    //clientThread2->join();

    return 0;
}
