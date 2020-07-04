




#include <Windows.h>

HKEY OpenRegKey(HKEY hKeyParent, LPCTSTR lpszKeyName, REGSAM samDesired);

LONG RegKey_QueryStringValue(HKEY hKey, LPCTSTR pszValueName, LPTSTR pszValue, ULONG* pnChars);

LONG RegKey_QueryDWORDValue(HKEY hKey, LPCTSTR pszValueName, DWORD* dwValue);

void AddSystemVariablesPath(const wchar_t* pathToAdd);

BOOL RegDelnode(HKEY hKeyRoot, LPTSTR lpSubKey);

BOOL ReadRegStr(HKEY hKeyParent,
                LPCTSTR pszSubkey,
                LPCTSTR pszKeyName,
                LPTSTR pszValue,
                ULONG* pnChars);








#include <tlhelp32.h>


//
// Some definitions from NTDDK and other sources
//

typedef LONG    NTSTATUS;
typedef LONG    KPRIORITY;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

#define STATUS_INFO_LENGTH_MISMATCH ((NTSTATUS)0xC0000004L)

#define SystemProcessesAndThreadsInformation    5

typedef struct _CLIENT_ID {
    DWORD        UniqueProcess;
    DWORD        UniqueThread;
} CLIENT_ID;

typedef struct _UNICODE_STRING {
    USHORT        Length;
    USHORT        MaximumLength;
    PWSTR        Buffer;
} UNICODE_STRING;

typedef struct _VM_COUNTERS {
    SIZE_T        PeakVirtualSize;
    SIZE_T        VirtualSize;
    ULONG        PageFaultCount;
    SIZE_T        PeakWorkingSetSize;
    SIZE_T        WorkingSetSize;
    SIZE_T        QuotaPeakPagedPoolUsage;
    SIZE_T        QuotaPagedPoolUsage;
    SIZE_T        QuotaPeakNonPagedPoolUsage;
    SIZE_T        QuotaNonPagedPoolUsage;
    SIZE_T        PagefileUsage;
    SIZE_T        PeakPagefileUsage;
} VM_COUNTERS;

typedef struct _SYSTEM_THREADS {
    LARGE_INTEGER   KernelTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   CreateTime;
    ULONG            WaitTime;
    PVOID            StartAddress;
    CLIENT_ID        ClientId;
    KPRIORITY        Priority;
    KPRIORITY        BasePriority;
    ULONG            ContextSwitchCount;
    LONG            State;
    LONG            WaitReason;
} SYSTEM_THREADS, * PSYSTEM_THREADS;

// Note that the size of the SYSTEM_PROCESSES structure is 
// different on NT 4 and Win2K, but we don't care about it, 
// since we don't access neither IoCounters member nor 
//Threads array

typedef struct _SYSTEM_PROCESSES {
    ULONG            NextEntryDelta;
    ULONG            ThreadCount;
    ULONG            Reserved1[6];
    LARGE_INTEGER   CreateTime;
    LARGE_INTEGER   UserTime;
    LARGE_INTEGER   KernelTime;
    UNICODE_STRING  ProcessName;
    KPRIORITY        BasePriority;
    ULONG            ProcessId;
    ULONG            InheritedFromProcessId;
    ULONG            HandleCount;
    ULONG            Reserved2[2];
    VM_COUNTERS        VmCounters;
#if _WIN32_WINNT >= 0x500
    IO_COUNTERS        IoCounters;
#endif
    SYSTEM_THREADS  Threads[1];
} SYSTEM_PROCESSES, * PSYSTEM_PROCESSES;


typedef HANDLE(WINAPI* PFCreateToolhelp32Snapshot)(
    DWORD dwFlags,
    DWORD th32ProcessID
    );

typedef BOOL(WINAPI* PFProcess32First)(
    HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe
    );

typedef BOOL(WINAPI* PFProcess32Next)(
    HANDLE hSnapshot,
    LPPROCESSENTRY32 lppe
    );

// Native NT API Definitions
typedef NTSTATUS(WINAPI* PFZwQuerySystemInformation)
(UINT, PVOID, ULONG, PULONG);
typedef HANDLE(WINAPI* PFGetProcessHeap)(VOID);
typedef LPVOID(WINAPI* PFHeapAlloc)
(HANDLE, DWORD, SIZE_T);
typedef BOOL(WINAPI* PFHeapFree)(HANDLE, DWORD, LPVOID);

struct KillProcess
{
    HMODULE            hNTLib;
    HMODULE            hKernelLib;
    // ToolHelp related functions
    PFCreateToolhelp32Snapshot    FCreateToolhelp32Snapshot;
    PFProcess32First            FProcess32First;
    PFProcess32Next                FProcess32Next;
    // native NT api functions
    PFZwQuerySystemInformation    FQuerySysInfo;
    PFGetProcessHeap            FGetProcessHeap;
    PFHeapAlloc                    FHeapAlloc;
    PFHeapFree                    FHeapFree;
};

void KillProcess_Init(struct KillProcess* pThis);
void KillProcess_Destroy(struct KillProcess* pThis);

HANDLE KillProcess_FindProcess(struct KillProcess* pThis,
                               const WCHAR* pstrProcessName,
                               DWORD* dwId);
HANDLE KillProcess_FindProcess(struct KillProcess* pThis,
                               const WCHAR* pstrProcessName,
                               DWORD* dwId);

HANDLE KillProcess_NTFindProcess(struct KillProcess* pThis, const WCHAR* pstrProcessName,
                                 DWORD* dwId);

HANDLE KillProcess_THFindProcess(struct KillProcess* pThis, const WCHAR* pstrProcessName, DWORD* dwId);

BOOL KillProcess_KillProcess(struct KillProcess* pThis, const WCHAR* pstrProcessName);

void SystemCreateProcess(const WCHAR* moduleName, const WCHAR* cmdline);




