# Little-sb

## Architecture
The project utilize C/S Architecture, thus consists of a client and a server.

## Prerequisite
To prepare for the build process, you should install the following software:
- [XMake](xmake.io)—builds the project

## Build
To build the project, simply run
```
xmake
```
And it builds both client and server. To build client or server seperately, use:
```
xmake build little-sb-client # Builds client
xmake build little-sb-server # Builds server
```
Use
```
xmake run little-sb-client
```
and
```
xmake run little-sb-server
```
to run them.
