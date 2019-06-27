project(eosrand)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_subdirectory(${CMAKE_SOURCE_DIR}/../lib/libsio4 ${CMAKE_BINARY_DIR}/../lib/libsio4)

add_contract(eosrand eosrand eosrand.cpp)
target_include_directories(eosrand PUBLIC ${CMAKE_SOURCE_DIR}/../include)
target_link_libraries(eosrand sio4)
