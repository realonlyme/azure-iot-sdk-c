// Copyright (c) Microsoft. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#include <stdlib.h>
#include <string.h>
#include "azure_c_shared_utility/gballoc.h"
#include "azure_c_shared_utility/xlogging.h"
#include "azure_c_shared_utility/strings.h"
#include "azure_c_shared_utility/threadapi.h"

#include "azure_prov_client/prov_device_ll_client.h"
#include "azure_prov_client/prov_security_factory.h"
#include "azure_prov_client/prov_transport.h"
#include "iothub_device_client_ll.h"
#include "iothub_device_client.h"

#include "azure_prov_client/prov_transport.h"
#include "azure_prov_client/internal/prov_sasl_tpm.h"
#include "iothub_client_manager/iothub_client_manager.h"

#ifdef USE_MQTT
#include "iothubtransportmqtt.h"
#include "azure_prov_client/prov_transport_mqtt_client.h"
#include "iothubtransportmqtt_websockets.h"
#include "azure_prov_client/prov_transport_mqtt_ws_client.h"
#endif // USE_MQTT
#ifdef USE_AMQP
#include "iothubtransportamqp.h"
#include "azure_prov_client/prov_transport_amqp_client.h"
#include "iothubtransportamqp_websockets.h"
#include "azure_prov_client/prov_transport_amqp_ws_client.h"
#endif // USE_AMQP
#ifdef USE_HTTP
#include "iothubtransportmqtt.h"
#include "azure_prov_client/prov_transport_http_client.h"
#endif // USE_HTTP

static const char* const global_prov_uri = "global.azure-devices-provisioning.net";

typedef enum OPERATION_STATE_TAG
{
    STATE_DPS_REGISTER,
    STATE_COMPLETE,
    STATE_ERROR
} CLIENT_STATE;

typedef struct IOTHUB_CLIENT_MGR_TAG
{
    CLIENT_STATE op_state;
    char* iothub_uri;
    char* device_id;
} IOTHUB_CLIENT_MGR;

static void dps_register_device_callback(PROV_DEVICE_RESULT register_result, const char* iothub_uri, const char* device_id, void* user_context)
{
    IOTHUB_CLIENT_MGR* iothub_mgr = (IOTHUB_CLIENT_MGR*)user_context;
    if (register_result != PROV_DEVICE_RESULT_OK)
    {
        iothub_mgr->op_state = STATE_ERROR;
    }
    else
    {
        if (mallocAndStrcpy_s(&iothub_mgr->iothub_uri, iothub_uri) != 0)
        {
            iothub_mgr->op_state = STATE_ERROR;
        }
        else if (mallocAndStrcpy_s(&iothub_mgr->device_id, device_id) != 0)
        {
            iothub_mgr->op_state = STATE_ERROR;
        }
        else
        {
            iothub_mgr->op_state = STATE_COMPLETE;
        }
    }
}

static PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION create_dps_transport(IOTHUB_CLIENT_TRANSPORT_PROVIDER iothub_prov)
{
    PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION result = NULL;
#if defined(USE_MQTT)
    if (iothub_prov == MQTT_Protocol)
    {
        result = Prov_Device_MQTT_Protocol;
    }
    else if (iothub_prov == MQTT_WebSocket_Protocol)
    {
        result = Prov_Device_MQTT_WS_Protocol;
    }
#endif // USE_MQTT
#ifdef USE_AMQP
    if (iothub_prov == AMQP_Protocol)
    {
        result = Prov_Device_AMQP_Protocol;
    }
    else if (iothub_prov == AMQP_Protocol_over_WebSocketsTls)
    {
        result = Prov_Device_AMQP_WS_Protocol;
    }
#endif // USE_AMQP
    return result;
}

static IOTHUB_CLIENT_TRANSPORT_PROVIDER create_iothub_transport(PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION dps_protocol)
{
    IOTHUB_CLIENT_TRANSPORT_PROVIDER result;
#if defined(USE_MQTT)
    if (dps_protocol == Prov_Device_MQTT_Protocol)
    {
        result = MQTT_Protocol;
    }
    else if (dps_protocol == Prov_Device_MQTT_WS_Protocol)
    {
        result = MQTT_WebSocket_Protocol;
    }
#endif // USE_MQTT
#ifdef USE_AMQP
    if (dps_protocol == Prov_Device_AMQP_Protocol)
    {
        result = AMQP_Protocol;
    }
    else if (dps_protocol == Prov_Device_AMQP_WS_Protocol)
    {
        result = AMQP_Protocol_over_WebSocketsTls;
    }
#endif // USE_AMQP
#ifdef USE_HTTP
    if (dps_protocol == Prov_Device_HTTP_Protocol)
    {
        result = MQTT_Protocol;
    }
#endif // USE_HTTP
    return result;
}

static int create_and_execute_dps(const char* dps_id_scope, PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION protocol, IOTHUB_CLIENT_MGR* iothub_mgr)
{
    int result;
    // Connect to DPS
    PROV_DEVICE_LL_HANDLE handle;
    if ((handle = Prov_Device_LL_Create(global_prov_uri, dps_id_scope, protocol)) == NULL)
    {
        LogError("failed calling Prov_Device_LL_Create\r\n");
        result = __FAILURE__;
    }
    else
    {
        if (Prov_Device_LL_Register_Device(handle, dps_register_device_callback, &iothub_mgr, NULL, NULL) != PROV_DEVICE_RESULT_OK)
        {
            (void)printf("failed calling Prov_Device_LL_Register_Device\r\n");
            result = __FAILURE__;
        }
        else
        {
            do
            {
                Prov_Device_LL_DoWork(handle);
                ThreadAPI_Sleep(10);
            } while (iothub_mgr->op_state == STATE_DPS_REGISTER);
        }
        Prov_Device_LL_Destroy(handle);
        if (iothub_mgr->device_id == NULL || iothub_mgr->iothub_uri == NULL || iothub_mgr->op_state == STATE_ERROR)
        {
            result = __FAILURE__;
        }
    }
    return result;
}

IOTHUB_DEVICE_CLIENT_LL_HANDLE IoThub_Mgr_CreateClient(const char* dps_id_scope, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol)
{
    IOTHUB_DEVICE_CLIENT_LL_HANDLE result;
    IOTHUB_CLIENT_MGR iothub_mgr;
    if (dps_id_scope == NULL)
    {
        result = NULL;
    }
    else
    {

        memset(&iothub_mgr, 0, sizeof(IOTHUB_CLIENT_MGR) );
        PROV_DEVICE_TRANSPORT_PROVIDER_FUNCTION prov_transport = create_dps_transport(protocol);
        if (create_and_execute_dps(dps_id_scope, prov_transport, &iothub_mgr) != 0)
        {
            result = NULL;
        }
        else
        {
            result = IoTHubDeviceClient_LL_CreateFromDeviceAuth(iothub_mgr.iothub_uri, iothub_mgr.device_id, protocol);
            if (result == NULL)
            {
            }
        }
    }
    return result;
}

IOTHUB_DEVICE_CLIENT_LL_HANDLE IoThub_Mgr_CreateClientAsync(const char* dps_id_scope, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol)
{
    IOTHUB_DEVICE_CLIENT_LL_HANDLE result;
    if (dps_id_scope == NULL)
    {
        result = NULL;
    }
    else
    {
        // Connect to DPS
        result = NULL;
    }
    return result;
}

IOTHUB_DEVICE_CLIENT_HANDLE IoThub_Mgr_CreateConvenienceClient(const char* dps_id_scope, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol)
{
    IOTHUB_DEVICE_CLIENT_HANDLE result;
    if (dps_id_scope == NULL)
    {
        result = NULL;
    }
    else
    {
        // Connect to DPS
        result = NULL;
    }
    return result; 
}

IOTHUB_DEVICE_CLIENT_HANDLE IoThub_Mgr_CreateConvenienceClientAsync(const char* dps_id_scope, IOTHUB_CLIENT_TRANSPORT_PROVIDER protocol)
{
    IOTHUB_DEVICE_CLIENT_HANDLE result;
    if (dps_id_scope == NULL)
    {
        result = NULL;
    }
    else
    {
        // Connect to DPS
        result = NULL;
    }
    return result;
}
