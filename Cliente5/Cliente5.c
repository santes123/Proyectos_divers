#include<stdlib.h>
#include <stdio.h>
#include<Windows.h>
#include <winioctl.h>


#define MI_IOCTL1 CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_READ_DATA)
#define MI_IOCTL2 CTL_CODE(FILE_DEVICE_UNKNOWN,0x802,METHOD_BUFFERED,FILE_WRITE_DATA)

int main() {
	int bytesretornados;
	HANDLE manejadorDispositivo;
	char entrada[1024] = "Escribiendo desde el modo usuario";
	char salida[1024];
	int tam;
	tam = sizeof(entrada);
	manejadorDispositivo = CreateFile(L"\\\\.\\milinkSimbolico5", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
		OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
	printf("\nllamada a createfile realizada");
	if (manejadorDispositivo != INVALID_HANDLE_VALUE) {
		printf("\nHandle valido");
		int opcion = -1;
		while (opcion != 0) {
			printf("\n\n--     Menu    --");
			printf("\n\n1 - Leer");
			printf("\n2 - Escribir ");
			printf("\n0 - Salir\n");
			scanf_s(&opcion);
			switch (opcion) {
			case 0:
				break;
			case 1:
				//read
				DeviceIoControl(manejadorDispositivo, MI_IOCTL1, entrada, tam, NULL, NULL, NULL, NULL);
				printf("\nNumero de bytes retornados: %d\nMensaje: %s\n", bytesretornados, salida);
				break;
			case 2:
				//write
				DeviceIoControl(manejadorDispositivo, MI_IOCTL2, NULL, NULL, salida, sizeof(salida), &bytesretornados, NULL);
				break;
			}
		}
		/*
		//write
		DeviceIoControl(manejadorDispositivo, MI_IOCTL2, NULL, NULL, salida, sizeof(salida), &bytesretornados, NULL);
		//read
		DeviceIoControl(manejadorDispositivo, MI_IOCTL1, entrada, tam, NULL, NULL, NULL, NULL);
		*/

	}
	else {
		printf("\nHandle no valido");
		printf("\nError:%ld", GetLastError());
	}
	system("pause");
	return 0;



}
