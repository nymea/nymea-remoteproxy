# nymea remote proxy server
----------------------------------------------

The nymea remote proxy server is the meeting point of nymea servers and nymea clients in order to establishing a secure remote connection.

# Build

In order to build the proxy server you need to install the qt default package.

    apt install qt5-default

Change into the source directory and run following commands

    cd nymea-remoteproxy
    mkdir build
    cd build
    qmake ../
    make -j$(nproc)

In the build directory you can find the resulting library and binary files.

If you want to start the proxy server from the build directory, you need to export the library path before starting the application:


    $ export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/libnymea-remoteproxy
    $ ./server/nymea-remoteproxy


# Install



# Configure



# Test

In order to run the test, you can call `make check` in the build directory or run the resulting executable:

    $ nymea-remoteproxy-tests

# Usage

In order to get information about the server you can start the command with the `--help` parameter.

    $ nymea-remoteproxy --help
    
    usage: nymea-remoteproxy [options]
    
    The nymea remote proxy server.
    
    Copyright © 2018 Simon Stürz <simon.stuerz@guh.io>
    
    Options:
      -h, --help                               Displays this help.
      -l, --logging <logfile>                  Write log file to the given logfile.
      -s, --server <url>                       The authentication server url.
      -p, --port <port>                        The proxy server port.
      -c, --certificate <certificate>          The path to the SSL certificate.
      -k, --certificate-key <certificate-key>  The path to the SSL certificate key
                                               (KEY).
      -v, --verbose                            Print the whole traffic.



# Server API

Once a client connects to the proxy server, he must authenticate him self by passing the token received from the nymea-cloud mqtt connection request.

## Message format

#### Request

    {
        "id": integer,
        "method": "Namespace.Method",
        "o:params" { }
    }

#### Response

    {
        "id": integer,
        "status": "string",
        "o:params" { },
        "o:error": "string"
    }

#### Notification

    {
        "id": integer,
        "notification": "Namespace.Notification",
        "o:params" { }
    }


## Authenticate the connection

The first data a client **must** send to the proxy server is the authentication request. This request contains the token which will be verified agains the nymea-cloud infrastructure.

#### Request

    {
        "id": 0,
        "method": "Authentication.Authenticate"
        "params": {
            "token": "tokenstring"
        }
    }

#### Response

* **On Success**: If the token was authenticated successfully, the response will look like this:

        {
            "id": 0,
            "status": "success"
        }

* **On Failure** If the token was invalid, the response will look like this and the server will close the connection immediatly:

        {
            "id": 0,
            "status": "error",
            "error": "Invalid token. You are not allowed to use this server."
        }

#### Tunnel established

Once the other client is here and ready, the server will send a notification to the clients indicating that the tunnel has been established successfully. This message is the last data comming from the proxy server.

> **Important:** Any data traffic following after this notification comes from the tunnel endpoint, __not__ from the __proxy server__ any more.

    {
        "id": "1",
        "notification": "Tunnel.Established"
    }


# License

This is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, version 3 of the License.
