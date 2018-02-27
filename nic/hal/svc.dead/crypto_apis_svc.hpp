#ifndef __CRYPTO_APIS_SVC_HPP__
#define __CRYPTO_APIS_SVC_HPP__
#include "nic/include/base.h"
#include "grpc++/grpc++.h"
#include "nic/gen/proto/hal/types.pb.h"
#include "nic/gen/proto/hal/crypto_apis.grpc.pb.h"

using grpc::ServerContext;
using grpc::Status;

using cryptoapis::CryptoApis;

using cryptoapis::CryptoApiRequest;
using cryptoapis::CryptoApiRequestMsg;
using cryptoapis::CryptoApiResponse;
using cryptoapis::CryptoApiResponseMsg;

class CryptoApisServiceImpl final : public CryptoApis::Service {

    public:
        Status CryptoApiInvoke(ServerContext* context,
                const CryptoApiRequestMsg* request,
                CryptoApiResponseMsg* response) override;
};



#endif  /* __CRYPTO_APIS_SVC_HPP__ */
