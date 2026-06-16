# Changelog
Any changes to this project, will be documented here

## 0.0.2 - 2026-06-16

### Added

- Unix/Local socket added and point to ```/tmp/kawaiserver-local``` for communicating with PHP (it's not finished yet).
- Built-in mime types checking. Previously used ```libmagic```, but it couldn't handle them properly.
- Specify the root path explicity.
- Signal handling for terminating the server.
- If clients request ```/```, then the server is going to send either ```/index.html``` or ```/index.php``` (based on the root path) instead.
- If clients request a directory, the server is going to find ```index.html``` or ```/index.php``` in that directory instead.
- Show how long the program have run in seconds.

### Changed

- Sending parts are in each methods.
- ```start()``` changed to ```run()``` because it's better.
- ```recv_data()``` can receive data of unlimited size now.
- ```send_data()``` sends data in chunks (each of them is 128 bytes or lower) instead of fullsize directly.
- ```send_file()``` sends data until the number of sent bytes returned is equal to the size of the original data.
- ```create_sock()``` changed to ```create_sockets()```, it only provides the host and the service, the rest of arguments is abstracted now.
- ```set_sockopt()``` changed to ```set_socket_options()```, it doesn't provide any arguments, the rest of arguments is abstracted now.
- ```bind_sock()``` changed to ```bind_sockets()```, only changed the method name for a clearer name.
- ```listen_sock()``` changed to ```listen_sockets()```, only changed the method name.

### Removed

- Dependence on ```libmagic``` is removed.

## 0.0.1 - 2026-06-10

### Added

- Logging capability.

### Changed

- ```addrsock_str()``` function in ```netlinux-utils.h``` header, have been replaced by ```Utils::converto()``` function in ```utils.hpp```.
- ```Server::get_addr()``` and ```Server::create_sock()``` are combined into ```Server::create_sock()```, since they do similar things.

### Removed

- ```netlinux-utils.h``` and ```macro.h``` are deleted.
- ```fatal()``` handling error function in ```main.cpp``` file is useless.
