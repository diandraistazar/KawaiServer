# Changelog
Any changes to this project, will be documented here
 
## 0.0.1 - 2026-06-10

### Added

- Logging capability.

### Changed

- ```addrsock_str()``` function in ```netlinux-utils.h``` header, have been replaced by ```Utils::converto()``` function in ```utils.hpp```.
- ```Server::get_addr()``` and ```Server::create_sock()``` are combined into ```Server::create_sock()```, since they do similar things.

### Removed

- ```netlinux-utils.h``` and ```macro.h``` are deleted.
- ```fatal()``` handling error function in ```main.cpp``` file is useless.
