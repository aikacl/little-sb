# Little-sb
This is a real-time battling game. In this game, you can talk with others,
battle with others, and buy goods to improve your fighting power. To find more,
see [Rules](./docs/rules.md).

Some of the screenshots:

- *Main window*
    ![main-window-screenshot-0](./docs/images/main-window-screenshot-0.png "main-window-screenshot-0")
- *Battling*
    ![battling-screenshot-0](./docs/images/battling-screenshot-0.png "battling-screenshot-0")

## Architecture
The project utilize C/S Architecture, thus consists of a client and a server.

## Prerequisite
To prepare for the building process, you should install the following software:
- [XMake](https://xmake.io)—builds the project

## Build
To build the project, simply run
```bash
xmake
```
And it builds both client and server. To build client or server seperately, use:
```bash
xmake build little-sb-client  # Builds client
xmake build little-sb-server  # Builds server
```
Use
```bash
xmake run little-sb-client    # Runs client
xmake run little-sb-server    # Runs server
```
to run them.
