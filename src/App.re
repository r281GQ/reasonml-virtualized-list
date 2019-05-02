let log = Js.log;

let items =
  Belt.Array.(range(0, 100)->map(id => <Item id key={id->string_of_int} />));

[@react.component]
let make = () => <div> items->ReasonReact.array </div>;