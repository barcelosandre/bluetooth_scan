from awscrt import mqtt, http
from awsiot import mqtt_connection_builder
import sys
import threading
import time
import json
import os
from hashlib import md5

# Path to the JSON file
json_file_path = '../utils/data/scanned_devices.json'

# Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0.

# This sample uses the Message Broker for AWS IoT to send and receive messages
# through an MQTT connection. On startup, the device connects to the server,
# subscribes to a topic, and begins publishing messages to that topic.
# The device should receive those same messages back from the message broker,
# since it is subscribed to that same topic.

# cmdData is the arguments/input from the command line placed into a single struct for
# use in this sample. This handles all of the command line parsing, validating, etc.
# See the Utils/CommandLineUtils for more information.
#cmdData = CommandLineUtils.parse_sample_input_pubsub()

received_count = 0
received_all_event = threading.Event()

# Callback when connection is accidentally lost.
def on_connection_interrupted(connection, error, **kwargs):
    print("Connection interrupted. error: {}".format(error))


# Callback when an interrupted connection is re-established.
def on_connection_resumed(connection, return_code, session_present, **kwargs):
    print("Connection resumed. return_code: {} session_present: {}".format(return_code, session_present))

    if return_code == mqtt.ConnectReturnCode.ACCEPTED and not session_present:
        print("Session did not persist. Resubscribing to existing topics...")
        resubscribe_future, _ = connection.resubscribe_existing_topics()

        # Cannot synchronously wait for resubscribe result because we're on the connection's event-loop thread,
        # evaluate result with a callback instead.
        resubscribe_future.add_done_callback(on_resubscribe_complete)


def on_resubscribe_complete(resubscribe_future):
    resubscribe_results = resubscribe_future.result()
    print("Resubscribe results: {}".format(resubscribe_results))

    for topic, qos in resubscribe_results['topics']:
        if qos is None:
            sys.exit("Server rejected resubscribe to topic: {}".format(topic))


# Callback when the subscribed topic receives a message
def on_message_received(topic, payload, dup, qos, retain, **kwargs):
    print("Received message from topic '{}': {}".format(topic, payload))
    global received_count
    received_count += 1
    if received_count == messageCount:
        received_all_event.set()

# Callback when the connection successfully connects
def on_connection_success(connection, callback_data):
    assert isinstance(callback_data, mqtt.OnConnectionSuccessData)
    print("Connection Successful with return code: {} session present: {}".format(callback_data.return_code, callback_data.session_present))

# Callback when a connection attempt fails
def on_connection_failure(connection, callback_data):
    assert isinstance(callback_data, mqtt.OnConnectionFailureData)
    print("Connection failed with error code: {}".format(callback_data.error))

# Callback when a connection has been disconnected or shutdown successfully
def on_connection_closed(connection, callback_data):
    print("Connection closed")

# Function to read the JSON file and return its content
def read_json_file(file_path):
    with open(file_path, 'r') as file:
        return json.load(file)
    
if __name__ == '__main__':

    # AWS IoT Core endpoint details
    host = "a20vw3ttgz0kkj-ats.iot.us-east-2.amazonaws.com"
    rootCAPath = os.getenv("BLE_CA_ROOT")
    certificatePath = os.getenv("BLE_CRT_PATH")
    privateKeyPath = os.getenv("BLE_KEY_PATH")
    portNumber = 8883 
    clientId = "bl_scanner"
    message_topic = "bl_scanner/scans"
    message_string = "Hello World!"
    messageCount = 5

    # Create a MQTT connection from the command line data
    mqtt_connection = mqtt_connection_builder.mtls_from_path(
        endpoint=host,
        port=portNumber,
        cert_filepath=certificatePath,
        pri_key_filepath=privateKeyPath,
        ca_filepath=rootCAPath,
        on_connection_interrupted=on_connection_interrupted,
        on_connection_resumed=on_connection_resumed,
        client_id=clientId,
        clean_session=False,
        keep_alive_secs=30,
        http_proxy_options=None,
        on_connection_success=on_connection_success,
        on_connection_failure=on_connection_failure,
        on_connection_closed=on_connection_closed)

    print(f"Connecting to {host} with client ID '{clientId}'...")

    connect_future = mqtt_connection.connect()

    # Future.result() waits until a result is available
    connect_future.result()
    print("Connected!")

    # # If you want to check the same topic and see the callback to receive messages
    # print("Subscribing to topic '{}'...".format(message_topic))
    # subscribe_future, packet_id = mqtt_connection.subscribe(
    #     topic=message_topic,
    #     qos=mqtt.QoS.AT_LEAST_ONCE,
    #     callback=on_message_received)
    # subscribe_result = subscribe_future.result()
    # print("Subscribed with {}".format(str(subscribe_result['qos'])))


    # Main loop
    last_hash = ''
    while True:
        # Read the current data from the JSON file
        current_data = read_json_file(json_file_path)

        # Create a hash of the current data for comparison
        current_hash = md5(json.dumps(current_data, sort_keys=True).encode('utf-8')).hexdigest()

        # Check if the current data is different from the last data
        if current_hash != last_hash:
            print('Data changed, sending to AWS IoT via MQTT.')
            mqtt_connection.publish(
                topic=message_topic,
                payload=json.dumps(current_data),
                qos=mqtt.QoS.AT_LEAST_ONCE
                )
            last_hash = current_hash
            print('Data sent!')
        else:
            print('Data unchanged.')

        print('Waiting for 60s to check the JSON file again...')
        time.sleep(60)

    # Wait for all messages to be received.
    # This waits forever if count was set to 0.
    if message_count != 0 and not received_all_event.is_set():
        print("Waiting for all messages to be received...")

    received_all_event.wait()
    print("{} message(s) received.".format(received_count))

    # Disconnect
    print("Disconnecting...")
    disconnect_future = mqtt_connection.disconnect()
    disconnect_future.result()
    print("Disconnected!")
