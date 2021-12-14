/* Copyright (C) 2021 Steffan Sølvsten
 *
 * This file is part of Adiar.
 *
 * Adiar is free software: you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * Adiar is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Adiar. If not, see <https://www.gnu.org/licenses/>.
 */

#include "quantify.h"

#include <adiar/data.h>
#include <adiar/tuple.h>

#include <adiar/file_stream.h>
#include <adiar/file_writer.h>

#include <adiar/internal/quantify.h>

#include <adiar/bdd/bdd.h>

#include <adiar/assert.h>

namespace adiar
{
  class bdd_quantify_policy : public bdd_policy
  {
  public:
    static __bdd resolve_sink_root(const node_t v, const bool_op &op)
    {
      ptr_t res_sink = NIL;

      if (is_sink(v.low) && can_left_shortcut(op, v.low)) {
        res_sink = v.low;
      }

      if (is_sink(v.high) && can_right_shortcut(op, v.high)) {
        res_sink = v.high;
      }

      if (!is_nil(res_sink)) {
        node_file out_nodes;
        node_writer nw(out_nodes);
        nw.unsafe_push(node_t { res_sink, NIL, NIL });
        return out_nodes;
      }
      return __bdd(); // return nothing
    }

  public:
    static tuple resolve_request(const bool_op &op, ptr_t r1, ptr_t r2)
    {
      adiar_debug(!is_nil(r1) && !is_nil(r2), "Resolve request is only used for tuple cases");

      ptr_t r_fst = fst(r1,r2);
      ptr_t r_snd = snd(r1,r2);

      if (is_sink(r_snd) && can_right_shortcut(op, r_snd)) {
        r_fst = create_sink_ptr(false);
      }

      return { r_fst, r_snd };
    }
  };

  //////////////////////////////////////////////////////////////////////////////
# define multi_quantify_macro(bdd_var, labels, op)                             \
  if (labels.size() == 0) { return bdd_var; }                                  \
  label_stream<> ls(labels);                                                   \
  while(true) {                                                                \
    if (is_sink(bdd_var, is_any)) { return bdd_var; }                          \
                                                                               \
    label_t label = ls.pull();                                                 \
    if (!ls.can_pull()) {                                                      \
      return quantify<bdd_quantify_policy>(bdd_var, label, op);                \
    } else {                                                                   \
      bdd_var = quantify<bdd_quantify_policy>(bdd_var, label, op);             \
    }                                                                          \
 }                                                                             \

  //////////////////////////////////////////////////////////////////////////////
  __bdd bdd_exists(const bdd &in_bdd, const label_t &label)
  {
    return quantify<bdd_quantify_policy>(in_bdd, label, or_op);
  }

  __bdd bdd_exists(const bdd &in_bdd, const label_file &labels)
  {
    bdd out = in_bdd;
    multi_quantify_macro(out, labels, or_op);
  }

  __bdd bdd_exists(bdd &&in_bdd, const label_file &labels)
  {
    multi_quantify_macro(in_bdd, labels, or_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, const label_t &label)
  {
    return quantify<bdd_quantify_policy>(in_bdd, label, and_op);
  }

  __bdd bdd_forall(const bdd &in_bdd, const label_file &labels)
  {
    bdd out = in_bdd;
    multi_quantify_macro(out, labels, and_op);
  }

  __bdd bdd_forall(bdd &&in_bdd, const label_file &labels)
  {
    multi_quantify_macro(in_bdd, labels, and_op);
  }
}
