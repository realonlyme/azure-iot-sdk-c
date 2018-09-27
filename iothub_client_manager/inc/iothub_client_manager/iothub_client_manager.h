// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IOTHUB_CLIENT_MANAGER

#include "azure_c_shared_utility/umock_c_prod.h"
#include "iothub_device_client_ll.h"
#include "iothub_device_client.h"

typedef void(*IoTHub_Mgr_Device_Handle_callback)(IOTHUB_DEVICE_CLIENT_LL_HANDLE device_handle, void* user_context);
typedef void(*IoTHub_Mgr_Convenience_Handle_callback)(IOTHUB_DEVICE_CLIENT_HANDLE device_handle, void* user_context);

typedef struct IOTHUB_CLIENT_MGR_TAG* IOTHUB_CLIENT_MGR_HANDLE;

MOCKABLE_FUNCTION(, IOTHUB_DEVICE_CLIENT_LL_HANDLE, IoThub_Mgr_CreateClient, const char*, dps_id_scope, IOTHUB_CLIENT_TRANSPORT_PROVIDER, protocol);
MOCKABLE_FUNCTION(, IOTHUB_DEVICE_CLIENT_HANDLE, IoThub_Mgr_CreateConvenienceClient, const char*, dps_id_scope, IOTHUB_CLIENT_TRANSPORT_PROVIDER, protocol);

MOCKABLE_FUNCTION(, IOTHUB_CLIENT_MGR_HANDLE, IoThub_Mgr_CreateConvenienceClientAsync, const char*, dps_id_scope, IOTHUB_CLIENT_TRANSPORT_PROVIDER, protocol, IoTHub_Mgr_Convenience_Handle_callback, device_handle_cb, void*, user_ctx);
MOCKABLE_FUNCTION(, IOTHUB_CLIENT_MGR_HANDLE, IoThub_Mgr_CreateClientAsync, const char*, dps_id_scope, IOTHUB_CLIENT_TRANSPORT_PROVIDER, protocol, IoTHub_Mgr_Device_Handle_callback, device_handle_cb, void*, user_ctx);
MOCKABLE_FUNCTION(, void, IoThub_Mgr_DoWork, IOTHUB_CLIENT_MGR_HANDLE, handle);

#endif // IOTHUB_CLIENT_MANAGER
