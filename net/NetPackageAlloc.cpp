#include "common/Stdafx.h"
#include "common/NetCommand.h"
#include "net/NetPackageAlloc.h"
#include "net/NetPackage.h"
#include "stream.h"
#include "net/NetCommon.h"
#include "common/Constants.h"

namespace net
{
    NetPackageAlloc::NetPackageAlloc(ByteArray& bytes)
        :m_bytes(bytes)
    {
    }

    NetPackageAlloc::~NetPackageAlloc()
    {
    }

    NetPackageAlloc& NetPackageAlloc::defaultInstance()
    {
        static ByteArray bytes;
        static NetPackageAlloc instance(bytes);
        return instance;
    }

    void* NetPackageAlloc::Malloc(size_t sz)
    {
        m_bytes.resize(NET_PACKAGE_HEADER_LENGTH + sz);
        return &m_bytes[NET_PACKAGE_HEADER_LENGTH];
    }

    void* NetPackageAlloc::Realloc(void *p, size_t sz)
    {
        ByteArray raw = m_bytes;
        m_bytes.resize(NET_PACKAGE_HEADER_LENGTH + sz);
        memcpy(&m_bytes[0], &raw[0], raw.size() * sizeof(BYTE));
        return &m_bytes[NET_PACKAGE_HEADER_LENGTH];
    }

    void NetPackageAlloc::Free(void *p)
    {
        m_bytes.clear();
    }

    ByteArray& NetPackageAlloc::makeRequest(const int& seq, const int& commandId, r_uint8 compress, int& size)
    {
        bson::bo obj((const char*)(&m_bytes[NET_PACKAGE_HEADER_LENGTH]));
        switch (compress)
        {
        case COMPRESS_ZLIB:
        case COMPRESS_DOUBLE_ZLIB:
            {
                bson::bo raw = obj.copy();
                bool bOK = zlibCompress((unsigned char*)raw.objdata(), raw.objsize(), m_bytes, NET_PACKAGE_HEADER_LENGTH);
                size = m_bytes.size();

                NetPackageHeader header(commandId, seq, compress, 0);
                if (!bOK)
                {
                    header.m_flag.compress = COMPRESS_NONE;//压缩失败的话
                }
                header.m_length = size;
                header.encode(m_bytes);
            }
            break;
        case COMPRESS_LZMA:
        case COMPRESS_DOUBLE_LZMA:
            {
                bson::bo raw = obj.copy();
                bool bOK = LzmaComp((unsigned char* )raw.objdata(), raw.objsize(), m_bytes, NET_PACKAGE_HEADER_LENGTH);
                size = m_bytes.size();
                
                NetPackageHeader header(commandId, seq, compress, 0);
                if (!bOK)
                {
                    header.m_flag.compress = COMPRESS_NONE;//压缩失败的话
                }
                header.m_length = size;
                header.encode(m_bytes);
            }
            break;
        default:
            {
                size = obj.objsize() + NET_PACKAGE_HEADER_LENGTH;

                NetPackageHeader header(commandId, seq, compress, 0);
                header.m_length = size;
                header.encode(m_bytes);
                break;
            }
        }
        return m_bytes;
    }
}