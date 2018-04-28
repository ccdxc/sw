// {C} Copyright 2018 Pensando Systems Inc. All rights reserved.

// TCP server implementation

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <netinet/in.h>
#include <resolv.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "sock_utils.hpp"

namespace delphi {
namespace messanger {

using namespace std;
using namespace google::protobuf::io;


// ------ utility functions
int
readHdr(char *buf, google::protobuf::uint32 *size)
{
  google::protobuf::io::ArrayInputStream ais(buf, 4);
  CodedInputStream coded_input(&ais);
  coded_input.ReadVarint32(size);
  int read = coded_input.CurrentPosition();

  return read;
}

// readBody reads a message from socket
MessagePtr
readBody(int csock, int headerSize, google::protobuf::uint32 siz)
{
    int bytecount;
    MessagePtr msg(make_shared<Message>());
    char buffer [headerSize + siz];//size of the payload and hdr
    assert(msg != NULL);

    //Read the entire buffer including the hdr
    if((bytecount = recv(csock, (void *)buffer, headerSize + siz, MSG_WAITALL))== -1){
      LogError("Error receiving data {}", errno);
      assert(0);
    }

    //Assign ArrayInputStream with enough memory
    google::protobuf::io::ArrayInputStream ais(buffer, headerSize + siz);
    CodedInputStream coded_input(&ais);

    //Read an unsigned integer with Varint encoding, truncating to 32 bits.
    coded_input.ReadVarint32(&siz);
    assert(siz != 0);

    //After the message's length is read, PushLimit() is used to prevent the CodedInputStream
    //from reading beyond that length.Limits are used when parsing length-delimited
    //embedded messages
    google::protobuf::io::CodedInputStream::Limit msgLimit = coded_input.PushLimit(siz);

    //De-Serialize
    bool succ = msg->ParseFromCodedStream(&coded_input);
    if (succ != true) {
        LogError("Error parsing message of size {}, bytecount {}", siz, bytecount);
        assert(succ == true);
    }

    //Once the embedded message has been parsed, PopLimit() is called to undo the limit
    coded_input.PopLimit(msgLimit);

    //Print the message
    LogDebug("Received message: \n{}", msg->DebugString());

    return msg;
}

// sendMsg sends a message on a socket
error sendMsg(int sock, MessagePtr msg) {
    int len = msg->ByteSize() + 8; // 8 is the maximum size of the Varint
    char *pkt = (char *)malloc(len);
    int packetLength = 0; // packetLength is the size of the message + the size of the length, which is varint

    // encode the size of the message followed by the message itself
    google::protobuf::io::ArrayOutputStream aos(pkt, len);
    CodedOutputStream *coded_output = new CodedOutputStream(&aos);
    coded_output->WriteVarint32(msg->ByteSize());
    msg->SerializeToCodedStream(coded_output);
    packetLength = coded_output->ByteCount();

    // send the message
    char *buffer = pkt;
    while(packetLength > 0) {
        int nsent = send(sock, buffer, packetLength, 0);
        if (nsent == -1) {
            LogError("Error sending data {}", errno);
            assert(nsent != -1);
            return error::New("Socket send failure");
        } else if (nsent == 0) {
            LogError("Remote side closed socket. errno {}", errno);
            return error::New("Remote side closed socket");
        } else {
            buffer += nsent;
            packetLength -= nsent;
        }
    }

/*
    int bytecount = send(sock, (void *)pkt, siz, 0);
    if(bytecount == -1) {
        LogError("Error sending data {}", errno);
        assert(bytecount != -1);
    }
    assert(bytecount == siz);
*/

    free(pkt);
    return 0;
}

} // namespace messanger
} // namespace delphi
