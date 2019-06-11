#include <eosio/eosio.hpp>

using namespace eosio;

class [[eosio::contract]] eosrand : public contract {
   public:
      using contract::contract;

      [[eosio::action]]
      uint8_t srand();
};
