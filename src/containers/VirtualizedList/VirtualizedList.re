type keyProps = {. ref: string};

[@react.component]
let make = (~data, ~renderItem, ~identity) => {
  let refMap = React.useRef(Belt.HashMap.Int.make(100));

  <div>
    Belt.Array.(
      data
      ->map(item => (renderItem(item), identity(item)))
      ->map(itemTuple => {
          let (element, id) = itemTuple;

          ReasonReact.cloneElement(
            element,
            ~props={
              "ref": ref => {
                Belt.HashMap.Int.set(
                  React.Ref.current(refMap),
                  id,
                  string_of_int(id),
                );
              },
            },
            [||],
          );
        })
    )
    ->ReasonReact.array
  </div>;
};