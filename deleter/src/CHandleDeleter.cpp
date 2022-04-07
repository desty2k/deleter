#include "Common.h"

#include <Windows.h>
#include <cstdio>
#include <process.h>
#include <vector>
#pragma comment(lib,"Advapi32.lib")

#define NT_SUCCESS(x) ((x) >= 0)
#define STATUS_INFO_LENGTH_MISMATCH 0xc0000004

#define SystemHandleInformation 16
#define ObjectBasicInformation 0
#define ObjectNameInformation 1
#define ObjectTypeInformation 2

typedef NTSTATUS(NTAPI* _NtQuerySystemInformation)(
	ULONG SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
	);

typedef NTSTATUS(NTAPI* _NtDuplicateObject)(
	HANDLE SourceProcessHandle,
	HANDLE SourceHandle,
	HANDLE TargetProcessHandle,
	PHANDLE TargetHandle,
	ACCESS_MASK DesiredAccess,
	ULONG Attributes,
	ULONG Options
	);

typedef NTSTATUS(NTAPI* _NtQueryObject)(
	HANDLE ObjectHandle,
	ULONG ObjectInformationClass,
	PVOID ObjectInformation,
	ULONG ObjectInformationLength,
	PULONG ReturnLength
	);

typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR Buffer;
} UNICODE_STRING, * PUNICODE_STRING;

typedef struct _SYSTEM_HANDLE {
	ULONG ProcessId;
	BYTE ObjectTypeNumber;
	BYTE Flags;
	USHORT Handle;
	PVOID Object;
	ACCESS_MASK GrantedAccess;
} SYSTEM_HANDLE, * PSYSTEM_HANDLE;

typedef struct _SYSTEM_HANDLE_INFORMATION {
	ULONG HandleCount;
	SYSTEM_HANDLE Handles[1];
} SYSTEM_HANDLE_INFORMATION, * PSYSTEM_HANDLE_INFORMATION;

typedef enum _POOL_TYPE {
	NonPagedPool,
	PagedPool,
	NonPagedPoolMustSucceed,
	DontUseThisType,
	NonPagedPoolCacheAligned,
	PagedPoolCacheAligned,
	NonPagedPoolCacheAlignedMustS
} POOL_TYPE, * PPOOL_TYPE;

typedef struct _OBJECT_TYPE_INFORMATION {
	UNICODE_STRING Name;
	ULONG TotalNumberOfObjects;
	ULONG TotalNumberOfHandles;
	ULONG TotalPagedPoolUsage;
	ULONG TotalNonPagedPoolUsage;
	ULONG TotalNamePoolUsage;
	ULONG TotalHandleTableUsage;
	ULONG HighWaterNumberOfObjects;
	ULONG HighWaterNumberOfHandles;
	ULONG HighWaterPagedPoolUsage;
	ULONG HighWaterNonPagedPoolUsage;
	ULONG HighWaterNamePoolUsage;
	ULONG HighWaterHandleTableUsage;
	ULONG InvalidAttributes;
	GENERIC_MAPPING GenericMapping;
	ULONG ValidAccess;
	BOOLEAN SecurityRequired;
	BOOLEAN MaintainHandleCount;
	USHORT MaintainTypeList;
	POOL_TYPE PoolType;
	ULONG PagedPoolUsage;
	ULONG NonPagedPoolUsage;
} OBJECT_TYPE_INFORMATION, * POBJECT_TYPE_INFORMATION;

PVOID GetLibraryProcAddress(PSTR LibraryName, PSTR ProcName) {
	return GetProcAddress(GetModuleHandleA(LibraryName), ProcName);
}

BOOL EnableDebugPrivilege(BOOL fEnable) {
	BOOL fOk = FALSE;
	HANDLE hToken;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &hToken)) {
		TOKEN_PRIVILEGES tp;
		tp.PrivilegeCount = 1;
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tp.Privileges[0].Luid);
		tp.Privileges[0].Attributes = fEnable ? SE_PRIVILEGE_ENABLED : 0;
		AdjustTokenPrivileges(hToken, FALSE, &tp, sizeof(tp), NULL, NULL);
		fOk = (GetLastError() == ERROR_SUCCESS);
		CloseHandle(hToken);
	}
	return(fOk);
}

void CheckBlockThreadFunc(void* param) {
    auto NtQueryObject = (_NtQueryObject)GetProcAddress(GetModuleHandleA("NtDll.dll"), "NtQueryObject");
	if (NtQueryObject != NULL) {
		PVOID objectNameInfo = NULL;
		ULONG returnLength;
		objectNameInfo = malloc(0x1000);
		NtQueryObject((HANDLE)param, ObjectNameInformation, objectNameInfo, 0x1000, &returnLength);
	}
}

BOOL IsBlockingHandle(HANDLE handle) {
	HANDLE hThread = (HANDLE)_beginthread(CheckBlockThreadFunc, 0, (void*)handle);
	if (WaitForSingleObject(hThread, 100) != WAIT_TIMEOUT) {
		return FALSE;
	}
	TerminateThread(hThread, 0);
	return TRUE;
}

struct wchar_t_array {
    wchar_t str[MAX_PATH+1];
};


static PyObject* run(PyObject * self, PyObject * args) {
	NTSTATUS status;
	PSYSTEM_HANDLE_INFORMATION handleInfo;
	ULONG handleInfoSize = 0x10000;
	HANDLE processHandle;
	ULONG i;
	DWORD ErrorPID = 0;
	SYSTEM_HANDLE handle;

//	char* path;
//	wchar_t wpath[MAX_PATH + 1];
	int close_handle;
	int success = 0;

    PyObject *paths;
//    wchar_t wpath[MAX_PATH + 1];
    std::vector<wchar_t_array> wpaths;
	if (!PyArg_ParseTuple(args, "O!p", &PyList_Type, &paths, &close_handle)) {
		return nullptr;
	}
    // first argument is list of file paths
    // second argument is whether to close handles if found
    for (int it = 0; it < PyList_Size(paths); it++) {
        PyObject* item = PyList_GetItem(paths, it);
        // check if item is a string
        if (!PyUnicode_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "Argument must be a list of strings");
            return nullptr;
        }

        if (PyUnicode_Check(item)) {
            // convert to wchar_t
            Py_ssize_t len;
            const char* str = PyUnicode_AsUTF8AndSize(item, &len);
            if (len > MAX_PATH) {
                PyErr_SetString(PyExc_ValueError, "Path is too long");
                return nullptr;
            }
            wchar_t_array wpath{};
            if (MultiByteToWideChar(CP_UTF8, 0, str, len, wpath.str, MAX_PATH) == 0) {
                PyErr_SetString(PyExc_ValueError, "Path is not valid");
                return nullptr;
            }
            wpaths.push_back(wpath);
        }
    }

	if (!EnableDebugPrivilege(TRUE)) {
		dprintf("[!] AdjustTokenPrivileges Failed.<%d>\n", GetLastError());
	}

    auto NtQuerySystemInformation = (_NtQuerySystemInformation)GetProcAddress(GetModuleHandleA("NtDll.dll"), "NtQuerySystemInformation");
	if (!NtQuerySystemInformation) {
		dprintf("[!] Could not find NtQuerySystemInformation entry point in NTDLL.DLL");
		return PyBool_FromLong(0);
	}
    auto NtDuplicateObject = (_NtDuplicateObject)GetProcAddress(GetModuleHandleA("NtDll.dll"), "NtDuplicateObject");
	if (!NtDuplicateObject) {
		dprintf("[!] Could not find NtDuplicateObject entry point in NTDLL.DLL");
		return PyBool_FromLong(0);
	}
	auto NtQueryObject = (_NtQueryObject)GetProcAddress(GetModuleHandleA("NtDll.dll"), "NtQueryObject");
	if (!NtQueryObject) {
		dprintf("[!] Could not find NtQueryObject entry point in NTDLL.DLL");
		return PyBool_FromLong(0);
	}

	handleInfo = (PSYSTEM_HANDLE_INFORMATION)malloc(handleInfoSize);
	while ((status = NtQuerySystemInformation(SystemHandleInformation, handleInfo, handleInfoSize, nullptr)) == STATUS_INFO_LENGTH_MISMATCH)
		handleInfo = (PSYSTEM_HANDLE_INFORMATION)realloc(handleInfo, handleInfoSize *= 2);
	if (!NT_SUCCESS(status)) {
		dprintf("[!] NtQuerySystemInformation failed!\n");
		return PyBool_FromLong(0);
	}

	UNICODE_STRING objectName;
	ULONG returnLength;
	for (i = 0; i < handleInfo->HandleCount; i++) {
		handle = handleInfo->Handles[i];
		HANDLE dupHandle = nullptr;
		POBJECT_TYPE_INFORMATION objectTypeInfo = nullptr;
		PVOID objectNameInfo = nullptr;

		if (handle.ProcessId == ErrorPID) {
			free(objectTypeInfo);
			free(objectNameInfo);
			CloseHandle(dupHandle);
			continue;
		}

		if (!(processHandle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, handle.ProcessId))) {
			dprintf("[!] Could not open PID %d!\n", handle.ProcessId);
			ErrorPID = handle.ProcessId;
			free(objectTypeInfo);
			free(objectNameInfo);
			CloseHandle(dupHandle);
			CloseHandle(processHandle);
			continue;
		}

		if (!NT_SUCCESS(NtDuplicateObject(processHandle, (HANDLE)handle.Handle, GetCurrentProcess(), &dupHandle, 0, 0, 0))) {
			//dprintf("[%#x] Error!\n", handle.Handle);
			free(objectTypeInfo);
			free(objectNameInfo);
			CloseHandle(dupHandle);
			CloseHandle(processHandle);
			continue;
		}
		objectTypeInfo = (POBJECT_TYPE_INFORMATION)malloc(0x1000);
		if (!NT_SUCCESS(NtQueryObject(dupHandle, ObjectTypeInformation, objectTypeInfo, 0x1000, nullptr))) {
			//dprintf("[%#x] Error!\n", handle.Handle);
			free(objectTypeInfo);
			free(objectNameInfo);
			CloseHandle(dupHandle);
			CloseHandle(processHandle);
			continue;
		}
		objectNameInfo = malloc(0x1000);

		if (IsBlockingHandle(dupHandle) == TRUE) {
			//filter out the object which NtQueryObject could hang on 
			free(objectTypeInfo);
			free(objectNameInfo);
			CloseHandle(dupHandle);
			CloseHandle(processHandle);
			continue;
		}

		if (!NT_SUCCESS(NtQueryObject(dupHandle, ObjectNameInformation, objectNameInfo, 0x1000, &returnLength))) {
			objectNameInfo = realloc(objectNameInfo, returnLength);
			if (!NT_SUCCESS(NtQueryObject(dupHandle, ObjectNameInformation, objectNameInfo, returnLength, nullptr))) {
				//dprintf("[%#x] %.*S: (could not get name)\n", handle.Handle, objectTypeInfo->Name.Length / 2, objectTypeInfo->Name.Buffer);
				free(objectTypeInfo);
				free(objectNameInfo);
				CloseHandle(dupHandle);
				CloseHandle(processHandle);
				continue;
			}
		}
		objectName = *(PUNICODE_STRING)objectNameInfo;
		if (objectName.Length) {
			wchar_t lpszFilePath[MAX_PATH + 1];
			GetFinalPathNameByHandleW(dupHandle, lpszFilePath, _countof(lpszFilePath) - 1, VOLUME_NAME_DOS);
//            dprintf("[%#x] %.*S: %.*S\n", handle.Handle, objectTypeInfo->Name.Length / 2, objectTypeInfo->Name.Buffer, objectName.Length / 2, objectName.Buffer);
            // find if any path in wpath_vec points to the same file
            for (auto wpath : wpaths) {
                // check if wpath.str is substring of lpszFilePath, skip first 4 chars (\\??\\) of lpszFilePath
                if (wcslen(wpath.str) <= wcslen(lpszFilePath) && wcscmp(wpath.str, lpszFilePath + 4) == 0) {

//                if (wcsstr(lpszFilePath, wpath.str) != nullptr) {
                    dprintf("[+] Found a match for %S -> %S\n", wpath.str, lpszFilePath);
                    dprintf("    [+] HandleName: %.*S\n", objectName.Length / 2, objectName.Buffer);
                    dprintf("    [+] Pid: %d\n", handle.ProcessId);
                    dprintf("    [+] Handle: %#x\n", handle.Handle);
                    dprintf("    [+] Type: %#x\n", handle.ObjectTypeNumber);
                    dprintf("    [+] ObjectAddress: 0x%p\n", handle.Object);
                    dprintf("    [+] GrantedAccess: %#x\n", handle.GrantedAccess);

                    if (close_handle == 1) {
                        dprintf("    [+] Trying to close the file's handle...\n");

                        if (DuplicateHandle(processHandle, (HANDLE) handle.Handle, GetCurrentProcess(), &dupHandle, 0,
                                            0, DUPLICATE_CLOSE_SOURCE)) {
                            CloseHandle(dupHandle);
                            dprintf("    [+] Successfully closed the handle!\n");
                            success = 1;
                        } else {
                            dprintf("    [-] Failed to close the handle!\n");
                            success = 0;
                        }
                    }
                } else {
                    //dprintf("[%#x] %.*S: (unnamed)\n",handle.Handle,objectTypeInfo->Name.Length / 2,objectTypeInfo->Name.Buffer);
                }
            }
            free(objectTypeInfo);
            free(objectNameInfo);
            CloseHandle(dupHandle);
            CloseHandle(processHandle);
		}
	}
	free(handleInfo);
	return PyBool_FromLong(success);
}


static PyMethodDef module_methods[] = {
	{
		"run", run, METH_VARARGS,
		"Delete all handles to file specified by path"
	},
	{
		"get_verbose_flag", get_verbose_flag, METH_NOARGS,
		"Return the Py_Verbose flag"
	},
	{nullptr, nullptr, 0, nullptr}
};


static struct PyModuleDef module_definition = {
	PyModuleDef_HEAD_INIT,
	"CHandleDeleter",
	"Unlock files locked by another process",
	-1,
	module_methods,
};


PyMODINIT_FUNC PyInit_CHandleDeleter() {
	return PyModule_Create(&module_definition);
}
