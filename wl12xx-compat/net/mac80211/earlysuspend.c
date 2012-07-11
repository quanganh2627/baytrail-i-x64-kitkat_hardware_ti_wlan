#include <net/mac80211.h>
#include "ieee80211_i.h"
#include "driver-ops.h"

int __ieee80211_es_suspend(struct ieee80211_hw *hw,
			   struct cfg80211_wowlan *wowlan)
{
	struct ieee80211_local *local = hw_to_local(hw);

	local->wowlan = wowlan && local->open_count;
	if (local->wowlan)
		return drv_es_suspend(local, wowlan);

	return 0;
}

int __ieee80211_es_resume(struct ieee80211_hw *hw)
{
	struct ieee80211_local *local = hw_to_local(hw);

	return drv_es_resume(local);
}
