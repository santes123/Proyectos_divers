#include <ntddk.h>

#define MI_IOCTL1 CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_READ_DATA)
#define MI_IOCTL2 CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_WRITE_DATA)



/****************************************************************
Declaracion de las funciones
******************************************************************/
void salir();
NTSTATUS rutinaTratamiento(PDEVICE_OBJECT, PIRP);



/****************************************************************
Declaracion de las variables globales
******************************************************************/

UNICODE_STRING nombreDispostivo = RTL_CONSTANT_STRING(L"\\Device\\miDispositivo5");

UNICODE_STRING linkSimbolico = RTL_CONSTANT_STRING(L"\\??\\milinkSimbolico5");

PDEVICE_OBJECT deviceObject = NULL;



/*******************************************************************************
Rutina principal (de entrada) del driver
*********************************************************************************/

NTSTATUS DriverEntry(PDRIVER_OBJECT driverObject, PUNICODE_STRING registryPath) {
	NTSTATUS estado;
	int i;
	driverObject->DriverUnload = salir;

	//creamos el dispositivo que trata este driver
	estado = IoCreateDevice(driverObject, 0, &nombreDispostivo, FILE_DEVICE_UNKNOWN, 0, 0, &deviceObject);

	if (estado == STATUS_SUCCESS) {
		DbgPrint("Dispositivo creado con exito\n");
		estado = IoCreateSymbolicLink(&linkSimbolico, &nombreDispostivo);
		if (estado == STATUS_SUCCESS) {
			DbgPrint("Enlace simbolico creado con exito\n");
			//registra quien tiene que atender a cada majorfunction
			for (i = 0; i < IRP_MJ_MAXIMUM_FUNCTION; i++) {
				driverObject->MajorFunction[i] = rutinaTratamiento;
			}
			DbgPrint("Registradas las major function\n");
		}
		else {//en caso de que no se haya creado el symboliclink
			DbgPrint("Error: No se ha creado el simbolic link\n");
			//Eliminamos el dispositivo creado en el paso anterior
			//para asegurar que dejamos el sistema en estado consistente
			IoDeleteDevice(deviceObject);
		}
	}
	else {
		DbgPrint("Error al crear el dispositivo\n");

	}
	return estado;

}

/**************************************************************************************
Funcion que se ejecuta al descargar el driver
Debería liberar los recursos que pueda haber obtenido dicho driver
***************************************************************************************/
void salir() {
	DbgPrint("Entrando en la rutina de salir . . .\n");
	IoDeleteSymbolicLink(&linkSimbolico);
	IoDeleteDevice(deviceObject);
	DbgPrint("Liberados los recursos");

}

/**************************************************************************************
Funcion que recoge la ioctl generada en aplicacion y realiza
las funciones que corresponden
(lee el mensaje que envian desde aplicacion y enviar uno desde el kernel
a la aplicacion)
***************************************************************************************/
NTSTATUS rutinaTratamiento(PDEVICE_OBJECT deviceObject, PIRP irp) {
	NTSTATUS estado = STATUS_SUCCESS;

	DbgPrint("Entrando en la rutina de tratamiento\n");
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(irp);
	DbgPrint("Obtenido el stack\n");
	//Mensaje a enviar desde el modo kernel
	char* mensaje = "escribiendo desde el kernel";
	int tam;
	tam = sizeof("escribiendo desde el kernel");
	DbgPrint("\n\n tam=%d", tam);
	//Declaramos punteros a caracter para el buffer de entrada y el de salida
	char* inputBuffer = NULL;
	char* outputBuffer = NULL;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case MI_IOCTL1:DbgPrint("Entrando en el caso de mi octl 1\n");

		//obtenemos tanto para la entrada como para la salida el buffer que nos
		//proporciona el sistema
		inputBuffer = irp->AssociatedIrp.SystemBuffer;
		//outputBuffer = irp->AssociatedIrp.SystemBuffer;
		DbgPrint("Intentado obtener buffer\n");
		if (inputBuffer != NULL) {
			DbgPrint("Buffer obtenido\n");
			//Comprobar si ha llegado datos desde el modo usuario
			if (stack->Parameters.DeviceIoControl.InputBufferLength != 0) {
				DbgPrint("Datos desde el modo usuario: %s", inputBuffer);
				//Vamos a enviar una respuesta

				//comprobamos que el tamaño del buffer de salida es suficiente
				//para nuestro mensaje

			}//fin if tamaño buffer entrada !=0
			else {
				DbgPrint("Buffer entrada vacio");
			}
		}
		else {
			DbgPrint("Problema al obtener los bufferes\n");
		}
		break;
	case MI_IOCTL2:DbgPrint("Entrando en el caso de mi octl 2\n");

		//obtenemos tanto para la entrada como para la salida el buffer que nos
		//proporciona el sistema
		outputBuffer = irp->AssociatedIrp.SystemBuffer;
		DbgPrint("Intentado obtener buffer de salida\n");
		if (outputBuffer != NULL) {
			DbgPrint("Buffer obtenido\n");
			//Comprobar si ha llegado datos desde el modo usuario
			if (stack->Parameters.DeviceIoControl.OutputBufferLength >= tam) {
				DbgPrint("Enviando datos desde el kernel . . .\n");
				RtlCopyMemory(outputBuffer, mensaje, tam);
				irp->IoStatus.Information = tam;
				//irp->IoStatus.Status = STATUS_SUCCESS;
			}
			else {//tamaño buffer insuficiente
				DbgPrint("Buffer no tiene la longitud necesaria\n");
				irp->IoStatus.Information = 0;
				//irp->IoStatus.Status = STATUS_BUFFER_TOO_SMALL;
				estado = STATUS_BUFFER_TOO_SMALL;
			}
		}
		else {
			DbgPrint("Problema al obtener los bufferes\n");
		}
		break;
	}

	irp->IoStatus.Status = estado;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return estado;
}