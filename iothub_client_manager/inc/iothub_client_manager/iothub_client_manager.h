// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#ifndef IOTHUB_CLIENT_MANAGER

#include "azure_c_shared_utility/umock_c_prod.h"
#include "azure_prov_client/prov_transport.h"
#include "iothub_device_client_ll.h"
#include "iothub_device_client.h"

MOCKABLE_FUNCTION(, IOTHUB_DEVICE_CLIENT_LL_HANDLE, IoThub_Mgr_CreateClient, const char*, dps_id_scope, PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION, protocol);
MOCKABLE_FUNCTION(, IOTHUB_DEVICE_CLIENT_LL_HANDLE, IoThub_Mgr_CreateClientAsync, const char*, dps_id_scope, PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION, protocol);

MOCKABLE_FUNCTION(, IOTHUB_DEVICE_CLIENT_HANDLE, IoThub_Mgr_CreateConvenienceClient, const char*, dps_id_scope, PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION, protocol);
MOCKABLE_FUNCTION(, IOTHUB_DEVICE_CLIENT_HANDLE, IoThub_Mgr_CreateConvenienceClientAsync, const char*, dps_id_scope, PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION, protocol);

#endif // IOTHUB_CLIENT_MANAGER
