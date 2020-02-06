/*++

Module Name:

    public.h

Abstract:

    This module contains the common declarations shared by driver
    and user applications.

Environment:

    user and kernel

--*/

//
// Define an Interface Guid so that apps can find the device and talk to it.
//

DEFINE_GUID (GUID_DEVINTERFACE_Driver5,
    0xe38f61d3,0x840c,0x4b5c,0x81,0x8c,0xd2,0x6d,0xbd,0xdb,0x7c,0xd3);
// {e38f61d3-840c-4b5c-818c-d26dbddb7cd3}
