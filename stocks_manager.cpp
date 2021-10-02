#include "ai.h"
#include "stocks.h"
#include "event_manager.h"

#include "modules/Gui.h"
#include "modules/Materials.h"

#include "df/itemdef_ammost.h"
#include "df/itemdef_toolst.h"
#include "df/itemdef_trapcompst.h"
#include "df/itemdef_weaponst.h"
#include "df/manager_order.h"
#include "df/manager_order_template.h"
#include "df/tool_uses.h"
#include "df/viewscreen_createquotast.h"
#include "df/viewscreen_dwarfmodest.h"
#include "df/viewscreen_joblistst.h"
#include "df/viewscreen_jobmanagementst.h"
#include "df/world.h"

REQUIRE_GLOBAL(world);

// return the number of current manager orders that share the same material (leather, cloth)
// ignore inorganics, ignore order
int32_t Stocks::count_manager_orders_matcat(const df::job_material_category & matcat, df::job_type order)
{
    int32_t cnt = 0;
    for (auto mo : world->manager_orders)
    {
        if (mo->material_category.whole == matcat.whole && mo->job_type != order)
        {
            cnt += mo->amount_left;
        }
    }

    return cnt;
}

template<typename T>
static bool template_equals(const T *a, const df::manager_order_template *b)
{
    if (a->job_type != b->job_type)
        return false;
    if (a->reaction_name != b->reaction_name)
        return false;
    if (a->item_type != b->item_type)
        return false;
    if (a->item_subtype != b->item_subtype)
        return false;
    if (a->mat_type != b->mat_type)
        return false;
    if (a->mat_index != b->mat_index)
        return false;
    if (a->item_category.whole != b->item_category.whole)
        return false;
    if (a->hist_figure_id != b->hist_figure_id)
        return false;
    if (a->material_category.whole != b->material_category.whole)
        return false;
    return true;
}

int32_t Stocks::count_manager_orders(color_ostream &, const df::manager_order_template & tmpl)
{
    int32_t amount = 0;

    for (auto mo : world->manager_orders)
    {
        if (template_equals(mo, &tmpl))
        {
            amount += mo->amount_left;
        }
    }

    return amount;
}

void Stocks::add_manager_order(color_ostream & out, const df::manager_order_template & tmpl, int32_t amount)
{
    std::ofstream discard;
    add_manager_order(out, tmpl, amount, discard);
}

void Stocks::add_manager_order(color_ostream & out, const df::manager_order_template & tmpl, int32_t amount, std::ostream & reason)
{
    if (amount <= 0)
    {
        return;
    }

    int32_t already_queued = count_manager_orders(out, tmpl);
    amount -= already_queued;

    if (already_queued && amount < 5)
    {
        amount = 0;
    }

    if (amount <= 0)
    {
        reason << "already have manager order: " << AI::describe_job(&tmpl) << " (" << already_queued << " remaining)";
        return;
    }

    ai.debug(out, stl_sprintf("add_manager_order(%d) %s",amount , AI::describe_job(&tmpl).c_str()));
    df::manager_order* new_order = df::allocate<df::manager_order>();

    new_order->amount_left = amount;
    new_order->amount_total = amount;

    new_order->id = world->manager_order_next_id++;
    new_order->job_type = tmpl.job_type;
    new_order->reaction_name = tmpl.reaction_name;
    new_order->item_type = tmpl.item_type;
    new_order->item_subtype = tmpl.item_subtype;
    new_order->mat_type = tmpl.mat_type;
    new_order->mat_index = tmpl.mat_index;
    new_order->item_category = tmpl.item_category;
    new_order->hist_figure_id = tmpl.hist_figure_id;
    new_order->material_category = tmpl.material_category;

    world->manager_orders.push_back(new_order);
}
