#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract]] eosrand : public contract {
public:
   using contract::contract;

   //static microseconds min_duration = 3 * 24 * 3600 * 1000; // 3 days

   struct grade {
      uint32_t score;
      asset reward;
      optional<uint32_t> limit;

      EOSLIB_SERIALIZE(grade, (score)(reward)(limit))
   };

   struct [[eosio::table]] scheme {
      name scheme_name;
      extended_asset budget;
      time_point_sec expiration;
      uint8_t precision;
      vector<grade> grades;
      asset out;
      vector<uint32_t> out_count;
      bool activated = false;

      uint64_t primary_key()const { return scheme_name.value; }

      EOSLIB_SERIALIZE(scheme, (scheme_name)(budget)(expiration)(precision)(grades)(out)(out_count)(activated))
   };

   struct [[eosio::table]] chance {
      uint64_t id;
      name owner;
      name dealer;
      name scheme_name;
      checksum256 dseedhash;
      checksum256 oseed;

      uint64_t primary_key()const { return id; }
      uint64_t by_owner()const { return owner.value; }

      EOSLIB_SERIALIZE(chance, (id)(owner)(dealer)(scheme_name)(dseedhash)(oseed))
   };

   typedef multi_index<"scheme"_n, scheme> schemes;
   typedef multi_index<"chance"_n, chance,
              indexed_by<"owner"_n, const_mem_fun<chance, uint64_t, &chance::by_owner>>
           > chances;

   // dummy action to resolve cdt v1.6.1 issue on notify handler
   [[eosio::on_notify("eosio.token::transfer")]]
   void on_eos_transfer(name from, name to, asset quantity, string memo) {
      on_transfer(from, to, quantity, memo);
   }

   [[eosio::on_notify("*::transfer")]]
   void on_transfer(name from, name to, asset quantity, string memo);
   typedef action_wrapper<"transfer"_n, &eosrand::on_transfer> transfer_action;

	[[eosio::action]]
	void withdraw(name dealer, name owner, uint64_t id, checksum256 oseed);

   uint64_t mixseed(const checksum256& dseed, const checksum256& oseed) const;

   [[eosio::action]]
   void newscheme(name dealer, name scheme_name, std::vector<grade> &grades, extended_asset budget, time_point_sec expiration, optional<uint8_t> precision);

   [[eosio::action]]
   void newchance(name dealer, name scheme_name, name recipient, checksum256 dseedhash, uint64_t id);

   [[eosio::action]]
   void setoseed(name owner, uint64_t id, checksum256 oseed);

   [[eosio::action]]
   void setdseed(name dealer, uint64_t id, checksum256 dseed);

   [[eosio::action]]
   void winreward(name owner, uint64_t id, uint32_t score, extended_asset value);

   [[eosio::action]]
   void raincheck(name owner, uint64_t id, uint32_t score);
};
