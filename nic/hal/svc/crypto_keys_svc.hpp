#ifndef __CRYPT_KEYS_SVC_HPP__
#define __CRYPT_KEYS_SVC_HPP__
#include <base.h>
#include <grpc++/grpc++.h>
#include <types.pb.h>
#include <crypto_keys.grpc.pb.h>

using grpc::ServerContext;
using grpc::Status;

using cryptokey::CryptoKey;
using cryptokey::CryptoKeySpec;

using cryptokey::CryptoKeyCreateRequest;
using cryptokey::CryptoKeyCreateRequestMsg;
using cryptokey::CryptoKeyCreateResponse;
using cryptokey::CryptoKeyCreateResponseMsg;

using cryptokey::CryptoKeyReadRequest;
using cryptokey::CryptoKeyReadRequestMsg;
using cryptokey::CryptoKeyReadResponse;
using cryptokey::CryptoKeyReadResponseMsg;

using cryptokey::CryptoKeyUpdateRequest;
using cryptokey::CryptoKeyUpdateRequestMsg;
using cryptokey::CryptoKeyUpdateResponse;
using cryptokey::CryptoKeyUpdateResponseMsg;


using cryptokey::CryptoKeyDeleteRequest;
using cryptokey::CryptoKeyDeleteRequestMsg;
using cryptokey::CryptoKeyDeleteResponse;
using cryptokey::CryptoKeyDeleteResponseMsg;


class CryptoKeyServiceImpl final : public CryptoKey::Service {

    public:
        Status CryptoKeyCreate(ServerContext* context,
                const CryptoKeyCreateRequestMsg* request,
                CryptoKeyCreateResponseMsg* response) override;

        Status CryptoKeyRead(ServerContext* context,
                const CryptoKeyReadRequestMsg* request,
                CryptoKeyReadResponseMsg* response) override;

        Status CryptoKeyUpdate(ServerContext* context,
                const CryptoKeyUpdateRequestMsg* request,
                CryptoKeyUpdateResponseMsg* response) override;

        Status CryptoKeyDelete(ServerContext* context,
                const CryptoKeyDeleteRequestMsg* request,
                CryptoKeyDeleteResponseMsg* response) override;
};

#endif  /* __CRYPT_KEYS_SVC_HPP__ */
