#include <eosio/eosio.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract]] eosrand : public contract {
   public:
      using contract::contract;
      struct [[eosio::table]] event {
         uint8_t random_number;
         checksum256 seed;
         name event_name;
         uint32_t prize_rank;

         uint64_t primary_key()const { return event_name.value; }
         EOSLIB_SERIALIZE(event, (random_number)(seed)(event_name)(prize_rank))
      };

      typedef multi_index<"event"_n, event> events;

      [[eosio::action]]
      uint8_t newevent(name event_name, uint32_t prize_rank, name owner, string srv_seed);

      [[eosio::action]]
      uint8_t srand();
};
