#include <ntddk.h>
PDEVICE_OBJECT deviceObject = NULL;



NTSTATUS  ObReferenceObjectByName(
    IN PUNICODE_STRING ObjectName,
    IN ULONG Attributes,
    IN PACCESS_STATE AccessState,
    IN ACCESS_MASK DesiredAccess,
    IN POBJECT_TYPE ObjectType,
    IN KPROCESSOR_MODE AccessMode,
    IN PVOID ParseContext,
    OUT PVOID* Object
);

typedef struct _KEYBOARD_INPUT_DATA {
    USHORT UnitId;
    USHORT MakeCode;
    USHORT Flags;
    USHORT Reserved;
    ULONG  ExtraInformation;
} KEYBOARD_INPUT_DATA, * PKEYBOARD_INPUT_DATA;

typedef struct {
    PDEVICE_OBJECT p;
}DEVICE_EXTENSION, * PDEVICE_EXTENSION;

extern POBJECT_TYPE* IoDriverObjectType;

VOID salir(PDRIVER_OBJECT driverObject) {
    DbgPrint("\n\nempezando a salir....\n");
    PDEVICE_OBJECT dispositivo = driverObject->DeviceObject;
    DbgPrint("empezando a salir2....\n");
    while (dispositivo) {
        DbgPrint("antes detach\n");
        IoDetachDevice(((PDEVICE_EXTENSION)dispositivo->DeviceExtension)->p);
        DbgPrint("antes next\n");
        dispositivo = dispositivo->NextDevice;
    }
    /* while (pendingKey) {
         KeDelayExecutionThread(KernelMode,FALSE,)
     }*/
    DbgPrint("antes segundo dispositivo\n");
    dispositivo = driverObject->DeviceObject;
    while (dispositivo) {
        DbgPrint("antes delete\n");
        IoDeleteDevice(dispositivo);
        DbgPrint("despues delete\n");

        dispositivo = dispositivo->NextDevice;

        DbgPrint("despues next\n");
    }
    DbgPrint("saliendo....\n");

}


NTSTATUS completarLectura(PDEVICE_OBJECT deviceObject, PIRP irp, PVOID Context) {

    DbgPrint("Se ha pulsado una tecla ...\n");
    char* flagTeclas[4] = { "keydown","keyup","e0","e1" };
    int i;
    PKEYBOARD_INPUT_DATA teclas = (PKEYBOARD_INPUT_DATA)irp->AssociatedIrp.SystemBuffer;
    DbgPrint("antes tam...\n");
    int tam = irp->IoStatus.Information / sizeof(PKEYBOARD_INPUT_DATA);
    DbgPrint("despues tam...\n");
    if (irp->IoStatus.Status == STATUS_SUCCESS) {
        for (i = 0; i < tam; i++) {
            DbgPrint("scan code: %x (%s)\n", teclas[i].MakeCode, flagTeclas[teclas->Flags]);
        }
    }
    if (irp->PendingReturned) {
        IoMarkIrpPending(irp);
    }
    return irp->IoStatus.Status;
}

NTSTATUS rutinaEnlace(PDRIVER_OBJECT driverObject) {

    NTSTATUS estado;
    UNICODE_STRING tecladoDestino = RTL_CONSTANT_STRING(L"\\Driver\\Kbdclass");
    PDRIVER_OBJECT driverTeclado = NULL;
    PDEVICE_OBJECT dispositivoActual = NULL, dispositivo = NULL;

    estado = ObReferenceObjectByName(&tecladoDestino, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&driverTeclado);
    if (estado != STATUS_SUCCESS) {
        DbgPrint("Fallo al referenciar\n");
        return estado;
    }
    ObDereferenceObject(driverTeclado);
    dispositivoActual = driverTeclado->DeviceObject;

    DbgPrint("antes de crear dispositivo\n");
    while (dispositivoActual) {
        estado = IoCreateDevice(driverObject, sizeof(DEVICE_EXTENSION), NULL, FILE_DEVICE_KEYBOARD, 0, FALSE, &dispositivo);
        DbgPrint("creado dispositivo\n");

        if (estado != STATUS_SUCCESS) {
            return estado;
        }
        else {
            RtlZeroMemory(dispositivo->DeviceExtension, sizeof(DEVICE_EXTENSION));


            DbgPrint("antes del attach\n");
            estado = IoAttachDeviceToDeviceStackSafe(dispositivo, dispositivoActual, &((PDEVICE_EXTENSION)dispositivo->DeviceExtension)->p);
            DbgPrint("despues del attach\n");
            if (estado != STATUS_SUCCESS) {
                DbgPrint("NO se ha podido realizar el attach\n");
                // IoDeleteDevice(deviceObject);
            }
            DbgPrint("atach realizado, antes flags\n");
            dispositivo->Flags |= DO_BUFFERED_IO;
            dispositivo->Flags &= ~DO_DEVICE_INITIALIZING;
            DbgPrint("antes next device\n");
            dispositivoActual = dispositivoActual->NextDevice;
        }
        DbgPrint("fin vuelta while\n");

    }
    DbgPrint("fuera while\n");
    return STATUS_SUCCESS;
}


NTSTATUS rutinaGenerica(PDEVICE_OBJECT deviceObject, PIRP irp) {
    NTSTATUS estado;

    DbgPrint("Entranddo en generica\n");
    IoCopyCurrentIrpStackLocationToNext(irp);
    estado = IoCallDriver(((PDEVICE_EXTENSION)deviceObject->DeviceExtension)->p, irp);
    return estado;

}

NTSTATUS rutinaLectura(PDEVICE_OBJECT deviceObject, PIRP irp) {
    NTSTATUS estado;

    DbgPrint("Entranddo en lectura\n");
    IoCopyCurrentIrpStackLocationToNext(irp);
    IoSetCompletionRoutine(irp, &completarLectura, NULL, TRUE, TRUE, TRUE);
    estado = IoCallDriver(((PDEVICE_EXTENSION)deviceObject->DeviceExtension)->p, irp);
    return estado;

}


NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
    NTSTATUS estado;
    int i;
    DbgPrint("Entrando en driverEntry\n");
    driverObject->DriverUnload = salir;
    DbgPrint("Entrando en driverEntry despues unload\n");
    for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
        driverObject->MajorFunction[i] = rutinaGenerica;
    }
    driverObject->MajorFunction[IRP_MJ_READ] = rutinaLectura;
    DbgPrint("Registradas major funcion\n");
    estado = rutinaEnlace(driverObject);
    if (estado == STATUS_SUCCESS) {
        DbgPrint("Enlazado\n");
    }
    else {
        DbgPrint("error al enlazar\n");
    }
    return STATUS_SUCCESS;
}

