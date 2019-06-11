project(eosrand)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract(eosrand eosrand eosrand.cpp)
target_include_directories(eosrand PUBLIC ${CMAKE_SOURCE_DIR}/../include)
