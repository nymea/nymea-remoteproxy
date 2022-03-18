#!/bin/bash

CHILD_PROCESSES=()
SERVER_URL=tcp://127.0.0.1:2213

# Start server: Arguments: serverName serverUuid
function createServer() {
    echo "--> Create server"
    nymea-remoteproxy-tunnelclient -s -n "$1" --uuid "$2" -u $SERVER_URL -i & CHILD_PROCESSES+=("$!")
}

# Create client: Arguments: clientName serverUuid
function createClient() {
    echo "--> Create client"
    nymea-remoteproxy-tunnelclient -c -n "$1" --server-uuid "$2" -u $SERVER_URL -i & CHILD_PROCESSES+=("$!")
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

sleep 300
