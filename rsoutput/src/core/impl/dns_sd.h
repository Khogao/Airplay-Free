#ifndef DNS_SD_H
#define DNS_SD_H

#include <stdint.h>

#if defined(__cplusplus)
extern "C"
{
#endif

#if defined(_WIN32)
#include <windows.h>
#define DNSSD_API __stdcall
#else
#define DNSSD_API
#endif

#define kDNSServiceType_A 1
#define kDNSServiceType_TXT 16
#define kDNSServiceMaxDomainName 1009
#define kDNSServiceInterfaceIndexAny 0
#define kDNSServiceClass_IN 1
#define kDNSServiceFlagsAdd 0x2
#define kDNSServiceFlagsLongLivedQuery 0x1
#define kDNSServiceFlagsAllowRemoteQuery 0x200
#define kDNSServiceFlagsSuppressUnusable 0x8000
#define kDNSServiceErr_ServiceNotRunning -65563
#define kDNSServiceProperty_DaemonVersion "DaemonVersion"

    typedef struct _DNSServiceRef_t *DNSServiceRef;
    typedef struct _TXTRecordRef_t
    {
        char private_data[16];
    } TXTRecordRef;
    typedef uint32_t DNSServiceFlags;
    typedef int32_t DNSServiceErrorType;

#define kDNSServiceErr_NoError 0
#define kDNSServiceErr_Unknown -65537

    typedef void(DNSSD_API *DNSServiceRegisterReply)(
        DNSServiceRef sdRef,
        DNSServiceFlags flags,
        DNSServiceErrorType errorCode,
        const char *name,
        const char *regtype,
        const char *domain,
        void *context);

    typedef void(DNSSD_API *DNSServiceBrowseReply)(
        DNSServiceRef sdRef,
        DNSServiceFlags flags,
        uint32_t interfaceIndex,
        DNSServiceErrorType errorCode,
        const char *serviceName,
        const char *regtype,
        const char *replyDomain,
        void *context);

    typedef void(DNSSD_API *DNSServiceResolveReply)(
        DNSServiceRef sdRef,
        DNSServiceFlags flags,
        uint32_t interfaceIndex,
        DNSServiceErrorType errorCode,
        const char *fullname,
        const char *hosttarget,
        uint16_t port,
        uint16_t txtLen,
        const unsigned char *txtRecord,
        void *context);

    typedef void(DNSSD_API *DNSServiceQueryRecordReply)(
        DNSServiceRef sdRef,
        DNSServiceFlags flags,
        uint32_t interfaceIndex,
        DNSServiceErrorType errorCode,
        const char *fullname,
        uint16_t rrtype,
        uint16_t rrclass,
        uint16_t rdlen,
        const void *rdata,
        uint32_t ttl,
        void *context);

    DNSServiceErrorType DNSSD_API DNSServiceRegister(
        DNSServiceRef *sdRef,
        DNSServiceFlags flags,
        uint32_t interfaceIndex,
        const char *name,
        const char *regtype,
        const char *domain,
        const char *host,
        uint16_t port,
        uint16_t txtLen,
        const void *txtRecord,
        DNSServiceRegisterReply callBack,
        void *context);

    DNSServiceErrorType DNSSD_API DNSServiceBrowse(
        DNSServiceRef *sdRef,
        DNSServiceFlags flags,
        uint32_t interfaceIndex,
        const char *regtype,
        const char *domain,
        DNSServiceBrowseReply callBack,
        void *context);

    DNSServiceErrorType DNSSD_API DNSServiceResolve(
        DNSServiceRef *sdRef,
        DNSServiceFlags flags,
        uint32_t interfaceIndex,
        const char *name,
        const char *regtype,
        const char *domain,
        DNSServiceResolveReply callBack,
        void *context);

    DNSServiceErrorType DNSSD_API DNSServiceQueryRecord(
        DNSServiceRef *sdRef,
        DNSServiceFlags flags,
        uint32_t interfaceIndex,
        const char *fullname,
        uint16_t rrtype,
        uint16_t rrclass,
        DNSServiceQueryRecordReply callBack,
        void *context);

    void DNSSD_API DNSServiceRefDeallocate(DNSServiceRef sdRef);
    DNSServiceErrorType DNSSD_API DNSServiceProcessResult(DNSServiceRef sdRef);

    void DNSSD_API TXTRecordCreate(
        TXTRecordRef *txtRecord,
        uint16_t bufferLen,
        void *buffer);

    void DNSSD_API TXTRecordDeallocate(TXTRecordRef *txtRecord);

    DNSServiceErrorType DNSSD_API TXTRecordSetValue(
        TXTRecordRef *txtRecord,
        const char *key,
        uint8_t valueSize,
        const void *value);

    const void *DNSSD_API TXTRecordGetValuePtr(
        uint16_t txtLen,
        const void *txtRecord,
        const char *key,
        uint8_t *valueLen);

    int DNSSD_API TXTRecordContainsKey(
        uint16_t txtLen,
        const void *txtRecord,
        const char *key);

    uint16_t DNSSD_API TXTRecordGetLength(const TXTRecordRef *txtRecord);
    const void *DNSSD_API TXTRecordGetBytesPtr(const TXTRecordRef *txtRecord);

#if defined(__cplusplus)
}
#endif

#endif
