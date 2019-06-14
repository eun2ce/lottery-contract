#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract]] eosrand : public contract {
public:
   using contract::contract;
   //using grade_value = std::pair<asset, uint64_t>;

   struct grade {
      asset reward;
      uint64_t value;

      EOSLIB_SERIALIZE(grade, (reward)(value))
   };
   std::vector<grade> luckybox;

   [[eosio::on_notify("*::transfer")]]
   void on_transfer(name from, name to, asset quantity, string memo);
   typedef action_wrapper<"transfer"_n, &eosrand::on_transfer> transfer_action;

   // dummy action to resolve cdt v1.6.1 issue on notify handler
   [[eosio::on_notify("eosio.token::transfer")]]
   void on_eos_transfer(name from, name to, asset quantity, string memo) {
      on_transfer(from, to, quantity, memo);
   }

   [[eosio::action]]
   checksum256 mixseed(const checksum256& sseed, checksum256& useed) const;

   [[eosio::action]]
   bool hashcheck(const checksum256 hash) const;

   [[eosio::action]]
   void setbox(name owner, name contract_name, const std::vector<grade>& lbox);

   [[eosio::action]]
   void newgacha(name owner, name contract_name, checksum256 rseed, uint8_t lucknum);
   //vector lucky box
   //[[eosio::action]]
   //newgacha(gamename, username, contrract_name, hash, luckybox number);

   [[eosio::action]]
   void newevent(name owner, name contract_name, name participant, extended_asset value, checksum256 oseed, time_point_sec timelock);

   [[eosio::action]]
   void setuserseed(name owner, name contract_name, name participant, checksum256 useed);

   [[eosio::action]]
   void setownseed(name owner, name contract_name, checksum256 oseed);

   [[eosio::action]]
   void withdraw(name owner, name contract_name, checksum256 preimage);


   struct st_seeds {
      checksum256 seed1;
      checksum256 seed2;
   };


   struct [[eosio::table]] event {
      name contract_name;
      name participant;
      extended_asset value;
      checksum256 preimage;
      checksum256 seed;
      time_point_sec timelock;
      bool activated;

      uint64_t primary_key()const { return contract_name.value; }
      EOSLIB_SERIALIZE(event, (contract_name)(participant)(value)(preimage)(seed)(timelock)(activated))
   };

   typedef multi_index<"event"_n, event> events;


   //[[eosio::action]]
   //uint8_t srand();
};
