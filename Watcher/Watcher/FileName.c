#include <fltKernel.h>
#include <dontuse.h>
#include <suppress.h>


// Minifilter vs regular kernel driver: we don't need to register our unload function in DriverObject


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
}