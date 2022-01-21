#include "cdeleter.h"


static HANDLE ds_open_handle(PWCHAR pwPath) {
    return CreateFileW(pwPath, DELETE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

static BOOL ds_rename_handle(HANDLE hHandle) {
    FILE_RENAME_INFO fRename;
    RtlSecureZeroMemory(&fRename, sizeof(fRename));

    // set our FileNameLength and FileName to DS_STREAM_RENAME
    LPWSTR lpwStream = DS_STREAM_RENAME;
    fRename.FileNameLength = sizeof(lpwStream);
    RtlCopyMemory(fRename.FileName, lpwStream, sizeof(lpwStream));

    return SetFileInformationByHandle(hHandle, FileRenameInfo, &fRename, sizeof(fRename) + sizeof(lpwStream));
}

static BOOL ds_deposite_handle(HANDLE hHandle) {
    // set FILE_DISPOSITION_INFO::DeleteFile to TRUE
    FILE_DISPOSITION_INFO fDelete;
    RtlSecureZeroMemory(&fDelete, sizeof(fDelete));

    fDelete.DeleteFile = TRUE;

    return SetFileInformationByHandle(hHandle, FileDispositionInfo, &fDelete, sizeof(fDelete));
}


static PyObject* delete(PyObject *self, PyObject *args) {
    WCHAR wcPath[MAX_PATH + 1];
    RtlSecureZeroMemory(wcPath, sizeof(wcPath));

    // get the path to the current running process ctx
    if (GetModuleFileNameW(NULL, wcPath, MAX_PATH) == 0) {
        DS_DEBUG_LOG(L"failed to get the current module handle");
        return PyLong_FromLong(0);
    }

    HANDLE hCurrent = ds_open_handle(wcPath);
    if (hCurrent == INVALID_HANDLE_VALUE) {
        DS_DEBUG_LOG(L"failed to acquire handle to current running process");
        return PyLong_FromLong(0);
    }

    // rename the associated HANDLE's file name
    DS_DEBUG_LOG(L"attempting to rename file name");
    if (!ds_rename_handle(hCurrent)) {
        DS_DEBUG_LOG(L"failed to rename to stream");
        return PyLong_FromLong(0);
    }

    DS_DEBUG_LOG(L"successfully renamed file primary :$DATA ADS to specified stream, closing initial handle");
    CloseHandle(hCurrent);

    // open another handle, trigger deletion on close
    hCurrent = ds_open_handle(wcPath);
    if (hCurrent == INVALID_HANDLE_VALUE) {
        DS_DEBUG_LOG(L"failed to reopen current module");
        return PyLong_FromLong(0);
    }

    if (!ds_deposite_handle(hCurrent)) {
        DS_DEBUG_LOG(L"failed to set delete deposition");
        return PyLong_FromLong(0);
    }

    // trigger the deletion deposition on hCurrent
    DS_DEBUG_LOG(L"closing handle to trigger deletion deposition");
    CloseHandle(hCurrent);

    // verify we've been deleted
    if (PathFileExistsW(wcPath)) {
        DS_DEBUG_LOG(L"failed to delete copy, file still exists");
        return PyLong_FromLong(0);
    }

    DS_DEBUG_LOG(L"successfully deleted self from disk");
    return PyLong_FromLong(1);
}


static PyObject * get_verbose_flag(PyObject *self, PyObject *args) {
    return PyLong_FromLong(Py_VerboseFlag);
}


static PyMethodDef module_methods[] = {
    {
        "delete", delete, METH_NOARGS,
        "Delete path"
    },
    {
        "get_verbose_flag", get_verbose_flag, METH_NOARGS,
        "Return the Py_Verbose flag"
    },
    {NULL, NULL, 0, NULL}
};


static struct PyModuleDef module_definition = {
    PyModuleDef_HEAD_INIT,
    "cdeleter",
    "Delete self from disk.",
    0,
    module_methods,
};


PyMODINIT_FUNC PyInit_cdeleter() {
    return PyModule_Create(&module_definition);
}
