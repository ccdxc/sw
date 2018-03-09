#include <gtest/gtest.h>

#include "nic/hal/plugins/alg_rtsp/rtsp_parse.hpp"

using namespace hal::plugins::alg_rtsp;

class rtsp_parse_test : public ::testing::Test {
protected:
    rtsp_parse_test() {
    }

    virtual ~rtsp_parse_test() {
    }

    // will be called immediately after the constructor before each test
    virtual void SetUp() {
    }

    // will be called immediately after each test before the destructor
    virtual void TearDown() {
    }
};

TEST_F(rtsp_parse_test, rtsp_setup)
{
    rtsp_msg_t msg;
    uint32_t offset;
    int len;

    const char *buf =
        "SETUP rtsp://example.com/foo/bar/baz.rm RTSP/1.0\r\n"
        "CSeq: 302\r\n"
        "Transport: RTP/AVP;unicast;client_port=4588-4589\r\n"
        "\r\n"
        "RTSP/1.0 200 OK\r\n"
        "CSeq: 302\r\n"
        "Date: 23 Jan 1997 15:35:06 GMT\r\n"
        "Session: 47112344\r\n"
        "Transport: RTP/AVP;unicast;\r\n"
        "  source=192.168.1.100;destination=10.0.0.100;\r\n"
        "  client_port=4588-4589;server_port=6256-6257\r\n";

    len = strlen(buf);
    offset = 0;

    // Request
    EXPECT_TRUE(rtsp_parse_msg(buf, len, &offset, &msg));

    EXPECT_EQ(msg.type, RTSP_MSG_REQUEST);
    EXPECT_EQ(msg.ver, RTSP_V1);
    EXPECT_EQ(msg.req.method, RTSP_METHOD_SETUP);

    EXPECT_EQ(msg.hdrs.valid.cseq, 1);
    EXPECT_EQ(msg.hdrs.cseq, 302);

    EXPECT_EQ(msg.hdrs.valid.transport, 1);
    EXPECT_EQ(msg.hdrs.transport.nspecs, 1);
    EXPECT_EQ(msg.hdrs.transport.specs[0].client_port_start,  4588);
    EXPECT_EQ(msg.hdrs.transport.specs[0].client_port_end, 4589);
    EXPECT_EQ(msg.hdrs.transport.specs[0].ip_proto, IP_PROTO_UDP);
    
    EXPECT_EQ(msg.hdrs.valid.session, 0);
    EXPECT_EQ(msg.hdrs.valid.content_length, 0);

    // Response
    EXPECT_TRUE(rtsp_parse_msg(buf, len, &offset, &msg));

    EXPECT_EQ(msg.type, RTSP_MSG_RESPONSE);
    EXPECT_EQ(msg.ver, RTSP_V1);
    EXPECT_EQ(msg.rsp.status_code, 200);

    EXPECT_EQ(msg.hdrs.valid.cseq, 1);
    EXPECT_EQ(msg.hdrs.cseq, 302);

    EXPECT_EQ(msg.hdrs.valid.transport, 1);
    EXPECT_EQ(msg.hdrs.transport.nspecs, 1);
    EXPECT_STREQ(ipaddr2str(&msg.hdrs.transport.specs[0].client_ip), "10.0.0.100");
    EXPECT_EQ(msg.hdrs.transport.specs[0].client_port_start, 4588);
    EXPECT_EQ(msg.hdrs.transport.specs[0].client_port_end, 4589);
    EXPECT_STREQ(ipaddr2str(&msg.hdrs.transport.specs[0].server_ip), "192.168.1.100");
    EXPECT_EQ(msg.hdrs.transport.specs[0].server_port_start, 6256);
    EXPECT_EQ(msg.hdrs.transport.specs[0].server_port_end, 6257);
    EXPECT_EQ(msg.hdrs.transport.specs[0].ip_proto, IP_PROTO_UDP);

    EXPECT_EQ(msg.hdrs.valid.session, 1);
    EXPECT_STREQ(msg.hdrs.session.id, "47112344");
    EXPECT_EQ(msg.hdrs.session.timeout, DEFAULT_SESSION_TIMEOUT);

    EXPECT_EQ(msg.hdrs.valid.content_length, 0);

}

TEST_F(rtsp_parse_test, rtsp_setupv2)
{
    rtsp_msg_t msg;
    uint32_t offset;
    int len;

    const char *buf =
        "SETUP rtsp://example.com/foo/bar/baz.rm RTSP/2.0\r\n"
        "CSeq: 303\r\n"
        "Transport: RTP/AVP;unicast;dest_addr=\":4588\"/\":4589\",\r\n"
        "   RTP/AVP/TCP;unicast;interleaved=0-1\r\n"
        "Accept-Ranges: npt, clock\r\n"
        "User-Agent: PhonyClient/1.2\r\n"
        "\r\n"
        "RTSP/2.0 200 OK\r\n"
        "CSeq: 303\r\n"
        "Date: Thu, 23 Jan 1997 15:35:06 GMT\r\n"
        "Server: PhonyServer/1.1\r\n"
        "Session: QKyjN8nt2WqbWw4tIYof52;timeout=100\r\n"
        "Transport: RTP/AVP;unicast;dest_addr=\"192.0.2.53:4588\"/\r\n"
        "  \"192.0.2.53:4589\"; src_addr=\"198.51.100.241:6256\"/\r\n"
        "  \"198.51.100.241:6257\"; ssrc=2A3F93ED\r\n"
        "Accept-Ranges: npt\r\n"
        "Media-Properties: Random-Access=3.2, Time-Progressing,\r\n"
        "  Time-Duration=3600.0\r\n"
        "Media-Range: npt=0-2893.23\r\n";

    len = strlen(buf);
    offset = 0;

    // Request
    EXPECT_TRUE(rtsp_parse_msg(buf, len, &offset, &msg));

    EXPECT_EQ(msg.type, RTSP_MSG_REQUEST);
    EXPECT_EQ(msg.ver, RTSP_V2);
    EXPECT_EQ(msg.req.method, RTSP_METHOD_SETUP);

    EXPECT_EQ(msg.hdrs.valid.cseq, 1);
    EXPECT_EQ(msg.hdrs.cseq, 303);

    EXPECT_EQ(msg.hdrs.valid.transport, 1);
    EXPECT_EQ(msg.hdrs.transport.nspecs, 1);
    EXPECT_EQ(msg.hdrs.transport.specs[0].client_port_start, 4588);
    EXPECT_EQ(msg.hdrs.transport.specs[0].client_port_end, 4589);
    EXPECT_EQ(msg.hdrs.transport.specs[0].ip_proto, IP_PROTO_UDP);

    EXPECT_EQ(msg.hdrs.valid.session, 0);
    EXPECT_EQ(msg.hdrs.valid.content_length, 0);

    // Response
    EXPECT_TRUE(rtsp_parse_msg(buf, len, &offset, &msg));

    EXPECT_EQ(msg.type, RTSP_MSG_RESPONSE);
    EXPECT_EQ(msg.ver, RTSP_V2);
    EXPECT_EQ(msg.rsp.status_code, 200);

    EXPECT_EQ(msg.hdrs.valid.cseq, 1);
    EXPECT_EQ(msg.hdrs.cseq, 303);

    EXPECT_EQ(msg.hdrs.valid.transport, 1);
    EXPECT_EQ(msg.hdrs.transport.nspecs, 1);
    EXPECT_STREQ(ipaddr2str(&msg.hdrs.transport.specs[0].client_ip), "192.0.2.53");
    EXPECT_EQ(msg.hdrs.transport.specs[0].client_port_start, 4588);
    EXPECT_EQ(msg.hdrs.transport.specs[0].client_port_end, 4589);
    EXPECT_STREQ(ipaddr2str(&msg.hdrs.transport.specs[0].server_ip), "198.51.100.241");
    EXPECT_EQ(msg.hdrs.transport.specs[0].server_port_start, 6256);
    EXPECT_EQ(msg.hdrs.transport.specs[0].server_port_end, 6257);
    EXPECT_EQ(msg.hdrs.transport.specs[0].ip_proto, IP_PROTO_UDP);
    
    EXPECT_EQ(msg.hdrs.valid.session, 1);
    EXPECT_STREQ(msg.hdrs.session.id, "QKyjN8nt2WqbWw4tIYof52");
    EXPECT_EQ(msg.hdrs.session.timeout, 100);

    EXPECT_EQ(msg.hdrs.valid.content_length, 0);
}

