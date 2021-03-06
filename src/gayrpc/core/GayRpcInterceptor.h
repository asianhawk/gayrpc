#pragma once

#include <functional>
#include <memory>
#include <vector>

#include <gayrpc/core/gayrpc_meta.pb.h>
#include <gayrpc/core/GayRpcType.h>

namespace gayrpc { namespace core {


    template<class... Args>
    UnaryServerInterceptor makeInterceptor(Args... args)
    {
        return [=](const RpcMeta& meta,
            const google::protobuf::Message& message,
            const UnaryHandler& tail,
            InterceptorContextType context) {

                class WrapNextInterceptor final
                {
                public:
                    WrapNextInterceptor(std::vector<UnaryServerInterceptor> interceptors,
                        UnaryHandler tail) noexcept
                        :
                        mCurIndex(0),
                        mInterceptors(std::move(interceptors)),
                        mTail(std::move(tail))
                    {
                    }

                    void operator () (const RpcMeta& meta,
                        const google::protobuf::Message& message,
                        InterceptorContextType context)
                    {
                        if (mInterceptors.size() == mCurIndex)
                        {
                            return mTail(meta, message, std::move(context));
                        }
                        else
                        {
                            mCurIndex++;
                            mInterceptors[mCurIndex - 1](meta, message, *this, std::move(context));
                        }
                    }

                    size_t  curIndex() const
                    {
                        return mCurIndex;
                    }

                private:
                    size_t  mCurIndex;
                    const std::vector<UnaryServerInterceptor> mInterceptors;
                    const UnaryHandler    mTail;
                };

                WrapNextInterceptor next({ args... }, tail);
                return next(meta, message, std::move(context));
        };
    }

} }
