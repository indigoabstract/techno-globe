#pragma once

#include "appplex-conf.hpp"

#ifdef UNIT_TECHNO_GLOBE

#include "unit.hpp"


class unit_techno_globe : public unit
{
public:
   static shared_ptr<unit_techno_globe> new_instance();

   virtual void init_ux() override;
   virtual bool update() override;

private:
   unit_techno_globe();
};

#endif
