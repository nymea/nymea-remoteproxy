#!/bin/bash

CHILD_PROCESSES=()
SERVER_URL=tcp://127.0.0.1:2213

SERVER_COUNTER=0
CLIENT_COUNTER=0
TOTAL_COUNTER=0

# Start server: Arguments: serverName serverUuid
function createServer() {
    echo "--> Create server"
    SERVER_COUNTER=$(($SERVER_COUNTER + 1))
    TOTAL_COUNTER=$(($TOTAL_COUNTER + 1))
    nymea-remoteproxy-tunnelclient -s -r -n "$1" --uuid "$2" -u $SERVER_URL -i & CHILD_PROCESSES+=("$!")
}

# Create client: Arguments: clientName serverUuid
function createClient() {
    echo "--> Create client"
    CLIENT_COUNTER=$(($CLIENT_COUNTER + 1))
    TOTAL_COUNTER=$(($TOTAL_COUNTER + 1))
    nymea-remoteproxy-tunnelclient -c -r -n "$1" --server-uuid "$2" -u $SERVER_URL -i & CHILD_PROCESSES+=("$!")
}


# Arguments: client count
function createTunnelConnections() {
    local serverUuid=$(uuidgen)
    createServer "Test server $serverUuid " $serverUuid
    sleep 0.5
    i=1
    while [[ $i -lt $1 ]] ; do
        createClient "Test client $i" $serverUuid
        ((i += 1))
        #sleep 0.5
    done

}


function cleanup {
    echo "Cleanup"
    kill "${CHILD_PROCESSES[@]}"
}

trap cleanup EXIT

createTunnelConnections 5
createTunnelConnections 3
createTunnelConnections 9
createTunnelConnections 6
createTunnelConnections 4
createTunnelConnections 3
createTunnelConnections 2
createTunnelConnections 1
createTunnelConnections 3
createTunnelConnections 2
createTunnelConnections 1
createTunnelConnections 8
createTunnelConnections 0
createTunnelConnections 0
createTunnelConnections 0
createTunnelConnections 0
createTunnelConnections 0
createTunnelConnections 0
createTunnelConnections 0
createTunnelConnections 0
createTunnelConnections 0
createTunnelConnections 0

sleep 10

echo "------------------------------"
echo "Total: $TOTAL_COUNTER"
echo "Servers: $SERVER_COUNTER"
echo "Clients: $CLIENT_COUNTER"

sleep 300
