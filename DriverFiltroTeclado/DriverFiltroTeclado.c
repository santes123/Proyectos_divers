#include <ntddk.h>

void salir(PDRIVER_OBJECT driverOjbject);
NTSTATUS rutinaGenerica(PDEVICE_OBJECT deviceObject, PIRP irp);
NTSTATUS miFuncionEnlace(PDRIVER_OBJECT driverObject);
//struct para poder acceder al teclado desde el driver
typedef struct _KEYBOARD_INPUT_DATA {
	USHORT UnitId;
	USHORT MakeCode;
	USHORT Flags;
	USHORT Reserved;
	ULONG ExtraInformation;
} KEYBOARD_INPUT_DATA, *PKEYBOARD_INPUT_DATA;
typedef struct {
	PDEVICE_OBJECT p;

} DEVIDE_EXTENSION, *PDEVICE_EXTENSION;
NTSTATUS estado;
PDEVICE_OBJECT deviceObject;
UNICODE_STRING tecladoDestino = RTL_CONSTANT_STRING(L"\\Device\\KeyboardClass0");

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registrypath) {
	driverObject->DriverUnload = salir;
	int i;
	DbgPrint("Entrando al diver entry");
	for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
		driverObject->MajorFunction[i] = rutinaGenerica;
	}
	estado = miFuncionEnlace;
	if(estado != STATUS_SUCCESS){
		DbgPrint("Error al crear o vincular el teclado");
	}
	else {
		DbgPrint("Teclado vinculado con exito");
	}

	return STATUS_SUCCESS;
}

//miFuncionEnlace

//rutinaGenerica
NTSTATUS rutinaGenerica(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS estado;
	IoCopyCurrentIrpStackLocationToNext(irp);
	estado = IoCallDriver(((PDEVICE_EXTENSION)(deviceObject->DeviceExtension))->p,irp);

	return estado;
}
//rutina que se encarga de crear un dispositivo y de enlazarla con el dispositivo real
NTSTATUS miFuncionEnlace(PDRIVER_OBJECT driverObject) {
	NTSTATUS estado;

	estado = IoCreateDevice(driverObject,sizeof(PDEVICE_EXTENSION),NULL,FILE_DEVICE_KEYBOARD,0,FALSE,&deviceObject);
	if (estado != STATUS_SUCCESS) {
		DbgPrint("Error al crear el dispositivo");
	}
	else {
		//reseteamos los flags para que no haya conflito con el dispositivo real
		deviceObject->Flags |= DO_BUFFERED_IO;
		deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;
		RtlZeroMemory(deviceObject->DeviceExtension,sizeof(PDEVICE_EXTENSION));

		estado = IoAttachDevice(deviceObject, &tecladoDestino,(PDEVICE_EXTENSION) &deviceObject->DeviceExtension);

		if(estado != STATUS_SUCCESS){
			DbgPrint("Error al vincular el dispositivo real");
		}
		else {
			DbgPrint("El dispositivo real se ha vinculado correctamente");
		}
		//DbgPrint("El dispositivo se ha creado correctamente");
	}
}
//salir
void salir(PDRIVER_OBJECT driverOjbject) {
	DbgPrint("Empezando a descargar el driver");
	IoDeleteDevice(deviceObject);
	IoDetachDevice(deviceObject);
	DbgPrint("Liberados los recursos");
}