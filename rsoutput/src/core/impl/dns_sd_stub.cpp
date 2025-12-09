#include "dns_sd.h"
#include <windows.h>
#include <string>
#include <vector>

// Function pointers
typedef DNSServiceErrorType(DNSSD_API *DNSServiceRegisterType)(DNSServiceRef *, DNSServiceFlags, uint32_t, const char *, const char *, const char *, const char *, uint16_t, uint16_t, const void *, DNSServiceRegisterReply, void *);
typedef DNSServiceErrorType(DNSSD_API *DNSServiceBrowseType)(DNSServiceRef *, DNSServiceFlags, uint32_t, const char *, const char *, DNSServiceBrowseReply, void *);
typedef DNSServiceErrorType(DNSSD_API *DNSServiceResolveType)(DNSServiceRef *, DNSServiceFlags, uint32_t, const char *, const char *, const char *, DNSServiceResolveReply, void *);
typedef DNSServiceErrorType(DNSSD_API *DNSServiceQueryRecordType)(DNSServiceRef *, DNSServiceFlags, uint32_t, const char *, uint16_t, uint16_t, DNSServiceQueryRecordReply, void *);
typedef void(DNSSD_API *DNSServiceRefDeallocateType)(DNSServiceRef);
typedef DNSServiceErrorType(DNSSD_API *DNSServiceProcessResultType)(DNSServiceRef);

typedef void(DNSSD_API *TXTRecordCreateType)(TXTRecordRef *, uint16_t, void *);
typedef void(DNSSD_API *TXTRecordDeallocateType)(TXTRecordRef *);
typedef DNSServiceErrorType(DNSSD_API *TXTRecordSetValueType)(TXTRecordRef *, const char *, uint8_t, const void *);
typedef const void *(DNSSD_API *TXTRecordGetValuePtrType)(uint16_t, const void *, const char *, uint8_t *);
typedef int(DNSSD_API *TXTRecordContainsKeyType)(uint16_t, const void *, const char *);
typedef uint16_t(DNSSD_API *TXTRecordGetLengthType)(const TXTRecordRef *);
typedef const void *(DNSSD_API *TXTRecordGetBytesPtrType)(const TXTRecordRef *);

static HMODULE g_dnssdLibrary = NULL;
static DNSServiceRegisterType fn_DNSServiceRegister = NULL;
static DNSServiceBrowseType fn_DNSServiceBrowse = NULL;
static DNSServiceResolveType fn_DNSServiceResolve = NULL;
static DNSServiceQueryRecordType fn_DNSServiceQueryRecord = NULL;
static DNSServiceRefDeallocateType fn_DNSServiceRefDeallocate = NULL;
static DNSServiceProcessResultType fn_DNSServiceProcessResult = NULL;

static TXTRecordCreateType fn_TXTRecordCreate = NULL;
static TXTRecordDeallocateType fn_TXTRecordDeallocate = NULL;
static TXTRecordSetValueType fn_TXTRecordSetValue = NULL;
static TXTRecordGetValuePtrType fn_TXTRecordGetValuePtr = NULL;
static TXTRecordContainsKeyType fn_TXTRecordContainsKey = NULL;
static TXTRecordGetLengthType fn_TXTRecordGetLength = NULL;
static TXTRecordGetBytesPtrType fn_TXTRecordGetBytesPtr = NULL;

bool EnsureBonjourLoaded()
{
    if (g_dnssdLibrary)
        return true;

    // Try to find the module if already loaded
    g_dnssdLibrary = GetModuleHandleA("dnssd.dll");

    if (!g_dnssdLibrary)
    {
        const char *paths[] = {
            "dnssd.dll",
            "C:\\Program Files\\Bonjour\\dnssd.dll",
            "C:\\Program Files (x86)\\Bonjour\\dnssd.dll"};

        for (const char *path : paths)
        {
            g_dnssdLibrary = LoadLibraryA(path);
            if (g_dnssdLibrary)
            {
                // Add to DLL search path
                std::string p = path;
                size_t lastSlash = p.find_last_of("\\/");
                if (lastSlash != std::string::npos)
                {
                    std::string dir = p.substr(0, lastSlash);
                    SetDllDirectoryA(dir.c_str());
                }
                break;
            }
        }
    }

    if (!g_dnssdLibrary)
        return false;

    fn_DNSServiceRegister = (DNSServiceRegisterType)GetProcAddress(g_dnssdLibrary, "DNSServiceRegister");
    fn_DNSServiceBrowse = (DNSServiceBrowseType)GetProcAddress(g_dnssdLibrary, "DNSServiceBrowse");
    fn_DNSServiceResolve = (DNSServiceResolveType)GetProcAddress(g_dnssdLibrary, "DNSServiceResolve");
    fn_DNSServiceQueryRecord = (DNSServiceQueryRecordType)GetProcAddress(g_dnssdLibrary, "DNSServiceQueryRecord");
    fn_DNSServiceRefDeallocate = (DNSServiceRefDeallocateType)GetProcAddress(g_dnssdLibrary, "DNSServiceRefDeallocate");
    fn_DNSServiceProcessResult = (DNSServiceProcessResultType)GetProcAddress(g_dnssdLibrary, "DNSServiceProcessResult");

    fn_TXTRecordCreate = (TXTRecordCreateType)GetProcAddress(g_dnssdLibrary, "TXTRecordCreate");
    fn_TXTRecordDeallocate = (TXTRecordDeallocateType)GetProcAddress(g_dnssdLibrary, "TXTRecordDeallocate");
    fn_TXTRecordSetValue = (TXTRecordSetValueType)GetProcAddress(g_dnssdLibrary, "TXTRecordSetValue");
    fn_TXTRecordGetValuePtr = (TXTRecordGetValuePtrType)GetProcAddress(g_dnssdLibrary, "TXTRecordGetValuePtr");
    fn_TXTRecordContainsKey = (TXTRecordContainsKeyType)GetProcAddress(g_dnssdLibrary, "TXTRecordContainsKey");
    fn_TXTRecordGetLength = (TXTRecordGetLengthType)GetProcAddress(g_dnssdLibrary, "TXTRecordGetLength");
    fn_TXTRecordGetBytesPtr = (TXTRecordGetBytesPtrType)GetProcAddress(g_dnssdLibrary, "TXTRecordGetBytesPtr");

    return true;
}

DNSServiceErrorType DNSServiceRegister(
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
    void *context)
{
    if (!EnsureBonjourLoaded() || !fn_DNSServiceRegister)
        return kDNSServiceErr_Unknown;
    return fn_DNSServiceRegister(sdRef, flags, interfaceIndex, name, regtype, domain, host, port, txtLen, txtRecord, callBack, context);
}

DNSServiceErrorType DNSServiceBrowse(
    DNSServiceRef *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    const char *regtype,
    const char *domain,
    DNSServiceBrowseReply callBack,
    void *context)
{
    if (!EnsureBonjourLoaded() || !fn_DNSServiceBrowse)
        return kDNSServiceErr_Unknown;
    return fn_DNSServiceBrowse(sdRef, flags, interfaceIndex, regtype, domain, callBack, context);
}

DNSServiceErrorType DNSServiceResolve(
    DNSServiceRef *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    const char *name,
    const char *regtype,
    const char *domain,
    DNSServiceResolveReply callBack,
    void *context)
{
    if (!EnsureBonjourLoaded() || !fn_DNSServiceResolve)
        return kDNSServiceErr_Unknown;
    return fn_DNSServiceResolve(sdRef, flags, interfaceIndex, name, regtype, domain, callBack, context);
}

DNSServiceErrorType DNSServiceQueryRecord(
    DNSServiceRef *sdRef,
    DNSServiceFlags flags,
    uint32_t interfaceIndex,
    const char *fullname,
    uint16_t rrtype,
    uint16_t rrclass,
    DNSServiceQueryRecordReply callBack,
    void *context)
{
    if (!EnsureBonjourLoaded() || !fn_DNSServiceQueryRecord)
        return kDNSServiceErr_Unknown;
    return fn_DNSServiceQueryRecord(sdRef, flags, interfaceIndex, fullname, rrtype, rrclass, callBack, context);
}

void DNSServiceRefDeallocate(DNSServiceRef sdRef)
{
    if (EnsureBonjourLoaded() && fn_DNSServiceRefDeallocate)
        fn_DNSServiceRefDeallocate(sdRef);
}

DNSServiceErrorType DNSServiceProcessResult(DNSServiceRef sdRef)
{
    if (!EnsureBonjourLoaded() || !fn_DNSServiceProcessResult)
        return kDNSServiceErr_Unknown;
    return fn_DNSServiceProcessResult(sdRef);
}

void TXTRecordCreate(
    TXTRecordRef *txtRecord,
    uint16_t bufferLen,
    void *buffer)
{
    if (EnsureBonjourLoaded() && fn_TXTRecordCreate)
        fn_TXTRecordCreate(txtRecord, bufferLen, buffer);
}

void TXTRecordDeallocate(TXTRecordRef *txtRecord)
{
    if (EnsureBonjourLoaded() && fn_TXTRecordDeallocate)
        fn_TXTRecordDeallocate(txtRecord);
}

DNSServiceErrorType TXTRecordSetValue(
    TXTRecordRef *txtRecord,
    const char *key,
    uint8_t valueSize,
    const void *value)
{
    if (!EnsureBonjourLoaded() || !fn_TXTRecordSetValue)
        return kDNSServiceErr_Unknown;
    return fn_TXTRecordSetValue(txtRecord, key, valueSize, value);
}

const void *TXTRecordGetValuePtr(
    uint16_t txtLen,
    const void *txtRecord,
    const char *key,
    uint8_t *valueLen)
{
    if (!EnsureBonjourLoaded() || !fn_TXTRecordGetValuePtr)
        return NULL;
    return fn_TXTRecordGetValuePtr(txtLen, txtRecord, key, valueLen);
}

int TXTRecordContainsKey(
    uint16_t txtLen,
    const void *txtRecord,
    const char *key)
{
    if (!EnsureBonjourLoaded() || !fn_TXTRecordContainsKey)
        return 0;
    return fn_TXTRecordContainsKey(txtLen, txtRecord, key);
}

uint16_t TXTRecordGetLength(const TXTRecordRef *txtRecord)
{
    if (!EnsureBonjourLoaded() || !fn_TXTRecordGetLength)
        return 0;
    return fn_TXTRecordGetLength(txtRecord);
}

const void *TXTRecordGetBytesPtr(const TXTRecordRef *txtRecord)
{
    if (!EnsureBonjourLoaded() || !fn_TXTRecordGetBytesPtr)
        return NULL;
    return fn_TXTRecordGetBytesPtr(txtRecord);
}
