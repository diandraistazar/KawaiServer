# Changelog
Any changes to this project, will be documented here

## 0.0.3 - 2026-06-21

### Added

- Query handling.
- HEAD method, actually using ```handle_get()``` with ```true``` value passed to ```bool header_only``` parameter.
- Error handling when parsing either HTTP request or HTTP response.

### Fixed

- Header parser in ```Http::parse_http_header()``` works fine now.

### Changed

- ```Server::php_service``` has additional argument named ```_request```, it used to give several informations to PHP service (e.g request_path, root_path).
- ```struct Logging``` is not structure anymore, it changed to ```namespace Logging```. So that, whenever you modify the logging behavior in any source files, it can influnce others.
- ```struct Http::Header headers``` member in ```struct Http::response_msg```, ```struct Http::request_msg```, and ```struct Http::msg``` replaced by ```std::unordered_map<string, string> headers```. It's just better.
- ```Http::push_token()``` adjusted based on ```std::unordered_map<string, string> headers```.

## 0.0.2 - 2026-06-19

### Added

- ```read_file()``` dedicated to reading read end pipe in PHP service.
- Logging with color suppport.
- PHP service, so that a file with .php extension can be processed by PHP and send the result to a client.
- Built-in mime types checking. Previously used ```libmagic```, but it couldn't handle them properly.
- Specify the root path explicity.
- Signal handling for terminating the server.
- If clients request ```/```, then the server is going to send either ```/index.html``` or ```/index.php``` (based on the root path) instead.
- If clients request a directory, the server is going to find ```index.html``` or ```index.php``` in that directory instead.
- Show how long the program have run in seconds.

### Changed

- Instead of hardcoded status codes and status descs, they stored into an unordered map named status_messages.
- Mime types are grouped into an unordered map with the file extension as the key.
- Sending parts are in each methods.
- ```recv_data()``` can receive data of unlimited size now.
- ```send_data()``` sends data in chunks (each of them is 64 bytes or lower) instead of fullsize directly.
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
