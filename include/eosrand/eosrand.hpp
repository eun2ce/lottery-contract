#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/crypto.hpp>

#include <sio4/crypto/xxhash.hpp>
#include <sio4/multi_index_wrapper.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract]] eosrand : public contract {
public:
   using contract::contract;

   //const microseconds min_duration =  3 * 24 * 3600 * 1000; // 3 days

   struct grade {
      asset               reward;
      uint32_t            score;
      optional<uint32_t>  limit;

      EOSLIB_SERIALIZE(grade, (reward)(score)(limit))
   };

   struct [[eosio::table]] scheme {
      name              scheme_name;
      uint32_t          withdraw_delay_sec = 0; //ms
      time_point_sec    expiration;
      asset             out;
      extended_asset    budget;
      vector<grade>     grades;
      vector<uint32_t>  out_count;
      uint8_t           precision;
      bool              activated = false;

      uint64_t primary_key()const { return scheme_name.value; }

      EOSLIB_SERIALIZE(scheme, (scheme_name)(withdraw_delay_sec)(expiration)(out)(budget)(grades)(out_count)(precision)(activated))
   };

   typedef multi_index<"scheme"_n, scheme> schemes;

   struct [[eosio::table]] chance {
      uint64_t    id;
      name        owner;
      name        dealer;
      name        scheme_name;
      checksum256 dseedhash;
      checksum256 oseed;

      static uint64_t hash(name dealer, name scheme_name, const checksum256& dseedhash) {
         std::array<char,2*8+32> data;
         datastream<char*> ds(data.data(), data.size());
         ds << dealer.value;
         ds << scheme_name.value;
         ds << dseedhash.extract_as_byte_array();
         return sio4::xxh64(data.data(), data.size());
      }

      uint64_t primary_key()const { return id; }
      uint64_t by_owner()const { return owner.value; }

      EOSLIB_SERIALIZE(chance, (id)(owner)(dealer)(scheme_name)(dseedhash)(oseed))
   };

   typedef multi_index<"chance"_n, chance,
              indexed_by<"owner"_n, const_mem_fun<chance, uint64_t, &chance::by_owner>>
           > chances;

   struct [[eosio::table("withdraws")]] withdrawal_request {
      uint64_t       id;
      name           owner;
      time_point_sec scheduled_time;

      uint64_t primary_key()const       { return id; }
      uint64_t by_scheduled_time()const { return static_cast<uint64_t>(scheduled_time.utc_seconds); }

      EOSLIB_SERIALIZE(withdrawal_request, (id)(owner)(scheduled_time))
   };

   typedef multi_index<"withdraws"_n, withdrawal_request,
           indexed_by<"schedtime"_n, const_mem_fun<withdrawal_request, uint64_t, &withdrawal_request::by_scheduled_time>>
              > withdraws;

   // dummy action to resolve cdt v1.6.1 issue on notify handler
   [[eosio::on_notify("eosio.token::transfer")]]
   void on_eos_transfer(name from, name to, asset quantity, string memo) {
      on_transfer(from, to, quantity, memo);
   }

   [[eosio::on_notify("*::transfer")]]
   void on_transfer(name from, name to, asset quantity, string memo);
   typedef action_wrapper<"transfer"_n, &eosrand::on_transfer> transfer_action;

   [[eosio::action]]
   void newscheme(name dealer, name scheme_name, std::vector<grade> &grades, extended_asset budget, uint32_t withdraw_delay_sec, time_point_sec expiration, optional<uint8_t> precision);

   [[eosio::action]]
   void newchance(name dealer, name scheme_name, name owner, checksum256 dseedhash, optional<uint64_t> id);

   [[eosio::action]]
   void setoseed(name owner, uint64_t id, checksum256 oseed);

   [[eosio::action]]
   void setdseed(name dealer, uint64_t id, checksum256 dseed);

   [[eosio::action]]
   void withdraw(name owner, uint64_t id);
   typedef action_wrapper<"withdraw"_n, &eosrand::withdraw> withdraw_processed;

   [[eosio::action]]
   void winreward(name owner, uint64_t id, uint32_t score, extended_asset value);

   [[eosio::action]]
   void raincheck(name owner, uint64_t id, uint32_t score);

   [[eosio::action]]
   void clrwithdraws(name owenr, uint64_t id);
   typedef action_wrapper<"clrwithdraws"_n, &eosrand::clrwithdraws> clear_withdraws;

   vector<int8_t> mixseed(const checksum256& dseed, const checksum256& oseed) const;

   class requests : public sio4::multi_index_wrapper<withdraws> {
   public:
      using multi_index_wrapper::multi_index_wrapper;

      requests(name code, name scope, uint64_t id)
         : multi_index_wrapper(code, scope, id)
      {}

      void refresh_schedule();
      void clear();
      inline name self()const    { return code(); } //_self
      inline name owner()const    { return scope(); } // owner
   };

};
