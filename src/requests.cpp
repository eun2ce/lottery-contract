void eosrand::requests::refresh_schedule() {
   auto _idx = get_index<"schedtime"_n>();
   auto _it = _idx.begin();

   chances chn(code(), code().value);
   const auto& cit = chn.find(id());

   schemes schm(code(), cit->dealer.value);
   const auto& sit = schm.get(cit->scheme_name.value);

   cancel_deferred(cit->owner.value);

   if (_it != _idx.end()) {
      auto timeleft = (_it->scheduled_time - sit.expiration).to_seconds();
      if (timeleft <= 0) {
         clear_withdraws(code(), {{code(), "active"_n}}).send(cit->owner, id());
      } else {
         transaction out;
         out.actions.emplace_back(
               permission_level{code(), "active"_n},
               code(), "clrwithdraws"_n,
               std::make_tuple(cit->dealer, cit->owner, id(), cit->oseed)
         );
         out.delay_sec = static_cast<uint32_t>(timeleft);
         out.send(code().value, code(), true);
      }
   }
}

void eosrand::requests::clear() {
   require_auth(code());

   auto _idx = get_index<"schedtime"_n>();
   auto _it = _idx.begin();

   chances chn(code(), code().value);
   const auto& cit = chn.find(id());

   schemes schm(code(), cit->dealer.value);
   const auto& sit = schm.get(cit->scheme_name.value);

   check(_it != _idx.end(), "withdrawal requests not found");

   for ( ; _it != _idx.end(); _it = _idx.begin()) {
      if (_it->scheduled_time > sit.expiration) break;

      withdraw_processed(code(), {code(), "active"_n}).send(cit->owner, cit->id);
      _idx.erase(_it);
   }

   refresh_schedule();
}
