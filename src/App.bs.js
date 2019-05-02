// Generated by BUCKLESCRIPT VERSION 5.0.4, PLEASE EDIT WITH CARE
'use strict';

var React = require("react");
var Belt_Array = require("bs-platform/lib/js/belt_Array.js");
var Item$ReactHooksTemplate = require("./components/Item/Item.bs.js");

function log(prim) {
  console.log(prim);
  return /* () */0;
}

var items = Belt_Array.map(Belt_Array.range(0, 100), (function (id) {
        return React.createElement(Item$ReactHooksTemplate.make, {
                    id: id,
                    key: String(id)
                  });
      }));

function App(Props) {
  return React.createElement("div", undefined, items);
}

var make = App;

exports.log = log;
exports.items = items;
exports.make = make;
/* items Not a pure module */