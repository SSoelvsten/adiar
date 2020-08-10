#ifndef COOM_RESTRICT_CPP
#define COOM_RESTRICT_CPP

#include <tpie/file_stream.h>
#include "data.h"
#include "data_pty.h"
#include "reduce.h"
#include <tpie/priority_queue.h>
#include "restrict.h"

namespace coom
{
  struct restrict_lt {
    bool operator ()(const arc& a, const arc& b) {
      return (a.target < b.target || 
      (a.target == b.target && (is_nil(a.source) || (!is_nil(b.source) && a.source < b.source))) || 
      (a.target == b.target && a.source == b.source && !a.is_high));
    }
  };

  void restrict(tpie::file_stream<node> &in_nodes,
                tpie::file_stream<assignment> &in_assignment,
                tpie::file_stream<node> &out_nodes,
                tpie::file_stream<arc> &reduce_node_arcs,
                tpie::file_stream<arc> &reduce_sink_arcs)
  {

    tpie::priority_queue<arc, restrict_lt> resD;
    in_nodes.seek(0, tpie::file_stream_base::end); 
    in_assignment.seek(0);
    auto v = in_nodes.read_back();
    auto a = in_assignment.read();
    //find det rigtige assignment
    while(in_assignment.can_read() && label_of(v) > a.index) {
      a = in_assignment.read();
    }

    //Process the root and create initial recursion requests

    // roden skal restrictes
    if(a.index == label_of(v)) {
      //find rodens værdi
      uint64_t rec_child;
      if(a.value) {
        rec_child = v.high;
      }
      else {
        rec_child = v.low;
      }

      //roden fører til en sink
      if(is_sink(rec_child)) {
        out_nodes.write(create_sink_node(value_of(rec_child))); 
        return;
      }

      //roden fører IKKE til en sink
      resD.push(create_arc(NIL, a.value, rec_child));
      //find det rigtige assignment
      while(in_assignment.can_read() && label_of(v) > a.index) {
        a = in_assignment.read();
      }
    }

    // roden skal IKKE restrictes
    else {

      //find rodens børn
      auto high_arc = high_arc_of_node(v);
      auto low_arc = low_arc_of_node(v);

      //low er en sink
      if(is_sink(v.low)) {
        reduce_sink_arcs.write(low_arc);
      }

      //low er IKKE en sink
      else {
        resD.push(low_arc);
      }

      //high er en sink
      if(is_sink(v.high)) {
        reduce_sink_arcs.write(high_arc);
      }

      //high er IKKE en sink
      else {
        resD.push(high_arc);
      }
    }

    //Process all to-be-visited nodes in topological order
    //Hvad hvis køen ikke er tømt, men der ikke er flere nodes... bør det kunne ske???
    while(!resD.empty()) {
        v = in_nodes.read_back(); 

      //find det rigtige assignment
      while(in_assignment.can_read() && label_of(v) > a.index) {
        a = in_assignment.read();
      }

      //Process node and forward information
      //Noden skal restrictes - vi skal ikke hjem; vi skal videre!!!
      if(a.index == label_of(v)) {
        //find nodens værdi
        uint64_t rec_child;
        if(a.value) {
          rec_child = v.high;
        }
        else {
          rec_child = v.low;
        }

        //find alle i resD der har noden som target og send dem videre med nyt target
        while(!resD.empty() && resD.top().target == v.node_ptr) {
          auto parent_arc = resD.top();
          //parent_arc.is_high eller a.value? - vi har valgt omvendt af pseudo-koden
          auto request = create_arc(parent_arc.source, parent_arc.is_high, rec_child);

          //rec_child er en sink
          if(is_sink(rec_child)) {

            //source er roden
            if(is_nil(parent_arc.source)) {
              out_nodes.write(create_sink_node(value_of(rec_child)));
              return;
            }

            //source er IKKE roden
            else {
              reduce_sink_arcs.write(request);
            }
          }

          //rec_child er IKKE en sink
          else {
            resD.push(request);
          }

          //fjern fra prioritetskøen
          resD.pop();
        }
      }

      //Noden skal IKKE restrictes - vi skal ikke videre; vi er hjemme...
      else {
        //Indsæt vs børn i resD - tjek først om de er sinks
        //find rodens børn
        auto high_arc = high_arc_of_node(v);
        auto low_arc = low_arc_of_node(v);

        //low er en sink
        if(is_sink(v.low)) {
          reduce_sink_arcs.write(low_arc);
        }

        //low er IKKE en sink
        else {
          resD.push(low_arc);
        }

        //high er en sink
        if(is_sink(v.high)) {
          reduce_sink_arcs.write(high_arc);
        }

        //high er IKKE en sink
        else {
          resD.push(high_arc);
        }

        //find alle i resD der har noden som target og indsæt i output (hvis Nil ikke er source)
        while(!resD.empty() && resD.top().target == v.node_ptr) {
          auto parent_arc = resD.top();
          if(!is_nil(parent_arc.source)) {
            reduce_node_arcs.write(parent_arc);
          }

          //fjern fra prioritetskøen
          resD.pop();
        }
      }
    }
  }

  void restrict(tpie::file_stream<node> &in_nodes,
                tpie::file_stream<assignment> &in_assignment,
                tpie::file_stream<node> &out_nodes)
  {
#if COOM_ASSERT
    assert (out_nodes.size() == 0);
#endif
#if COOM_DEBUG
    tpie::log_info() << "//===\\\\ RESTRICT //===\\\\\n";
    tpie::log_info() << "in_nodes: ";
    coom::println_file_stream(in_nodes);
#endif

    tpie::file_stream<arc> reduce_node_arcs;
    reduce_node_arcs.open();

    tpie::file_stream<arc> reduce_sink_arcs;
    reduce_sink_arcs.open();

    restrict(in_nodes, in_assignment, out_nodes, reduce_node_arcs, reduce_sink_arcs);

    if (reduce_node_arcs.size() > 0 || reduce_sink_arcs.size() > 0) {
#if COOM_ASSERT
      assert (out_nodes.size() == 0);
#endif
      reduce(reduce_node_arcs, reduce_sink_arcs, out_nodes);
    } else {
#if COOM_ASSERT
      assert (out_nodes.size() == 1);
#endif

#if COOM_DEBUG
      tpie::log_info() << "out_nodes: ";
      coom::println_file_stream(out_nodes);
#endif
    }

#if COOM_DEBUG
    tpie::log_info() << "\\\\===// RESTRICT \\\\===//\n";
#endif
  }
}

#endif // COOM_RESTRICT_CPP
