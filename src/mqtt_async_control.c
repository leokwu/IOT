//
// Created by leok on 2022/2/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <cJSON.h>
#include <MQTTAsync.h>
#include <unistd.h>

#include "mqtt_async_control.h"
#include "package_control.h"

#define ADDRESS             "tcp://218.104.230.76:17124"
//#define ADDRESS           "tcp://10.10.10.10:1883"
#define CLIENTID            "toybrick"
#define DISCOVERY_TOPIC      "homeassistant/sensor/device1/config"

#define QOS                  1
#define TIMEOUT              10000L

#define USENAME              "hm"
#define PASSWORD             "IpYMEneM6K"

int g_finished = 0;

MQTTAsync g_client;


void connlost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
        g_finished = 1;
	}
}

void onDisconnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Disconnect failed\n");
    g_finished = 1;
}

void onDisconnect(void* context, MQTTAsync_successData* response)
{
	printf("Successful disconnection\n");
    g_finished = 1;
}

void onSendFailure(void* context, MQTTAsync_failureData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	printf("Message send failed token %d error code %d\n", response->token, response->code);
	opts.onSuccess = onDisconnect;
	opts.onFailure = onDisconnectFailure;
	opts.context = client;
	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start disconnect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}

void onSend(void* context, MQTTAsync_successData* response)
{
    printf("publish message success\n");
}


void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response ? response->code : 0);
    g_finished = 1;
}



void onSubscribe(void* context, MQTTAsync_successData* response)
{
    printf("Subscribe succeeded\n");
}

void onSubscribeFailure(void* context, MQTTAsync_failureData* response)
{
    printf("Subscribe failed, rc %d\n", response->code);
    g_finished = 1;
}


static void freeJson(cJSON *json) {
    if (json != NULL) {
        cJSON_Delete(json);
    }
}


void mqttMessagePublish(const char* topic, void *payload)
{
    if (NULL == g_client || NULL == topic || NULL == payload) {
        printf("%s: input NULL!\n", __func__ );
        return;
    }

    MQTTAsync client = g_client;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
    int rc;

    printf("Successful connection\n");
    opts.onSuccess = onSend;
    opts.onFailure = onSendFailure;
    opts.context = client;
    pubmsg.payload = payload;
    pubmsg.payloadlen = (int)strlen(payload);
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    if ((rc = MQTTAsync_sendMessage(client, topic, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start sendMessage, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
}


void mqttMessageSubscribe(const char* topic)
{
    if (NULL == topic) {
        printf("%s: input NULL!\n", __func__ );
        return;
    }

    MQTTAsync client = g_client;
    MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
    int rc;

    printf("Successful connection\n");

    printf("Subscribing to topic %s\nfor client %s using QoS %d\n\n", topic, CLIENTID, QOS);
    opts.onSuccess = onSubscribe;
    opts.onFailure = onSubscribeFailure;
    opts.context = client;
    if ((rc = MQTTAsync_subscribe(client, topic, QOS, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start subscribe, return code %d\n", rc);
        g_finished = 1;
    }
}

#if 0
static void onlinePublish() // for test
{
    cJSON *item = cJSON_CreateObject();
    cJSON_AddStringToObject(item, "deviceId", "100000001");
    cJSON_AddStringToObject(item, "status", "1");


    char *cjson = cJSON_Print(item);
    printf("json:%s\n", cjson);

    mqttMessagePublish(ONLINE_TOPIC, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
    }
}
#endif

void discoveryPublish()
{
    cJSON *item = cJSON_CreateObject();

    cJSON_AddStringToObject(item, "name", "test_temp");
    cJSON_AddStringToObject(item, "command_topic", "toybrick/sensor/temp/set");
    cJSON_AddStringToObject(item, "state_topic", "toybrick/sensor/temp/state");

    char *cjson = cJSON_Print(item);
    printf("%s: json:%s\n", __func__ , cjson);

    mqttMessagePublish(DISCOVERY_TOPIC, cjson);

    freeJson(item);

    if (cjson != NULL) {
        free(cjson);
        cjson = NULL;
    }

}


void reportPropertySubscribe()
{
    mqttMessageSubscribe(REPORT_PROPERTIES_TOPIC);
}

void onlineSubscribe()
{
    mqttMessageSubscribe(ONLINE_TOPIC);
}


void tagsSubscribe()
{
    mqttMessageSubscribe(TAGS_TOPIC);
}

void inbvokeFunctionSubscribe()
{
    mqttMessageSubscribe(INVOKE_FUNCTION_TOPIC);
}


void onConnect(void* context, MQTTAsync_successData* response)
{
    inbvokeFunctionSubscribe();
}


int messageArrived(void* context, char* topicName, int topicLen, MQTTAsync_message* message)
{
    printf("Message arrived\n");
    printf("     topic: %s\n", topicName);
    printf("     topicLen: %d\n", topicLen);
    printf("     message: %.*s\n", message->payloadlen, (char*)message->payload);


    deserialize_cloud_package(topicName, topicLen, message->payloadlen, message->payload);

    if (message) {
        MQTTAsync_freeMessage(&message);
    }

    if (topicName) {
        MQTTAsync_free(topicName);
    }

	return 1;
}


void freeClient(MQTTAsync client)
{
    if (client) {
        MQTTAsync_destroy(&client);
        client = NULL;
    }
}


void mqttDisconnect()
{

    MQTTAsync client = g_client;
    MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
    int rc;

    printf("mqttDisconnect---\n");
    opts.onSuccess = onDisconnect;
    opts.onFailure = onDisconnectFailure;
    opts.context = client;
    if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
    {
        printf("Failed to start disconnect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
}


int mqttMainProcess()
{
	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;
    int ch;

    g_finished = 0;

	if ((rc = MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to create client object, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
    g_client = client;

	if ((rc = MQTTAsync_setCallbacks(client, NULL, connlost, messageArrived, NULL)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to set callback, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
	conn_opts.username = USENAME;
    conn_opts.password = PASSWORD;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

    do
    {
        ch = getchar();
        usleep(10000L);
    } while (ch!='Q' && ch != 'q');


	while (!g_finished) {
        usleep(10000L);
	}

    mqttDisconnect();
    freeClient(client);

 	return rc;
}

//int main()
//{
//
//    int ret = 0;
//    ret = mqttMainProcess();
//    return ret;
//
//}
