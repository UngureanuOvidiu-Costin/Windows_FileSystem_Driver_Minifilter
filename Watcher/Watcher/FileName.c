#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>


// Minifilter vs regular kernel driver: we don't need to register our unload function in DriverObject
NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags);
FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags);
FLT_PREOP_CALLBACK_STATUS MiniPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext);
FLT_PREOP_CALLBACK_STATUS MiniPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext);


NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath);

const FLT_OPERATION_REGISTRATION Callbacks[] = {
	{IRP_MJ_CREATE, 0, MiniPreCreate,MiniPostCreate},
	{IRP_MJ_WRITE,0, MiniPreWrite,NULL},
	{IRP_MJ_OPERATION_END}
};

PFLT_FILTER FilterHandle = NULL;
const FLT_REGISTRATION FilterRegistration = {
	sizeof(FLT_REGISTRATION),
	FLT_REGISTRATION_VERSION,
	0,
	NULL,
	Callbacks,
	MiniUnload,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL
};


NTSTATUS MiniUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {

	KdPrint(("driver unload \r\n"));
	FltUnregisterFilter(FilterHandle);

	return STATUS_SUCCESS;
}



FLT_POSTOP_CALLBACK_STATUS MiniPostCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext, FLT_POST_OPERATION_FLAGS Flags) {
	KdPrint(("Post create is running \r\n"));

	return FLT_POSTOP_FINISHED_PROCESSING;
}


// Data contains the data that we will retreive
// FltObjects, insert "We don't do that here" meme template
FLT_PREOP_CALLBACK_STATUS MiniPreCreate(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
	// Extract the file name
	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;
	NTSTATUS statusParse;
	WCHAR Name[200] = { 0 };


	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

	if (NT_SUCCESS(status)) {
		statusParse = FltParseFileNameInformation(FileNameInfo);
	}

	// If parsing the file name is successfully
	if (NT_SUCCESS(statusParse)) {
		if (FileNameInfo->Name.MaximumLength < 200) {
			RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);
			KdPrint(("Create file: %ws \r\n", Name));
		}
	}

	// Must release the file name information
	FltReleaseFileNameInformation(FileNameInfo);

	// NO_CALLBACK because we didn't implement POST Write Op Callback, else we will use WITH_CALLBACK
	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
 

FLT_PREOP_CALLBACK_STATUS MiniPreWrite(PFLT_CALLBACK_DATA Data, PCFLT_RELATED_OBJECTS FltObjects, PVOID* CompletionContext) {
	// Extract the file name
	PFLT_FILE_NAME_INFORMATION FileNameInfo;
	NTSTATUS status;
	NTSTATUS statusParse;
	WCHAR Name[200] = { 0 };


	status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

	if (NT_SUCCESS(status)) {
		statusParse = FltParseFileNameInformation(FileNameInfo);
	}

	// If parsing the file name is successfully
	if (NT_SUCCESS(statusParse)) {
		if (FileNameInfo->Name.MaximumLength < 200) {
			RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength);

			_wcsupr(Name);

			if (wcsstr(Name, L"OPENME.TXT") != NULL) {
				KdPrint(("write file: %ws blocked /r/n", Name));
				Data->IoStatus.Status = STATUS_INVALID_PARAMETER;
				Data->IoStatus.Information = 0;
				FltReleaseFileNameInformation(FileNameInfo);
				return FLT_PREOP_COMPLETE;
			}
			KdPrint(("Create file: %ws \r\n", Name));
		}
	}

	// Must release the file name information
	FltReleaseFileNameInformation(FileNameInfo);

	// NO_CALLBACK because we didn't implement POST Write Op Callback, else we will use WITH_CALLBACK
	return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


// This is how we define the entry
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {


	NTSTATUS status;

	// Register ourself to the Filter Manager
	// FilterRegistration, contruction of our registration proccess
	// FilterHandle, which we will use in the following functions
	status = FltRegisterFilter(DriverObject, &FilterRegistration, &FilterHandle);

	// We chack the status and if this function fails, we return the status so our driver will not be loaded
	if (!NT_SUCCESS(status)) {
		return status; // Return the status so the driver is not loaded
	}

	// If the registration of the filter fails, we must write code to unregister
	status = FltStartFiltering(FilterHandle);
	if (!NT_SUCCESS(status)) {
		FltUnregisterFilter(FilterHandle);
	}

	return status;
}
